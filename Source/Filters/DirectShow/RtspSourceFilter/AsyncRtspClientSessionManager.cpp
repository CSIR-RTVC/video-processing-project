/** @file

MODULE				: RtspSourceFilter

FILE NAME			: AsyncRtspClientSessionManager.cpp

DESCRIPTION			: RTSP client session manager - encapsulates liveMedia functionality
					  
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
#include "AsyncRtspClientSessionManager.h"

// boost
#include <boost/bind.hpp>

// RTVC
#include "RtvcRtpSink.h"

// live555
#include <MPEG4LATMAudioRTPSource.hh>

AsyncRtspClientSessionManager::AsyncRtspClientSessionManager(MediaPacketManager& rMediaPacketManager)
  :m_eState(RTSP_INIT),
  m_rMediaPacketManager(rMediaPacketManager),
  m_bStreamUsingTcp(true),
  m_pScheduler(NULL),
  m_pEnv(NULL),
  m_pRtspClient(NULL),
  m_pSession(NULL),
  m_pSubsession(NULL),
  m_uiSubsessionCount(0),
  m_uiSuccessfulSubsessions(0),
  m_pSubsessionIter(NULL),
  m_watchVariable(0),
  m_iLastLiveMediaErrorCode(0)
{
}

AsyncRtspClientSessionManager::~AsyncRtspClientSessionManager()
{
}

void AsyncRtspClientSessionManager::createMediaSession( const std::string& sUrl )
{
  DbgLog((LOG_TRACE, 0, TEXT("AsyncRtspClientSessionManager::createMediaSession")));

  // Can only be called once!
  assert(m_pRtspClient == NULL && m_pSession == NULL);

  m_sUrl = sUrl;
  const char* szUrl = sUrl.c_str();

  if (!m_bStreamOverHttp)
    m_pRtspClient = AsyncRtspClientAdapter::createNew(*m_pEnv, sUrl.c_str());
  else
    m_pRtspClient = AsyncRtspClientAdapter::createNew(*m_pEnv, sUrl.c_str(), 0, 0, 80);

  m_pRtspClient->setOptionsCallback(boost::bind(&AsyncRtspClientSessionManager::continueAfterOPTIONS, this, _1, _2));
  m_pRtspClient->setDescribeCallback(boost::bind(&AsyncRtspClientSessionManager::continueAfterDESCRIBE, this, _1, _2));
  m_pRtspClient->setSetupCallback(boost::bind(&AsyncRtspClientSessionManager::continueAfterSETUP, this, _1, _2));
  m_pRtspClient->setPlayCallback(boost::bind(&AsyncRtspClientSessionManager::continueAfterPLAY, this, _1, _2));
  m_pRtspClient->setTeardownCallback(boost::bind(&AsyncRtspClientSessionManager::continueAfterTEARDOWN, this, _1, _2));

  DbgLog((LOG_TRACE, 0, TEXT("AsyncRtspClientSessionManager: Sending OPTIONS")));

  int nTimeOut = -1;
  m_pRtspClient->sendOptionsCommand(AsyncRtspClientAdapter::optionsHandler, NULL);

#if 0
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
  //unsigned statusCode = m_pRtspClient->describeStatus();
  //*m_pEnv << "Status code: " << statusCode << "\n";

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
#endif
}

void AsyncRtspClientSessionManager::setupMediaSession()
{
  DbgLog((LOG_TRACE, 0, TEXT("AsyncRtspClientSessionManager::setupMediaSession")));

  // Setup RTP sources
  MediaSubsessionIterator iter(*m_pSession);
  MediaSubsession *pSubsession;
  while ((pSubsession = iter.next()) != NULL)
  {
    if (!createRtpSource(pSubsession))
    {
      endLiveMediaEventLoop();
    }
    else
    {
      // count number of subsessions: used in setup handlers
      ++m_uiSubsessionCount;
    }
  }

  // Setup subsessions
  m_pSubsessionIter = new MediaSubsessionIterator(*m_pSession);
  pSubsession = m_pSubsessionIter->next();
  setupMediaSubsession(pSubsession);
}

bool AsyncRtspClientSessionManager::createRtpSource( MediaSubsession* pSubsession )
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

    // for AAC LATM we want to only the raw AAC data
    if (strcmp(pSubsession->codecName(), "MP4A-LATM") == 0) 
    { 
      // MPEG-4 LATM audio
      MPEG4LATMAudioRTPSource* pSource = (MPEG4LATMAudioRTPSource*)pSubsession->rtpSource();
      pSource->omitLATMDataLengthField();
    }

    return true;
  }
}

void AsyncRtspClientSessionManager::teardownMediaSession()
{
  // Close our output files:
  closeMediaSinks();
  // should onlyteardown streams if the setup was successful
  // Teardown, then shutdown, any outstanding RTP/RTCP subsessions
  if (m_eState != RTSP_INIT)
    tearDownStreams();
  Medium::close(m_pSession);
  m_pSession = NULL;
  // Finally, shut down our client:
  Medium::close(m_pRtspClient);
  m_pRtspClient = NULL;
}

void AsyncRtspClientSessionManager::subsessionAfterPlaying( RtvcRtpSink* pSink )
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

void AsyncRtspClientSessionManager::sessionAfterPlaying()
{
  endLiveMediaEventLoop();
}

void AsyncRtspClientSessionManager::closeMediaSinks()
{
  if (m_pSession == NULL) return;
  MediaSubsessionIterator iter(*m_pSession);
  MediaSubsession* subsession;
  while ((subsession = iter.next()) != NULL) {
    Medium::close(subsession->sink);
    subsession->sink = NULL;
  }
}

void AsyncRtspClientSessionManager::tearDownStreams()
{
  if (m_pRtspClient == NULL || m_pSession == NULL) return;
  m_pRtspClient->sendTeardownCommand(*m_pSession, AsyncRtspClientAdapter::teardownHandler);
}

void AsyncRtspClientSessionManager::endLiveMediaEventLoop()
{
  setWatchVariable();
}

void AsyncRtspClientSessionManager::startRtspSession(const std::string& sUrl)
{
  m_eState = RTSP_INIT;
  resetWatchVariable();
  const char* szUrl = sUrl.c_str();

  // Setup environment
  m_pScheduler = BasicTaskScheduler::createNew();
  m_pEnv = BasicUsageEnvironment::createNew(*m_pScheduler);

  createMediaSession(sUrl);

  m_pEnv->taskScheduler().doEventLoop(&m_watchVariable); // does not return	until watch variable is set

  if (m_eState == RTSP_INIT)
  {
    // signal end in case the async RTSP requests were unsuccessful: in that case the RtspSourceFilter::Load
    // is waiting for single object
    HANDLE& rHandle = m_rMediaPacketManager.getTypeInfoCompleteEventHandle();
    SetEvent(rHandle);
  }

  cleanupLiveMedia();
  m_eState = RTSP_INIT;

#if 0
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
#endif
}

bool AsyncRtspClientSessionManager::createReceivers(MediaSession* pSession)
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
      pSubsession->rtcpInstance()->setByeHandler(&AsyncRtspClientSessionManager::subsessionByeHandler, this);
    }
    madeProgress = True;
    ++uiSinkID;
  }
  return madeProgress;
}

void AsyncRtspClientSessionManager::continueAfterOPTIONS(int resultCode, char* resultString)
{
  DbgLog((LOG_TRACE, 0, TEXT("AsyncRtspClientSessionManager::continueAfterOPTIONS")));

  m_iLastLiveMediaErrorCode = resultCode;
  if (resultCode == 0)
  {
    // continue async operations  
    // Get SDP description
    DbgLog((LOG_TRACE, 0, TEXT("AsyncRtspClientSessionManager: Sending DESCRIBE")));
    m_pRtspClient->sendDescribeCommand(AsyncRtspClientAdapter::describeHandler, NULL);
  }
  else
  {
    // end of async chain
    m_sLastError = std::string(m_pEnv->getResultMsg());
    
    DbgLog((LOG_TRACE, 0, TEXT("AsyncRtspClientSessionManager: Error OPTIONS: %s"), m_sLastError.c_str()));
    endLiveMediaEventLoop();
  }
}

void AsyncRtspClientSessionManager::continueAfterDESCRIBE( int resultCode, char* sdpDescription )
{
  DbgLog((LOG_TRACE, 0, TEXT("AsyncRtspClientSessionManager::continueAfterDESCRIBE")));

  m_iLastLiveMediaErrorCode = resultCode;
  if (resultCode == 0)
  {
    DbgLog((LOG_TRACE, 0, TEXT("AsyncRtspClientSessionManager: DESCRIBE success")));
    if (sdpDescription == NULL)
    {
      m_sLastError = "Failed to get a SDP description from URL \"" + m_sUrl + "\": " + std::string(m_pEnv->getResultMsg());
      endLiveMediaEventLoop();
    }

    *m_pEnv << "Opened URL \"" << m_sUrl.c_str() << "\", returning a SDP description:\n" << sdpDescription << "\n";

    // continue async operations  
    // Create a media session object from this SDP description:
    m_pSession = MediaSession::createNew(*m_pEnv, sdpDescription);
    delete[] sdpDescription;
    if (!m_pSession) 
    {
      m_sLastError = "Failed to create a MediaSession object from the SDP description: " + std::string(m_pEnv->getResultMsg());
      endLiveMediaEventLoop();
    } 
    else if (!m_pSession->hasSubsessions()) 
    {
      m_sLastError = "This session has no media subsessions (i.e., \"m=\" lines)";
      endLiveMediaEventLoop();
    }

    setupMediaSession();
  }
  else 
  {
    // end of async chain
    m_sLastError = std::string(m_pEnv->getResultMsg());
    DbgLog((LOG_TRACE, 0, TEXT("AsyncRtspClientSessionManager: Error DESCRIBE: Code: %d: %s"), resultCode, m_sLastError.c_str()));
    endLiveMediaEventLoop();
  }
}

void AsyncRtspClientSessionManager::setupReceivers()
{
  // All subsessions have been setup: create receivers
  unsigned uiSinkID = 0;
  MediaSubsessionIterator iter(*m_pSession);
  MediaSubsession* pSubsession = NULL;
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
      endLiveMediaEventLoop();
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
      pSubsession->rtcpInstance()->setByeHandler(&AsyncRtspClientSessionManager::subsessionByeHandler, this);
    }
    ++uiSinkID;
  }
}

void AsyncRtspClientSessionManager::setupMediaSubsession( MediaSubsession* pSubsession )
{
  if (pSubsession)
  {
    if (pSubsession->clientPortNum() == 0)
    {
      std::ostringstream ostr;
      ostr << "Invalid port number for SETUP \"" << pSubsession->mediumName() << "/" << pSubsession->codecName() << "\" subsession: " << m_pEnv->getResultMsg();
      m_sLastError = ostr.str();
      *m_pEnv << m_sLastError.c_str() << "\n";
      endLiveMediaEventLoop();
    }

    m_pSubsession = pSubsession;
    m_pRtspClient->sendSetupCommand(*m_pSubsession, AsyncRtspClientAdapter::setupHandler, False, m_bStreamUsingTcp);
  }
  else
  {
    m_sLastError = "Invalid media subsession";  
    endLiveMediaEventLoop();
  }
}

void AsyncRtspClientSessionManager::continueAfterSETUP( int resultCode, char* resultString )
{
  m_iLastLiveMediaErrorCode = resultCode;

  if (resultCode == 0)
  {
    m_eState = RTSP_SETUP;
    *m_pEnv << "Setup \"" << m_pSubsession->mediumName() << "/" << m_pSubsession->codecName()  << "\" subsession (client ports " << m_pSubsession->clientPortNum()<< "-" << m_pSubsession->clientPortNum()+1 << ")\n";

    ++m_uiSuccessfulSubsessions;
    if (m_uiSuccessfulSubsessions < m_uiSubsessionCount)
    {
      assert(m_pSubsessionIter);
      m_pSubsession = m_pSubsessionIter->next();
      setupMediaSubsession(m_pSubsession);
    }
    else if (m_uiSuccessfulSubsessions == m_uiSubsessionCount)
    {
      setupReceivers();
      // Play streams
      m_pRtspClient->sendPlayCommand(*m_pSession, AsyncRtspClientAdapter::playHandler);
    }
  }
  else
  {
    std::ostringstream ostr;
    ostr << "Failed to setup \"" << m_pSubsession->mediumName() << "/" << m_pSubsession->codecName() << "\" subsession: " << m_pEnv->getResultMsg();
    m_sLastError = ostr.str();
    *m_pEnv << m_sLastError.c_str() << "\n";
    endLiveMediaEventLoop();
  } 
}

void AsyncRtspClientSessionManager::continueAfterPLAY( int resultCode, char* resultString )
{
  m_iLastLiveMediaErrorCode = resultCode;

  if (resultCode == 0)
  {
    // all good
    m_eState = RTSP_PLAYING;
  }
  else
  {
    m_sLastError =  "Failed to start playing session: " +  std::string(m_pEnv->getResultMsg());
    endLiveMediaEventLoop();
  }
}

void AsyncRtspClientSessionManager::continueAfterTEARDOWN( int resultCode, char* resultString )
{
  m_iLastLiveMediaErrorCode = resultCode;

  if (resultCode == 0)
  {
    // teardown successful
  }
  else
  {
    m_sLastError =  "Failed to teardown session: " +  std::string(m_pEnv->getResultMsg());
  }
  endLiveMediaEventLoop();
}
