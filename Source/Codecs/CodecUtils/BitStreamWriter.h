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

LICENSE	: GNU Lesser General Public License

Copyright (c) 2008 - 2012, CSIR
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
