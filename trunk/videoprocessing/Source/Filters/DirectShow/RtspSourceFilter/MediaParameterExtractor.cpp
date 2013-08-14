/** @file

MODULE				: RtspSourceFilter

FILE NAME			: MediaParameterExtractor.cpp

DESCRIPTION			: Class to extract media parameters inband
					  
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
#include "MediaParameterExtractor.h"

// RTVC
#include <Shared/StringUtil.h>

MediaParameterExtractor::MediaParameterExtractor()
{

}

bool MediaParameterExtractor::addMediaChannel( unsigned uiChannelId, MediaSubsession* pSubsession )
{
  // Make sure each entry is unique
  MediaSubsessionMap_t::iterator it = m_mMediaSubsessionTypes.find(uiChannelId);
  assert( it == m_mMediaSubsessionTypes.end() );
  m_mMediaSubsessionTypes[uiChannelId] = pSubsession;

  return initialiseParameterExtractionMap(uiChannelId, pSubsession);
}

bool MediaParameterExtractor::haveAllParametersBeenExtracted() const
{
  for (InitialisationMap_t::const_iterator it = m_mInitMap.begin(); it != m_mInitMap.end(); ++it)
  {
    if (!it->second) return false;
  }
  return true;
}

StringMap_t& MediaParameterExtractor::getParameterMap( MediaSubsession* pSubsession )
{
  return m_mSubsessionParameters[pSubsession];
}

void MediaParameterExtractor::extractMediaParametersFromMediaData( unsigned uiID, unsigned char* data, unsigned dataSize )
{
  // process sample
  if (m_mInitMap[uiID] == false)
  {
    // try and parse data according to subsession type
    const char* szCodec = m_mMediaSubsessionTypes[uiID]->codecName();

    if ( strcmp(szCodec, "MPA")==0 )
    {
      MediaSubsession* pSubsession = m_mMediaSubsessionTypes[uiID];
      StringMap_t& rParameterMap = m_mSubsessionParameters[pSubsession];
      if (parseMp3HeaderInfo( data, dataSize, rParameterMap ))
      {
        m_mInitMap[uiID] = true;
      }
    }
    else
    {
      // Unsupported media type
      assert(false);
    }
  }
}

bool MediaParameterExtractor::initialiseParameterExtractionMap( unsigned uiChannelId, MediaSubsession* pSubsession )
{
  const char* szMedium = pSubsession->mediumName();
  const char* szCodec = pSubsession->codecName();

  bool bResult = true;
  if ( strcmp(szMedium, "audio")==0 )
  {
    if (( strcmp(szCodec, "L8")==0 ) ||
      (strcmp(szCodec, "L16")==0) ||
      (strcmp(szCodec, "AMR")==0))
    {
      m_mInitMap[uiChannelId] = true;
    }
    // MP3
    else if ( strcmp(szCodec, "MPA")==0 )
    {
      m_mInitMap[uiChannelId] = false;
    }
    // AAC audio
    else if ( strcmp(szCodec, "MPEG4-GENERIC")==0 )
    {
      m_mInitMap[uiChannelId] = true;
    }
    else if ( strcmp(szCodec, "MP4A-LATM")==0 )
    {
      m_mInitMap[uiChannelId] = true;
    }
    else
    {
      // Unsupported audio codec
      bResult = false;
    }
  }
  else if ( strcmp(szMedium, "video")==0 )
  {
    if ( strcmp(szCodec, "H264")==0 )
    {
      m_mInitMap[uiChannelId] = true;
    }
    else
    {
      // Unsupported video codec
      bResult = false;
    }
  }
  return bResult;
}

bool MediaParameterExtractor::parseMp3HeaderInfo( unsigned char* data, unsigned dataSize, StringMap_t& rParameterMap )
{
  // parse audio version ID
  unsigned uiAudioVersionId = ((data[1]>>3) & 0x03);
  // parse sampling rate index
  unsigned uiSamplingRateIndex = ((data[2]>>2) & 0x03);
  switch (uiAudioVersionId)
  {
  case MPEG_VERSION_2_5:
    {
      switch (uiSamplingRateIndex)
      {
      case 0:
        {
          rParameterMap[SAMPLING_RATE] = "11025";
          return true;
        }
      case 1:
        {
          rParameterMap[SAMPLING_RATE] = "12000";
          return true;
        }
      case 2:
        {
          rParameterMap[SAMPLING_RATE] = "8000";
          return true;
        }
      case 3:
        {
          return false;
        }
      }
      break;
    }
  case MPEG_VERSION_2:
    {
      switch (uiSamplingRateIndex)
      {
      case 0:
        {
          rParameterMap[SAMPLING_RATE] = "22050";
          return true;
        }
      case 1:
        {
          rParameterMap[SAMPLING_RATE] = "24000";
          return true;
        }
      case 2:
        {
          rParameterMap[SAMPLING_RATE] = "16000";
          return true;
        }
      case 3:
        {
          return false;
        }
      }
      break;
    }
  case MPEG_VERSION_1:
    {
      switch (uiSamplingRateIndex)
      {
      case 0:
        {
          rParameterMap[SAMPLING_RATE] = "44100";
          return true;
        }
      case 1:
        {
          rParameterMap[SAMPLING_RATE] = "48000";
          return true;
        }
      case 2:
        {
          rParameterMap[SAMPLING_RATE] = "32000";
          return true;
        }
      case 3:
        {
          return false;
        }
      }
      break;
    }
  }
  return false;
}