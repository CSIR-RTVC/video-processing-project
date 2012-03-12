/** @file

MODULE				: YUVSource

FILE NAME			: YuvSource.cpp

DESCRIPTION			: 

LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008 - 2012, CSIR
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
#include <streams.h>

#include <fstream>
#include <sstream>

#include "YuvSource.h"

#include <Shared/Conversion.h>
#include <Shared/StringUtil.h>

const AMOVIESETUP_MEDIATYPE sudOpPinTypes =
{
  &MEDIATYPE_Video,       // Major type
  &MEDIASUBTYPE_NULL      // Minor type
};

DEFINE_GUID(MEDIASUBTYPE_I420, 0x30323449, 0x0000, 0x0010, 0x80, 0x00,
  0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71); 

/**********************************************
*
*  YuvOutputPin Class
*  
*
**********************************************/

YuvOutputPin::YuvOutputPin(HRESULT *phr, YuvSourceFilter* pFilter)
  : CSourceStream(NAME("CSIR VPP YUV Source"), phr, pFilter, L"Out"),
  m_pYuvFilter(pFilter),
  m_iCurrentFrame(0),
  m_rtFrameLength(FPS_30) // Display 5 bitmap frames per second: TODO_ move to property page
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
  VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER*)pMediaType->AllocFormatBuffer( sizeof(VIDEOINFOHEADER) );
  if (pvi == 0) 
    return(E_OUTOFMEMORY);

  ZeroMemory(pvi, pMediaType->cbFormat);   
  pvi->AvgTimePerFrame = m_rtFrameLength;
  pvi->dwBitRate = ((int)(m_pYuvFilter->m_iFramesPerSecond * m_pYuvFilter->m_iFrameSize * m_pYuvFilter->m_dBitsPerPixel)) << 3;

  pvi->bmiHeader.biBitCount = 12;

  // For some reason IYUV requires a color converter before the renderer and I420 does not even though 
  // these media types are the same
  pvi->bmiHeader.biCompression = MAKEFOURCC('I', '4', '2', '0');
  // Works too but DS inserts color space converter before renderer
  //pvi->bmiHeader.biCompression = MAKEFOURCC('I', 'Y', 'U', 'V');
  
  pvi->bmiHeader.biClrImportant = 0;
  pvi->bmiHeader.biClrUsed = 0;
  pvi->bmiHeader.biPlanes = 1;
  pvi->bmiHeader.biXPelsPerMeter = 0;
  pvi->bmiHeader.biYPelsPerMeter = 0;
  pvi->bmiHeader.biWidth = m_pYuvFilter->m_iWidth;
  pvi->bmiHeader.biHeight = -1 * m_pYuvFilter->m_iHeight;
  pvi->bmiHeader.biSizeImage = m_pYuvFilter->m_iFrameSize;
  pvi->bmiHeader.biSize = 40;

  // Clear source and target rectangles
  SetRectEmpty(&(pvi->rcSource)); // we want the whole image area rendered
  SetRectEmpty(&(pvi->rcTarget)); // no particular destination rectangle

  pMediaType->SetType(&MEDIATYPE_Video);
  pMediaType->SetFormatType(&FORMAT_VideoInfo);
  pMediaType->SetTemporalCompression(FALSE);

  // Both of these work
  pMediaType->SetSubtype(&MEDIASUBTYPE_I420);
  //pMediaType->SetSubtype(&MEDIASUBTYPE_IYUV);

  // Tried other media types MEDIASUBTYPE_IMC3, MEDIASUBTYPE_IMC2, MEDIASUBTYPE_IMC4, etc
  // These have no effect on the connection
  // The most important thing seems to be the bmiHeader.biCompression being set correctly
  // pMediaType->SetSubtype(&MEDIASUBTYPE_IMC3);

  pMediaType->SetSampleSize( m_pYuvFilter->m_iFrameSize );
  return S_OK;
}


HRESULT YuvOutputPin::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest)
{
  HRESULT hr;
  CAutoLock cAutoLock(m_pFilter->pStateLock());

  CheckPointer(pAlloc, E_POINTER);
  CheckPointer(pRequest, E_POINTER);

  VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER*) m_mt.Format();

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

  if ( m_iCurrentFrame < m_pYuvFilter->m_iNoFrames )
  {
    if (!m_pYuvFilter->readFrame())
      return S_FALSE;

    // Access the sample's data buffer
    pSample->GetPointer(&pData);
    cbData = pSample->GetSize();

    // Check that we're still using video
    ASSERT(m_mt.formattype == FORMAT_VideoInfo);

    VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)m_mt.pbFormat;

    memcpy( pData, m_pYuvFilter->m_pYuvBuffer, m_pYuvFilter->m_iFrameSize );
    
