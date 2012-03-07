/** @file

MODULE				: H264

FILE NAME			: H264OutputPin.cpp

DESCRIPTION			: H.264 source filter output pin implementation
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2012, CSIR
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
#include "stdafx.h"
#include "H264OutputPin.h"
#include "H264Source.h"

#include <dvdmedia.h>
#include <wmcodecdsp.h>

H264OutputPin::H264OutputPin(HRESULT *phr, H264SourceFilter* pFilter)
  : CSourceStream(NAME("CSIR RTVC H264 Source"), phr, pFilter, L"Out"),
  m_pFilter(pFilter),
  //m_iWidth(DEFAULT_WIDTH),
  //m_iHeight(DEFAULT_HEIGHT),
  m_iCurrentFrame(0),
  m_rtFrameLength(FPS_25) 
{

}


H264OutputPin::~H264OutputPin()
{

}

// GetMediaType: This method tells the downstream pin what types we support.
HRESULT H264OutputPin::GetMediaType(CMediaType *pMediaType)
{
  CAutoLock cAutoLock(m_pFilter->pStateLock());

  CheckPointer(pMediaType, E_POINTER);
  if (!m_pFilter->m_bUseRtvcH264)
  {
    pMediaType->InitMediaType();    
    pMediaType->SetType(&MEDIATYPE_Video);
    pMediaType->SetSubtype(&MEDIASUBTYPE_H264);
    pMediaType->SetFormatType(&FORMAT_VideoInfo2);
    VIDEOINFOHEADER2* pvi2 = (VIDEOINFOHEADER2*)pMediaType->AllocFormatBuffer(sizeof(VIDEOINFOHEADER2));
    ZeroMemory(pvi2, sizeof(VIDEOINFOHEADER2));
    pvi2->bmiHeader.biBitCount = 24;
    pvi2->bmiHeader.biSize = 40;
    pvi2->bmiHeader.biPlanes = 1;
    pvi2->bmiHeader.biWidth = m_pFilter->m_iWidth;
    pvi2->bmiHeader.biHeight = m_pFilter->m_iHeight;
    pvi2->bmiHeader.biSize = m_pFilter->m_iWidth * m_pFilter->m_iHeight * 3;
    pvi2->bmiHeader.biSizeImage = DIBSIZE(pvi2->bmiHeader);
    pvi2->bmiHeader.biCompression = DWORD('1cva');
    //pvi2->AvgTimePerFrame = m_tFrame;
    //pvi2->AvgTimePerFrame = 1000000;
    const REFERENCE_TIME FPS_25 = UNITS / 25;
    pvi2->AvgTimePerFrame = FPS_25;
    //SetRect(&pvi2->rcSource, 0, 0, m_cx, m_cy);
    SetRect(&pvi2->rcSource, 0, 0, m_pFilter->m_iWidth, m_pFilter->m_iHeight);
    pvi2->rcTarget = pvi2->rcSource;

    pvi2->dwPictAspectRatioX = m_pFilter->m_iWidth;
    pvi2->dwPictAspectRatioY = m_pFilter->m_iHeight;
  }
  else
  {
    // Allocate enough room for the VIDEOINFOHEADER
    VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER*)pMediaType->AllocFormatBuffer( sizeof(VIDEOINFOHEADER) );
    if (pvi == 0) 
      return(E_OUTOFMEMORY);

    ZeroMemory(pvi, pMediaType->cbFormat);   
    pvi->AvgTimePerFrame = m_rtFrameLength;
    //pvi->dwBitRate = ((int)(m_pFilter->m_iFramesPerSecond * m_pFilter->m_iFrameSize * m_pFilter->m_dBitsPerPixel)) << 3;

    pvi->bmiHeader.biBitCount = 12;
    pvi->bmiHeader.biCompression = DWORD('1cva');

    pvi->bmiHeader.biClrImportant = 0;
    pvi->bmiHeader.biClrUsed = 0;
    pvi->bmiHeader.biPlanes = 1;
    pvi->bmiHeader.biXPelsPerMeter = 0;
    pvi->bmiHeader.biYPelsPerMeter = 0;
    pvi->bmiHeader.biWidth = m_pFilter->m_iWidth;
    pvi->bmiHeader.biHeight = m_pFilter->m_iHeight;

    unsigned uiFramesize = m_pFilter->m_iWidth * m_pFilter->m_iHeight * 3;
    pvi->bmiHeader.biSizeImage = uiFramesize;
    pvi->bmiHeader.biSize = 40;

    // Clear source and target rectangles
    SetRectEmpty(&(pvi->rcSource)); // we want the whole image area rendered
    SetRectEmpty(&(pvi->rcTarget)); // no particular destination rectangle

    pMediaType->SetType(&MEDIATYPE_Video);
    pMediaType->SetFormatType(&FORMAT_VideoInfo);
    pMediaType->SetTemporalCompression(FALSE);

    static const GUID MEDIASUBTYPE_H264M = 
    { 0xbdf25152, 0x46b, 0x4509, { 0x8e, 0x55, 0x6c, 0x73, 0x83, 0x1c, 0x8d, 0xc4 } };

    pMediaType->SetSubtype(&MEDIASUBTYPE_H264M);

    pMediaType->SetSampleSize( uiFramesize );

    // Add SPS and PPS to the media type
    // Store SPS and PPS in media format header
    int nCurrentFormatBlockSize = pMediaType->cbFormat;

      if (m_pFilter->m_uiSeqParamSetLen + m_pFilter->m_uiPicParamSetLen > 0)
      {
        // old size + one int to store size of SPS/PPS + SPS/PPS/prepended by start codes
        int iAdditionalLength = sizeof(int) + m_pFilter->m_uiSeqParamSetLen + m_pFilter->m_uiPicParamSetLen;
        int nNewSize = nCurrentFormatBlockSize + iAdditionalLength;
        pMediaType->ReallocFormatBuffer(nNewSize);

        BYTE* pFormat = pMediaType->Format();
        BYTE* pStartPos = &(pFormat[nCurrentFormatBlockSize]);
        // copy SPS
        memcpy(pStartPos, m_pFilter->m_pSeqParamSet, m_pFilter->m_uiSeqParamSetLen);
        pStartPos += m_pFilter->m_uiSeqParamSetLen;
        // copy PPS
        memcpy(pStartPos, m_pFilter->m_pPicParamSet, m_pFilter->m_uiPicParamSetLen);
        pStartPos += m_pFilter->m_uiPicParamSetLen;
        // Copy additional header size
        memcpy(pStartPos, &iAdditionalLength, sizeof(int));
      }
      else
      {
        // not configured: just copy in size of 0
        pMediaType->ReallocFormatBuffer(nCurrentFormatBlockSize + sizeof(int));
        BYTE* pFormat = pMediaType->Format();
        memset(pFormat + nCurrentFormatBlockSize, 0, sizeof(int));
      }
  }
  return S_OK;

#if 0
//#define TRY_USE_MS_H264_DECODER
#ifdef TRY_USE_MS_H264_DECODER
  
  pMediaType->InitMediaType();    
  pMediaType->SetType(&MEDIATYPE_Video);
  pMediaType->SetSubtype(&MEDIASUBTYPE_H264);
  pMediaType->SetFormatType(&FORMAT_VideoInfo2);
  VIDEOINFOHEADER2* pvi2 = (VIDEOINFOHEADER2*)pMediaType->AllocFormatBuffer(sizeof(VIDEOINFOHEADER2));
  ZeroMemory(pvi2, sizeof(VIDEOINFOHEADER2));
  pvi2->bmiHeader.biBitCount = 24;
  pvi2->bmiHeader.biSize = 40;
  pvi2->bmiHeader.biPlanes = 1;
  pvi2->bmiHeader.biWidth = m_pFilter->m_iWidth;
  pvi2->bmiHeader.biHeight = m_pFilter->m_iHeight;
  pvi2->bmiHeader.biSize = m_pFilter->m_iWidth * m_pFilter->m_iHeight * 3;
  pvi2->bmiHeader.biSizeImage = DIBSIZE(pvi2->bmiHeader);
  pvi2->bmiHeader.biCompression = DWORD('1cva');
  //pvi2->AvgTimePerFrame = m_tFrame;
  //pvi2->AvgTimePerFrame = 1000000;
  const REFERENCE_TIME FPS_25 = UNITS / 25;
  pvi2->AvgTimePerFrame = FPS_25;
  //SetRect(&pvi2->rcSource, 0, 0, m_cx, m_cy);
  SetRect(&pvi2->rcSource, 0, 0, m_pFilter->m_iWidth, m_pFilter->m_iHeight);
  pvi2->rcTarget = pvi2->rcSource;

  pvi2->dwPictAspectRatioX = m_pFilter->m_iWidth;
  pvi2->dwPictAspectRatioY = m_pFilter->m_iHeight;
  return S_OK;
#else
  // Allocate enough room for the VIDEOINFOHEADER
  VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER*)pMediaType->AllocFormatBuffer( sizeof(VIDEOINFOHEADER) );
  if (pvi == 0) 
    return(E_OUTOFMEMORY);

  ZeroMemory(pvi, pMediaType->cbFormat);   
  pvi->AvgTimePerFrame = m_rtFrameLength;
  //pvi->dwBitRate = ((int)(m_pFilter->m_iFramesPerSecond * m_pFilter->m_iFrameSize * m_pFilter->m_dBitsPerPixel)) << 3;

  pvi->bmiHeader.biBitCount = 12;
  pvi->bmiHeader.biCompression = DWORD('1cva');

  pvi->bmiHeader.biClrImportant = 0;
  pvi->bmiHeader.biClrUsed = 0;
  pvi->bmiHeader.biPlanes = 1;
  pvi->bmiHeader.biXPelsPerMeter = 0;
  pvi->bmiHeader.biYPelsPerMeter = 0;
  pvi->bmiHeader.biWidth = m_pFilter->m_iWidth;
  pvi->bmiHeader.biHeight = m_pFilter->m_iHeight;


  unsigned uiFramesize = m_pFilter->m_iWidth * m_pFilter->m_iHeight * 3;
  pvi->bmiHeader.biSizeImage = uiFramesize;
  pvi->bmiHeader.biSize = 40;

  // Clear source and target rectangles
  SetRectEmpty(&(pvi->rcSource)); // we want the whole image area rendered
  SetRectEmpty(&(pvi->rcTarget)); // no particular destination rectangle

  pMediaType->SetType(&MEDIATYPE_Video);
  pMediaType->SetFormatType(&FORMAT_VideoInfo);
  pMediaType->SetTemporalCompression(FALSE);

  static const GUID MEDIASUBTYPE_H264M = 
  { 0xbdf25152, 0x46b, 0x4509, { 0x8e, 0x55, 0x6c, 0x73, 0x83, 0x1c, 0x8d, 0xc4 } };

  pMediaType->SetSubtype(&MEDIASUBTYPE_H264M);

  pMediaType->SetSampleSize( uiFramesize );
  return S_OK;
#endif
#endif
}


HRESULT H264OutputPin::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest)
{
  HRESULT hr;
  CAutoLock cAutoLock(m_pFilter->pStateLock());

  CheckPointer(pAlloc, E_POINTER);
  CheckPointer(pRequest, E_POINTER);

  //VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER*) m_mt.Format();

  // Ensure a minimum number of buffers
  if (pRequest->cBuffers == 0)
  {
    pRequest->cBuffers = 1;
  }
  //pRequest->cbBuffer = pvi->bmiHeader.biSizeImage;
  // hard-coding for now
  pRequest->cbBuffer = 352 * 288 * 3;

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


// This is where we insert the H264 NAL units into the video stream.
// FillBuffer is called once for every sample in the stream.
HRESULT H264OutputPin::FillBuffer(IMediaSample *pSample)
{
  BYTE *pData;
  long cbData;

  CheckPointer(pSample, E_POINTER);
  CAutoLock cAutoLockShared(&m_cSharedState);

  if (!m_pFilter->readNalUnit())
    return S_FALSE;

  // Access the sample's data buffer
  pSample->GetPointer(&pData);
  cbData = pSample->GetSize();

  // Check that we're still using video
  ASSERT(m_mt.formattype == FORMAT_VideoInfo2 || m_mt.formattype == FORMAT_VideoInfo);

  VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)m_mt.pbFormat;

  ASSERT(m_pFilter->m_uiCurrentNalUnitSize > 0);
  memcpy( pData, m_pFilter->m_pBuffer, m_pFilter->m_uiCurrentNalUnitSize );
#if 1
#if 1
  if (!m_pFilter->isParameterSet(m_pFilter->m_pBuffer[0]))
  {
    // TODO: non VLC NAL units should not increment the timestamp
    REFERENCE_TIME rtStart = m_iCurrentFrame * m_rtFrameLength;
    REFERENCE_TIME rtStop  = rtStart + m_rtFrameLength;
    pSample->SetTime( &rtStart, &rtStop );
    ++m_iCurrentFrame;
  }
  else
  {
    pSample->SetTime( NULL, NULL );
  }
#else
  // setting timestamps to NULL until we can parse the NAL units to check the type
  pSample->SetTime( NULL, NULL );
#endif

  if (m_pFilter->isIdrFrame(m_pFilter->m_pBuffer[0]))
  {
    pSample->SetSyncPoint(TRUE);
  }
  else
  {
    pSample->SetSyncPoint(false);
  }

  pSample->SetActualDataLength(m_pFilter->m_uiCurrentNalUnitSize);
#else
#if 0
  // TODO: non VLC NAL units should not increment the timestamp
  REFERENCE_TIME rtStart = m_iCurrentFrame * m_rtFrameLength;
  REFERENCE_TIME rtStop  = rtStart + m_rtFrameLength;
  pSample->SetTime( &rtStart, &rtStop );
#else
  // setting timestamps to NULL until we can parse the NAL units to check the type
  pSample->SetTime( NULL, NULL );
#endif

  // Set TRUE on every sample for uncompressed frames
  pSample->SetSyncPoint(TRUE);

  pSample->SetActualDataLength(m_pFilter->m_uiCurrentNalUnitSize);

  m_iCurrentFrame++;
#endif

  //Sleep(30);
  //Sleep(30);
  //Sleep(30);
  //Sleep(30);
  //Sleep(100);
  return S_OK;
}
