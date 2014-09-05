/** @file

MODULE				: YUVSource

FILE NAME			: YuvOutputPin.cpp

DESCRIPTION			:

LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008 - 2014, CSIR
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of the CSIR nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

===========================================================================
*/
#include "YuvOutputPin.h"
#include "YuvSource.h"

DEFINE_GUID(MEDIASUBTYPE_I420, 0x30323449, 0x0000, 0x0010, 0x80, 0x00,
  0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

YuvOutputPin::YuvOutputPin(HRESULT *phr, YuvSourceFilter* pFilter)
  : CSourceStream(NAME("CSIR VPP YUV Source"), phr, pFilter, L"Out"),
  m_pYuvFilter(pFilter),
  m_iCurrentFrame(0),
  m_rtFrameLength(FPS_30)
{
}

YuvOutputPin::~YuvOutputPin()
{
}

// GetMediaType: This method tells the downstream pin what types we support.
HRESULT YuvOutputPin::GetMediaType(CMediaType *pMediaType)
{
  CAutoLock cAutoLock(m_pFilter->pStateLock());

  CheckPointer(pMediaType, E_POINTER);

  // Allocate enough room for the VIDEOINFOHEADER
  VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER*)pMediaType->AllocFormatBuffer(sizeof(VIDEOINFOHEADER));
  if (pvi == 0)
    return(E_OUTOFMEMORY);

  ZeroMemory(pvi, pMediaType->cbFormat);
  pvi->AvgTimePerFrame = m_rtFrameLength;
  pvi->dwBitRate = ((int)(m_pYuvFilter->m_iFramesPerSecond * m_pYuvFilter->m_iFrameSize * m_pYuvFilter->m_dBytesPerPixel)) << 3;

  switch (m_pYuvFilter->m_nYuvFormat)
  {
  case YUV420P:
  {
    pvi->bmiHeader.biBitCount = 12;
    // For some reason IYUV requires a color converter before the renderer and I420 does not even though 
    // these media types are the same
    pvi->bmiHeader.biCompression = MAKEFOURCC('I', '4', '2', '0');
    // Works too but DS inserts color space converter before renderer
    //pvi->bmiHeader.biCompression = MAKEFOURCC('I', 'Y', 'U', 'V');
    // Both of these work
    pMediaType->SetSubtype(&MEDIASUBTYPE_I420);
    // Tried other media types MEDIASUBTYPE_IMC3, MEDIASUBTYPE_IMC2, MEDIASUBTYPE_IMC4, etc
    // These have no effect on the connection
    // The most important thing seems to be the bmiHeader.biCompression being set correctly
    // pMediaType->SetSubtype(&MEDIASUBTYPE_IMC3);
    break;
  }
  case YUV444I:
  {
    pvi->bmiHeader.biBitCount = 32;
    pvi->bmiHeader.biCompression = MAKEFOURCC('A', 'Y', 'U', 'V');
    pMediaType->SetSubtype(&MEDIASUBTYPE_AYUV);    break;
  }
  }


  pvi->bmiHeader.biClrImportant = 0;
  pvi->bmiHeader.biClrUsed = 0;
  pvi->bmiHeader.biPlanes = 1;
  pvi->bmiHeader.biXPelsPerMeter = 0;
  pvi->bmiHeader.biYPelsPerMeter = 0;
  pvi->bmiHeader.biWidth = m_pYuvFilter->m_iWidth;
  pvi->bmiHeader.biHeight = m_pYuvFilter->m_iHeight;
  // should the sign of height is ignored for YUV DIB types?
  // http://msdn.microsoft.com/en-us/library/windows/desktop/dd407212(v=vs.85).aspx
  pvi->bmiHeader.biSizeImage = m_pYuvFilter->m_iFrameSize;
  pvi->bmiHeader.biSize = 40;

  // Clear source and target rectangles
  SetRectEmpty(&(pvi->rcSource)); // we want the whole image area rendered
  SetRectEmpty(&(pvi->rcTarget)); // no particular destination rectangle

  pMediaType->SetType(&MEDIATYPE_Video);
  pMediaType->SetFormatType(&FORMAT_VideoInfo);
  pMediaType->SetTemporalCompression(FALSE);

  pMediaType->SetSampleSize(m_pYuvFilter->m_iFrameSize);
  return S_OK;
}


HRESULT YuvOutputPin::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest)
{
  HRESULT hr;
  CAutoLock cAutoLock(m_pFilter->pStateLock());

  CheckPointer(pAlloc, E_POINTER);
  CheckPointer(pRequest, E_POINTER);

  VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER*)m_mt.Format();

  // Ensure a minimum number of buffers
  if (pRequest->cBuffers == 0)
  {
    pRequest->cBuffers = 1;
  }
  pRequest->cbBuffer = pvi->bmiHeader.biSizeImage;

  ALLOCATOR_PROPERTIES Actual;
  hr = pAlloc->SetProperties(pRequest, &Actual);
  if (FAILED(hr))
  {
    return hr;
  }

  // Is this allocator unsuitable?
  if (Actual.cbBuffer < pRequest->cbBuffer)
  {
    return E_FAIL;
  }

  return S_OK;
}


