/** @file

MODULE				: RtspSession

FILE NAME			: RtspDataSession.cpp

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
#include "RtspDataSession.h"

// LiveMedia
#include "BasicUsageEnvironment.hh"

// RTVC
#include "RtspHelper.h"
#include "RtvcRtpSink.h"
#include "RtpPacketManager.h"

RtspSession::RtspSession(RtpPacketManager* pRtpPacketManager)
	:m_pScheduler(NULL),
	m_pEnv(NULL),
	m_pRtpPacketManager(pRtpPacketManager),
	m_pRtspClient(NULL),
	m_pSession(NULL),
	m_bStreamUsingTCP(true),
	m_watchVariable(0),
	m_sLastError("")
{
    // Setup environment
    m_pScheduler = BasicTaskScheduler::createNew();
    m_pEnv = BasicUsageEnvironment::createNew(*m_pScheduler);
}

RtspSession::~RtspSession()
{
    // Cleanup RTSP Media session
    if (m_pRtspClient || m_pSession)
		RtspHelper::shutdownMediaSession(m_pRtspClient, m_pSession);

    m_pEnv->reclaim();
    delete m_pScheduler;
}

bool RtspSession::playMediaSession( int nTimeOut )
{
	if (!m_pSession) return false;

    return playStreams(m_pEnv, m_pSession);
}

void RtspSession::teardownMediaSession()
{
	if (m_pRtspClient || m_pSession)
	// Cleanup RTSP Media session
		RtspHelper::shutdownMediaSession(m_pRtspClient, m_pSession);
	m_pRtspClient = NULL;
	m_pSession = NULL;
}

bool RtspSession::playStreams(UsageEnvironment* pEnv, MediaSession* pSession)
{
	if (!pSession || !m_pRtspClient) return false;

	// Setup and play the streams 
	if (!RtspHelper::createRtpSources(pEnv, pSession))
	{
		m_sLastError = "Failed to create RTP sources";
		return false;
	}

	// Setup RTP source
	if (!RtspHelper::setupStreams(m_pRtspClient, pSession, m_bStreamUsingTCP))
	{
		m_sLastError = "Failed to setup RTP streams";
		return false;
	}

	if (!createReceivers(pSession))
	{
		m_sLastError =  "Failed to create RTP receivers: " +  std::string(pEnv->getResultMsg());
		return false;
	}

	// Play streams
	if (!m_pRtspClient->playMediaSession(*pSession)) 
	{
		m_sLastError =  "Failed to start playing session: " +  std::string(pEnv->getResultMsg());
		return false;
	}

	// Start the liveMedia eventloop
	pEnv->taskScheduler().doEventLoop(&m_watchVariable); // does not return	until watch variable is set
	return true;
}

bool RtspSession::createReceivers(MediaSession* pSession)
{
	// Create and start media sinks for each subsession:
	bool madeProgress = False;
	MediaSubsessionIterator iter(*m_pSession);
	MediaSubsession *pSubsession;
	int nSinkID = -1;
	while ((pSubsession = iter.next()) != NULL) 
	{
		++nSinkID;

		// Create our RTPSinks that will receive the data
		if (pSubsession->readSource() == NULL) continue;// was not initiated

		int nSinkBufferSize = 20000;

		// Create a new queue in the packet manager
		m_pRtpPacketManager->createQueue(nSinkID);
		// Link the Rtp sink to the ID of the queue in the packet manager
		RtvcRtpSink<RtpPacketManager>* pSink = new RtvcRtpSink<RtpPacketManager>(pSession->envir(), nSinkID, nSinkBufferSize, m_pRtpPacketManager);
		pSubsession->sink = pSink;

		// TODO Set after playing pointers
		pSubsession->sink->startPlaying(*(pSubsession->readSource()), NULL, NULL);

		// Also set a handler to be called if a RTCP "BYE" arrives
		// for this subsession:
		if (pSubsession->rtcpInstance() != NULL) 
		{
			pSubsession->rtcpInstance()->setByeHandler(&RtspSession::subsessionByeHandler, this/*subsession*/);
		}
		madeProgress = True;
	}
	return madeProgress;
}

bool RtspSession::setupMediaSession( const std::string& sUrl )
{
	// Create client
	if (!m_pRtspClient)
		m_pRtspClient = RTSPClient::createNew(*m_pEnv);

    return RtspHelper::createMediaSession(sUrl, m_pRtspClient, &m_pSession, m_sLastError);
}

void RtspSession::endSession()
{
	m_pRtpPacketManager->eof(true);
	setWatchVariable();
}