/** @file

MODULE                : FramerateDisplayFilter

FILE NAME             : FramerateDisplayFilter.h

DESCRIPTION           : Filter that logs media sample timestamps and durations to file specified using IFileSourceFilter interface

LICENSE: Software License Agreement (BSD License)

Copyright (c) 2011, CSIR
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

#include <DirectShow/CSettingsInterface.h>

#include <deque>
#define ULONG_PTR ULONG
#include <gdiplus.h>
#include <GdiPlusInit.h>

#pragma comment(lib, "comctl32.lib")

// {8E974B99-BC09-4041-98F4-1103BAA1B0EA}
static const GUID CLSID_RTVCFramerateDisplayFilter = 
{ 0x8e974b99, 0xbc09, 0x4041, { 0x98, 0xf4, 0x11, 0x3, 0xba, 0xa1, 0xb0, 0xea } };

enum FramerateEstimationMode
{
  FRM_TIMESTAMP = 0,
  FRM_SYSTEM_TIME = 1
};

// {2B67CF84-C1C1-481F-AF87-B26046AFBBAE}
static const GUID CLSID_FramerateDisplayProperties = 
{ 0x2b67cf84, 0xc1c1, 0x481f, { 0xaf, 0x87, 0xb2, 0x60, 0x46, 0xaf, 0xbb, 0xae } };

/**
* \ingroup DirectShowFilters
*/
class FramerateDisplayFilter :  public CTransInPlaceFilter,
                                public CSettingsInterface,
					                      public ISpecifyPropertyPages
{
public:
  DECLARE_IUNKNOWN

  /// Constructor
  FramerateDisplayFilter(LPUNKNOWN pUnk, HRESULT *pHr);
  /// Destructor
  ~FramerateDisplayFilter();

  /// Static object-creation method (for the class factory)
  static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr); 

  /// override this to publicize our interfaces
  STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

  //Overriding various CTransInPlace methods
  HRESULT Transform(IMediaSample *pSample);/* Overrriding the receive method. 
                                           This method receives a media sample, processes it, and delivers it to the downstream filter.*/

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
		pPages->pElems[0] = CLSID_FramerateDisplayProperties;
		return S_OK;
	}

  /// Overridden from CSettingsInterface
  virtual void initParameters() 
  { 
    addParameter("X", &m_uiX, 0);
    addParameter("Y", &m_uiY, 0);
    addParameter("estimatedframerate", &m_dEstimatedFramerate, 0);
    addParameter("mode", &m_uiFramerateEstimationMode, 1);
  }
private:

  unsigned m_uiX;
  unsigned m_uiY;
  double m_dEstimatedFramerate;
  unsigned m_uiFramerateEstimationMode;
  bool m_bSeenFirstFrame;
  REFERENCE_TIME m_previousTimestamp;  
  std::deque<REFERENCE_TIME> m_qDurations;
  double m_dTimerFrequency;

  Gdiplus::GdiplusStartupInput m_gdiplusStartupInput;
  ULONG_PTR m_pGdiToken;
};
