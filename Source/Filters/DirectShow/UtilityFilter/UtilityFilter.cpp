/** @file

MODULE                : UtilityFilter

FILE NAME			  : UtilityFilter.cpp

DESCRIPTION           :

LICENSE: Software License Agreement (BSD License)

Copyright (c) 2015, CSIR
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
#include "UtilityFilter.h"
#include <DirectShow/DirectShowHelper.h>
#include <Shared/Conversion.h>
#include <Shared/StringUtil.h>
#include <Gdiplus.h>
#include <GdiPlusInit.h>
#include <numeric>

using namespace Gdiplus;

UtilityFilter::UtilityFilter(LPUNKNOWN pUnk, HRESULT *pHr)
  : CTransInPlaceFilter(NAME("CSIR RTVC Utility Filter"), pUnk, CLSID_VPP_UtilityFilter, pHr, false),
  //m_bIsAdvertType(false),
  m_dEstimatedFramerate(0.0),
  m_bSeenFirstFrame(false),
  m_previousTimestamp(0),
  m_tMaxDifferenceBetweenFrames(50000000), // 5 seconds max
  m_dTimerFrequency(0.0),
  m_gdiplusStartupInput(new Gdiplus::GdiplusStartupInput()),
  m_pGdiToken(NULL),
  m_hrInterfaceAquired(S_OK)
{
  GdiplusStartup(&m_pGdiToken, m_gdiplusStartupInput, NULL); //gdi+ init
  // Init parameters
  initParameters();
}

UtilityFilter::~UtilityFilter()
{
  GdiplusShutdown(m_pGdiToken); //gdi+ end session
  delete m_gdiplusStartupInput;
}

CUnknown * WINAPI UtilityFilter::CreateInstance( LPUNKNOWN pUnk, HRESULT *pHr )
{
  UtilityFilter *pFilter = new UtilityFilter(pUnk, pHr);
  if (pFilter== NULL) 
  {
    *pHr = E_OUTOFMEMORY;
  }
  return pFilter;
}

STDMETHODIMP UtilityFilter::NonDelegatingQueryInterface( REFIID riid, void **ppv )
{
  if (riid == IID_ISpecifyPropertyPages)
  {
    return GetInterface(static_cast<ISpecifyPropertyPages*>(this), ppv);
  }
  else if(riid == (IID_ISettingsInterface))
  {
    return GetInterface((ISettingsInterface*) this, ppv);
  }
  else if (riid == (IID_IFilterInfoSourceInterface))
  {
    return GetInterface((IFilterInfoSourceInterface*) this, ppv);
  }
  else
  {
    return CTransInPlaceFilter::NonDelegatingQueryInterface(riid, ppv);
  }
}

HRESULT UtilityFilter::drawTextOntoFrame(const std::string& sText, IMediaSample *pSample)
{
  // get image properties
  AM_MEDIA_TYPE mt;
  HRESULT hr = InputPin()->ConnectionMediaType(&mt);
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

  wchar_t* wsText = StringUtil::stlToWide(sText);

  BITMAPINFO bitmapInfo;
  bitmapInfo.bmiHeader = *pbmi;

  Bitmap bm(&bitmapInfo, (void*)pBuffer);
  Graphics* pGraphics = Graphics::FromImage(&bm);

  // Initialize font
  Gdiplus::Font myFont(L"Arial", 16);
  //RectF layoutRect(0.0f, 0.0f, 200.0f, 50.0f);
  RectF layoutRect(m_uiX, m_uiY, 350.0f, 50.0f);
  StringFormat format;
  format.SetAlignment(StringAlignmentNear);
  SolidBrush blackBrush(Color(255, 0, 0, 0));
  SolidBrush greenBrush(Color(255, 0, 255, 0));

  // Draw string
  pGraphics->DrawString(
    wsText,
    sText.length(),
    &myFont,
    layoutRect,
    &format,
    &greenBrush);

  delete[] wsText;
  // Do we need to delete it?
  delete pGraphics;
  return S_OK;
}

HRESULT UtilityFilter::Transform(IMediaSample *pSample)
{
  using namespace Gdiplus;

  std::string sText;

  switch (m_uiFramerateEstimationMode)
  {
  case UTIL_NONE:
    {
      return S_OK;
    }
  case UTIL_TIME_STAMP_FRAME:
    {
      // %X = time
      sText = StringUtil::GetTimeString("%X");
      break;
    }
  case UTIL_DATE_TIME_STAMP_FRAME:
    {
      // %d/%m/%Y date
      sText = StringUtil::GetTimeString("%d/%m/%Y %X");
      break;
    }
  case UTIL_EST_FRAMERATE_TIMESTAMP:
  case UTIL_EST_FRAMERATE_SYSTEM_TIME:
  case UTIL_DATE_TIME_STAMP_AND_EST_FPS_TS:
  case UTIL_DATE_TIME_STAMP_AND_EST_FPS_ST:
    {
      REFERENCE_TIME tStart = 0, tStop = 0;
      HRESULT hr = pSample->GetTime(&tStart, &tStop);
      if (SUCCEEDED(hr))
      {
        if (!m_bSeenFirstFrame)
        {
          initFramerateEstimation(tStart);
          m_bSeenFirstFrame = true;
          return S_OK;
        }
        else
        {
          updateFramerateEstimation(tStart);
          double dAverageFramerate = calculateFramerate();
          m_dEstimatedFramerate = dAverageFramerate;          
          sText = getString(dAverageFramerate);
          break;
        }
      }
      else
      {
        return hr;
      }
    }
  case UTIL_MEASURE_BITRATE:
  {
    REFERENCE_TIME tStart = 0, tStop = 0;
    HRESULT hr = pSample->GetTime(&tStart, &tStop);
    if (SUCCEEDED(hr))
    {
      updateForBitrateMeasurement(tStart, pSample->GetActualDataLength());
      double dBitrate = calculateBitrate();
      sText = getString(dBitrate);
    }
    break;
  }
  case UTIL_DETECT_UPSTREAM_SOURCE:
  {
    if (FAILED(m_hrInterfaceAquired)) return m_hrInterfaceAquired;
    int iLength = BUFFER_SIZE;
#if 1
    if (!m_pFilterInfoSourceInterface)
    {
      m_hrInterfaceAquired = CDirectShowHelper::FindFirstInterface(this, PINDIR_INPUT, IID_IFilterInfoSourceInterface, (void **)&m_pFilterInfoSourceInterface);
      if (FAILED(m_hrInterfaceAquired))
      {
        DbgLog((LOG_TRACE, 0, TEXT("Failed to find upstream IID_IFilterInfoSourceInterface interface")));
        return m_hrInterfaceAquired;
      }
    }

    HRESULT hr = m_pFilterInfoSourceInterface->GetMeasurement(m_pBuffer, &iLength);
    if (FAILED(hr))
    {
      DbgLog((LOG_TRACE, 0, TEXT("Failed to find get measurement from IID_IFilterInfoSourceInterface interface")));
      return hr;
    }
#endif
    sText = std::string(m_pBuffer, iLength);
    break;
  }
  default:
    {
      return S_OK;
    }
  }

  // check flag if text should be written to frame
  if (m_bWriteOnFrame)
  {
    DbgLog((LOG_TRACE, 0, TEXT("Utility: %s"), sText.c_str()));
    return drawTextOntoFrame(sText, pSample);
  }
  else
    return S_OK;
}

HRESULT UtilityFilter::CheckInputType(const CMediaType* mtIn)
{
  // Check the major type.
  if (mtIn->majortype != MEDIATYPE_Video)
  {
    return VFW_E_TYPE_NOT_ACCEPTED;
  }

  if (mtIn->formattype != FORMAT_VideoInfo)
  {
    return VFW_E_TYPE_NOT_ACCEPTED;
  }
  return S_OK;
}

HRESULT UtilityFilter::Run( REFERENCE_TIME tStart )
{
  return CTransInPlaceFilter::Run(tStart);
}

HRESULT UtilityFilter::Stop( void )
{
  m_qDurations.clear();
  m_dEstimatedFramerate = 0.0;
  m_bSeenFirstFrame = false;
  m_previousTimestamp = 0;

  return CTransInPlaceFilter::Stop();
}

void UtilityFilter::updateForBitrateMeasurement(REFERENCE_TIME tSampleStartTime, unsigned uiSampleSize)
{
  m_qDurations.push_back(SampleInfo(tSampleStartTime, uiSampleSize));
  // limit the number of measurements
  if (m_qDurations.size() > m_uiHistorySize)
  {
    m_qDurations.pop_front();
  }
}

void UtilityFilter::initFramerateEstimation(REFERENCE_TIME tSampleStartTime)
{
  switch (m_uiFramerateEstimationMode)
  {
  case UTIL_EST_FRAMERATE_TIMESTAMP:
  case UTIL_DATE_TIME_STAMP_AND_EST_FPS_TS:
    {
      m_previousTimestamp = tSampleStartTime;
      break;
    }
  case UTIL_EST_FRAMERATE_SYSTEM_TIME:
  case UTIL_DATE_TIME_STAMP_AND_EST_FPS_ST:
    {
      QueryPerformanceCounter((LARGE_INTEGER*)&m_previousTimestamp);
      break;
    }
  }

  // clear queue in case mode was changed
  m_qDurations.clear();
}

void UtilityFilter::updateFramerateEstimation( REFERENCE_TIME tSampleStartTime)
{
  switch (m_uiFramerateEstimationMode)
  {
  case UTIL_EST_FRAMERATE_TIMESTAMP:
  case UTIL_DATE_TIME_STAMP_AND_EST_FPS_TS:
    {
      REFERENCE_TIME tDiff = tSampleStartTime - m_previousTimestamp;
      // Make sure timestamps are increasing
      if (tDiff > 0 && tDiff < m_tMaxDifferenceBetweenFrames)
      {
        m_qDurations.push_back(SampleInfo(tDiff));
        m_previousTimestamp = tSampleStartTime;

        // limit the number of measurements
        if (m_qDurations.size() > m_uiHistorySize)
        {
          m_qDurations.pop_front();
        }
      }
      else
      {
        // reset vars and restart estimation
        initFramerateEstimation(tSampleStartTime);
      }
      break;
    }
  case UTIL_EST_FRAMERATE_SYSTEM_TIME:
  case UTIL_DATE_TIME_STAMP_AND_EST_FPS_ST:
    {
      unsigned __int64 tNow;
      unsigned __int64 freq;
      QueryPerformanceCounter((LARGE_INTEGER *)&tNow);
      QueryPerformanceFrequency((LARGE_INTEGER*)&freq);

      REFERENCE_TIME tDiff = ((tNow - m_previousTimestamp)* 10000000 / freq) ;
      // Make sure timestamps are increasing
      if (tDiff > 0 && tDiff < m_tMaxDifferenceBetweenFrames)
      {
        m_qDurations.push_back(SampleInfo(tDiff));
        m_previousTimestamp = tNow;

        // limit the number of measurements
        if (m_qDurations.size() > m_uiHistorySize)
        {
          m_qDurations.pop_front();
        }
      }
      else
      {
        // reset vars and restart estimation
        initFramerateEstimation(tSampleStartTime);
      }
      break;
    }
  }
}

double UtilityFilter::calculateFramerate()
{
  if (!m_qDurations.empty())
  {
    // calc avg framerate
    REFERENCE_TIME uiTotal = 0;
    for (size_t i = 0; i < m_qDurations.size(); ++i)
    {
      uiTotal += m_qDurations[i].TimeStamp;
    }
    uiTotal /= m_qDurations.size();

    // Avoid div by zero in cases where the timestamps are invalid
    if (uiTotal > 0)
    {
      double dAverageFramerate = static_cast<double>(UNITS / static_cast<double>(uiTotal));
      return dAverageFramerate;
    }
  }
  return 0.0;
}

double UtilityFilter::calculateBitrate()
{
  if (!m_qDurations.empty())
  {
    // calc avg bitrate
    REFERENCE_TIME tFirst = m_qDurations.front().TimeStamp;
    REFERENCE_TIME tLast = m_qDurations.back().TimeStamp;
    REFERENCE_TIME tDiff = tLast - tFirst;
    double dSeconds = tDiff / (double)UNITS;

    unsigned uiTotalBytes = 0;
    for (size_t i = 0; i < m_qDurations.size(); ++i)
    {
      uiTotalBytes += m_qDurations[i].Size;
    }
    
    double dBitrate = uiTotalBytes/1000 * 8 / dSeconds;

    return dBitrate;
  }
  return 0.0;
}

STDMETHODIMP UtilityFilter::GetMeasurement(char* value, int* buffersize)
{
  if (m_sLastValue.length() > *buffersize)
    return E_FAIL;
  memcpy(value, m_sLastValue.c_str(), m_sLastValue.length());
  *buffersize = m_sLastValue.length();
  return S_OK;
}

const std::string UtilityFilter::getString( double dAverageFramerate )
{
  switch (m_uiFramerateEstimationMode)
  {
  case UTIL_EST_FRAMERATE_TIMESTAMP:
  case UTIL_EST_FRAMERATE_SYSTEM_TIME:
    {
      m_sLastValue = StringUtil::doubleToString(dAverageFramerate, m_uiPrecision) + " fps";
      return m_sLastValue;
    }
  case UTIL_DATE_TIME_STAMP_AND_EST_FPS_TS:
  case UTIL_DATE_TIME_STAMP_AND_EST_FPS_ST:
    {
      std::string sFramerate = StringUtil::doubleToString(dAverageFramerate, m_uiPrecision) + " fps";
      std::string m_sLastValue = StringUtil::GetTimeString("%d/%m/%Y %X") + " " + sFramerate;
      return m_sLastValue;
    }
  case UTIL_MEASURE_BITRATE:
    {
      m_sLastValue = StringUtil::doubleToString(dAverageFramerate, m_uiPrecision) + " kbps";
      return m_sLastValue;
  }
  default:
    return "";
  }
}

STDMETHODIMP UtilityFilter::SetParameter( const char* type, const char* value )
{
  if (SUCCEEDED(CSettingsInterface::SetParameter(type, value)))
  {
    // reset members
    m_bSeenFirstFrame = false;
    m_previousTimestamp = 0.0;
    return S_OK;
  }
  else
  {
    return E_FAIL;
  }
}
