/** @file

MODULE				: RtspSourceFilter

FILE NAME			: MediaPacketManager.cpp

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
#include "stdafx.h"
#include "MediaPacketManager.h"

const double FIVE_SECONDS = 5.0;

MediaPacketManager::MediaPacketManager() 
  :m_bEof(false),
  m_bInitialised(false),
  m_bReady(false),
  m_bHaveAllQueuesBeenSyncedUsingRtcp(false),
  m_dMinBufferTime(FIVE_SECONDS),
  m_dInitialOffset(-1.0),
  m_dSynchronisedOffset(-1.0)
{
  // Create handle for media type info completion- this will be used to notify the main thread that it can continue building the media pipeline
  m_hTypeInfoCompleteEvent = CreateEvent(
    NULL,                     // default security attributes
    FALSE,                    // auto-reset event
    FALSE,                    // initial state is nonsignaled
    TEXT("MediaTypeInfoComplete") // object name
    );

  InitializeCriticalSection(&m_critSec);
  InitializeCriticalSection(&m_critSecOffset);
}

MediaPacketManager::~MediaPacketManager()
{
  reset();

  DeleteCriticalSection(&m_critSec);
  DeleteCriticalSection(&m_critSecOffset);
}

bool MediaPacketManager::initialiseChannelFromMediaSubsession( unsigned uiID, MediaSubsession* pSubsession )
{
  EnterCriticalSection(&m_critSec);

  // Make sure each entry is unique
  MediaPacketQueueMap_t::iterator it = m_mMediaPacketQueues.find(uiID);
  assert(it == m_mMediaPacketQueues.end());
  m_mMediaPacketQueues[uiID] = new MediaPacketQueue(*this);
  bool bResult = m_parameterExtractor.addMediaChannel(uiID, pSubsession);
  LeaveCriticalSection(&m_critSec);
  return bResult;
}

void MediaPacketManager::addMediaPacket( unsigned uiID, unsigned char* data, unsigned dataSize, double dStartTime, bool bHasBeenRtcpSynchronised )
{
  // Check if media packet manager has been completely initialised
  if (!m_bInitialised)
  {
    EnterCriticalSection(&m_critSec);
    m_parameterExtractor.extractMediaParametersFromMediaData(uiID, data, dataSize);
    // notify if all queues have been initialised
    if (m_parameterExtractor.haveAllParametersBeenExtracted())
    {
      m_bInitialised = true;
      SetEvent(m_hTypeInfoCompleteEvent);
    }
    LeaveCriticalSection(&m_critSec);
  }

  // Add sample to correct queue
  EnterCriticalSection(&m_critSec);
  MediaPacketQueue* pQueue = m_mMediaPacketQueues[uiID];
  LeaveCriticalSection(&m_critSec);

  pQueue->addMediaSample(data, dataSize, dStartTime, bHasBeenRtcpSynchronised);
}

bool MediaPacketManager::hasSamples( unsigned uiID )
{
  bool bResult = false;
  //EnterCriticalSection(&m_critSec);
#if 1
  if (haveAllQueuesBeenSyncedUsingRtcp())
  {
    // Once all queues have been synced we only return true if the queue 
    // with the requested contains the next sample
    // This is to provide another level of synchronisation
    unsigned uiNextSampleId = getIdOfNextSample();
    bResult = (uiNextSampleId == uiID) ? true : false;
  }
  else
  {
    MediaPacketQueueMap_t::iterator it = m_mMediaPacketQueues.find(uiID);
    if (it != m_mMediaPacketQueues.end())
    {
      bResult = it->second->hasSamples();
    }
  }
#endif

#if 0
  MediaPacketQueueMap_t::iterator it = m_mMediaPacketQueues.find(uiID);
  if (it != m_mMediaPacketQueues.end())
  {
    bResult = it->second->hasSamples();
  }
#endif
  //LeaveCriticalSection(&m_critSec);
  return bResult;
}

MediaSample* MediaPacketManager::getNextSample( unsigned uiID )
{
#ifdef LOG_TO_DEBUG_FILE
  RtvcLogger& rLogger = RtvcLogger::getInstance();
  std::string sFunc("MediaPacketManager::getNextSample");
#endif
  //EnterCriticalSection(&m_critSec);
  MediaPacketQueueMap_t::iterator it = m_mMediaPacketQueues.find(uiID);
  MediaSample* pSample = NULL;
  if (it != m_mMediaPacketQueues.end())
  {
    pSample = it->second->getNextSample();
#ifdef LOG_TO_DEBUG_FILE
    LOG_INFO(rLogger, sFunc, "Sample ID: %1% TS: %2% Size: %3% RTCP Sync: %4%", uiID, pSample->StartTime(), pSample->getSize(), pSample->isMarkerSet());
#endif
  }
  //LeaveCriticalSection(&m_critSec);
  return pSample;
}

void MediaPacketManager::clear()
{
  EnterCriticalSection(&m_critSec);
  // Empty up all queues
  for (MediaPacketQueueMap_t::iterator it = m_mMediaPacketQueues.begin(); it != m_mMediaPacketQueues.end(); ++it)
  {
    it->second->clear();
  }
  LeaveCriticalSection(&m_critSec);
}

void MediaPacketManager::reset()
{
  EnterCriticalSection(&m_critSec);
  m_bEof = false;
  m_bReady = false;
  m_bInitialised = false;
  m_bHaveAllQueuesBeenSyncedUsingRtcp = false;
  m_dInitialOffset = -1.0;
  m_dSynchronisedOffset = -1.0;

  for (MediaPacketQueueMap_t::iterator it = m_mMediaPacketQueues.begin(); it != m_mMediaPacketQueues.end(); ++it)
  {
    delete it->second;
  }

  m_mMediaPacketQueues.clear();
  m_parameterExtractor.clear();
  clear();
  LeaveCriticalSection(&m_critSec);
}

void MediaPacketManager::stop()
{
  EnterCriticalSection(&m_critSec);
  m_bEof = true;
  m_bReady = false;
  m_bInitialised = false;
  m_bHaveAllQueuesBeenSyncedUsingRtcp = false;
  m_dInitialOffset = -1.0;
  m_dSynchronisedOffset = -1.0;
  LeaveCriticalSection(&m_critSec);
}

inline bool MediaPacketManager::haveAllQueuesBeenSyncedUsingRtcp() const
{
  if (!m_bHaveAllQueuesBeenSyncedUsingRtcp)
  {
    for (MediaPacketQueueMap_t::const_iterator it = m_mMediaPacketQueues.begin(); it != m_mMediaPacketQueues.end(); ++it)
    {
      MediaSample* pFirstSample = it->second->peekNextSample();
      if (pFirstSample)
      {
        if (!pFirstSample->isMarkerSet())
          return false;
      }
    }
    m_bHaveAllQueuesBeenSyncedUsingRtcp = true;
  }
  return m_bHaveAllQueuesBeenSyncedUsingRtcp;
}

inline unsigned MediaPacketManager::getIdOfNextSample() const
{
  unsigned uiMinId = UINT_MAX;
  double dMinStartTime = 0.0;
  for ( MediaPacketQueueMap_t::const_iterator it = m_mMediaPacketQueues.begin(); it != m_mMediaPacketQueues.end(); ++it)
  {
    MediaSample* pSample = it->second->peekNextSample();
    if (pSample)
    {
      if (uiMinId != UINT_MAX)
      {
        if (pSample->StartTime() < dMinStartTime)
        {
          uiMinId = it->first;
          dMinStartTime = pSample->StartTime();
        }
      }
      else
      {
        // Init first min
        uiMinId = it->first;
        dMinStartTime = pSample->StartTime();
      }
    }
  }
  return uiMinId;
}

bool MediaPacketManager::isBufferingComplete() const
{
#define NO_BUFFERING
#ifdef NO_BUFFERING
  return true;
#endif

#define USING_BUFFER_SIZE
#ifdef USING_BUFFER_SIZE
  const int INITIAL_BUFFER_SIZE = 20;
  // Check if one of the queues contains sufficient media data
  for ( MediaPacketQueueMap_t::const_iterator it = m_mMediaPacketQueues.begin(); it != m_mMediaPacketQueues.end(); ++it)
  {
    if (it->second->size() > INITIAL_BUFFER_SIZE)
      return true;
  }
  return false;
#endif

  // Default
  return true;
}
