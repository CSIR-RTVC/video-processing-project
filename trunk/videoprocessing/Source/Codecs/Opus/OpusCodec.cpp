/** @file

MODULE				: OpusCodec

FILE NAME			: OpusCodec.cpp

DESCRIPTION			:

LICENSE: Software License Agreement (BSD License)

Copyright (c) 2014, CSIR
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
#include <string.h>
#include <stdlib.h>
#include "OpusCodec.h"
#include "opus.h"

const float Opus_Frame_Size[] = 
{
	2.5, 5, 10, 20, 40, 60
};


const char*	OpusCodec::PARAMETER_LIST[] = 
{
	"parameters",
  "samples_per_second",
  "channels",
  "bits_per_sample",
  "max_compr_size"
};


OpusCodec::OpusCodec()
  : m_pOpusEncoder(NULL), 
  m_pOpusDecoder(NULL),
  m_uiSamplesPerSecond(0),
  m_uiChannels(0),
  m_uiBitsPerSample(0),
  m_uiCompressedByteLength(0)
{
}

OpusCodec::~OpusCodec()
{
  if (m_pOpusEncoder || m_pOpusDecoder)
		Close();
}

int	OpusCodec::GetParameter(const char* type, int* length, void* value)
{
	char *p = (char *)type;
	int len	= (int)strlen(p);

	if( _strnicmp(p,"parameters",len) == 0 )
	{
		_itoa_s(sizeof(PARAMETER_LIST)/sizeof(PARAMETER_LIST[0]),(char *)value, *length, 10);
	}
  else if (_strnicmp(p, "samples_per_second", len) == 0)
    _itoa(m_uiSamplesPerSecond, (char *)value, 10);
  else if (_strnicmp(p, "channels", len) == 0)
    _itoa(m_uiChannels, (char *)value, 10);
  else if (_strnicmp(p, "bits_per_sample", len) == 0)
    _itoa(m_uiBitsPerSample, (char *)value, 10);
  else if (_strnicmp(p, "max_compr_size", len) == 0)
    _itoa(m_uiMaxCompressedSize, (char *)value, 10);
	else
	{
		_errorStr = "Opus: Read parameter not supported";
		return(0);
	}

	*length = (int)strlen((char *)value);
	return(1);

	return(0);
}
void OpusCodec::GetParameterName(int ordinal, const char** name, int* length)
{
	if( (ordinal < 0) || (ordinal >= sizeof(PARAMETER_LIST)/sizeof(PARAMETER_LIST[0])) )
		return;
	*name = (const char *)PARAMETER_LIST[ordinal];
	*length = (int)strlen(PARAMETER_LIST[ordinal]);
}
int OpusCodec::SetParameter(const char* type, const char* value)
{
	char *p = (char *)type;
	char *v = (char *)value;
	int  len	= (int)strlen(p);

  if (_strnicmp(p, "samples_per_second", len) == 0)
    m_uiSamplesPerSecond = (int)(atoi(v));
  else if (_strnicmp(p, "channels", len) == 0)
    m_uiChannels = (int)(atoi(v));
  else if (_strnicmp(p, "bits_per_sample", len) == 0)
    m_uiBitsPerSample = (int)(atoi(v));
  else if (_strnicmp(p, "max_compr_size", len) == 0)
    m_uiMaxCompressedSize = (int)(atoi(v));
  else
	{
    _errorStr = "Opus: Write parameter not supported";
		return(0);
	}
	return(1);
}

int	OpusCodec::Code(void* pSrc, void* pCmp, int codeParameter)
{
	if(!m_pOpusEncoder)
	{
		_errorStr = "Opus:[Code] Codec is not open";
		return(0);
	}

  // make sure sample size is valid
  unsigned uiTotalBytes = codeParameter;
  int iDataPerChannel = uiTotalBytes / (m_uiChannels * m_uiBitsPerSample / 8);
  int iDurationMs = iDataPerChannel * 1000 / m_uiSamplesPerSecond;
  // TODO: verify that iDurationMs = the selected frame size e.g. 20 ms

  unsigned char* pSourceBuffer = (unsigned char*)pSrc;
  unsigned char* pDestBuffer = (unsigned char*)pCmp;
#if 0
  // Which size should be set by codeParameter? The size of the sample, or the max compressed size?
  int iMaxCompressedSize = codeParameter;
#else
  int iMaxCompressedSize = m_uiMaxCompressedSize;
#endif
  // TODO: this must be configured prior to using the encoder
  //int frame_size_20_ms = m_uiSamplesPerSecond / 50;
  //int iFrameSizePerChannel = frame_size_20_ms;

  // TODO: when stopping the graph, the buffer might contain to little data to encode.
  // Add a check that when this happens, an error message is returned.
  int iCompressedSize = opus_encode(m_pOpusEncoder, (opus_int16 *)pSourceBuffer, iDataPerChannel, pDestBuffer, iMaxCompressedSize);
  if (iCompressedSize <= 0)
  {
    _errorStr = "[OpusCodec::Code] Error in encoding";
    return 0;
  }
  else if (iCompressedSize == 1)
  {
    // a size of 1 means that it doesn't have to be transmitted
    m_uiCompressedByteLength = 0;
  }
  else if (iCompressedSize > 1)
  {
    // a size of 1 means that it doesn't have to be transmitted
    m_uiCompressedByteLength = iCompressedSize;
  }
  return 1;
}

int	OpusCodec::Decode(void *src, int bitLength, void *dst)
{
  if (!m_pOpusDecoder)
	{
		_errorStr = "Opus:[Decode] Codec is not open";
		return(-1);
	}

  unsigned char* pSourceBuffer = (unsigned char*)src;
  unsigned char* pDestBuffer = (unsigned char*)dst;
  int iUncompressedSize = 0;
  int iFrameSize = m_uiSamplesPerSecond;
  unsigned int lSourceSize = bitLength / 8;
  iUncompressedSize = opus_decode(m_pOpusDecoder, pSourceBuffer, lSourceSize, (opus_int16 *)pDestBuffer, iFrameSize, 0);
  // set to size in bytes!
  m_uiCompressedByteLength = iUncompressedSize * m_uiChannels * sizeof(opus_int16);
  return 1;
}

int OpusCodec::Open(void)
{
	Close();

  int res = 0;
  m_pOpusEncoder = opus_encoder_create(m_uiSamplesPerSecond, m_uiChannels, OPUS_APPLICATION_VOIP, &res);

  if (res != OPUS_OK)
  {
    return 0;
  }

  m_pOpusDecoder = opus_decoder_create(m_uiSamplesPerSecond, m_uiChannels, &res);
  if (res != OPUS_OK)
  {
    return 0;
  }

	return (1);
}

int OpusCodec::Close(void)
{
  if (m_pOpusEncoder)
	{
    opus_encoder_destroy(m_pOpusEncoder);
    m_pOpusEncoder = NULL;
	}

  if (m_pOpusDecoder)
  {
    opus_decoder_destroy(m_pOpusDecoder);
    m_pOpusDecoder = NULL;
  }

	return(1);
}

void OpusCodec::Restart(void)
{
}

void* OpusCodec::GetReference(int refNum)
{
	return 0;
}

char* OpusCodec::GetErrorStr(void)
{ 
	return((char *)_errorStr);
}

int	OpusCodec::Ready(void) 
{ 
  return(m_pOpusEncoder != 0 && 
    m_pOpusDecoder != 0 &&
    m_uiSamplesPerSecond > 0 && 
    m_uiChannels > 0 && 
    m_uiBitsPerSample > 0);
}

int	OpusCodec::GetCompressedBitLength(void) 
{ 
	return GetCompressedByteLength()*8; 
}

int	OpusCodec::GetCompressedByteLength(void) 
{ 
	return m_uiCompressedByteLength; 
}


