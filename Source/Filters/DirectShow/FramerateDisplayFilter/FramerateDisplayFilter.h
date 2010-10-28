/** @file

MODULE                : FramerateDisplayFilter

FILE NAME             : FramerateDisplayFilter.h

DESCRIPTION           : Filter that logs media sample timestamps and durations to file specified using IFileSourceFilter interface

LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008, CSIR
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

#include <deque>

// {8E974B99-BC09-4041-98F4-1103BAA1B0EA}
static const GUID CLSID_RTVCFramerateDisplayFilter = 
{ 0x8e974b99, 0xbc09, 0x4041, { 0x98, 0xf4, 0x11, 0x3, 0xba, 0xa1, 0xb0, 0xea } };

// Forward declarations

/**
* \ingroup DirectShowFilters
*/
class FramerateDisplayFilter : public CTransInPlaceFilter
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

private:

  bool m_bSeenFirstFrame;
  REFERENCE_TIME m_previousTimestamp;  
  std::deque<REFERENCE_TIME> m_qDurations;
};
