/** @file

MODULE				: RtspSourceFilter

FILE NAME			: RtspSourceFilter.h

DESCRIPTION			: RTSP source filter which currently only supports PCM audio format. This project has been included to demonstrate
					how DirectShow and the liveMedia RTSP RTP library can be combined.
					  
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

// DirectShow includes
#pragma warning(push)     // disable for this header only
#pragma warning(disable:4312) 
// DirectShow
#include <Streams.h>
#pragma warning(pop)      // restore original warning level

// STL includes
#include <string>
#include <vector>

// RTVC includes
#include "RtpPacketManager.h"
#include <DirectShow/CStatusInterface.h>
#include <DirectShow/CSettingsInterface.h>

// Forward
class RtspSourceOutputPin;
class RtspDataSession;
class MediaSubsession;

// Filter name strings
#define g_wszFilterName    L"CSIR RTVC RTSP Source Filter(Audio)"

#define STREAM_USING_TCP	"streamusingtcp"

// {CF5878AC-78F0-4544-83B9-A940E20D9690}
static const GUID CLSID_RTVC_RtspAudioSourceFilter = 
{ 0xcf5878ac, 0x78f0, 0x4544, { 0x83, 0xb9, 0xa9, 0x40, 0xe2, 0xd, 0x96, 0x90 } };

// {3955BE86-A1E9-4fa3-8654-623D73DD2F29}
static const GUID CLSID_RtspProperties = 
{ 0x3955be86, 0xa1e9, 0x4fa3, { 0x86, 0x54, 0x62, 0x3d, 0x73, 0xdd, 0x2f, 0x29 } };

/**
* RTSP Source filter that receives PCM audio data from an RTSP server using the LGPL liveMedia streaming library.
* The liveMedia library can be obtained at http://www.live555.com
*/
class RtspSourceFilter :	public CSource,				/* Source Filter */
							public CSettingsInterface,	/* Rtvc Settings Interface */
							public CStatusInterface,	/* Rtvc Status Interface */
							public IFileSourceFilter,	/* To facilitate loading of URL */
							public IAMFilterMiscFlags,	/* For Live Source purposes */
							public ISpecifyPropertyPages
{
	///this needs to be declared for the extra interface (adds the COM AddRef, etc methods)
	DECLARE_IUNKNOWN;

	/// Give the pin friend access
	friend class RtspSourceOutputPin;

public:
	/// DLL factory method
	static CUnknown *WINAPI CreateInstance(IUnknown* pUnk, HRESULT* phr);

	/// override this to publicize our interfaces
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);
	
	/// From CSettingsInterface
	virtual void initParameters()
	{
		addParameter(STREAM_USING_TCP, &m_bStreamUsingTCP, true);
	}

	/// From IFileSourceFilter
	STDMETHODIMP Load(LPCOLESTR lpwszFileName, const AM_MEDIA_TYPE *pmt);
	/// From IFileSourceFilter
	STDMETHODIMP GetCurFile(LPOLESTR * ppszFileName, AM_MEDIA_TYPE *pmt);

	/// From IAMFilterMiscFlags
	virtual ULONG STDMETHODCALLTYPE GetMiscFlags()
	{
		return AM_FILTER_MISC_FLAGS_IS_SOURCE;
	}

	/// CBase Filter methods - Overridden since we don't just have one input and output pin as the standard transform filter does
	virtual int GetPinCount();
	/// Override this method: the pins get created in this method of the transform filter
	virtual CBasePin * GetPin(int n);
	/// Method needed to connect pins based on their names
	virtual STDMETHODIMP FindPin(LPCWSTR Id, IPin **ppPin);

	/// From CSource
	STDMETHODIMP Stop();
	/// From CSource
	STDMETHODIMP Pause();
	
	/// From CBaseFilter TO PREVENT THE VIDEO RENDERER FROM BLOCKING: READ http://msdn2.microsoft.com/en-us/library/ms783675(VS.85).aspx
	STDMETHODIMP GetState( DWORD dwMilliSecsTimeout, FILTER_STATE *State	);

	/// This method starts an RTSP session in a new thread provided one hasn't been started already. In that case this method does nothing
	void StartRtspServerThreadIfNotStarted();
	/// Starts an RTSP session. This is just a helper method which is called from the thread spawned by StartRtspServerThreadIfNotStarted
	void StartRtspSession();

	/// Creates an output pin based on the passed in MediaSubsession and adds it to the vector of output pins
	/// @param[in] pSubsession a liveMedia MediaSubsession that has been obtained using RTSP and contains information such as media type, subtype- etc.
	/// @param[out] pHr Set this value to S_OK before passing pHr into this method. If the method fails, the error code will be returned in this out parameter.
	/// @remark Currently creates pins for 8/16 bit PCM audio and the RTVC H263 video media formats
	void createOutputPin(MediaSubsession *pSubsession, HRESULT* phr);

	STDMETHODIMP GetPages(CAUUID *pPages)
	{
		if (pPages == NULL) return E_POINTER;
		pPages->cElems = 1;
		pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
		if (pPages->pElems == NULL) 
		{
			return E_OUTOFMEMORY;
		}
		pPages->pElems[0] = CLSID_RtspProperties;
		return S_OK;
	}
private:

	///Private Constructor
	RtspSourceFilter(IUnknown* pUnk, HRESULT* phr);
	virtual ~RtspSourceFilter(void);

	/// Notifies the filter of the start time offset.
	/// The filter will set this offset on all output pins.
	void notifyFilterAboutOffset(double dOffset);

	/// vector of output pins
	std::vector<RtspSourceOutputPin*> m_vOutputPins;

	/// Url
	std::string m_sUrl;

	CCritSec m_stateLock;

	/// Stores whether this filter should connect using TCP or UDP
	bool m_bStreamUsingTCP;

	/// Stores streaming state
	bool m_bStreaming;

	/// Handle to let the filter know that the RTSP thread has finished
	/// The Stop method waits for a signal that the liveMedia event loop has ended before it proceeds
	HANDLE m_hLiveMediaStopEvent;

	/// Stream time offset
	REFERENCE_TIME m_tStreamTimeOffset;
	double m_dStreamTimeOffset;

	// RTP Packet Manager
	RtpPacketManager m_rtpPacketManager;
	
	/// Rtsp Session
	RtspDataSession* m_pRtspDataSession;
};
