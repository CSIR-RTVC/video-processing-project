/** @file

MODULE				: RtspSourceFilter

FILE NAME			: RtspSourceFilter.cpp

DESCRIPTION			: 
					  
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
#include "stdafx.h"

// STL
#include <iostream>

// RTVC
#include "RtspSourceFilter.h"
#include "RtspSourceOutputPin.h"

// LiveMedia
#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <RTSPClient.hh>

#include "RtspDataSession.h"

#include <Shared/StringUtil.h>

RtspSourceFilter::RtspSourceFilter( IUnknown* pUnk, HRESULT* phr )
	: CSource(NAME("RTVC Live Media RTSP Source Filter"), pUnk, CLSID_RTVC_RtspAudioSourceFilter),
	m_bStreamUsingTCP(false),
	m_bStreaming(false),
    m_bMediaSessionSetupComplete(false),
	m_tStreamTimeOffset(0),
	m_dStreamTimeOffset(0.0),
	m_hLiveMediaStopEvent(NULL),
	m_pRtspSession(new RtspSession(&m_rtpPacketManager))
{
	// Init CSettingsInterface
	initParameters();
	m_bStreaming = false;
}

RtspSourceFilter::~RtspSourceFilter(void)
{
    //m_pRtspSession->teardownMediaSession();
    delete m_pRtspSession;
    m_pRtspSession = NULL;

	for (int i = 0; i < m_vOutputPins.size();i++)
	{
		delete m_vOutputPins[i];
		m_vOutputPins[i] = NULL;
	}
}

CUnknown *WINAPI RtspSourceFilter::CreateInstance( IUnknown* pUnk, HRESULT* phr )
{
	RtspSourceFilter* pAdvertSource = new RtspSourceFilter(pUnk, phr);
	if (!pAdvertSource)
	{
		*phr = E_OUTOFMEMORY;
	}
	return pAdvertSource;
}

STDMETHODIMP RtspSourceFilter::NonDelegatingQueryInterface( REFIID riid, void **ppv )
{
	if(riid == (IID_ISettingsInterface))
	{
		return GetInterface((ISettingsInterface*) this, ppv);
	}
	else if (riid == IID_IStatusInterface)
	{
		return GetInterface((IStatusInterface*) this, ppv);
	}
	else if (riid == IID_IFileSourceFilter)
	{
		return GetInterface((IFileSourceFilter*) this, ppv);
	}
	else if (riid == IID_ISpecifyPropertyPages)
	{
		return GetInterface(static_cast<ISpecifyPropertyPages*>(this), ppv);
	}
	else
	{
		return CSource::NonDelegatingQueryInterface(riid, ppv);
	}
}


STDMETHODIMP RtspSourceFilter::Load( LPCOLESTR lpwszFileName, const AM_MEDIA_TYPE *pmt )
{
    HRESULT hr;
	// Store the URL
	m_sUrl = StringUtil::wideToStl(lpwszFileName);

	if ( m_pRtspSession->setupMediaSession( m_sUrl ) )
	{
        m_bMediaSessionSetupComplete = true;
		MediaSubsession* pSubsession = NULL;
		MediaSubsessionIterator it(*m_pRtspSession->getSession());
		while ((pSubsession = it.next()) != NULL)
		{
			createOutputPin(pSubsession, &hr);
			if (FAILED(hr))
			{
				return hr;
			}
		}
		// At this point we could start the scheduler and call WaitForSingleObject or WaitForMultipleObjects
		// Until we receive the I-Frame allowing us to configure the media types correctly
		return S_OK;
	}
	else
	{
		SetLastError(m_pRtspSession->getLastError(), true);
		return E_FAIL;
	}
}

STDMETHODIMP RtspSourceFilter::GetCurFile( LPOLESTR * ppszFileName, AM_MEDIA_TYPE *pmt )
{
	// If RTSP exchange has successfully been completed
	// TOREVISE: It does not make sense since this filter will hardly ever have only one media type.
	if (pmt != NULL)
	{
		// Get media type from pin
		if (m_vOutputPins.size() > 0)
		{
			// Get media type from pin
			CMediaType* pMediaType = m_vOutputPins[0]->m_pMediaType;
			if (pMediaType == NULL)
			{
				//some error has occurred
				return E_FAIL;
			}
			else
			{
				// copy media type
				HRESULT hr = CopyMediaType(pmt, (AM_MEDIA_TYPE*) pMediaType);
				if (hr = E_UNEXPECTED)
				{
					return E_FAIL;
				}
			}
		}
		else
		{
			return E_FAIL;
		}
	}
	// Copied from Async filter sample
	*ppszFileName = NULL;

	if (m_sUrl.length()!=0) 
	{
		WCHAR* pFileName = (StringUtil::stlToWide(m_sUrl));	

		DWORD n = sizeof(WCHAR)*(1+lstrlenW(pFileName));

		*ppszFileName = (LPOLESTR) CoTaskMemAlloc( n );
		if (*ppszFileName!=NULL) {
			CopyMemory(*ppszFileName, pFileName, n);
		}
		delete[] pFileName;
	}

	return NOERROR;
}

void RtspSourceFilter::createOutputPin(MediaSubsession *pSubsession, HRESULT* pHr)
{
	// Create the output pin using the size of the output pin vector as an ID
	// This ID will be used by the pins buffer processing loop to query the packet manager for packets
	RtspSourceOutputPin* pPin = new RtspSourceOutputPin(pHr, this, pSubsession, m_vOutputPins.size());
	// Add to list
	m_vOutputPins.push_back(pPin);
	// Refresh enumerator
	IncrementPinVersion();
}

int RtspSourceFilter::GetPinCount()
{
	return (int)(m_vOutputPins.size());
}

CBasePin * RtspSourceFilter::GetPin( int n )
{
	if ((n >= GetPinCount()) || (n < 0))
		return NULL;

	return m_vOutputPins[n];
}

STDMETHODIMP RtspSourceFilter::FindPin( LPCWSTR Id, IPin **ppPin )
{
	//Find pin according to it's name
	CheckPointer(ppPin,E_POINTER);
	ValidateReadWritePtr(ppPin,sizeof(IPin *));
	//Todo: Modify for this MUX: Check what ids this method is called with

	const char* szInput = StringUtil::wideToStl(Id).c_str();

	char szTemp[10];
	ZeroMemory(szTemp, 10);
	memcpy(szTemp, szInput, 6);
	szTemp[6] = '\0';
	if (0==strcmp(szTemp, "Output")) 
	{
		const char* szID = szInput + 7;
		int nId = atoi(szID);
		*ppPin = GetPin(nId);
	} 
	else 
	{
		*ppPin = NULL;
		return VFW_E_NOT_FOUND;
	}

	HRESULT hr = NOERROR;
	//  AddRef() returned pointer - but GetPin could fail if memory is low.
	if (*ppPin) 
	{
		(*ppPin)->AddRef();
	} 
	else 
	{
		hr = E_OUTOFMEMORY;  // probably.  There's no pin anyway.
	}
	return hr;
}

STDMETHODIMP RtspSourceFilter::Stop()
{
	CAutoLock cAutoLock(&m_stateLock);
	// Update streaming state
	m_bStreaming = false;
    m_bMediaSessionSetupComplete = false;
	// This will cause the liveMedia loop to exit
	m_pRtspSession->setWatchVariable();

	// Wait for the liveMedia eventloop to finish
	DWORD result = WaitForSingleObject(m_hLiveMediaStopEvent, INFINITE);
	return CSource::Stop();
}

STDMETHODIMP RtspSourceFilter::Pause()
{
	CAutoLock cAutoLock(&m_stateLock);
	StartRtspServerThreadIfNotStarted();
    // TODO: could WaitForSingleObject here to see if Rtsp was successful?

	return CSource::Pause();
}

/// Forward declaration for thread method
static DWORD WINAPI LiveRTSPServerThreadFunc(LPVOID pvParam);

void RtspSourceFilter::StartRtspServerThreadIfNotStarted()
{
	CAutoLock cAutoLock(&m_stateLock);
	if (!m_bStreaming)
	{
		m_bStreaming = true;
		m_rtpPacketManager.clear();
		m_rtpPacketManager.eof(false);
		//m_pRtspSession->resetWatchVariable();
		m_pRtspSession->reset();

		if (m_hLiveMediaStopEvent == NULL)
		{
			// Create Live Media Event loop handle - this will be used to notify the main thread that the live Media RTSP thread has finished
			m_hLiveMediaStopEvent = CreateEvent(
			NULL,               // default security attributes
			FALSE,              // auto-reset event
			FALSE,              // initial state is nonsignaled
			TEXT("LiveMediaEventLoop")  // object name
			);
		}
		else
		{
			ResetEvent(m_hLiveMediaStopEvent);
		}

		DWORD dwThreadID = 0;
		// Create a new thread for the RTSP liveMedia event loop
		HANDLE hThread = CreateThread(0, 0, LiveRTSPServerThreadFunc, (void*)this, 0, &dwThreadID);
	}
}

static DWORD WINAPI LiveRTSPServerThreadFunc(LPVOID pvParam)
{
	RtspSourceFilter* pSourceFilter = (RtspSourceFilter*)pvParam;
	pSourceFilter->StartRtspSession();
	return S_OK;
}

void RtspSourceFilter::StartRtspSession()
{
	//ASSERT(m_pRtspSession == NULL);
	// Encapsulating RTSP code into RtspClientSession class 
	// Create RTSP Client Session object
    bool bSetupComplete = true;
	if (!m_bMediaSessionSetupComplete)
    {
        bSetupComplete = m_pRtspSession->setupMediaSession(m_sUrl);
        m_bMediaSessionSetupComplete = true;
    }

    if (!bSetupComplete)
    {
        // TODO: Need to notify main thread of failure
    }

	m_pRtspSession->streamUsingTCP(m_bStreamUsingTCP);
	// Start the liveMedia thread: this method does not return until the liveMedia watch variable is set in the STOP method
	if (!m_pRtspSession->playMediaSession())
    {
        // Signal error?
        SetLastError(m_pRtspSession->getLastError(), true);
        // Could notify main thread here to either continue or abort Pause operation
    }
    // Shutdown media session
    m_pRtspSession->teardownMediaSession();

	for (int i = 0; i < m_vOutputPins.size(); ++i)
	{
		m_vOutputPins[i]->Reset();
	}
	// The Stop method waits for this event
	SetEvent(m_hLiveMediaStopEvent);
}

STDMETHODIMP RtspSourceFilter::GetState( DWORD dwMilliSecsTimeout, FILTER_STATE *pState )
{
	// Edit: 24/09/2008
	// From http://msdn.microsoft.com/en-us/library/ms787518(VS.85).aspx
	CheckPointer(pState, E_POINTER);
	*pState = m_State;
	if (m_State == State_Paused)
		return VFW_S_CANT_CUE;
	else
		return S_OK;
}

void RtspSourceFilter::notifyFilterAboutOffset( double dOffset)
{
	CAutoLock cAutoLock(&m_stateLock);

	// Get the current stream time to make sure that we always generate samples that have a starting time in the future
	// Otherwise the samples might be late by the time they reach the renderer

	// Add stream time + 50ms as further offset to make sure sample is rendered at the correct time
 	// Get the current stream time
 	CRefTime streamTime;
 	StreamTime(streamTime);
 	// Add a tiny offset to put us into the future
 	m_tStreamTimeOffset = streamTime.GetUnits() + 500000;
 	// Convert to double
 	m_dStreamTimeOffset = m_tStreamTimeOffset / 10000000.0;

	//TOREVISE: add the streamtime offset here: it hasn't been added yet!!!
	// Iterate over all pins and set the offset
	for (int i = 0; i < m_vOutputPins.size(); i++)
	{
		m_vOutputPins[i]->setOffset(dOffset);
	}
}
