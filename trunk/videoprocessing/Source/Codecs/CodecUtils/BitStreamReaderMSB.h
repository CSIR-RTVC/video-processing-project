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

LICENSE	: GNU Lesser General Public License

Copyright (c) 2008 - 2013, CSIR
All rights reserved.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.


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

	void* GetStream(void) { return(BitStreamBaseMSB::GetStream()); }	

	void Reset(void) { BitStreamBaseMSB::Reset(); }

	int Seek(int streamBitPos) { return(BitStreamBaseMSB::Seek(streamBitPos)); }

	int GetStreamBitPos(void) { return(BitStreamBaseMSB::GetStreamBitPos()); }

	int GetStreamBytePos(void) { return(BitStreamBaseMSB::GetStreamBytePos()); }

	void SetStreamBitSize(int bitSize) { BitStreamBaseMSB::SetStreamBitSize(bitSize); }

	int GetStreamBitSize(void) { return(BitStreamBaseMSB::GetStreamBitSize()); }

	int GetStreamBitsRemaining(void) { return(BitStreamBaseMSB::GetStreamBitsRemaining()); }

	void Copy(IBitStreamReader* pFrom)
    {
      _bitStream  = (unsigned char*)pFrom->GetStream();
      _bitSize    = pFrom->GetStreamBitSize();
      _bytePos    = pFrom->GetStreamBytePos();
      _bitPos     = pFrom->GetStreamBitPos() % 8;
    }//end Copy.
};// end class BitStreamReaderMSB.

#endif	// _BITSTREAMREADERMSB_H
