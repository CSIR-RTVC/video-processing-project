/** @file

MODULE				: BitStreamReaderMSB

TAG						: BSRMSB

FILE NAME			: BitStreamReaderMSB.h

DESCRIPTION		: A bit stream reader implementation of the BitStreamBase
								base class with the first bit as the MSB of the byte. 
								Add the functionality to do the bit reading.
								Basic operation:
									BitStreamReaderMSB* pBsr = new BitStreamReaderMSB();
									pBsr->SetStream((void *)pStream, (streamLen * sizeof(pStream[0]) * 8));
									pBsr->Read(5, codeWord);
									.
									.
									delete pBsr;

COPYRIGHT			: (c)CSIR 2007-2010 all rights resevered

LICENSE				: Software License Agreement (BSD License)

RESTRICTIONS	: Redistribution and use in source and binary forms, with or without 
								modification, are permitted provided that the following conditions 
								are met:

								* Redistributions of source code must retain the above copyright notice, 
								this list of conditions and the following disclaimer.
								* Redistributions in binary form must reproduce the above copyright notice, 
								this list of conditions and the following disclaimer in the documentation 
								and/or other materials provided with the distribution.
								* Neither the name of the CSIR nor the names of its contributors may be used 
								to endorse or promote products derived from this software without specific 
								prior written permission.

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
#ifndef _BITSTREAMREADERMSB_H
#define _BITSTREAMREADERMSB_H

#pragma once

#include "BitStreamBaseMSB.h"
#include "IBitStreamReader.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class BitStreamReaderMSB : public BitStreamBaseMSB, public IBitStreamReader
{
public:
	BitStreamReaderMSB();
	virtual ~BitStreamReaderMSB();

/// Interface implementation.
public:
	/** Read a single bit.
	Read from the current bit position in the stream.
	@param val	: Bit value to write.
	@return			: The bit [0,1].
	*/
	int Read(void);

	/** Read bits from the stream.
	Read multiple bits from the most significant bit downwards
	from the current stream position.
	@param numBits	: No. of bits to read.
	@return					: The code.
	*/
	int Read(int numBits);

	/** Peek bits in the stream.
	Read multiple bits from the most significant bit downwards
	from the specified stream position without disturbing the
	current stream position.
	@param bitLoc		: Bit pos in stream.
	@param numBits	: No. of bits to read.
	@return					: The code.
	*/
	int Peek(int bitLoc, int numBits);

///	Use base class implementations.
public:
	void SetStream(void* stream, int bitSize) { BitStreamBaseMSB::SetStream(stream, bitSize); }	

	void Reset(void) { BitStreamBaseMSB::Reset(); }

	int Seek(int streamBitPos) { return(BitStreamBaseMSB::Seek(streamBitPos)); }

	int GetStreamBitPos(void) { return(BitStreamBaseMSB::GetStreamBitPos()); }

	void SetStreamBitSize(int bitSize) { BitStreamBaseMSB::SetStreamBitSize(bitSize); }

	int GetStreamBitSize(void) { return(BitStreamBaseMSB::GetStreamBitSize()); }

	int GetStreamBitsRemaining(void) { return(BitStreamBaseMSB::GetStreamBitsRemaining()); }

};// end class BitStreamReaderMSB.

#endif	// _BITSTREAMREADERMSB_H
