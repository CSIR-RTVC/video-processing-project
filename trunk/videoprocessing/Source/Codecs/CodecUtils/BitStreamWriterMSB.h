/** @file

MODULE				: BitStreamWriterMSB

TAG						: BSWMSB

FILE NAME			: BitStreamWriterMSB.h

DESCRIPTION		: A bit stream writer implementation of the extending the
								BitStreamBase	base class with the first bit as the MSB 
								of the byte. Implements a IBitStreamWriter interface. 
								Add the functionality to do the bit writing.
								Basic operation:
									BitStreamWriterMSB* pBsr= new BitStreamWriterMSB();
									pBsr->SetStream((void *)pStream, (streamLen * sizeof(pStream[0]) * 8));
									pBsr->Write(5, codeWord);
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
#ifndef _BITSTREAMWRITERMSB_H
#define _BITSTREAMWRITERMSB_H

#pragma once

#include "BitStreamBaseMSB.h"
#include "IBitStreamWriter.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class BitStreamWriterMSB : public BitStreamBaseMSB, public IBitStreamWriter
{
public:
	BitStreamWriterMSB();
	virtual ~BitStreamWriterMSB();

// Interface implementation.
public:
	/** Write a single bit.
	Write to the current bit position into the stream.
	@param val	: Bit value to write.
	@return			: none.
	*/
	void Write(int val);

	/** Write bits to the stream 32 bits.
	Write multiple bits from the most significant bit downwards
	into the current stream position.
	@param numBits	: No. of bits to write.
	@param val			: Bit value to write.
	@return					: none.
	*/
	void Write(int numBits, int val);

	/** Write bits to the stream - 64 bits.
	Write multiple bits into the current stream position for 64 bit values.
	@param numBits	: No. of bits to write.
	@param valH			: Upper 32 bit value to write.
	@param valL			: Lower 32 bit value to write.
	@return					: none.
	*/
	void Write(int numBits, int valH, int valL)
  {
    if(numBits > 32)
    {
      Write(numBits-32, valH);  ///< MSBs first.
      Write(32, valL);
    }//end if numBits...
    else
      Write(numBits, valL);
  }//end Write.

	/** Poke bits to the stream.
	Write multiple bits from the most significant bit downwards
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
	void SetStream(void* stream, int bitSize) { BitStreamBaseMSB::SetStream(stream, bitSize); }	

	void* GetStream(void) { return(BitStreamBaseMSB::GetStream()); }	

	void Reset(void) { BitStreamBaseMSB::Reset(); }

	int Seek(int streamBitPos) { return(BitStreamBaseMSB::Seek(streamBitPos)); }

	int GetStreamBitPos(void) { return(BitStreamBaseMSB::GetStreamBitPos()); }

	int GetStreamBytePos(void) { return(BitStreamBaseMSB::GetStreamBytePos()); }

	void SetStreamBitSize(int bitSize) { BitStreamBaseMSB::SetStreamBitSize(bitSize); }

	int GetStreamBitSize(void) { return(BitStreamBaseMSB::GetStreamBitSize()); }

	int GetStreamBitsRemaining(void) { return(BitStreamBaseMSB::GetStreamBitsRemaining()); }

  void Copy(IBitStreamWriter* pFrom)
  {
    _bitStream  = (unsigned char*)pFrom->GetStream();
    _bitSize    = pFrom->GetStreamBitSize();
    _bytePos    = pFrom->GetStreamBytePos();
    _bitPos     = pFrom->GetStreamBitPos() % 8;
  }//end Copy.

};// end class BitStreamWriterMSB.

#endif	// _BITSTREAMWRITERMSB_H
