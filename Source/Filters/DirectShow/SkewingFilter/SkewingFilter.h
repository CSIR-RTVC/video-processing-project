/** @file

MODULE                : SkewingFilter

FILE NAME             : SkewingFilter.h

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

// {01A5D2B0-E1F5-4fbf-97A7-2025FC13FB63}
static const GUID CLSID_VPP_SkewingFilter = 
{ 0x1a5d2b0, 0xe1f5, 0x4fbf, { 0x97, 0xa7, 0x20, 0x25, 0xfc, 0x13, 0xfb, 0x63 } };

// {3B0BEFCE-7127-4484-B908-FE35FC2F96B8}
static const GUID CLSID_SkewingProperties = 
{ 0x3b0befce, 0x7127, 0x4484, { 0xb9, 0x8, 0xfe, 0x35, 0xfc, 0x2f, 0x96, 0xb8 } };


// Forward declarations

class SkewingFilter : public CTransInPlaceFilter,
                      public CSettingsInterface,
                      public ISpecifyPropertyPages
                       
{
public:
  DECLARE_IUNKNOWN

  /// Constructor
  SkewingFilter(LPUNKNOWN pUnk, HRESULT *pHr);
  /// Destructor
  ~SkewingFilter();

  /// Static object-creation method (for the class factory)
  static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr); 

  /// override this to publicize our interfaces
  STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

  //Overriding various CTransInPlace methods
 HRESULT Transform(IMediaSample *pSample);/* Overrriding the receive method. 
                                           This method receives a media sample, processes it, and delivers it to the downstream filter.*/

  HRESULT CheckInputType(const CMediaType* mtIn);

  STDMETHODIMP GetPages(CAUUID *pPages) // For the Skewing Property Page
	{
		if (pPages == NULL) return E_POINTER;
		pPages->cElems = 1;
		pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
		if (pPages->pElems == NULL) 
		{
			return E_OUTOFMEMORY;
		}
		pPages->pElems[0] = CLSID_SkewingProperties;
		return S_OK;
	}

  STDMETHODIMP Run( REFERENCE_TIME tStart );
  STDMETHODIMP Stop(void);

   /// Overridden from CSettingsInterface
  virtual void initParameters() { addParameter("skewTimestamp", &add_time, 0); }

private:

  bool seenFirstFrame;
  REFERENCE_TIME previousTimestamp;  
  
  //for adding time to the timestamp filter
  int add_time;
   
};
