/** @file

MODULE				: RtspSourceFilter

FILE NAME			: AsyncRtspClientAdapter.cpp

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
#include "stdafx.h"
#include "AsyncRtspClientAdapter.h"
#include "AsyncRtspClientSessionManager.h"

AsyncRtspClientAdapter* AsyncRtspClientAdapter::createNew(AsyncRtspClientSessionManager& rtspClientSessionManager, UsageEnvironment& env, char const* rtspURL,
  int verbosityLevel,
  char const* applicationName,
  portNumBits tunnelOverHTTPPortNum) 
{
  return new AsyncRtspClientAdapter(rtspClientSessionManager, env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum);
}

AsyncRtspClientAdapter::AsyncRtspClientAdapter(AsyncRtspClientSessionManager& rtspClientSessionManager, UsageEnvironment& env, char const* rtspURL, int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum)
  : RTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, -1),
  m_rtspClientSessionManager(rtspClientSessionManager)
{

}

void AsyncRtspClientAdapter::doHandleOptions(int resultCode, char* resultString)
{
  m_rtspClientSessionManager.continueAfterOPTIONS(resultCode, resultString);
}

void AsyncRtspClientAdapter::doHandleDescribe(int resultCode, char* resultString)
{
  m_rtspClientSessionManager.continueAfterDESCRIBE(resultCode, resultString);
}

void AsyncRtspClientAdapter::doHandleSetup(int resultCode, char* resultString)
{
  m_rtspClientSessionManager.continueAfterSETUP(resultCode, resultString);
}

void AsyncRtspClientAdapter::doHandlePlay(int resultCode, char* resultString)
{
  m_rtspClientSessionManager.continueAfterPLAY(resultCode, resultString);
}

void AsyncRtspClientAdapter::doHandleTeardown(int resultCode, char* resultString)
{
  m_rtspClientSessionManager.continueAfterTEARDOWN(resultCode, resultString);
}