#if 0
    // debug top-bottom
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
    memset( pData + iSY, 128, iSUV*2 );
    //blue
    memset( pData + iSY,            255, iWUV);
    memset( pData + iSY + iWUV,     255, iWUV);
    memset( pData + iSY + 2 * iWUV, 255, iWUV);
    memset( pData + iSY + 3 * iWUV, 255, iWUV);
    memset( pData + iSY + 4 * iWUV, 255, iWUV);

    //memset( pData + iSY,            0, iWUV);
    //memset( pData + iSY + iWUV,     0, iWUV);
    //memset( pData + iSY + 2 * iWUV, 0, iWUV);
    //memset( pData + iSY + 3 * iWUV, 0, iWUV);
    //memset( pData + iSY + 4 * iWUV, 0, iWUV);
    
    // red
    //memset( pData + iSY + iSUV,            255, iWUV);
    //memset( pData + iSY + iSUV + iWUV,     255, iWUV);
    //memset( pData + iSY + iSUV + 2 * iWUV, 255, iWUV);
    //memset( pData + iSY + iSUV + 3 * iWUV, 255, iWUV);
    //memset( pData + iSY + iSUV + 4 * iWUV, 255, iWUV);

    //memset( pData + iSY + iSUV,            0, iWUV);
    //memset( pData + iSY + iSUV + iWUV,     0, iWUV);
    //memset( pData + iSY + iSUV + 2 * iWUV, 0, iWUV);
    //memset( pData + iSY + iSUV + 3 * iWUV, 0, iWUV);
    //memset( pData + iSY + iSUV + 4 * iWUV, 0, iWUV);
#endif

    // Set the timestamps that will govern playback frame rate.
    // If this file is getting written out as an AVI,
    // then you'll also need to configure the AVI Mux filter to 
    // set the Average Time Per Frame for the AVI Header.
    // The current time is the sample's start
    REFERENCE_TIME rtStart = m_iCurrentFrame * m_rtFrameLength;
    REFERENCE_TIME rtStop  = rtStart + m_rtFrameLength;

    pSample->SetTime( &rtStart, &rtStop );

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


/**********************************************
*
*  YuvSourceFilter Class
*
**********************************************/

CUnknown * WINAPI YuvSourceFilter::CreateInstance(IUnknown *pUnk, HRESULT *phr)
{
  YuvSourceFilter *pNewFilter = new YuvSourceFilter(pUnk, phr );
  if (phr)
  {
    if (pNewFilter == NULL) 
      *phr = E_OUTOFMEMORY;
    else
      *phr = S_OK;
  }
  return pNewFilter;
}

YuvSourceFilter::YuvSourceFilter(IUnknown *pUnk, HRESULT *phr)
  : CSource(NAME("CSIR RTVC YUV Source"), pUnk, CLSID_YUVSource),
  m_iWidth(352),
  m_iHeight(288),
  m_sDimensions("352x288"),
  m_iFramesPerSecond(30),
  m_iNoFrames(150),		  //TODO: move to property page
  m_dBitsPerPixel(1.5),	//TODO: move to property page
  m_pYuvBuffer(NULL),
  m_iFileSize(0),
  m_iRead(0)
{
  // Init CSettingsInterface
  initParameters();

  m_pPin = new YuvOutputPin(phr, this);

  if (phr)
  {
    if (m_pPin == NULL)
      *phr = E_OUTOFMEMORY;
    else
      *phr = S_OK;
  }  
}

YuvSourceFilter::~YuvSourceFilter()
{
  if ( m_in1.is_open() )
  {
    m_in1.close();
  }
  if (m_pYuvBuffer)
    delete[] m_pYuvBuffer;

  delete m_pPin;
}

STDMETHODIMP YuvSourceFilter::Load( LPCOLESTR lpwszFileName, const AM_MEDIA_TYPE *pmt )
{
  // Store the URL
  m_sFile = StringUtil::wideToStl(lpwszFileName);

  m_in1.open(m_sFile.c_str(), std::ifstream::in | std::ifstream::binary);
  if ( m_in1.is_open() )
  {
    m_in1.seekg( 0, std::ios::end );
    m_iFileSize = m_in1.tellg();
    m_in1.seekg( 0 , std::ios::beg );

    recalculate();
    m_pYuvBuffer = new unsigned char[m_iFrameSize];

    guessDimensions();
    return S_OK;
  }
  else
  {
    SetLastError("Failed to open file: " + m_sFile, true);
    return E_FAIL;
  }
}

