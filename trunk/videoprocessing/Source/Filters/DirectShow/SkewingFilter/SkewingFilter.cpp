/** @file

MODULE                : SkewingFilter

FILE NAME			        : SkewingFilter.cpp

DESCRIPTION           : This filter skips a specified number of frames depending on the parameter denoted by "skipFrame"

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
#include "stdafx.h"

#include "SkewingFilter.h"
#include <Shared/StringUtil.h>

SkewingFilter::SkewingFilter(LPUNKNOWN pUnk, HRESULT *pHr)
: CTransInPlaceFilter(NAME("CSIR RTVC Skewing Filter"), pUnk, CLSID_RTVCSkewingFilter, pHr, false),
  seenFirstFrame(false),
  previousTimestamp(0),
  add_time(0)
{
  // Init parameters
	initParameters();
}

SkewingFilter::~SkewingFilter()
{;}

CUnknown * WINAPI SkewingFilter::CreateInstance( LPUNKNOWN pUnk, HRESULT *pHr )
{
  SkewingFilter *pFilter = new SkewingFilter(pUnk, pHr);
  if (pFilter== NULL) 
  {
    *pHr = E_OUTOFMEMORY;
  }
  return pFilter;
}

STDMETHODIMP SkewingFilter::NonDelegatingQueryInterface( REFIID riid, void **ppv )
{
  if(riid == (IID_ISettingsInterface))
  {
    return GetInterface((ISettingsInterface*) this, ppv);
  }
  if (riid == (IID_ISpecifyPropertyPages))
	{
		return GetInterface(static_cast<ISpecifyPropertyPages*>(this), ppv);
	}
  else
  {
    return CTransInPlaceFilter::NonDelegatingQueryInterface(riid, ppv);
  }
}

HRESULT SkewingFilter::Transform(IMediaSample *pSample)
{
   
    REFERENCE_TIME tStart;
	  REFERENCE_TIME tStop;
    REFERENCE_TIME newStart;
	  REFERENCE_TIME newStop;

    //int add_time = 100; // parameter to define how many frames to skip

    HRESULT hr = pSample->GetTime(&tStart, &tStop); //get the current time now
       
    REFERENCE_TIME tDiff = tStart - previousTimestamp;
    // Make sure timestamps are increasing
    if (tDiff > 0)
    {           
        //newStart = tStart + (add_time*1000000); //add time to start time in multiples of 1000 000
        //newStop  = tStop  + (add_time*1000000); //add time to stop time in multiples of 1000 000

        newStart = tStart + (add_time); 
        newStop  = tStop  + (add_time);
       
        hr = pSample->SetTime(&newStart, &newStop); //get the current time now
        previousTimestamp = tStart;
    }
	  return S_OK;
}

HRESULT SkewingFilter::CheckInputType(const CMediaType* mtIn)
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

HRESULT SkewingFilter::Run( REFERENCE_TIME tStart )
{
  return CTransInPlaceFilter::Run(tStart);
}

HRESULT SkewingFilter::Stop( void )
{
  add_time = 0;
  seenFirstFrame = false;
  previousTimestamp = 0;

  return CTransInPlaceFilter::Stop();
}

