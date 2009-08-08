/** @file

MODULE				: RtspSnifferSession

FILE NAME			: RtspSnifferSession.cpp

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
#include "RtspSnifferSession.h"

// LiveMedia
#include "BasicUsageEnvironment.hh"

// RTVC
#include "RtspHelper.h"
#include "RtvcRtpSink.h"

RtspSnifferSession::RtspSnifferSession()
	:m_pRtspClient(NULL),
	m_pSession(NULL),
	m_bStreamUsingTCP(true),
	m_watchVariable(0),
	m_sLastError("")
{;}

RtspSnifferSession::~RtspSnifferSession()
{
	if (m_pRtspClient && m_pSession)
	// Cleanup RTSP Media session
	RtspHelper::shutdownMediaSession(m_pRtspClient, m_pSession);
}

bool RtspSnifferSession::getMediaSessionInformation(const std::string sUrl, int nTimeOut)
{
	const char* szUrl = sUrl.c_str();

	// Setup environment
	TaskScheduler* pScheduler = BasicTaskScheduler::createNew();
	UsageEnvironment* pEnv = BasicUsageEnvironment::createNew(*pScheduler);

	bool bSuccess = true;

	do
	{
		// Create client
		m_pRtspClient = RTSPClient::createNew(*pEnv);

		if (!RtspHelper::createMediaSession(sUrl, m_pRtspClient, &m_pSession, m_sLastError))
		{
			bSuccess = false;
			break;
		}
		
		// Iterate over session and add subsessions to vectors
		MediaSubsession* pSubsession = NULL;
		MediaSubsessionIterator it(*m_pSession);
		while ((pSubsession = it.next()) != NULL)
		{
			m_vSubsessions.push_back(pSubsession);
		}
		// Create our mappings
		/*if (!mapLiveMediaToRtvc(m_pSession))
		{
			bSuccess = false;
			break;
		}*/

		//bSuccess = playStreamsIfNecessary(pEnv, m_pSession);
		break;
	}while (true);
	return bSuccess;
}

void RtspSnifferSession::shutdown()
{
	// Cleanup RTSP Media session
	RtspHelper::shutdownMediaSession(m_pRtspClient, m_pSession);
}

//bool RtspSnifferSession::playStreamsIfNecessary(UsageEnvironment* pEnv, MediaSession* pSession)
//{
//	if (needToPlayStreams(pSession))
//	{
//		// Setup and play the streams 
//		if (!RtspHelper::createRtpSources(pEnv, pSession))
//		{
//			m_sLastError = "Failed to create RTP sources";
//			return false;
//		}
//
//		// Setup RTP source
//		if (!RtspHelper::setupStreams(m_pRtspClient, pSession, m_bStreamUsingTCP))
//		{
//			m_sLastError = "Failed to setup RTP streams";
//			return false;
//		}
//
//		if (!createReceivers(pSession))
//		{
//			m_sLastError =  "Failed to create RTP receivers: " +  std::string(pEnv->getResultMsg());
//			return false;
//		}
//
//		// Play streams
//		if (!m_pRtspClient->playMediaSession(*pSession)) 
//		{
//			m_sLastError =  "Failed to start playing session: " +  std::string(pEnv->getResultMsg());
//			return false;
//		}
//
//		// Start the liveMedia eventloop
//		pEnv->taskScheduler().doEventLoop(&m_watchVariable); // does not return	until watch variable is set
//	}
//	return true;
//}

//bool RtspSnifferSession::createReceivers(MediaSession* pSession)
//{
//	// Create and start media sinks for each subsession:
//	bool madeProgress = False;
//	MediaSubsessionIterator iter(*m_pSession);
//	MediaSubsession *pSubsession;
//	
//	while ((pSubsession = iter.next()) != NULL) 
//	{
//		// Create our RTPSinks that will receive the data
//		if (pSubsession->readSource() == NULL) continue;// was not initiated
//
//		if ( strcmp(pSubsession->mediumName(), "video") == 0 && 
//			 ((strcmp(pSubsession->codecName(), "H263-1998") == 0)||
//			  (strcmp(pSubsession->codecName(), "H263-2000") == 0))
//			)
//		{
//			int nSinkBufferSize = 20000;
//			// Create our H263Sniffer
//			m_pFormatSniffer = new H263FormatSniffer(&(m_subsessionMap[pSubsession]), m_watchVariable);
//			RtvcRtpSink<H263FormatSniffer>* pSink = new RtvcRtpSink<H263FormatSniffer>(
//					pSession->envir(), 
//					0 /*Not interested in source id in sniffing session*/, 
//					nSinkBufferSize, 
//					m_pFormatSniffer
//					);
//			pSubsession->sink = pSink;
//		}
//		else
//		{
//			// Don't create any sinks since there's no data to sniff
//			continue;
//		}
//
//		// TODO Set after playing pointers
//		pSubsession->sink->startPlaying(*(pSubsession->readSource()), NULL/*subsessionAfterPlaying*/, NULL/*subsession*/);
//
//		// Also set a handler to be called if a RTCP "BYE" arrives
//		// for this subsession:
//		if (pSubsession->rtcpInstance() != NULL) 
//		{
//			pSubsession->rtcpInstance()->setByeHandler(&RtspSnifferSession::subsessionByeHandler, this/*subsession*/);
//		}
//		madeProgress = True;
//	}
//	return madeProgress;
//}

//bool RtspSnifferSession::mapLiveMediaToRtvc(MediaSession* pSession)
//{
//	// Check if all subsession could get mapped
//	return ((nLiveMediaSubsessionCount == m_subsessionMap.size())? true : false);
//}


bool RtspSnifferSession::needToPlayStreams(MediaSession* pSession)
{
	MediaSubsessionIterator iter(*pSession);
	MediaSubsession *pSubsession = NULL;
	bool bFailed = false;
	while ((pSubsession = iter.next()) != NULL && !bFailed) 
	{
		const char* szMedium = pSubsession->mediumName();
		const char* szCodec = pSubsession->codecName();
		if ((strcmp(szMedium, "video")==0) && 
			((strcmp(szCodec, "H263-1998")==0)||(strcmp(szCodec, "H263-2000")==0))
			)
		{
			return true;
		}
	}
	return false;
}
