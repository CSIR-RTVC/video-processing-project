/** @file

MODULE				: RtspSourceOutputPin

FILE NAME			: RtvcRtpSink.h

DESCRIPTION			: Class for liveMedia integration
                  Processes incoming samples received by the LiveMedia library
					  
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
#include "MediaPacketManager.h"
#include <Shared/MediaSample.h>

class RtvcRtpSink : public MediaSink
{
public:
  /// Constructor
  RtvcRtpSink(UsageEnvironment& env, unsigned uiSourceID, RtspClientSessionManager& rSessionManager, MediaSubsession* pSubsession, unsigned uiBufferSize)
    : MediaSink(env),
    m_uiSourceID(uiSourceID),
    m_rSessionManager(rSessionManager),
    m_rMediaPacketManager(rSessionManager.getMediaPacketManager()),
    m_pSubsession(pSubsession),
    m_pRtpSource(pSubsession->rtpSource()),
    m_pAmrSource(NULL),
    m_uiBufferSize(uiBufferSize),
    m_pBuffer(new unsigned char[uiBufferSize]),
    m_bFirstPacketDelivered(false)
  {
  }

  /// Destructor
  virtual ~RtvcRtpSink()
  {
    // Free memory
    delete[] m_pBuffer;
  }

  MediaSubsession* getSubsession() const { return m_pSubsession; }

  static void endSubsession(void* clientData)
  {
    RtvcRtpSink* pSink = (RtvcRtpSink*)clientData;
    pSink->doEndSubsession();
  }

  void doEndSubsession()
  {
    m_rSessionManager.subsessionAfterPlaying(this);
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
    processData(m_pBuffer, frameSize, presentationTime);
    // Then try getting the next frame:
    continuePlaying();
  }

  /// LiveMedia method
  virtual Boolean continuePlaying()
  {
    if (fSource == NULL) return False;

    fSource->getNextFrame(m_pBuffer, m_uiBufferSize,
      afterGettingFrame, this,
      onSourceClosure, this);

    return True;
  }


  void processData( unsigned char* data, unsigned dataSize, struct timeval presentationTime )
  {
    if (m_bFirstPacketDelivered)
    {
      if (m_pAmrSource)
      {
        u_int8_t uiHeader = m_pAmrSource->lastFrameHeader();
        double dStartTime = presentationTime.tv_sec + (presentationTime.tv_usec / 1000000.0);
        BYTE* pByte = new BYTE[dataSize + 1];
        pByte[0] = uiHeader;
        memcpy(pByte + 1, data, dataSize);
        bool bSynced = (m_pRtpSource) ? !!m_pRtpSource->hasBeenSynchronizedUsingRTCP() : false;
        m_rMediaPacketManager.addMediaPacket(m_uiSourceID, pByte, dataSize + 1, dStartTime, bSynced);
        delete[] pByte;
      }
      else
      {
        // Convert presentation time to double
        double dStartTime = presentationTime.tv_sec + (presentationTime.tv_usec / 1000000.0);
        bool bSynced = (m_pRtpSource) ? !!m_pRtpSource->hasBeenSynchronizedUsingRTCP() : false;
        m_rMediaPacketManager.addMediaPacket(m_uiSourceID, data, dataSize, dStartTime, bSynced);
      }
    }
    else
    {
      m_bFirstPacketDelivered = true;
      // Take dynamic cast hit only the first time
      m_pAmrSource = dynamic_cast<AMRAudioSource*>(fSource);
      processData(data, dataSize, presentationTime);
    }
  }

private:
  /// ID of the sink which is the "source" of the output pin
  unsigned m_uiSourceID;
  /// Session manager
  RtspClientSessionManager& m_rSessionManager;
  /// The handler that processes the media samples
  MediaPacketManager& m_rMediaPacketManager;
  /// Subsession
  MediaSubsession* m_pSubsession;
  /// RTP source
  RTPSource* m_pRtpSource;
  /// AMR source
  AMRAudioSource* m_pAmrSource;
  /// Buffer size
  unsigned m_uiBufferSize;
  /// Buffer for the media sample raw data
  unsigned char* m_pBuffer;
  /// Flag for first packet delivered
  bool m_bFirstPacketDelivered;
};
