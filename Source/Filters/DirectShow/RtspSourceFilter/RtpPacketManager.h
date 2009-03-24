/** @file

MODULE				: RtpPacketManager

FILE NAME			: RtpPacketManager.h

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
#include <map>

// RTVC
#include "RtpDataQueue.h"

/// Creates/manages data queues per RTP source.
/// Each queue is identified via an ID.
class RtpPacketManager
{
public:

	RtpPacketManager(){;}

	~RtpPacketManager()
	{
		// Free up all queue
		for (std::map<int, RtpDataQueue*>::iterator it = m_dataQueues.begin(); it != m_dataQueues.end(); it++)
		{
			delete it->second;
		}
	}

	void processMediaSample(int nSourceID, unsigned char* data, unsigned dataSize, double dStartTime, bool bHasBeenRtcpSynchronised)
	{
		// Lookup which queue
		std::map<int, RtpDataQueue*>::iterator it = m_dataQueues.find(nSourceID);
		if (it != m_dataQueues.end())
		{
			// Add sample to correct queue
			it->second->addMediaSample(data, dataSize, dStartTime, bHasBeenRtcpSynchronised);
		}
	}

	bool createQueue(int nSourceID)
	{
		// Make sure it doesn't already exist
		std::map<int, RtpDataQueue*>::iterator it = m_dataQueues.find(nSourceID);
		if (it == m_dataQueues.end())
		{
			RtpDataQueue* pRtpQueue = new RtpDataQueue();
			m_dataQueues[nSourceID] = pRtpQueue;
			return true;
		}
		else
		{
			return false;
		}
	}

	bool hasSamples(int nID)
	{
		if (!isBuffering())
		{
			std::map<int, RtpDataQueue*>::iterator it = m_dataQueues.find(nID);
			if (it != m_dataQueues.end())
			{
				return it->second->hasSamples();
			}
		}
		return false;
	}

	MediaSample* getNextSample(int nID)
	{
		std::map<int, RtpDataQueue*>::iterator it = m_dataQueues.find(nID);
		if (it != m_dataQueues.end())
		{
			return it->second->getNextSample();
		}
		return NULL;
	}

	bool isBuffering()
	{
		return false;
	}

private:
	std::map<int, RtpDataQueue*> m_dataQueues;
};