void YuvSourceFilter::guessDimensions()
{
  // try and guess what the dimensions are based on file name
  std::string sFile = m_sFile;
  std::transform(sFile.begin(), sFile.end(), sFile.begin(), ::tolower);

  size_t pos = sFile.find("qcif");
  if (pos != std::string::npos)
  {
    setDimensions("176x144");
    return;
  }

  // NB search for cif last since 'qcif' and '4cif' contain 'cif'
  pos = sFile.find("cif");
  if (pos != std::string::npos)
  {
    setDimensions("352x288");
    return;
  }

  // try searching for x's
  pos = sFile.find("x");
  while (pos != std::string::npos)
  {
    std::string sWidth, sHeight;
    // check if there is at least a number before and after the x, else continue
    size_t uiBeforePos = pos - 1;
    while (isdigit(sFile[uiBeforePos]) && uiBeforePos >= 0)
    {
      --uiBeforePos;
    }
    if (uiBeforePos != pos - 1)
    {
      // we found at least one digit
      sWidth = sFile.substr(uiBeforePos + 1, pos - uiBeforePos - 1);
    }
    else
    {
      pos = sFile.find("x", pos + 1);
      continue;
    }

    size_t uiAfterPos = pos + 1;
    while (isdigit(sFile[uiAfterPos]) && uiAfterPos < sFile.length())
    {
      ++uiAfterPos;
    }
    if (uiAfterPos != pos + 1)
    {
      // we found at least one digit
      sHeight = sFile.substr(pos + 1, uiAfterPos - pos - 1);
    }
    else
    {
      pos = sFile.find("x", pos + 1);
      continue;
    }

    // try and convert to int
    int iWidth = convert<int>(sWidth);
    int iHeight = convert<int>(sHeight);
    setDimensions(iWidth, iHeight);
    break;
  }
}

STDMETHODIMP YuvSourceFilter::GetCurFile( LPOLESTR * ppszFileName, AM_MEDIA_TYPE *pmt )
{
  *ppszFileName = NULL;

  if (m_sFile.length()!=0) 
  {
    WCHAR* pFileName = (StringUtil::stlToWide(m_sFile));	

    DWORD n = sizeof(WCHAR)*(1+lstrlenW(pFileName));

    *ppszFileName = (LPOLESTR) CoTaskMemAlloc( n );
    if (*ppszFileName!=NULL) {
      CopyMemory(*ppszFileName, pFileName, n);
    }
    delete[] pFileName;
  }
  return NOERROR;
}

STDMETHODIMP YuvSourceFilter::NonDelegatingQueryInterface( REFIID riid, void **ppv )
{
  if(riid == (IID_ISettingsInterface))
  {
    return GetInterface((ISettingsInterface*) this, ppv);
  }
  else if (riid == IID_IStatusInterface)
  {
    return GetInterface((IStatusInterface*) this, ppv);
  }
  else if (riid == IID_IFileSourceFilter)
  {
    return GetInterface((IFileSourceFilter*) this, ppv);
  }
  else if (riid == IID_ISpecifyPropertyPages)
  {
    return GetInterface(static_cast<ISpecifyPropertyPages*>(this), ppv);
  }
  else
  {
    return CSource::NonDelegatingQueryInterface(riid, ppv);
  }
}

STDMETHODIMP YuvSourceFilter::Stop()
{
  m_pPin->m_iCurrentFrame = 0;
  m_in1.seekg( 0 , std::ios::beg );
  return CSource::Stop();
}

STDMETHODIMP YuvSourceFilter::SetParameter( const char* type, const char* value )
{
  if (strcmp(type, SOURCE_DIMENSIONS) == 0)
  {
    if ( setDimensions(value) )
      return CSettingsInterface::SetParameter(type, value);
    else
      return E_FAIL;
  }
  else
  {
    HRESULT hr = CSettingsInterface::SetParameter(type, value);
    if (SUCCEEDED(hr)) recalculate();
    return hr;
  }
}

void YuvSourceFilter::recalculate()
{
  m_iFrameSize = m_iWidth*m_iHeight*m_dBitsPerPixel;
  m_iNoFrames = m_iFileSize/m_iFrameSize;
  m_pPin->m_rtFrameLength = UNITS/m_iFramesPerSecond;
}

bool YuvSourceFilter::readFrame()
{
  if (m_in1.is_open())
  {
    m_in1.read( (char*)m_pYuvBuffer, m_iFrameSize );
    return true;
  }
  return false;
}

bool YuvSourceFilter::setDimensions( const std::string& sDimensions )
{
  size_t pos = sDimensions.find( "x" );
  if (pos != std::string::npos ) 
  {
    int iWidth = 0, iHeight = 0;
    std::istringstream istr( sDimensions.substr( 0, pos ) );
    istr >> iWidth;
    std::istringstream istr2( sDimensions.substr( pos + 1) );
    istr2 >> iHeight;
    return setDimensions(iWidth, iHeight);
  }
  return false;
}

bool YuvSourceFilter::setDimensions(int iWidth, int iHeight)
{
  if (iWidth > 0 && iHeight > 0)
  {
    m_iWidth = iWidth;
    m_iHeight = iHeight;
    recalculate();

    if (m_pYuvBuffer)
    {
      delete[] m_pYuvBuffer;
    }
    m_pYuvBuffer = new unsigned char[m_iFrameSize];
    return true;
  }
  else
  {
    return false;
  }
}
