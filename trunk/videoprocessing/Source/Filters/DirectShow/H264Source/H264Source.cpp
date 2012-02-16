/** @file

MODULE				: H264Source

FILE NAME			: H264Source.cpp

DESCRIPTION			: 

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
#include "H264Source.h"
#include "H264OutputPin.h"

#include <Shared/Conversion.h>
#include <Shared/StringUtil.h>

const AMOVIESETUP_MEDIATYPE sudOpPinTypes =
{
  &MEDIATYPE_Video,       // Major type
  &MEDIASUBTYPE_NULL      // Minor type
};

#define DEFAULT_WIDTH 352
#define DEFAULT_HEIGHT 288

const unsigned char g_startCode[] = { 0, 0, 0, 1};

CUnknown * WINAPI H264SourceFilter::CreateInstance(IUnknown *pUnk, HRESULT *phr)
{
  H264SourceFilter *pNewFilter = new H264SourceFilter(pUnk, phr );
  if (phr)
  {
    if (pNewFilter == NULL) 
      *phr = E_OUTOFMEMORY;
    else
      *phr = S_OK;
  }
  return pNewFilter;
}

const unsigned MINIMUM_BUFFER_SIZE = 1024;
H264SourceFilter::H264SourceFilter(IUnknown *pUnk, HRESULT *phr)
  : CSource(NAME("CSIR RTVC H264 Source"), pUnk, CLSID_H264Source),
  m_iWidth(DEFAULT_WIDTH),
  m_iHeight(DEFAULT_HEIGHT),
  m_iFramesPerSecond(30),
  m_uiCurrentBufferSize(MINIMUM_BUFFER_SIZE),
  m_pBuffer(NULL),
  m_uiBytesInBuffer(0),
  m_uiCurrentNalUnitSize(0),
  m_pSeqParamSet(0),
  m_uiSeqParamSetLen(0),
  m_pPicParamSet(0),
  m_uiPicParamSetLen(0),
  m_iFileSize(0),
  m_iRead(0)
{
  // Init CSettingsInterface
  initParameters();

  m_pPin = new H264OutputPin(phr, this);

  if (phr)
  {
    if (m_pPin == NULL)
      *phr = E_OUTOFMEMORY;
    else
      *phr = S_OK;
  }  
}

H264SourceFilter::~H264SourceFilter()
{
  if (m_pSeqParamSet) delete[] m_pSeqParamSet;
  if (m_pPicParamSet) delete[] m_pPicParamSet;

  if ( m_in1.is_open() )
  {
    m_in1.close();
  }
  if (m_pBuffer)
    delete[] m_pBuffer;

  delete m_pPin;
}

void H264SourceFilter::reset()
{
  m_pPin->m_iCurrentFrame = 0;
  m_uiBytesInBuffer = 0;
  m_uiCurrentNalUnitSize = 0;
  m_in1.clear(); // clear for next play
  m_in1.seekg( 0 , std::ios::beg );
}

STDMETHODIMP H264SourceFilter::Load( LPCOLESTR lpwszFileName, const AM_MEDIA_TYPE *pmt )
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
    m_pBuffer = new unsigned char[m_uiCurrentBufferSize];

    bool bSps = false, bPps = false;
    // now try to search for SPS and PPS
    while (true)
    {
      if (!readNalUnit())
      {
        // failed to find SPS and PPS
        return E_FAIL;
      }

      // check for SPS and PPS
      if (isSps(m_pBuffer[4]))
      {
        bSps = true;
        if (m_pSeqParamSet) delete[] m_pSeqParamSet;
        m_pSeqParamSet = new unsigned char[m_uiCurrentNalUnitSize];
        memcpy(m_pSeqParamSet, m_pBuffer, m_uiCurrentNalUnitSize);
        m_uiSeqParamSetLen = m_uiCurrentNalUnitSize;
      }

      if (isPps(m_pBuffer[4]))
      {
        bPps = true;
        if (m_pPicParamSet) delete[] m_pPicParamSet;
        m_pPicParamSet = new unsigned char[m_uiCurrentNalUnitSize];
        memcpy(m_pPicParamSet, m_pBuffer, m_uiCurrentNalUnitSize);
        m_uiPicParamSetLen = m_uiCurrentNalUnitSize;
      }

      if (bSps && bPps) break;
    }

    // reset file pointer
    reset();

    return S_OK;
  }
  else
  {
    SetLastError("Failed to open file: " + m_sFile, true);
    return E_FAIL;
  }
}

STDMETHODIMP H264SourceFilter::GetCurFile( LPOLESTR * ppszFileName, AM_MEDIA_TYPE *pmt )
{
  *ppszFileName = NULL;

  if (m_sFile.length() != 0) 
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

STDMETHODIMP H264SourceFilter::NonDelegatingQueryInterface( REFIID riid, void **ppv )
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

STDMETHODIMP H264SourceFilter::Stop()
{
  reset();
  return CSource::Stop();
}

STDMETHODIMP H264SourceFilter::SetParameter( const char* type, const char* value )
{
#if 0
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
#else
  return CSettingsInterface::SetParameter(type, value);
#endif
}

void H264SourceFilter::recalculate()
{
  m_pPin->m_rtFrameLength = UNITS/m_iFramesPerSecond;
}

int H264SourceFilter::findIndexOfNextStartCode(unsigned uiStartingPos)
{
  for (size_t i = uiStartingPos; i < m_uiBytesInBuffer - 4; ++i)
  {
    if (memcmp((char*)&m_pBuffer[i], g_startCode, 4) == 0)
    {
      return i;
    }
  }
  return -1;
}

bool H264SourceFilter::readNalUnit()
{
  if (m_in1.is_open())
  {
    // shift previous NAL unit out of buffer
    if (m_uiCurrentNalUnitSize > 0 && m_uiBytesInBuffer >= m_uiCurrentNalUnitSize)
    {
      ASSERT(m_uiBytesInBuffer >= m_uiCurrentNalUnitSize);
      if (m_uiBytesInBuffer > m_uiCurrentNalUnitSize)
      {
        memmove(m_pBuffer, m_pBuffer + m_uiCurrentNalUnitSize, m_uiBytesInBuffer - m_uiCurrentNalUnitSize);
      }

      m_uiBytesInBuffer -= m_uiCurrentNalUnitSize;
      m_uiCurrentNalUnitSize = 0;
    }

    // try and keep buffer full
    m_in1.read( (char*)m_pBuffer + m_uiBytesInBuffer, m_uiCurrentBufferSize - m_uiBytesInBuffer );
    m_uiBytesInBuffer += m_in1.gcount();

    if (m_uiBytesInBuffer < 4 ) return false;

    // check if we have enough data in the buffer to read the NAL unit
    if (memcmp(g_startCode, m_pBuffer, 4) == 0)
    {
      unsigned uiStartingPos = 4;
      while (true)
      {
        // need to find next start code or EOF
        int iIndex = findIndexOfNextStartCode(uiStartingPos);
        if (iIndex != -1)
        {
          // found the end of the nal unit
          m_uiCurrentNalUnitSize = iIndex;
          return true;
        }
        else
        {
          // didn't find next start code: read more data if not EOF
          if (m_in1.eof())
          {
            if (m_uiBytesInBuffer > 0)
            {
              m_uiCurrentNalUnitSize = m_uiBytesInBuffer;
              return true; // EOF: return last NAL unit
            }
            else
            {
              return false;
            }
          }
          if (m_in1.fail()) return false;

          // expand buffer 
          BYTE* pNewBuffer = new BYTE[m_uiCurrentBufferSize * 2];
          memcpy(pNewBuffer, m_pBuffer, m_uiCurrentBufferSize);
          delete[] m_pBuffer;
          m_pBuffer = pNewBuffer;
          uiStartingPos = m_uiCurrentBufferSize;
          
          // read new data
          m_in1.read( (char*)m_pBuffer + m_uiBytesInBuffer, m_uiCurrentBufferSize);
          m_uiBytesInBuffer += m_in1.gcount();

          m_uiCurrentBufferSize *= 2;
        }
      }
    }
    else
    {
      // no start code: exit
      return false;
    }
  }
  return false;
}

