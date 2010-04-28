/** @file

MODULE				: RtspSourceFilter

FILE NAME			: MediaPacketQueue.cpp

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
#include "stdafx.h"
#include "MediaPacketQueue.h"
#include "MediaPacketManager.h"

MediaPacketQueue::MediaPacketQueue(MediaPacketManager& rPacketManager)
  :m_rPacketManager(rPacketManager),
  m_bFirstSample(false),
  m_bFirstSyncedSample(false)
{
  InitializeCriticalSection(&m_critSec);
}

MediaPacketQueue::~MediaPacketQueue()
{
  clear();
  DeleteCriticalSection(&m_critSec);
}

// This is only an approximation!!!
double MediaPacketQueue::getMediaDurationInQueue() const
{
  if (m_qSamples.size() > 1)
  {
    MediaSample* pFirst = m_qSamples.front();
    MediaSample* pLast = m_qSamples.back();
    bool bSynced1 = pFirst->isMarkerSet();
    bool bSynced2 = pLast->isMarkerSet();
    if (bSynced1 == bSynced2)
      return pLast->StartTime() - pFirst->StartTime();
    else
    {
      // Add the total time made up of samples that have been synced
      // To the total time of samples that haven't been synced
      MediaSampleQueue_t::const_iterator it = find_if(m_qSamples.begin(), m_qSamples.end(), isSynced);
      const MediaSample* pFirstSynced = *it;
      const MediaSample* pLastUnsynced = *(--it);
      // Add the two differences
      return (pLastUnsynced->StartTime() - pFirst->StartTime()) + (pLast->StartTime() - pFirstSynced->StartTime());
    }
  }
  return 0.0;
}

void MediaPacketQueue::addMediaSample(unsigned char* data, unsigned dataSize, double dStartTime, bool bHasBeenRtcpSynchronised)
{
  if ( !m_bFirstSample )
  {
    m_bFirstSample = true;
    m_rPacketManager.setInitialOffset(dStartTime);
  }
  else if (!m_bFirstSyncedSample && bHasBeenRtcpSynchronised)
  {
    m_bFirstSyncedSample = true;
    m_rPacketManager.setSynchronisedOffset(dStartTime);
  }

  double dAdjustedStartTime = dStartTime - getStartTimeOffset();
  MediaSample* pMediaSample = MediaSample::createMediaSample(data, dataSize, dAdjustedStartTime, bHasBeenRtcpSynchronised);

  EnterCriticalSection(&m_critSec);
  m_qSamples.push_back(pMediaSample);
  LeaveCriticalSection(&m_critSec);
}

MediaSample* MediaPacketQueue::getNextSample()
{
  MediaSample* pSample = NULL;
  EnterCriticalSection(&m_critSec);
  if (!m_qSamples.empty())
  {
    pSample = m_qSamples.front();
    m_qSamples.pop_front();
  }
  LeaveCriticalSection(&m_critSec);
  return pSample;
}

MediaSample* MediaPacketQueue::peekNextSample()
{
  MediaSample* pSample = NULL;
  EnterCriticalSection(&m_critSec);
  if (!m_qSamples.empty())
  {
    pSample = m_qSamples.front();
  }
  LeaveCriticalSection(&m_critSec);
  return pSample;
}

bool MediaPacketQueue::hasSamples()
{
  EnterCriticalSection(&m_critSec);
  bool bHasSamples = !m_qSamples.empty();
  LeaveCriticalSection(&m_critSec);
  return bHasSamples;
}

void MediaPacketQueue::clear()
{
  EnterCriticalSection(&m_critSec);
  while ( !m_qSamples.empty() )
  {
    MediaSample* pSample = m_qSamples.front();
    m_qSamples.pop_front();
    delete pSample;
  }
  LeaveCriticalSection(&m_critSec);
}

double MediaPacketQueue::getStartTimeOffset() const
{
  if (m_bFirstSyncedSample)
    return m_rPacketManager.m_dSynchronisedOffset;
  else
    return m_rPacketManager.m_dInitialOffset;
}
