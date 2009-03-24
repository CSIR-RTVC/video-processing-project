/** @file

MODULE				: RtpDataQueue

FILE NAME			: RtpDataQueue.h

DESCRIPTION			: 
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008, Meraka Institute
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of the Meraka Institute nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

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
#include <queue>

// RTVC
#include <Shared/MediaSample.h>

/// Data queue that synchronises adding data and removing data between different threads
class RtpDataQueue
{
public:

	RtpDataQueue()
		:m_bHasBeenSyncedUsingRtcp(false)
	{;}

	~RtpDataQueue()
	{
		CAutoLock lock(&m_lock); 
		while ( !m_qSamples.empty() )
		{
			MediaSample* pSample = m_qSamples.front();
			m_qSamples.pop();
			delete pSample;
		}
	}

	void addMediaSample(unsigned char* data, unsigned dataSize, double dStartTime, bool bHasBeenRtcpSynchronised)
	{
		CAutoLock lock(&m_lock); 
		if (!m_bHasBeenSyncedUsingRtcp)
		{
			if (bHasBeenRtcpSynchronised)
			{
				m_bHasBeenSyncedUsingRtcp = true;
				// Create a marked sample
				MediaSample* pMediaSample = MediaSample::createMediaSample(data, dataSize, dStartTime, true);
				m_qSamples.push(pMediaSample);
			}
		}
		// Else add a normal sample to the queue
		MediaSample* pMediaSample = MediaSample::createMediaSample(data, dataSize, dStartTime /*, false*/);
		m_qSamples.push(pMediaSample);
	}

	MediaSample* getNextSample()
	{
		CAutoLock lock(&m_lock); 
		if (!m_qSamples.empty())
		{
			MediaSample* pSample = m_qSamples.front();
			m_qSamples.pop();
			return pSample;
		}
		return NULL;
	}

	bool hasSamples()
	{
		CAutoLock lock(&m_lock); 
		return !m_qSamples.empty();
	}

private:
	bool m_bHasBeenSyncedUsingRtcp;

	CCritSec m_lock;
	std::queue<MediaSample*> m_qSamples;
};
