/** @file

MODULE                : FrameSkipperFilter

FILE NAME			        : FrameSkipperFilter.cpp

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

#include "FrameSkipperFilter.h"
#include <Shared/StringUtil.h>



FrameSkipperFilter::FrameSkipperFilter(LPUNKNOWN pUnk, HRESULT *pHr)
: CTransInPlaceFilter(NAME("CSIR RTVC Frame Skipping Filter"), pUnk, CLSID_FrameSkipperFilter, pHr, false),
  m_SkipFrameNumber(0),
  seenFirstFrame(false),
  previousTimestamp(0),
  in_framecount(0)
{
  // Init parameters
	initParameters();
}

FrameSkipperFilter::~FrameSkipperFilter()
{;}

CUnknown * WINAPI FrameSkipperFilter::CreateInstance( LPUNKNOWN pUnk, HRESULT *pHr )
{
  FrameSkipperFilter *pFilter = new FrameSkipperFilter(pUnk, pHr);
  if (pFilter== NULL) 
  {
    *pHr = E_OUTOFMEMORY;
  }
  return pFilter;
}

STDMETHODIMP FrameSkipperFilter::NonDelegatingQueryInterface( REFIID riid, void **ppv )
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

HRESULT FrameSkipperFilter::Transform(IMediaSample *pSample)
{  
    REFERENCE_TIME tStart;
    REFERENCE_TIME tStop;

    //int m_SkipFrameNumber = 1; // parameter to define how many frames to skip

    HRESULT hr = pSample->GetTime(&tStart, &tStop); //get the current time now
       
    in_framecount++ ; // counting the incoming samples

    REFERENCE_TIME tDiff = tStart - previousTimestamp;
    // Make sure timestamps are increasing
    if (tDiff > 0)
    { 
      if(m_SkipFrameNumber != 0)
      {

        if((in_framecount % (m_SkipFrameNumber+1))!= 0) // if the in_framecount is not equal to the specified parameter, skip that frame
        {
          return S_FALSE; 
        }        
      }
      previousTimestamp = tStart;
    }
	  return S_OK; // Let the frame through if it is indeed equal to that parameter
}

HRESULT FrameSkipperFilter::CheckInputType(const CMediaType* mtIn)
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

HRESULT FrameSkipperFilter::Run( REFERENCE_TIME tStart )
{
  return CTransInPlaceFilter::Run(tStart);
}

HRESULT FrameSkipperFilter::Stop( void )
{
  seenFirstFrame = false;
  previousTimestamp = 0;
  in_framecount = 0;
  m_SkipFrameNumber = 0;

  return CTransInPlaceFilter::Stop();
}

