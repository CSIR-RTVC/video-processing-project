/** @file

MODULE				: RtspSourceOutputPin

FILE NAME			: RtspSourceOutputPin.h

DESCRIPTION			: Output pin for RtspSourceFilter
					        This pin currently supports PCM, MP3 and AMR-NB media types.
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2010, CSIR
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

// Forward
class MediaSample;
class RtspSourceFilter;
class MediaSubsession;

typedef std::map<std::string, std::string> StringMap_t;

/**
 * \ingroup DirectShowFilters
 * Output pin for RtspSourceFilter
 * This pin currently caters for PCM output media types.
 */
class RtspSourceOutputPin : public CSourceStream,
                            public IAMPushSource
{
	///this needs to be declared for the extra interface (adds the COM AddRef, etc methods)
	DECLARE_IUNKNOWN; // Need this for the IAMPushSource interface

	friend class RtspSourceFilter;
public:
	/// Constructor
	RtspSourceOutputPin(HRESULT* pHr, RtspSourceFilter* pFilter, MediaSubsession* pMediaSubsession, const StringMap_t& rParams, int nID);

	/// Destructor
	virtual ~RtspSourceOutputPin(void);

	/// We do not currently support quality control
	STDMETHODIMP Notify(IBaseFilter* pSelf, Quality q)
	{
		return E_FAIL;
	}

	/// override this to publicise our interfaces
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, __deref_out void **ppv)
	{
		if (riid == IID_IAMPushSource)
			return GetInterface((IAMPushSource *)this, ppv);
		else
			return CSourceStream::NonDelegatingQueryInterface(riid, ppv); 
	}

	/// From IAMPushSource
	STDMETHODIMP GetMaxStreamOffset(REFERENCE_TIME *prtMaxOffset){return E_NOTIMPL;}
	/// From IAMPushSource
	STDMETHODIMP GetPushSourceFlags(ULONG *pFlags)
	{ 
		*pFlags = AM_PUSHSOURCECAPS_PRIVATE_CLOCK;return S_OK;
	}
	/// From IAMPushSource
	STDMETHODIMP GetStreamOffset(REFERENCE_TIME *prtOffset){return E_NOTIMPL;}
	/// From IAMPushSource
	STDMETHODIMP SetMaxStreamOffset(REFERENCE_TIME rtMaxOffset)	{return E_NOTIMPL;}
	/// From IAMPushSource
	STDMETHODIMP SetPushSourceFlags(ULONG Flags)	{return E_NOTIMPL;}
	/// From IAMPushSource
	STDMETHODIMP SetStreamOffset(REFERENCE_TIME rtOffset){return E_NOTIMPL;}
	/// From IAMPushSource
	STDMETHODIMP GetLatency(REFERENCE_TIME *prtLatency)
	{ 
		return E_NOTIMPL;
		//TODO: set different latencies for audio and video
		//Set 450ms Latency 
		*prtLatency = 45000000; return S_OK;
	}

protected:

	///Override CSourceStream methods
	virtual HRESULT GetMediaType(CMediaType* pMediaType);

	/// Decides how big the buffer must be for the media sample
	HRESULT DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pRequestProperties);

	/// method which fills frame buffer with media sample data and sets start times
	HRESULT FillBuffer(IMediaSample* pSample);

	/// Buffer processing loop
	virtual HRESULT DoBufferProcessingLoop(void);    // the loop executed whilst running
	
private:
  void initialiseMediaType(MediaSubsession* pMediaSubsession, const StringMap_t& rParams, HRESULT* phr);
  void copyMediaDataIntoSample( IMediaSample* pSample, MediaSample* pSampleData );
  void setSampleTimestamps( IMediaSample* pSample, MediaSample* pSampleData );
  void setSynchronisationMarker( IMediaSample* pSample, MediaSample* pSampleData );
  void resetTimeStampOffsets();
  bool isIdrFrame(unsigned char nalUnitHeader)
  {
    unsigned uiForbiddenZeroBit = nalUnitHeader & 0x80;
    //assert(uiForbiddenZeroBit == 0);
    unsigned uiNalRefIdc = nalUnitHeader & 0x60;
    unsigned char uiNalUnitType = nalUnitHeader & 0x1f;
    switch (uiNalUnitType)
    {
      // IDR nal unit types
    case 5:
      return true;
    default:
      return false;
    }
  }
	/// Source filter
	RtspSourceFilter* m_pFilter;
	/// Protects our internal state
	CCritSec m_cSharedState;

	/// Media type of this output pin
	CMediaType* m_pMediaType;

	// Audio parameters
  int m_nBitsPerSample;
  int m_nChannels;
  int m_nBytesPerSecond;
  int m_nSamplesPerSecond;
  unsigned m_uiSamplingRate;

	// ID
	int m_nID;

  // Synchronisation
  bool m_bFirstSample;
  bool m_bFirstSyncedSample;
  double m_dStreamTimeOffset;
};
