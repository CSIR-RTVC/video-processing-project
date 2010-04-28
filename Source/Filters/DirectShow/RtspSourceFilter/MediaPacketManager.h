/** @file

MODULE				: RtspSourceFilter

FILE NAME			: MediaPacketManager.h

DESCRIPTION			: Packet manager for incoming media samples
					  
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
#include "MediaPacketQueue.h"
#include "MediaParameterExtractor.h"

class MediaSubsession;

class MediaPacketManager
{
  friend class MediaPacketQueue;

public:
  typedef std::map<unsigned, MediaPacketQueue*> MediaPacketQueueMap_t;

  MediaPacketManager();
  ~MediaPacketManager();

  bool isBufferingComplete() const;

  bool eof() const { return m_bEof; }
  void eof(bool val){ m_bEof = val; }

  void reset();
  void stop();

  bool isReady() const { return m_bReady; }
  void setReady(bool val) { m_bReady = val; }

  double getMinBufferTime() const { return m_dMinBufferTime; }
  void setMinBufferTime(double val) { m_dMinBufferTime = val; }

  HANDLE& getTypeInfoCompleteEventHandle() { return m_hTypeInfoCompleteEvent; }
  void resetTypeInfoCompleteEventHandle() { ResetEvent(m_hTypeInfoCompleteEvent); }

  StringMap_t& getParameterMap(MediaSubsession* pSubsession)
  {
    return m_parameterExtractor.getParameterMap(pSubsession);
  }

  bool initialiseChannelFromMediaSubsession(unsigned uiID, MediaSubsession* pSubsession);

  void addMediaPacket(unsigned uiID, unsigned char* data, unsigned dataSize, double dStartTime, bool bHasBeenRtcpSynchronised);

  bool hasSamples(unsigned uiID);
  MediaSample* getNextSample(unsigned uiID);

  void clear();

private:
  bool haveAllQueuesBeenSyncedUsingRtcp() const;
  unsigned getIdOfNextSample() const;

  // Offsets
  void setInitialOffset( double dOffset)
  {
    EnterCriticalSection(&m_critSecOffset);
    if (m_dInitialOffset == -1.0)
      m_dInitialOffset = dOffset;
    LeaveCriticalSection(&m_critSecOffset);
  }

  void setSynchronisedOffset( double dOffset )
  {
    EnterCriticalSection(&m_critSecOffset);
    if (m_dSynchronisedOffset == -1.0)
      m_dSynchronisedOffset = dOffset;
    LeaveCriticalSection(&m_critSecOffset);
  }

  MediaPacketQueueMap_t m_mMediaPacketQueues;
  // This component exists to extract inband parameters from the media stream before
  // playback can resume
  MediaParameterExtractor m_parameterExtractor;

  bool m_bEof;

  /// This component becomes initialised once all media type information has been collected 
  bool m_bInitialised;
  /// This component becomes ready once sufficient data has been buffered;
  bool m_bReady;
  /// RTCP Sync
  mutable bool m_bHaveAllQueuesBeenSyncedUsingRtcp;
  /// Buffer in micro seconds
  double m_dMinBufferTime;
  /// Handle for media session type 
  HANDLE m_hTypeInfoCompleteEvent;
  /// Critsec for multi-threaded access
  CRITICAL_SECTION m_critSec;
  CRITICAL_SECTION m_critSecOffset;

  // Start time offsets
  double m_dInitialOffset;
  double m_dSynchronisedOffset;
};
