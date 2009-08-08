/** @file

MODULE				: RtvcRtpSink

FILE NAME			: RtvcRtpSink.h

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

// DirectShow includes
#pragma warning(push)     // disable for this header only
#pragma warning(disable:4312) 
// DirectShow
#include <Streams.h>
#pragma warning(pop)      // restore original warning level

#include <Windows.h>

// STL
#include <iostream>

// LiveMedia
#include "MediaSink.hh"
#include <RTPSource.hh>

// RTVC
#include <Shared/MediaSample.h>

/**
* LiveMedia extension - Base RTP Sink for received media. This class is the interface between the liveMedia RTP library 
* and our DirectShow media queue. It receive raw media data from the RTP Source and adds it to the media queue. 
*/

template<class T>
class RtvcRtpSink : public MediaSink
{
public:
	/// Constructor
	RtvcRtpSink(UsageEnvironment& env, int nSourceID, unsigned bufferSize, T* pSampleHandler)
		: MediaSink(env),
		m_pSampleHandler(pSampleHandler),
		m_bHasBeenSyncedUsingRtcp(false),
		m_nSourceID(nSourceID)
	{
		// Create memory buffer
		fBuffer = new unsigned char[bufferSize];
		fBufferSize = bufferSize;
	}

	/// Destructor
	virtual ~RtvcRtpSink( void )
	{
		// Free memory
		delete[] fBuffer;
	}
	
protected:
	/// LiveMedia method
	static void afterGettingFrame( void* clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds )
	{
		RtvcRtpSink* sink = (RtvcRtpSink*)clientData;
		sink->afterGettingFrame1(frameSize, presentationTime);
	}

	/// LiveMedia method
	virtual void afterGettingFrame1( unsigned frameSize, struct timeval presentationTime )
	{
		processData(fBuffer, frameSize, presentationTime);
		// Then try getting the next frame:
		continuePlaying();
	}

	/// LiveMedia method
	virtual Boolean continuePlaying()
	{
		if (fSource == NULL) return False;

		fSource->getNextFrame(fBuffer, fBufferSize,
			afterGettingFrame, this,
			onSourceClosure, this);

		return True;
	}

	void processData( unsigned char* data, unsigned dataSize, struct timeval presentationTime )
	{
		// Check if stream has been synced using RTCP: to do this we need to get hold of the RTPSource object
		RTPSource* pRtpSource = (RTPSource*)fSource;
		// Convert presentation time to double
		double dStartTime = presentationTime.tv_sec + (presentationTime.tv_usec / 1000000.0);
		// Implicit Template Declaration
		m_pSampleHandler->processMediaSample(m_nSourceID, data, dataSize, dStartTime, pRtpSource->hasBeenSynchronizedUsingRTCP());
	}

private:
	/// The handler that processes the media samples
	T* m_pSampleHandler;

	/// Buffer for the media sample raw data
	unsigned char* fBuffer;
	/// Buffer size
	unsigned fBufferSize;

	/// This member gets set when the first RTCP synced sample is received
	bool m_bHasBeenSyncedUsingRtcp;

	/// ID of the sink which is the "source" of the output pin
	int m_nSourceID;
};
