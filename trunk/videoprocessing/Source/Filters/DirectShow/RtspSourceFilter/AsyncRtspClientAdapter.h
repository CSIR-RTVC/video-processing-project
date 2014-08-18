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
#include <boost/function.hpp>

class AsyncRtspClientAdapter : public RTSPClient
{
public:
  typedef boost::function<void(int resultCode, char* resultString)> RtspCallback_t;

public:
  static AsyncRtspClientAdapter* createNew(UsageEnvironment& env, char const* rtspURL,
    int verbosityLevel = 0,
    char const* applicationName = NULL,
    portNumBits tunnelOverHTTPPortNum = 0);

  AsyncRtspClientAdapter(UsageEnvironment& env, char const* rtspURL, int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum);

  // Callbacks
  void setOptionsCallback(AsyncRtspClientAdapter::RtspCallback_t val) { m_optionsCallback = val; }
  void setDescribeCallback(AsyncRtspClientAdapter::RtspCallback_t val) { m_describeCallback = val; }
  void setSetupCallback(AsyncRtspClientAdapter::RtspCallback_t val) { m_setupCallback = val; }
  void setPlayCallback(AsyncRtspClientAdapter::RtspCallback_t val) { m_playCallback = val; }
  void setTeardownCallback(AsyncRtspClientAdapter::RtspCallback_t val) { m_teardownCallback = val; }

  // Need a static method to get a regular function pointer
  static void optionsHandler(RTSPClient* _this, int resultCode, char* resultString) 
  {
    AsyncRtspClientAdapter* pClient = static_cast<AsyncRtspClientAdapter*>(_this);
    pClient->doHandleOptions(resultCode, resultString);
  }

  void doHandleOptions(int resultCode, char* resultString) 
  {
    if (m_optionsCallback)
      m_optionsCallback(resultCode, resultString);
  }

  // Need a static method to get a regular function pointer
  static void describeHandler(RTSPClient* _this, int resultCode, char* resultString) 
  {
    AsyncRtspClientAdapter* pClient = static_cast<AsyncRtspClientAdapter*>(_this);
    pClient->doHandleDescribe(resultCode, resultString);
  }

  void doHandleDescribe(int resultCode, char* resultString) 
  {
    if (m_describeCallback)
      m_describeCallback(resultCode, resultString);
  }

  // Need a static method to get a regular function pointer
  static void setupHandler(RTSPClient* _this, int resultCode, char* resultString) 
  {
    AsyncRtspClientAdapter* pClient = static_cast<AsyncRtspClientAdapter*>(_this);
    pClient->doHandleSetup(resultCode, resultString);
  }

  void doHandleSetup(int resultCode, char* resultString) 
  {
    if (m_setupCallback)
      m_setupCallback(resultCode, resultString);
  }

  // Need a static method to get a regular function pointer
  static void playHandler(RTSPClient* _this, int resultCode, char* resultString) 
  {
    AsyncRtspClientAdapter* pClient = static_cast<AsyncRtspClientAdapter*>(_this);
    pClient->doHandlePlay(resultCode, resultString);
  }

  void doHandlePlay(int resultCode, char* resultString) 
  {
    if (m_playCallback)
      m_playCallback(resultCode, resultString);
  }

  // Need a static method to get a regular function pointer
  static void teardownHandler(RTSPClient* _this, int resultCode, char* resultString) 
  {
    AsyncRtspClientAdapter* pClient = static_cast<AsyncRtspClientAdapter*>(_this);
    pClient->doHandleTeardown(resultCode, resultString);
  }

  void doHandleTeardown(int resultCode, char* resultString) 
  {
    if (m_teardownCallback)
      m_teardownCallback(resultCode, resultString);
  }

private:
  RtspCallback_t m_optionsCallback;
  RtspCallback_t m_describeCallback;
  RtspCallback_t m_setupCallback;
  RtspCallback_t m_playCallback;
  RtspCallback_t m_teardownCallback;
};
