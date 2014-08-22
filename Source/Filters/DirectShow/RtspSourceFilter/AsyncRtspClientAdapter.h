/** @file

MODULE				: RtspSourceFilter

FILE NAME			: AsyncRtspClientAdapter.h

DESCRIPTION			: Adapter class for asynchronous callbacks
					  
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

class AsyncRtspClientSessionManager;

/**
 * @brief The AsyncRtspClientAdapter is an adapter class to forward
 * the results of the async RTSP requests to our AsyncRtspClientSessionManager
 * object.
 */
class AsyncRtspClientAdapter : public RTSPClient
{
public:

public:
  /**
   * @brief Named constructor
   */
  static AsyncRtspClientAdapter* createNew(AsyncRtspClientSessionManager& rtspClientSessionManager, UsageEnvironment& env, char const* rtspURL,
    int verbosityLevel = 0,
    char const* applicationName = NULL,
    portNumBits tunnelOverHTTPPortNum = 0);
  /**
   * @brief Constructor
   */
  AsyncRtspClientAdapter(AsyncRtspClientSessionManager& rtspClientSessionManager, UsageEnvironment& env, char const* rtspURL, int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum);
  /**
   * @brief Method to handle OPTIONS response of live555 RTSPClient
   */ 
  static void optionsHandler(RTSPClient* _this, int resultCode, char* resultString) 
  {
    AsyncRtspClientAdapter* pClient = static_cast<AsyncRtspClientAdapter*>(_this);
    pClient->doHandleOptions(resultCode, resultString);
  }
  /**
   * @brief Method to handle DESCRIBE response of live555 RTSPClient
   */ 
  static void describeHandler(RTSPClient* _this, int resultCode, char* resultString) 
  {
    AsyncRtspClientAdapter* pClient = static_cast<AsyncRtspClientAdapter*>(_this);
    pClient->doHandleDescribe(resultCode, resultString);
  }
  /**
   * @brief Method to handle SETUP response of live555 RTSPClient
   */ 
  static void setupHandler(RTSPClient* _this, int resultCode, char* resultString) 
  {
    AsyncRtspClientAdapter* pClient = static_cast<AsyncRtspClientAdapter*>(_this);
    pClient->doHandleSetup(resultCode, resultString);
  }
  /**
   * @brief Method to handle PLAY response of live555 RTSPClient
   */ 
  static void playHandler(RTSPClient* _this, int resultCode, char* resultString) 
  {
    AsyncRtspClientAdapter* pClient = static_cast<AsyncRtspClientAdapter*>(_this);
    pClient->doHandlePlay(resultCode, resultString);
  }
  /**
   * @brief Method to handle TEARDOWN response of live555 RTSPClient
   */ 
  static void teardownHandler(RTSPClient* _this, int resultCode, char* resultString) 
  {
    AsyncRtspClientAdapter* pClient = static_cast<AsyncRtspClientAdapter*>(_this);
    pClient->doHandleTeardown(resultCode, resultString);
  }
  /**
   * @brief Handles OPTIONS response by forwarding to RTSP session manager
   */
  void doHandleOptions(int resultCode, char* resultString);
  /**
   * @brief Handles DESCRIBE response
   */
  void doHandleDescribe(int resultCode, char* resultString);
  /**
   * @brief Handles SETUP response by forwarding to RTSP session manager
   */
  void doHandleSetup(int resultCode, char* resultString);
  /**
   * @brief Handles PLAY response by forwarding to RTSP session manager
   */
  void doHandlePlay(int resultCode, char* resultString);
  /**
   * @brief Handles TEARDOWN response by forwarding to RTSP session manager
   */
  void doHandleTeardown(int resultCode, char* resultString);

private:
  AsyncRtspClientSessionManager& m_rtspClientSessionManager;
};
