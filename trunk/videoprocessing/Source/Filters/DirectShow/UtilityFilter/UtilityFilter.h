/** @file

MODULE                : UtilityFilter

FILE NAME             : UtilityFilter.h

DESCRIPTION           : Filter estimates framerate
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
#pragma once
#pragma warning(push)     // disable for this header only
#pragma warning(disable:4312) 
// DirectShow
#include <streams.h>
#pragma warning(pop)      // restore original warning level
#include <deque>
#include <comdef.h>
#include <DirectShow/CSettingsInterface.h>
#include <DirectShow/IFileInfoSourceInterface.h>

// {5E0F5204-F598-46F7-9D05-D0CCD195E545}
static const GUID CLSID_RTVC_UtilityFilter =
{ 0x5e0f5204, 0xf598, 0x46f7, { 0x9d, 0x5, 0xd0, 0xcc, 0xd1, 0x95, 0xe5, 0x45 } };

_COM_SMARTPTR_TYPEDEF(IFilterInfoSourceInterface, IID_IFilterInfoSourceInterface);

// fwd
namespace Gdiplus
{
struct GdiplusStartupInput;
}

#define ULONG_PTR ULONG

// {F97EA37A-5338-417D-AFC7-891482789536}
static const GUID CLSID_UtilityProperties = 
{ 0xf97ea37a, 0x5338, 0x417d, { 0xaf, 0xc7, 0x89, 0x14, 0x82, 0x78, 0x95, 0x36 } };

enum UtilityMode
{
  UTIL_EST_FRAMERATE_TIMESTAMP              = 0,
  UTIL_EST_FRAMERATE_SYSTEM_TIME            = 1,
  UTIL_TIME_STAMP_FRAME 		                = 2,
  UTIL_DATE_TIME_STAMP_FRAME 		            = 3,
  UTIL_DATE_TIME_STAMP_AND_EST_FPS_TS 		  = 4, // Combo
  UTIL_DATE_TIME_STAMP_AND_EST_FPS_ST 		  = 5, // Combo
  UTIL_MEASURE_BITRATE                      = 6, 
  UTIL_DETECT_UPSTREAM_SOURCE               = 7,
  UTIL_NONE = 8
};

static bool isFramerateEstimationMode(UtilityMode eMode )
{
  switch (eMode)
  {
  case UTIL_EST_FRAMERATE_TIMESTAMP:
  case UTIL_EST_FRAMERATE_SYSTEM_TIME:
  case UTIL_DATE_TIME_STAMP_AND_EST_FPS_TS:
  case UTIL_DATE_TIME_STAMP_AND_EST_FPS_ST:
    return true;
  default:
    {
      return false;
    }
  }
}

#define BUFFER_SIZE 256

/**
 * @brief The utility filter can be used to overlay estimated frame rates, the current date/time
 * estimate bit rates and overlay info from an upstream IFilterInfoSourceInterface.
 * The functionality is configured by setting the "mode" parameter of the filter using the
 * CSettingInterface COM interface.
 * 
 * Estimate framerate based on timestamps
 * \ingroup DirectShowFilters
 */
class UtilityFilter : public CTransInPlaceFilter,
                      public CSettingsInterface,
                      public ISpecifyPropertyPages,
                      public IFilterInfoSourceInterface
{
public:
  DECLARE_IUNKNOWN

  /// Constructor
  UtilityFilter(LPUNKNOWN pUnk, HRESULT *pHr);
  /// Destructor
  ~UtilityFilter();

  /// Static object-creation method (for the class factory)
  static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr); 

  /// override this to publicize our interfaces
  STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

  //Overriding various CTransInPlace methods
  HRESULT Transform(IMediaSample *pSample);

  HRESULT CheckInputType(const CMediaType* mtIn);

  STDMETHODIMP Run( REFERENCE_TIME tStart );
  STDMETHODIMP Stop(void);

  STDMETHODIMP GetPages(CAUUID *pPages)
	{
		if (pPages == NULL) return E_POINTER;
		pPages->cElems = 1;
		pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
		if (pPages->pElems == NULL) 
		{
			return E_OUTOFMEMORY;
		}
		pPages->pElems[0] = CLSID_UtilityProperties;
		return S_OK;
	}

  /// Overridden from CSettingsInterface
  virtual void initParameters() 
  {
    addParameter("X", &m_uiX, 0);
    addParameter("Y", &m_uiY, 0);
    addParameter("estimatedframerate", &m_dEstimatedFramerate, 0);
    addParameter("mode", &m_uiFramerateEstimationMode, 1);
    addParameter("writeonframe", &m_bWriteOnFrame, true);
    addParameter("precision", &m_uiPrecision, 1);
    addParameter("history_size", &m_uiHistorySize, 50);
  }
  STDMETHODIMP SetParameter( const char* type, const char* value);

  STDMETHODIMP GetMeasurement(char* value, int* buffersize);

private:

  HRESULT drawTextOntoFrame(const std::string& sText, IMediaSample *pSample);
  void initFramerateEstimation(REFERENCE_TIME tSampleStartTime);
  void updateFramerateEstimation(REFERENCE_TIME tSampleStartTime);
  void updateForBitrateMeasurement(REFERENCE_TIME tSampleStartTime, unsigned uiSampleSize);

  double calculateFramerate();
  double calculateBitrate();
  const std::string getString(double dFramerate);

  // parameters
  unsigned m_uiX;
  unsigned m_uiY;
  double m_dEstimatedFramerate;
  unsigned m_uiFramerateEstimationMode;
  bool m_bWriteOnFrame;
  unsigned m_uiPrecision;

  bool m_bSeenFirstFrame;
  REFERENCE_TIME m_previousTimestamp;
  REFERENCE_TIME m_tMaxDifferenceBetweenFrames;
  double m_dTimerFrequency;

  struct SampleInfo
  {
    SampleInfo()
      :TimeStamp(0),
      Size(0)
    {

    }
    SampleInfo(REFERENCE_TIME ts)
      :TimeStamp(ts),
      Size(0)
    {

    }
    SampleInfo(REFERENCE_TIME ts, unsigned size)
      :TimeStamp(ts),
      Size(size)
    {

    }
    REFERENCE_TIME TimeStamp;
    unsigned Size;
  };
  std::deque<SampleInfo> m_qDurations;
  unsigned m_uiHistorySize;
  std::string m_sLastValue;

  Gdiplus::GdiplusStartupInput* m_gdiplusStartupInput;
  ULONG_PTR m_pGdiToken;

  // for upstream mode
  IFilterInfoSourceInterfacePtr m_pFilterInfoSourceInterface;
  HRESULT m_hrInterfaceAquired;
  char m_pBuffer[BUFFER_SIZE];

};
