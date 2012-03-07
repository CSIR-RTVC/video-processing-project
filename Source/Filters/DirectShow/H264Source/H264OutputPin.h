/** @file

MODULE				: H264

FILE NAME			: H264OutputPin.h

DESCRIPTION			: DirectShow H.264 source filter output pin header
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2012, CSIR
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

// fwd
class H264SourceFilter;

class H264OutputPin : public CSourceStream
{
  friend class H264SourceFilter;

public:
  H264OutputPin(HRESULT *phr, H264SourceFilter *pFilter);
  ~H264OutputPin();

  // Override the version that offers exactly one media type
  HRESULT GetMediaType(CMediaType *pMediaType);
  HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest);
  HRESULT FillBuffer(IMediaSample *pSample);

  // Quality control
  // Not implemented because we aren't going in real time.
  // If the file-writing filter slows the graph down, we just do nothing, which means
  // wait until we're unblocked. No frames are ever dropped.
  STDMETHODIMP Notify(IBaseFilter *pSelf, Quality q)
  {
    return E_FAIL;
  }

protected:
  H264SourceFilter* m_pFilter;

  //int m_iWidth;
  //int m_iHeight;
  int m_iCurrentFrame;
  REFERENCE_TIME m_rtFrameLength;

  CCritSec m_cSharedState;            // Protects our internal state
};
