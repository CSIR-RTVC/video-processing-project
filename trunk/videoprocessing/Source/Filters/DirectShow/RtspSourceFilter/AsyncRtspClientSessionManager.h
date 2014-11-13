/** @file

MODULE				: RtspSourceFilter

FILE NAME			: AsyncRtspClientSessionManager.h

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
#pragma once
#include "AsyncRtspClientAdapter.h"
#include "MediaPacketManager.h"

// Forward
class TaskScheduler;
class UsageEnvironment;
class RTSPClient;
class MediaSession;
class MediaSubsession;
class RtvcRtpSink;

class AsyncRtspClientSessionManager
{
  friend class AsyncRtspClientAdapter;

public:

  AsyncRtspClientSessionManager(MediaPacketManager& rMediaPacketManager);
  ~AsyncRtspClientSessionManager();

  std::string getLastError() const { return m_sLastError; }
  int getLastLiveMediaErrorCode() const { return m_iLastLiveMediaErrorCode; }

  bool getStreamUsingTcp() const { return m_bStreamUsingTcp; }
  void setStreamUsingTcp(bool val) { m_bStreamUsingTcp = val; }

  bool getStreamOverHttp() const { return m_bStreamOverHttp; }
  void setStreamOverHttp(bool val) { m_bStreamOverHttp = val; }

  MediaPacketManager& getMediaPacketManager() const { return m_rMediaPacketManager; }
  MediaSession* getMediaSession() const { return m_pSession; }

  bool setupSuccessful() const
  {
    return m_eState == RTSP_SETUP || m_eState == RTSP_PLAYING;
  }

  void startRtspSession(const std::string& sUrl);
  void endLiveMediaEventLoop();

  void subsessionAfterPlaying(RtvcRtpSink* pSink);

private:
  // Prohibit copying
  AsyncRtspClientSessionManager(const AsyncRtspClientSessionManager&);
  AsyncRtspClientSessionManager operator=(const AsyncRtspClientSessionManager&);

  void createMediaSession( const std::string& sUrl );
  void setupMediaSession();
  void setupMediaSubsession(MediaSubsession* pSubsession);

  void setupReceivers();

  void teardownMediaSession();
  void endSession();

  void cleanupLiveMedia()
  {
    // Cleanup RTSP Media session
    teardownMediaSession();

    if (m_pScheduler)
      delete m_pScheduler;
    m_pScheduler = NULL;

    if (m_pEnv)
      m_pEnv->reclaim(); // Deletes itself
    m_pEnv = NULL;

    m_uiSubsessionCount = 0;
    m_uiSuccessfulSubsessions = 0;

    // memory is managed by livemedia
    m_pSubsession = NULL;

    if (m_pSubsessionIter)
    {
      delete m_pSubsessionIter;
      m_pSubsessionIter = NULL;
    }

    resetWatchVariable();
  }

  /// Bye Handler
  static void subsessionByeHandler(void* clientData) 
  {
    AsyncRtspClientSessionManager* pRtspSessionManager = (AsyncRtspClientSessionManager*) clientData;
    pRtspSessionManager->endLiveMediaEventLoop();
  }

  void sessionAfterPlaying();
private:

  bool createRtpSource(MediaSubsession* pSubsession);
  void closeMediaSinks();
  void tearDownStreams();

  /// Call this method to end the life media event loop
  void setWatchVariable() { m_watchVariable = 1; }
  void resetWatchVariable() { m_watchVariable = 0; }
  bool createReceivers(MediaSession* pSession);

private:

  // Forward function definitions:
  void continueAfterOPTIONS(int resultCode, char* resultString);
  void continueAfterDESCRIBE(int resultCode, char* resultString);
  void continueAfterSETUP(int resultCode, char* resultString);
  void continueAfterPLAY(int resultCode, char* resultString);
  void continueAfterTEARDOWN(int resultCode, char* resultString);

private:

  enum RtspState
  {
    RTSP_INIT,    // state until RTSP SETUP is successful
    RTSP_SETUP,   // state until RTSP PLAY is successful
    RTSP_PLAYING
  };
  RtspState m_eState;

  // Stores last error
  std::string m_sLastError;

  MediaPacketManager& m_rMediaPacketManager;
  /// TCP
  bool m_bStreamUsingTcp;
  /// HTTP tunneling
  bool m_bStreamOverHttp;

  /// Livemedia scheduler
  TaskScheduler* m_pScheduler;
  /// Usage environment
  UsageEnvironment* m_pEnv;

  AsyncRtspClientAdapter* m_pRtspClient;

  std::string m_sUrl;
  /// MediaSession
  MediaSession* m_pSession;
  /// MediaSubsession
  MediaSubsession* m_pSubsession;
  /// count of media subsessions used for async setup
  unsigned m_uiSubsessionCount;
  unsigned m_uiSuccessfulSubsessions;
  /// iterator used for async setup
  MediaSubsessionIterator* m_pSubsessionIter;
  /// LiveMediaEventLoop watch variable
  char m_watchVariable;
  /// Live media error code
  int m_iLastLiveMediaErrorCode;
};

