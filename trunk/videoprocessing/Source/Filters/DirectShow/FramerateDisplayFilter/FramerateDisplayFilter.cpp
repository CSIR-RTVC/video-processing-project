/** @file

MODULE                : FramerateDisplayFilter

FILE NAME			  : FramerateDisplayFilter.cpp

DESCRIPTION           :

LICENSE: Software License Agreement (BSD License)

Copyright (c) 2010, CSIR
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

#include "FramerateDisplayFilter.h"
#include <Shared/StringUtil.h>

#include <Gdiplus.h>

#include <numeric>
//
//static wchar_t* stlToWide(std::string sString)
//{
//  return(asciiToWide(sString.c_str()));
//}
//
///// The caller must free the memory for the result
//static wchar_t* asciiToWide(const char* szString)
//{
//  size_t uiLen = strlen(szString);
//  wchar_t* wszBuffer = new wchar_t[uiLen + 1];
//  size_t uiRes = mbstowcs(wszBuffer, szString, uiLen);
//  if (uiRes != -1)
//  {
//    wszBuffer[uiRes] = L'\0';
//    return wszBuffer;
//  }
//  else
//  {
//    delete[] wszBuffer;
//    return NULL;
//  }
//}

const unsigned MAX_FRAMERATE_INTERVAL_SIZE = 50;

FramerateDisplayFilter::FramerateDisplayFilter(LPUNKNOWN pUnk, HRESULT *pHr)
  : CTransInPlaceFilter(NAME("CSIR RTVC Framerate Display Filter"), pUnk, CLSID_RTVCFramerateDisplayFilter, pHr, false),
  m_bSeenFirstFrame(false),
  m_previousTimestamp(0)
{;}

FramerateDisplayFilter::~FramerateDisplayFilter()
{;}

CUnknown * WINAPI FramerateDisplayFilter::CreateInstance( LPUNKNOWN pUnk, HRESULT *pHr )
{
  FramerateDisplayFilter *pFilter = new FramerateDisplayFilter(pUnk, pHr);
  if (pFilter== NULL) 
  {
    *pHr = E_OUTOFMEMORY;
  }
  return pFilter;
}

STDMETHODIMP FramerateDisplayFilter::NonDelegatingQueryInterface( REFIID riid, void **ppv )
{
  return CTransInPlaceFilter::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT FramerateDisplayFilter::Transform(IMediaSample *pSample)
{
  using namespace Gdiplus;

  REFERENCE_TIME tStart = 0, tStop = 0;
  HRESULT hr = pSample->GetTime(&tStart, &tStop);
  if (!m_bSeenFirstFrame)
  {
    if (SUCCEEDED(hr))
    {
      m_previousTimestamp = tStart;
      m_bSeenFirstFrame = true;
    }
    return S_OK;
  }
  else
  {
    if (SUCCEEDED(hr))
    {
      REFERENCE_TIME tDiff = tStart - m_previousTimestamp;
      // Make sure timestamps are increasing
      if (tDiff > 0)
      {
        m_qDurations.push_back(tDiff);
        m_previousTimestamp = tStart;

        // limit the number of measurements
        if (m_qDurations.size() > MAX_FRAMERATE_INTERVAL_SIZE)
        {
          m_qDurations.pop_front();
        }
      }
    }
  }

  if (!m_qDurations.empty())
  {
    // calc avg framerate
    REFERENCE_TIME tAvg = std::accumulate(m_qDurations.begin(), m_qDurations.end(), 0) / m_qDurations.size();

    // Avoid div by zero in cases where the timestamps are invalid
    if (tAvg > 0)
    {
      double dAverageFramerate = static_cast<double>(UNITS/tAvg);
      // Create a string.
      std::string sFramerate = StringUtil::doubleToString(dAverageFramerate) + " fps";
      wchar_t* wsFramerate = StringUtil::stlToWide(sFramerate);

      // get image properties
      AM_MEDIA_TYPE mt;
      hr = InputPin()->ConnectionMediaType(&mt);
      if (FAILED(hr))
      {
        return hr;
      }

      ASSERT(mt.formattype == FORMAT_VideoInfo);
      BITMAPINFOHEADER *pbmi = HEADER(mt.pbFormat);

      // Get image buffer
      BYTE *pBuffer(NULL);
      hr = pSample->GetPointer(&pBuffer);
      if (FAILED(hr))
      {
        return hr;
      }

      BITMAPINFO bitmapInfo;
      bitmapInfo.bmiHeader = *pbmi;

      Bitmap bm(&bitmapInfo, (void*)pBuffer);
      Graphics* pGraphics = Graphics::FromImage(&bm);

      // Initialize font
      Font myFont(L"Arial", 16);
      RectF layoutRect(0.0f, 0.0f, 200.0f, 50.0f);
      StringFormat format;
      format.SetAlignment(StringAlignmentNear);
      SolidBrush blackBrush(Color(255, 0, 0, 0));
      SolidBrush greenBrush(Color(255, 0, 255, 0));

      // Draw string
      pGraphics->DrawString(
        wsFramerate,
        sFramerate.length(),
        &myFont,
        layoutRect,
        &format,
        &greenBrush);

      delete[] wsFramerate;

      // Do we need to delete it?
      delete pGraphics;
    }
  }
  
  return S_OK;
}

HRESULT FramerateDisplayFilter::CheckInputType(const CMediaType* mtIn)
{
  // Check the major type.
	if (mtIn->majortype != MEDIATYPE_Video)
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	// Adding advert media type to this method
	if ((mtIn->subtype != MEDIASUBTYPE_RGB24) && (mtIn->subtype != MEDIASUBTYPE_RGB32) )
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	if (mtIn->formattype != FORMAT_VideoInfo)
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}
  return S_OK;
}

HRESULT FramerateDisplayFilter::Run( REFERENCE_TIME tStart )
{
  return CTransInPlaceFilter::Run(tStart);
}

HRESULT FramerateDisplayFilter::Stop( void )
{
  return CTransInPlaceFilter::Stop();
}

