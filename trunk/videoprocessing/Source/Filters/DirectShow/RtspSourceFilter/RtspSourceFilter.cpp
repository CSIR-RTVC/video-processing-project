/** @file

MODULE				: RtspSourceFilter

FILE NAME			: RtspSourceFilter.cpp

DESCRIPTION			: DirectShow RTSP source filter
					  
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
#include "stdafx.h"

// RTVC
#include "RtspSourceFilter.h"
#include "RtspSourceOutputPin.h"

#include <Shared/StringUtil.h>

const unsigned TIMEOUT_MILLISECONDS = 3000;

RtspSourceFilter::RtspSourceFilter( IUnknown* pUnk, HRESULT* phr )
	: CSource(NAME("RTVC VPP RTSP Source Filter"), pUnk, CLSID_RTVC_RtspOsSourceFilter),
	m_bStreamUsingTCP(false),
  m_dInitialOffset(-1.0),
  m_dInitialStreamTimeOffset(0.0),
  m_dSynchronisedOffset(-1.0),
  m_dSynchronisedStreamTimeOffset(0.0),
  m_hLiveMediaStopEvent(NULL),
  m_hLiveMediaThreadHandle(NULL),
  m_dwThreadID(0),
  m_rtspSessionManager(m_mediaPacketManager)
{
	// Init CSettingsInterface
	initParameters();
	
  // Create Live Media Event loop handle - this will be used to notify the main thread that the live Media RTSP thread has finished
  m_hLiveMediaStopEvent = CreateEvent(
    NULL,                       // default security attributes
    FALSE,                      // auto-reset event
    FALSE,                      // initial state is nonsignaled
    TEXT("LiveMediaStopEvent")  // object name
    );
}

RtspSourceFilter::~RtspSourceFilter(void)
{
  if (m_hLiveMediaThreadHandle)
  {
    stopLiveMediaSession();

    CloseHandle(m_hLiveMediaThreadHandle);
    m_hLiveMediaThreadHandle = NULL;
  }

  CloseHandle(m_hLiveMediaStopEvent);
  m_hLiveMediaStopEvent = NULL;

  for (int i = 0; i < m_vOutputPins.size();i++)
  {
    delete m_vOutputPins[i];
    m_vOutputPins[i] = NULL;
  }
}

CUnknown *WINAPI RtspSourceFilter::CreateInstance( IUnknown* pUnk, HRESULT* phr )
{
	RtspSourceFilter* pSource = new RtspSourceFilter(pUnk, phr);
	if (!pSource)
	{
		*phr = E_OUTOFMEMORY;
	}
	return pSource;
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

static DWORD WINAPI LiveMediaThread(LPVOID pvParam)
{
  RtspSourceFilter* pSourceFilter = (RtspSourceFilter*)pvParam;
  pSourceFilter->startRtspSession();
  return S_OK;
}

void RtspSourceFilter::startRtspSession()
{
  ResetEvent(m_hLiveMediaStopEvent);
  m_mediaPacketManager.reset();
  // Blocks until end of liveMedia eventloop
  m_rtspSessionManager.startRtspSession(m_sUrl);
  // Notify media packet manager of eof
  m_mediaPacketManager.stop();
  // The Stop method waits for this event
  SetEvent(m_hLiveMediaStopEvent);
}

STDMETHODIMP RtspSourceFilter::Load( LPCOLESTR lpwszFileName, const AM_MEDIA_TYPE *pmt )
{
  m_mediaPacketManager.resetTypeInfoCompleteEventHandle();

  // Store the URL
  m_sUrl = StringUtil::wideToStl(lpwszFileName);
  m_rtspSessionManager.setStreamUsingTcp(m_bStreamUsingTCP);

  // Create a new thread for the RTSP liveMedia event loop
  m_hLiveMediaThreadHandle = CreateThread(0, 0, LiveMediaThread, (void*)this, 0, &m_dwThreadID);
#if 1
  DWORD dwResult = WaitForSingleObject(m_mediaPacketManager.getTypeInfoCompleteEventHandle(), TIMEOUT_MILLISECONDS);
#else
  // FOR DEBUGGING AMR
  DWORD dwResult = WaitForSingleObject(m_mediaPacketManager.getTypeInfoCompleteEventHandle(), INFINITE);
#endif

  if (dwResult == WAIT_OBJECT_0)
  {
    HRESULT hr = createOutputPins();
    if (FAILED(hr))
    {
      // stop livemedia event loop
      m_rtspSessionManager.endLiveMediaEventLoop();
    }
    return hr;
  }
  else
  {
    // stop livemedia event loop
    m_rtspSessionManager.endLiveMediaEventLoop();
    std::ostringstream ostr;
    ostr << "Failed to obtain complete media type info" << m_rtspSessionManager.getLastError();
    SetLastError(ostr.str().c_str(), true);
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
				if (hr == E_UNEXPECTED)
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

STDMETHODIMP RtspSourceFilter::createOutputPins()
{
  HRESULT hr = S_OK;
  // Proceed with building pipeline
  MediaSubsession* pSubsession = NULL;
  MediaSubsessionIterator it(*m_rtspSessionManager.getMediaSession());

  while ((pSubsession = it.next()) != NULL)
  {
    StringMap_t& rParams = m_mediaPacketManager.getParameterMap(pSubsession);
    createOutputPin(pSubsession, rParams, &hr);
    if (FAILED(hr))
    {
      return hr;
    }
  }
  return hr;
}

void RtspSourceFilter::createOutputPin(MediaSubsession *pSubsession, const StringMap_t& rParams, HRESULT* pHr)
{
	// Create the output pin using the size of the output pin vector as an ID
	// This ID will be used by the pins buffer processing loop to query the packet manager for packets
	RtspSourceOutputPin* pPin = new RtspSourceOutputPin(pHr, this, pSubsession, rParams, m_vOutputPins.size());
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
  stopLiveMediaSession();
  // Notify media packet manager of eof: happend at the end of the media session
  m_mediaPacketManager.stop();
  resetOffsets();
  return CSource::Stop();
}

STDMETHODIMP RtspSourceFilter::Pause()
{
  CAutoLock cAutoLock(&m_stateLock);
  if ( m_hLiveMediaThreadHandle == NULL )
  {
    m_rtspSessionManager.setStreamUsingTcp(m_bStreamUsingTCP);
    // Create a new thread for the RTSP liveMedia event loop
    m_hLiveMediaThreadHandle = CreateThread(0, 0, LiveMediaThread, (void*)this, 0, &m_dwThreadID);
  }
  return CSource::Pause();
}

STDMETHODIMP RtspSourceFilter::GetState( DWORD dwMilliSecsTimeout, FILTER_STATE *pState )
{
  // Edit: 24/09/2008
  // From http://msdn.microsoft.com/en-us/library/ms787518(VS.85).aspx
  CheckPointer(pState, E_POINTER);
  *pState = m_State;
  if (m_State == State_Paused)
  {
    if (!m_mediaPacketManager.isBufferingComplete())
    {
      // try and emit buffering signal: not sure what effect this will have
      if (m_pMediaEventSink)
        m_pMediaEventSink->Notify(EC_BUFFERING_DATA, TRUE, 0);
      return VFW_S_STATE_INTERMEDIATE;
    }
    else
    {
      if (m_pMediaEventSink)
        m_pMediaEventSink->Notify(EC_BUFFERING_DATA, FALSE, 0);
      m_mediaPacketManager.setReady(true);
      return VFW_S_CANT_CUE;
    }
  }
  else
    return S_OK;
}

void RtspSourceFilter::stopLiveMediaSession()
{
  m_rtspSessionManager.endLiveMediaEventLoop();
  // Wait for the liveMedia eventloop to finish
  DWORD result = WaitForSingleObject(m_hLiveMediaStopEvent, INFINITE);
  CloseHandle(m_hLiveMediaThreadHandle);
  m_hLiveMediaThreadHandle = NULL;
}

void RtspSourceFilter::setInitialOffset( double dOffset )
{
  CAutoLock cAutoLock(&m_stateLock);
  if (m_dInitialOffset == -1.0)
  {
    m_dInitialOffset = dOffset;
    // Get the current stream time to make sure that we always generate samples that have a starting time in the future
    // Otherwise the samples might be late by the time they reach the renderer

    // Add stream time + 50ms as further offset to make sure sample is rendered at the correct time
    // Get the current stream time
    CRefTime streamTime;
    StreamTime(streamTime);
    // Add a tiny offset to put us into the future
    REFERENCE_TIME tStreamTimeOffset = streamTime.GetUnits() + 500000;
    // Convert to double
    m_dInitialStreamTimeOffset = tStreamTimeOffset / 10000000.0;
  }
}

void RtspSourceFilter::setSynchronisedOffset( double dOffset )
{
  CAutoLock cAutoLock(&m_stateLock);
  if (m_dSynchronisedOffset == -1.0)
  {
    m_dSynchronisedOffset = dOffset;
    CRefTime streamTime;
    StreamTime(streamTime);
    // Add a tiny offset to put us into the future
    REFERENCE_TIME tStreamTimeOffset = streamTime.GetUnits() + 500000;
    // Convert to double
    m_dSynchronisedStreamTimeOffset = tStreamTimeOffset / 10000000.0;
  }
}