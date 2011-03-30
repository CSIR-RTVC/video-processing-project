/** @file

MODULE                : FrameSkipperFilter

FILE NAME             : FrameSkipperFilter.h

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

// {07815D2B-8276-4e90-8A7B-965B0EEB9DC0}
static const GUID CLSID_FrameSkipperFilter = 
{ 0x7815d2b, 0x8276, 0x4e90, { 0x8a, 0x7b, 0x96, 0x5b, 0xe, 0xeb, 0x9d, 0xc0 } };

// {E7EBA8F2-D55B-4523-988B-5B9137B2D948}
static const GUID CLSID_FrameSkipperProperties  = 
{ 0xe7eba8f2, 0xd55b, 0x4523, { 0x98, 0x8b, 0x5b, 0x91, 0x37, 0xb2, 0xd9, 0x48 } };


// Forward declarations

class FrameSkipperFilter : public CTransInPlaceFilter,
                           public CSettingsInterface,
                           public ISpecifyPropertyPages
{
public:
  DECLARE_IUNKNOWN

  /// Constructor
  FrameSkipperFilter(LPUNKNOWN pUnk, HRESULT *pHr);
  /// Destructor
  ~FrameSkipperFilter();

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
		pPages->pElems[0] = CLSID_FrameSkipperProperties;
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
