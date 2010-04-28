/** @file

MODULE				: RtspSourceFilter

FILE NAME			: MediaPacketQueue.h

DESCRIPTION			: Packet queue used to transfer media samples between liveMedia and DirectShow
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2010, CSIR
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

// RTVC
#include <Shared/MediaSample.h>

class MediaPacketManager;

static bool isSynced(MediaSample* pSample) 
{
  return pSample->isMarkerSet();
}

/// Data queue that synchronizes adding data and removing data between different threads
class MediaPacketQueue
{
public:
  typedef std::deque<MediaSample*> MediaSampleQueue_t;

  MediaPacketQueue(MediaPacketManager& rPacketManager);

  ~MediaPacketQueue();

  size_t size() const 
  {
    return m_qSamples.size();
  }

  bool hasBeenSyncedUsingRtcp() const
  {
    return m_bFirstSyncedSample;
  }

  // This is only an approximation!!!
  double getMediaDurationInQueue() const;
  
  void addMediaSample(unsigned char* data, unsigned dataSize, double dStartTime, bool bHasBeenRtcpSynchronised);

  MediaSample* getNextSample();

  MediaSample* peekNextSample();

  bool hasSamples();

  void clear();

private:
  // Prohibit copying
  MediaPacketQueue(const MediaPacketQueue&);
  MediaPacketQueue operator=(const MediaPacketQueue&);

  double getStartTimeOffset() const;

  MediaPacketManager& m_rPacketManager;

  CRITICAL_SECTION m_critSec;

  MediaSampleQueue_t m_qSamples;

  bool m_bFirstSample;
  bool m_bFirstSyncedSample;
};
