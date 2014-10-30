/** @file

MODULE                : FrameSkippingFilter

FILE NAME			        : FrameSkippingFilter.cpp

DESCRIPTION           : This filter skips a specified number of frames depending on the parameter denoted by "skipFrame"

LICENSE: Software License Agreement (BSD License)

Copyright (c) 2014, CSIR
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
#include "FrameSkippingFilter.h"
#include <set>
#include <Shared/StringUtil.h>

FrameSkippingFilter::FrameSkippingFilter(LPUNKNOWN pUnk, HRESULT *pHr)
: CTransInPlaceFilter(NAME("CSIR VPP Frame Skipping Filter"), pUnk, CLSID_RTVC_VPP_FrameSkippingFilter, pHr, false),
  m_uiSkipFrameNumber(0),
  m_uiTotalFrames(0),
  m_uiCurrentFrame(0)
{
  // Init parameters
	initParameters();
}

FrameSkippingFilter::~FrameSkippingFilter()
{;}

CUnknown * WINAPI FrameSkippingFilter::CreateInstance( LPUNKNOWN pUnk, HRESULT *pHr )
{
  FrameSkippingFilter *pFilter = new FrameSkippingFilter(pUnk, pHr);
  if (pFilter== NULL) 
  {
    *pHr = E_OUTOFMEMORY;
  }
  return pFilter;
}

STDMETHODIMP FrameSkippingFilter::NonDelegatingQueryInterface( REFIID riid, void **ppv )
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

HRESULT FrameSkippingFilter::Transform(IMediaSample *pSample)
{
  /*  Check for other streams and pass them on */

  // don't skip control info
  AM_SAMPLE2_PROPERTIES * const pProps = m_pInput->SampleProps();
  if (pProps->dwStreamId != AM_STREAM_MEDIA) {
    return S_OK;
  }

  if (m_vFramesToBeSkipped.empty())
    return S_OK;

  int iSkip = m_vFramesToBeSkipped[m_uiCurrentFrame++];
  if (m_uiCurrentFrame >= m_vFramesToBeSkipped.size())
  {
    m_uiCurrentFrame = 0;
  }

  if (iSkip == 1)
  {
    return S_FALSE; 
  }        
  return S_OK;
}

HRESULT FrameSkippingFilter::CheckInputType(const CMediaType* mtIn)
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

HRESULT FrameSkippingFilter::Run( REFERENCE_TIME tStart )
{
  m_vFramesToBeSkipped.clear();
  // calculate which frames should be dropped
  if (m_uiSkipFrameNumber < m_uiTotalFrames)
  {
    double dRatio = m_uiTotalFrames / static_cast<double>(m_uiSkipFrameNumber);
    std::set<int> toBeSkipped;
    std::set<int> toBePlayed;
    // populate to be skipped: note that this index is 1-indexed
    for (size_t iCount = 1; iCount <= m_uiSkipFrameNumber; ++iCount)
    {
#if _MSC_VER > 1600
      int iToBeSkipped = static_cast<int>(round(iCount * dRatio));
#else
      int iToBeSkipped = static_cast<int>(floor(iCount * dRatio + 0.5));
#endif
      toBeSkipped.insert(iToBeSkipped);
    }

    // populate to be played
    for (size_t iCount = 1; iCount <= m_uiTotalFrames; ++iCount)
    {
      auto found = toBeSkipped.find(iCount);
      if (found == toBeSkipped.end())
      {
        toBePlayed.insert(iCount);
        m_vFramesToBeSkipped.push_back(0);
      }
      else
      {
        m_vFramesToBeSkipped.push_back(1);
      }
    }
  }
  else
  {
    // invalid input
    m_vFramesToBeSkipped.clear();
  }

  return CTransInPlaceFilter::Run(tStart);
}

HRESULT FrameSkippingFilter::Stop( void )
{
  m_uiCurrentFrame = 0;
  m_vFramesToBeSkipped.clear();
  return CTransInPlaceFilter::Stop();
}

