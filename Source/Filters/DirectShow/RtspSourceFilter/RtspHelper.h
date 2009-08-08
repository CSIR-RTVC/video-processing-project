/** @file

MODULE				: RtspHelper

FILE NAME			: RtspHelper.h

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
#pragma once

// STL
#include <string>
#include <vector>

// liveMedia
#include <liveMedia.hh>
#include <GroupsockHelper.hh>

/// Helper class for common RTSP related liveMedia functionality
class RtspHelper
{
public:

	static bool createMediaSession(const std::string& sUrl, RTSPClient* pRtspClient, MediaSession** pMediaSession, std::string& sResult, int nTimeOut = -1)
	{
		UsageEnvironment* pEnv = &(pRtspClient->envir());

		const char* szUrl = sUrl.c_str();

		// Get options response
		char* optionsResponse = pRtspClient->sendOptionsCmd(szUrl, NULL, NULL, NULL, nTimeOut);
		*pEnv << "RTSP Options response: " << optionsResponse << "\r\n";
		delete[] optionsResponse;

		// Get SDP description
		char* sdpDescription = pRtspClient->describeURL(szUrl, NULL, false, nTimeOut);
		if (sdpDescription == NULL) 
		{
			sResult = "Failed to get a SDP description from URL \"" + sUrl + "\": " + std::string(pEnv->getResultMsg());
			return false;
		}

		*pEnv << "Opened URL \"" << szUrl << "\", returning a SDP description:\n" << sdpDescription << "\n";
		unsigned statusCode = pRtspClient->describeStatus();
		*pEnv << "Status code: " << statusCode << "\n";

		// Create a media session object from this SDP description:
		*pMediaSession = MediaSession::createNew(*pEnv, sdpDescription);
		delete[] sdpDescription;
		if (*pMediaSession == NULL) 
		{
			sResult = "Failed to create a MediaSession object from the SDP description: " + std::string(pEnv->getResultMsg());
			return false;
		} 
		else if (!(*pMediaSession)->hasSubsessions()) 
		{
			sResult = "This session has no media subsessions (i.e., \"m=\" lines)";
			return false;
		}
		return true;
	}

	static bool createRtpSource(UsageEnvironment* pEnv, MediaSubsession* pSubsession)
	{
		if (!pSubsession->initiate()) 
		{
			*pEnv << "Unable to create receiver for \"" << pSubsession->mediumName() << "/" << pSubsession->codecName() << "\" subsession: " << pEnv->getResultMsg() << "\n"; 
			return false;
		} 
		else 
		{
			*pEnv << "Created receiver for \"" << pSubsession->mediumName() << "/" << pSubsession->codecName() << "\" subsession (client ports " << pSubsession->clientPortNum() << "-" << pSubsession->clientPortNum()+1 << ")\n";
			if (pSubsession->rtpSource() != NULL) 
			{
				//TOREVISE
				// Because we're saving the incoming data, rather than playing
				// it in real time, allow an especially large time threshold
				// (1 second) for reordering misordered incoming packets:
				unsigned const thresh = 1000000; // 1 second 
				pSubsession->rtpSource()->setPacketReorderingThresholdTime(thresh);

				int socketInputBufferSize = 0;
				// TOREMOVE: Left over from Live555 examples
				if (socketInputBufferSize > 0) 
				{
					// Set the RTP source's input buffer size as specified:
					int socketNum = pSubsession->rtpSource()->RTPgs()->socketNum();
					unsigned curBufferSize = getReceiveBufferSize(*pEnv, socketNum);
					unsigned newBufferSize = setReceiveBufferTo(*pEnv, socketNum, socketInputBufferSize);
					*pEnv << "Changed socket receive buffer size for the \"" << pSubsession->mediumName() << "/" << pSubsession->codecName() << "\" subsession from " << curBufferSize << " to " << newBufferSize << " bytes\n";
				}
			}
			return true;
		}
	}

	static bool createRtpSources(UsageEnvironment* pEnv, MediaSession* pSession)
	{
		Boolean madeProgress = False;
		MediaSubsessionIterator iter(*pSession);
		MediaSubsession *pSubsession;
		while ((pSubsession = iter.next()) != NULL) 
		{
			if (createRtpSource(pEnv, pSubsession))
			{
				madeProgress = true;
			}
		}
		return madeProgress;
	}

	static bool setupStream( RTSPClient* pRtspClient, MediaSubsession* pSubsession, bool bStreamUsingTCP)
	{
		UsageEnvironment* pEnv = &(pRtspClient->envir());
		if (pSubsession->clientPortNum() == 0) return false; // port # was not set

		if (!pRtspClient->setupMediaSubsession(*pSubsession, False, bStreamUsingTCP)) 
		{
			*pEnv << "Failed to setup \"" << pSubsession->mediumName() << "/" << pSubsession->codecName() << "\" subsession: " << pEnv->getResultMsg() << "\n";
			return false;
		} 
		else 
		{
			*pEnv << "Setup \"" << pSubsession->mediumName() << "/" << pSubsession->codecName()  << "\" subsession (client ports " << pSubsession->clientPortNum()<< "-" << pSubsession->clientPortNum()+1 << ")\n";
			return true;
		}
	}
	static bool setupStreams(RTSPClient* pRtspClient, MediaSession* pSession, bool bStreamUsingTCP)
	{
		//UsageEnvironment* pEnv = &(pSession->envir());
		Boolean madeProgress = False;
		MediaSubsessionIterator iter(*pSession);
		MediaSubsession *pSubsession;
		while ((pSubsession = iter.next()) != NULL) 
		{
			if (setupStream(pRtspClient, pSubsession, bStreamUsingTCP))
			{
				madeProgress = True;
			}
		}
		return madeProgress;
	}

	static void shutdownMediaSession(RTSPClient* pRtspClient, MediaSession* pMediaSession)
	{
		if (pMediaSession)
		{
			// Close subsessions
			MediaSubsessionIterator iter(*pMediaSession);
			MediaSubsession* pSubsession;
			while ((pSubsession = iter.next()) != NULL) 
			{
				Medium::close(pSubsession->sink);
				pSubsession->sink = NULL;
			}
			// Teardown session
			if (pRtspClient) pRtspClient->teardownMediaSession(*pMediaSession);
			// Close session and client
			Medium::close(pMediaSession);
			if (pRtspClient) Medium::close(pRtspClient);
		}
	}
};
