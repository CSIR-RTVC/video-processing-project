/** @file

MODULE				: BitStreamWriter

TAG						: BSW

FILE NAME			: BitStreamWriter.h

DESCRIPTION		: A bit stream writer implementation of the BitStreamBase
								base class. Add the functionality to do the bit writing.
								Basic operation:
									BitStreamWriter* pBsr= new BitStreamWriter();
									pBsr->SetStream((void *)pStream, (streamLen * sizeof(pStream[0]) * 8));
									pBsr->Write(5, codeWord);
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
#ifndef _BITSTREAMWRITER_H
#define _BITSTREAMWRITER_H

#pragma once

#include "BitStreamBase.h"
#include "IBitStreamWriter.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class BitStreamWriter : public BitStreamBase, public IBitStreamWriter
{
public:
	BitStreamWriter();
	virtual ~BitStreamWriter();

/// Interface implementation.
public:
	/** Write a single bit.
	Write to the current bit position into the stream.
	@param val	: Bit value to write.
	@return			: none.
	*/
	void Write(int val);

	/** Write bits to the stream.
	Write multiple bits from the least significant bit upwards
	into the current stream position.
	@param numBits	: No. of bits to write.
	@param val			: Bit value to write.
	@return					: none.
	*/
	void Write(int numBits, int val);

	/** Poke bits to the stream.
	Write multiple bits from the least significant bit upwards
	into the specified stream position without disturbing the
	current stream position.
	@param bitLoc		: Bit pos in stream.
	@param numBits	: No. of bits to write.
	@param val			: Bit value to write.
	@return					: none.
	*/
	void Poke(int bitLoc, int numBits, int val);

///	Use base class implementations.
public:
	void SetStream(void* stream, int bitSize) { BitStreamBase::SetStream(stream, bitSize); }	

	void* GetStream(void) { return(BitStreamBase::GetStream()); }	

	void Reset(void) { BitStreamBase::Reset(); }

	int Seek(int streamBitPos) { return(BitStreamBase::Seek(streamBitPos)); }

	int GetStreamBitPos(void) { return(BitStreamBase::GetStreamBitPos()); }

	int GetStreamBytePos(void) { return(BitStreamBase::GetStreamBytePos()); }

	void SetStreamBitSize(int bitSize) { BitStreamBase::SetStreamBitSize(bitSize); }

};// end class BitStreamWriter.

#endif	// _BITSTREAMWRITER_H
