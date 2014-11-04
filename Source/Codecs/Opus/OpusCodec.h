/** @file

MODULE				: OpusCodec

FILE NAME			: OpusCodec.h

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
#pragma once
#include "ICodecv2.h"

// fwd
typedef struct OpusDecoder OpusDecoder;
typedef struct OpusEncoder OpusEncoder;

/**
 * @brief Opus ICodecv2 Interface Implementation
 */
class OpusCodec : public ICodecv2
{
public:
	OpusCodec();
	virtual ~OpusCodec();
public:
	int		GetParameter(const char* type, int* length, void* value);
	void	GetParameterName(int ordinal, const char** name, int* length);
	int		SetParameter(const char* type, const char* value);
	char* GetErrorStr(void);
	int		Ready(void);
	int		GetCompressedBitLength(void);
	int		GetCompressedByteLength(void);
	void* GetReference(int refNum);

	void	Restart(void);
	int		Open(void);
	int		Close(void);
  /**
   * @brief 
   * @param codeParameter Size of in-buffer pSrc in bytes
   */
	int		Code(void* pSrc, void* pCmp, int codeParameter);
	int		Decode(void* pCmp, int bitLength, void* pDst);

private:

	static const char* PARAMETER_LIST[];
	const char * _errorStr;
  /// handle to encoder state
  OpusEncoder* m_pOpusEncoder;
  /// handle to decoder state
  OpusDecoder* m_pOpusDecoder;
  /// samples per second of source
  unsigned int m_uiSamplesPerSecond;
  /// number of channels of source
  unsigned int m_uiChannels;
  /// bits per sample of source
  unsigned int m_uiBitsPerSample;
  /// the maximum number of bytes to be used when compressing
  unsigned int m_uiMaxCompressedSize;
  /// size in bytes of last compressed frame
  unsigned m_uiCompressedByteLength;
};
