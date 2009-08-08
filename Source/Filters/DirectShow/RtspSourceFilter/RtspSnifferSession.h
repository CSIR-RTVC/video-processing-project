/** @file

MODULE				: RtspSnifferSession

FILE NAME			: RtspSnifferSession.h

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
#include <map>
#include <string>
#include <vector>

// LiveMedia
#include <liveMedia.hh>

/**
* This class is used to detect media information given an rtsp URL. 
* This class uses the liveMedia library for the RTSP communication with the RTSP server
* It stores media subsessions in a vector which are then used to create and configure the 
* DirectShow output pins with the correct media parameters
*/
class RtspSnifferSession
{
public:

	/// Constructor
	RtspSnifferSession();

	/// Constructor
	~RtspSnifferSession();
	/// This method returns the media type information necessary to construct the filter's output pins
	/// @param[in] sUrl The url of the media stream
	/// @param[out] A the vector that is going to receive the sniffed information
	/// @param[in] The RTSP client request timeout
	bool getMediaSessionInformation(const std::string sUrl, int nTimeOut = -1);

	std::vector<MediaSubsession*> getMediaSubsessions()
	{
		return m_vSubsessions;
	}

	std::string getLastError()
	{
		return m_sLastError;
	}

	/// Shutdown RTSP session cleanly
	void shutdown();

	/// Bye Handler
	static void subsessionByeHandler(void* clientData) 
	{
		RtspSnifferSession* pSnifferSession = (RtspSnifferSession*) clientData;
		pSnifferSession->shutdown();
	}

private:

	bool playStreamsIfNecessary(UsageEnvironment* pEnv, MediaSession* pSession);

	bool createReceivers(MediaSession* pSession);
	/// This method creates a mapping between the liveMedia media subsession which contains all the media information
	/// And our rtvc::MediaSubsession, which stores all data required to create the output pins
	/// We could just have used the liveMedia class and created our output pins using that but this method seperates the livemedia 
	/// Code from our own and makes our Outputpin implementation independent of the liveMedia library
	//bool mapLiveMediaToRtvc(MediaSession* pSession);

	/// Currently only H263 streams need to be played back so that we can extract the width and the height from the raw data stream
	bool needToPlayStreams(MediaSession* pSubsession);
	
	/// Format sniffer for H263
	//H263FormatSniffer* m_pFormatSniffer;

	/// Rtsp client
	RTSPClient* m_pRtspClient;

	/// MediaSession
	MediaSession* m_pSession;

	/// TCP
	bool m_bStreamUsingTCP;

	/// LiveMediaEventLoop watch variable
	char m_watchVariable;

	// Stores last error
	std::string m_sLastError;

	// Store media subsessions
	std::vector<MediaSubsession*> m_vSubsessions;
};