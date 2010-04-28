/** @file

MODULE				: RtspSourceFilter

FILE NAME			: MediaParameterExtractor.h

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
#pragma once

// Forward
class MediaSubsession;

/**
 * \ingroup DirectShowFilters
 * Some media formats require that certain media parameters are extracted inband. Since DS requires the media type
 * to be negotiated during pipeline setup, we use this class to extract format-specific parameters.
 */
class MediaParameterExtractor
{
  typedef std::map<unsigned, MediaSubsession*>  MediaSubsessionMap_t;
  typedef std::map<MediaSubsession*, StringMap_t>  SubsessionParameterMap_t;
  typedef std::map<unsigned, bool> InitialisationMap_t;

public:

  MediaParameterExtractor();

  void clear()
  {
    m_mMediaSubsessionTypes.clear();
    m_mSubsessionParameters.clear();
    m_mInitMap.clear();
  }

  bool addMediaChannel(unsigned uiChannelId, MediaSubsession* pSubsession);

  bool haveAllParametersBeenExtracted() const;

  StringMap_t& getParameterMap(MediaSubsession* pSubsession);

  void extractMediaParametersFromMediaData( unsigned uiID, unsigned char* data, unsigned dataSize );

private:

  bool initialiseParameterExtractionMap(unsigned uiChannelId, MediaSubsession* pSubsession);
  bool parseMp3HeaderInfo( unsigned char* data, unsigned dataSize, StringMap_t& rParameterMap);

  enum Mp3AudioVersion
  {
    MPEG_VERSION_2_5  = 0,
    RESERVED          = 1,
    MPEG_VERSION_2    = 2,
    MPEG_VERSION_1    = 3
  };

  enum Mp3SamplingRates
  {
    MPEG_1_44100,
    MPEG_1_48000,
    MPEG_1_32000,
    MPEG_2_22050,
    MPEG_2_24000,
    MPEG_2_16000,
    MPEG_2_5_11025,
    MPEG_2_5_12000,
    MPEG_2_5_8000,
    MPEG_RESERVED    
  };

  MediaSubsessionMap_t m_mMediaSubsessionTypes;
  SubsessionParameterMap_t m_mSubsessionParameters;
  InitialisationMap_t m_mInitMap;
};