// This is where we insert the YUV frames into the video stream.
// FillBuffer is called once for every sample in the stream.
HRESULT YuvOutputPin::FillBuffer(IMediaSample *pSample)
{
  BYTE *pData;
  long cbData;

  CheckPointer(pSample, E_POINTER);

  CAutoLock cAutoLockShared(&m_cSharedState);

  if (m_iCurrentFrame < m_pYuvFilter->m_iNoFrames)
  {
    if (!m_pYuvFilter->readFrame())
      return S_FALSE;

    // Access the sample's data buffer
    pSample->GetPointer(&pData);
    cbData = pSample->GetSize();

    // Check that we're still using video
    ASSERT(m_mt.formattype == FORMAT_VideoInfo);

    VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)m_mt.pbFormat;

    memcpy(pData, m_pYuvFilter->m_pYuvBuffer, m_pYuvFilter->m_iFrameSize);

    // debug top-bottom
    // The following code was used to determine the orientation of the image in the raw YUV file:
    // The raw YUV used in the test sequences is top-down: http://msdn.microsoft.com/en-us/library/windows/desktop/dd407212(v=vs.85).aspx

#if 0
    // set luminance for 1st row to zero
    int iW = m_pYuvFilter->m_iWidth;
    int iWUV = m_pYuvFilter->m_iWidth >> 1;
    int iH = m_pYuvFilter->m_iHeight;
    int iHUV = m_pYuvFilter->m_iHeight >> 1;
    int iSY = m_pYuvFilter->m_iWidth * m_pYuvFilter->m_iHeight;
    int iSUV = iSY >> 2;

    // YUV is stored top down
    //Luminance
    // 10 white lines
    //memset( pData, 255, iW * 10);
    //memset( pData + ((iH/2 - 10) * iW), 255, iW * 10);
    //memset( pData + ((iH - 10) * iW), 255, iW * 10);
    // Single white lines
    //memset( pData, 255, iW);
    //memset( pData + ((iH/2) * iW), 255, iW);
    //memset( pData + ((iH - 1) * iW), 255, iW);

    // Chrominance
    // change to grey scale
    memset(pData + iSY, 128, iSUV * 2);
#if 0
    // blue componenent
    memset(pData + iSY, 255, iWUV);
    memset(pData + iSY + iWUV, 255, iWUV);
    memset(pData + iSY + 2 * iWUV, 255, iWUV);
    memset(pData + iSY + 3 * iWUV, 255, iWUV);
    memset(pData + iSY + 4 * iWUV, 255, iWUV);
#else
    memset(pData + iSY, 0, iWUV);
    memset(pData + iSY + iWUV, 0, iWUV);
    memset(pData + iSY + 2 * iWUV, 0, iWUV);
    memset(pData + iSY + 3 * iWUV, 0, iWUV);
    memset(pData + iSY + 4 * iWUV, 0, iWUV);
#endif    

#if 1
    // red component
    memset(pData + iSY + iSUV, 255, iWUV);
    memset(pData + iSY + iSUV + iWUV, 255, iWUV);
    memset(pData + iSY + iSUV + 2 * iWUV, 255, iWUV);
    memset(pData + iSY + iSUV + 3 * iWUV, 255, iWUV);
    memset(pData + iSY + iSUV + 4 * iWUV, 255, iWUV);
#else
    memset(pData + iSY + iSUV, 0, iWUV);
    memset(pData + iSY + iSUV + iWUV, 0, iWUV);
    memset(pData + iSY + iSUV + 2 * iWUV, 0, iWUV);
    memset(pData + iSY + iSUV + 3 * iWUV, 0, iWUV);
    memset(pData + iSY + iSUV + 4 * iWUV, 0, iWUV);
#endif
#endif

    // Set the timestamps that will govern playback frame rate.
    // If this file is getting written out as an AVI,
    // then you'll also need to configure the AVI Mux filter to 
    // set the Average Time Per Frame for the AVI Header.
    // The current time is the sample's start
    REFERENCE_TIME rtStart = m_iCurrentFrame * m_rtFrameLength;
    REFERENCE_TIME rtStop = rtStart + m_rtFrameLength;

    pSample->SetTime(&rtStart, &rtStop);

    // Set TRUE on every sample for uncompressed frames
    pSample->SetSyncPoint(TRUE);

    m_iCurrentFrame++;
    return S_OK;
  }
  else
  {
    // EOF
    return S_FALSE;
  }
}

