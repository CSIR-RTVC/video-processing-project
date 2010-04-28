/** @file

MODULE				: RtspSourceFilter

FILE NAME			: RtspClientSessionManager.cpp

DESCRIPTION			: RTSP client session manager - encapsulates liveMedia functionality
					  
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
#include "RtspClientSessionManager.h"

// RTVC
#include "RtvcRtpSink.h"

RtspClientSessionManager::RtspClientSessionManager(MediaPacketManager& rMediaPacketManager)
  :m_rMediaPacketManager(rMediaPacketManager),
  m_bStreamUsingTcp(true),
  m_pScheduler(NULL),
  m_pEnv(NULL),
  m_pRtspClient(NULL),
  m_pSession(NULL),
  m_watchVariable(0)
{
}

RtspClientSessionManager::~RtspClientSessionManager()
{
}

bool RtspClientSessionManager::createMediaSession( const std::string& sUrl )
{
  // Can only be called once!
  assert(m_pRtspClient == NULL && m_pSession == NULL);

  const char* szUrl = sUrl.c_str();

  m_pRtspClient = RTSPClient::createNew(*m_pEnv);
  int nTimeOut = -1;

  // Get options response
  char* optionsResponse = m_pRtspClient->sendOptionsCmd(szUrl, NULL, NULL, NULL, nTimeOut);
  *m_pEnv << "RTSP Options response: " << optionsResponse << "\r\n";
  delete[] optionsResponse;

  // Get SDP description
  char* sdpDescription = m_pRtspClient->describeURL(szUrl, NULL, false, nTimeOut);
  if (sdpDescription == NULL) 
  {
    m_sLastError = "Failed to get a SDP description from URL \"" + sUrl + "\": " + std::string(m_pEnv->getResultMsg());
    return false;
  }

  *m_pEnv << "Opened URL \"" << szUrl << "\", returning a SDP description:\n" << sdpDescription << "\n";
  unsigned statusCode = m_pRtspClient->describeStatus();
  *m_pEnv << "Status code: " << statusCode << "\n";

  // Create a media session object from this SDP description:
  m_pSession = MediaSession::createNew(*m_pEnv, sdpDescription);
  delete[] sdpDescription;
  if (!m_pSession) 
  {
    m_sLastError = "Failed to create a MediaSession object from the SDP description: " + std::string(m_pEnv->getResultMsg());
    return false;
  } 
  else if (!m_pSession->hasSubsessions()) 
  {
    m_sLastError = "This session has no media subsessions (i.e., \"m=\" lines)";
    return false;
  }
  
  return true;
}

bool RtspClientSessionManager::setupMediaSession()
{
  // Setup RTP sources
  MediaSubsessionIterator iter(*m_pSession);
  MediaSubsession *pSubsession;
  while ((pSubsession = iter.next()) != NULL)
  {
    if (!createRtpSource(pSubsession))
    {
      return false;
    }
  }

  // Setup session
  iter.reset();
  while ((pSubsession = iter.next()) != NULL) 
  {
    if (pSubsession->clientPortNum() == 0)
    {
      std::ostringstream ostr;
      ostr << "Invalid port number for SETUP \"" << pSubsession->mediumName() << "/" << pSubsession->codecName() << "\" subsession: " << m_pEnv->getResultMsg();
      m_sLastError = ostr.str();
      *m_pEnv << m_sLastError.c_str() << "\n";
      return false; // port # was not set
    }

    if (!m_pRtspClient->setupMediaSubsession(*pSubsession, False, m_bStreamUsingTcp)) 
    {
      std::ostringstream ostr;
      ostr << "Failed to setup \"" << pSubsession->mediumName() << "/" << pSubsession->codecName() << "\" subsession: " << m_pEnv->getResultMsg();
      m_sLastError = ostr.str();
      *m_pEnv << m_sLastError.c_str() << "\n";
      return false;
    } 
    else 
    {
      *m_pEnv << "Setup \"" << pSubsession->mediumName() << "/" << pSubsession->codecName()  << "\" subsession (client ports " << pSubsession->clientPortNum()<< "-" << pSubsession->clientPortNum()+1 << ")\n";
    }
  }

  // Create receivers
  unsigned uiSinkID = 0;
  iter.reset();
  while ((pSubsession = iter.next()) != NULL) 
  {
    // Create our RTPSinks that will receive the data
    if (pSubsession->readSource() == NULL) continue;// was not initiated

    // Check if the we support the subsession media type
    if ( !m_rMediaPacketManager.initialiseChannelFromMediaSubsession(uiSinkID, pSubsession) )
    {
      std::ostringstream ostr;
      ostr << "Failed to configure media packet queue for type \"" << pSubsession->mediumName() << "/" << pSubsession->codecName() << "\"";
      m_sLastError = ostr.str();
      return false;
    }

    unsigned uiSinkBufferSize = 20000;
    // Link the Rtp sink to the ID of the queue in the packet manager
    RtvcRtpSink* pSink = new RtvcRtpSink(m_pSession->envir(), uiSinkID, *this, pSubsession, uiSinkBufferSize);
    pSubsession->sink = pSink;

    // Set after playing pointers
    pSubsession->sink->startPlaying(*(pSubsession->readSource()), &RtvcRtpSink::endSubsession, pSink);

    // Also set a handler to be called if a RTCP "BYE" arrives
    // for this subsession:
    if (pSubsession->rtcpInstance() != NULL) 
    {
      pSubsession->rtcpInstance()->setByeHandler(&RtspClientSessionManager::subsessionByeHandler, this);
    }
    ++uiSinkID;
  }

  return true;
}

bool RtspClientSessionManager::createRtpSource( MediaSubsession* pSubsession )
{
  if (!pSubsession->initiate()) 
  {
    std::ostringstream ostr;
    ostr << "Unable to create receiver for \"" << pSubsession->mediumName() << "/" << pSubsession->codecName() << "\" subsession: " << m_pEnv->getResultMsg(); 
    m_sLastError = ostr.str();
    *m_pEnv << m_sLastError.c_str() << "\n";
    return false;
  } 
  else 
  {
    *m_pEnv << "Created receiver for \"" << pSubsession->mediumName() << "/" << pSubsession->codecName() << "\" subsession (client ports " << pSubsession->clientPortNum() << "-" << pSubsession->clientPortNum()+1 << ")\n";
    assert(pSubsession->rtpSource());
    //TOREVISE
    // Because we're saving the incoming data, rather than playing
    // it in real time, allow an especially large time threshold
    // (1 second) for reordering misordered incoming packets:
    unsigned const thresh = 1000000; // 1 second 
    pSubsession->rtpSource()->setPacketReorderingThresholdTime(thresh);

    int socketInputBufferSize = 0;
    if (socketInputBufferSize > 0) 
    {
      // Set the RTP source's input buffer size as specified:
      int socketNum = pSubsession->rtpSource()->RTPgs()->socketNum();
      unsigned curBufferSize = getReceiveBufferSize(*m_pEnv, socketNum);
      unsigned newBufferSize = setReceiveBufferTo(*m_pEnv, socketNum, socketInputBufferSize);
      *m_pEnv << "Changed socket receive buffer size for the \"" << pSubsession->mediumName() << "/" << pSubsession->codecName() << "\" subsession from " << curBufferSize << " to " << newBufferSize << " bytes\n";
    }
    return true;
  }
}

void RtspClientSessionManager::teardownMediaSession()
{
  // Close our output files:
  closeMediaSinks();
  // Teardown, then shutdown, any outstanding RTP/RTCP subsessions
  tearDownStreams();
  Medium::close(m_pSession);
  m_pSession = NULL;
  // Finally, shut down our client:
  Medium::close(m_pRtspClient);
  m_pRtspClient = NULL;
}

void RtspClientSessionManager::subsessionAfterPlaying( RtvcRtpSink* pSink )
{
  // Clean up subsession
  MediaSubsession* pSubsession = pSink->getSubsession();
  Medium::close(pSubsession->sink);
  pSubsession->sink = NULL;

  // Next, check whether *all* subsessions' streams have now been closed:
  MediaSession& session = pSubsession->parentSession();
  MediaSubsessionIterator iter(session);
  while ((pSubsession = iter.next()) != NULL) 
  {
    if (pSubsession->sink != NULL) return; // this subsession is still active
  }

  // All subsessions' streams have now been closed
  sessionAfterPlaying();
}

void RtspClientSessionManager::sessionAfterPlaying()
{
  endLiveMediaEventLoop();
}

void RtspClientSessionManager::closeMediaSinks()
{
  if (m_pSession == NULL) return;
  MediaSubsessionIterator iter(*m_pSession);
  MediaSubsession* subsession;
  while ((subsession = iter.next()) != NULL) {
    Medium::close(subsession->sink);
    subsession->sink = NULL;
  }
}

void RtspClientSessionManager::tearDownStreams()
{
  if (m_pRtspClient == NULL || m_pSession == NULL) return;
  m_pRtspClient->teardownMediaSession(*m_pSession);
}

bool RtspClientSessionManager::playMediaSession()
{
  // Play streams
  if (!m_pRtspClient->playMediaSession(*m_pSession)) 
  {
    m_sLastError =  "Failed to start playing session: " +  std::string(m_pEnv->getResultMsg());
    return false;
  }
  return true;
}

void RtspClientSessionManager::endLiveMediaEventLoop()
{
  setWatchVariable();
}

void RtspClientSessionManager::startRtspSession(const std::string& sUrl)
{
  resetWatchVariable();
  const char* szUrl = sUrl.c_str();

  // Setup environment
  m_pScheduler = BasicTaskScheduler::createNew();
  m_pEnv = BasicUsageEnvironment::createNew(*m_pScheduler);

  bool bSuccess = true;

  do
  {
    if (!createMediaSession(sUrl))
      break;

    assert( m_pRtspClient && m_pSession );

    if (!setupMediaSession())
    {
      break;
    }
    if (!playMediaSession())
    {
      break;
    }

    m_pEnv->taskScheduler().doEventLoop(&m_watchVariable); // does not return	until watch variable is set
    break;

  }while (true);

  // Cleanup RTSP Media session

  teardownMediaSession();

  if (m_pScheduler)
    delete m_pScheduler;
  m_pScheduler = NULL;

  if (m_pEnv)
    m_pEnv->reclaim(); // Deletes itself
  m_pEnv = NULL;

  resetWatchVariable();
}

bool RtspClientSessionManager::createReceivers(MediaSession* pSession)
{
  // Create and start media sinks for each subsession:
  bool madeProgress = False;
  MediaSubsessionIterator iter(*m_pSession);
  MediaSubsession *pSubsession;
  int uiSinkID = 0;
  while ((pSubsession = iter.next()) != NULL) 
  {
    // Create our RTPSinks that will receive the data
    if (pSubsession->readSource() == NULL) continue;// was not initiated

    int nSinkBufferSize = 20000;

    // Check if the we support the subsession media type
    if ( !m_rMediaPacketManager.initialiseChannelFromMediaSubsession(uiSinkID, pSubsession) )
    {
      std::ostringstream ostr;
      ostr << "Failed to configure media packet queue for type \"" << pSubsession->mediumName() << "/" << pSubsession->codecName() << "\"";
      m_sLastError = ostr.str();
      return false;
    }

    unsigned uiSinkBufferSize = 20000;
    // Link the Rtp sink to the ID of the queue in the packet manager
    RtvcRtpSink* pSink = new RtvcRtpSink(m_pSession->envir(), uiSinkID, *this, pSubsession, uiSinkBufferSize);
    pSubsession->sink = pSink;

    // Set after playing pointers
    pSubsession->sink->startPlaying(*(pSubsession->readSource()), &RtvcRtpSink::endSubsession, pSink);

    // Also set a handler to be called if a RTCP "BYE" arrives
    // for this subsession:
    if (pSubsession->rtcpInstance() != NULL) 
    {
      pSubsession->rtcpInstance()->setByeHandler(&RtspClientSessionManager::subsessionByeHandler, this);
    }
    madeProgress = True;
    ++uiSinkID;
  }
  return madeProgress;
}
