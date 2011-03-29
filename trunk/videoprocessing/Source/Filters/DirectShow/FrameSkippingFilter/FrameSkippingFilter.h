/** @file

MODULE                : FrameSkippingFilter

FILE NAME             : FrameSkippingFilter.h

DESCRIPTION           :  This filter skips a specified number of frames depending on the parameter denoted by "skipparam"

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
#include <streams.h>

//#include <Filters/DirectShow/DirectShowFilterGuids.h>
#include <DirectShow/CSettingsInterface.h>

// {8E974B99-BC09-4041-98F4-1103BAA1B0EA}
static const GUID CLSID_RTVCFrameSkippingFilter = 
{ 0xbbf2f0af, 0x9f7f, 0x4406, { 0xae, 0x9c, 0xe5, 0xf, 0x92, 0xc4, 0x63, 0xbb } };

//// {F0A41B88-2311-42f9-8E26-679BE4FFC176}
static const GUID CLSID_FrameSkippingProperties = 
{ 0xf0a41b88, 0x2311, 0x42f9, { 0x8e, 0x26, 0x67, 0x9b, 0xe4, 0xff, 0xc1, 0x76 } };



// Forward declarations

class FrameSkippingFilter : public CTransInPlaceFilter,
                            public CSettingsInterface,
                            public ISpecifyPropertyPages
{
public:
  DECLARE_IUNKNOWN

  /// Constructor
  FrameSkippingFilter(LPUNKNOWN pUnk, HRESULT *pHr);
  /// Destructor
  ~FrameSkippingFilter();

  /// Static object-creation method (for the class factory)
  static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr); 

  /// override this to publicize our interfaces
  STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

  //Overriding various CTransInPlace methods
  HRESULT Transform(IMediaSample *pSample);/* Overrriding the receive method. 
                                           This method receives a media sample, processes it, and delivers it to the downstream filter.*/

  HRESULT CheckInputType(const CMediaType* mtIn);

  STDMETHODIMP GetPages(CAUUID *pPages) // For the Skipping Property Page
	{
		if (pPages == NULL) return E_POINTER;
		pPages->cElems = 1;
		pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
		if (pPages->pElems == NULL) 
		{
			return E_OUTOFMEMORY;
		}
		pPages->pElems[0] = CLSID_FrameSkippingProperties;
		return S_OK;
	}

  STDMETHODIMP Run( REFERENCE_TIME tStart );
  STDMETHODIMP Stop(void);

  /// Overridden from CSettingsInterface
  virtual void initParameters() { addParameter("skipframe", &m_SkipFrameNumber, 0); }


private:

  bool seenFirstFrame;
  REFERENCE_TIME previousTimestamp;  
  
  //for frame skipper filter
  int in_framecount;
  int m_SkipFrameNumber;

 
};
