/** @file

MODULE				: IBitStreamReader

TAG						: IBSR

FILE NAME			: IBitStreamReader.h

DESCRIPTION		: A IBitStreamReader interface as an abstract base class 
								to BitStreamReader implementations that operate on 
								bit streams. 

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
#ifndef _IBITSTREAMREADER_H
#define _IBITSTREAMREADER_H

#pragma once

class IBitStreamReader
{
public:
	virtual ~IBitStreamReader() {}

/// Interface implementation.
public:
	/** Read a single bit.
	Read from the current bit position in the stream.
	@return	: The bit [0,1].
	*/
	virtual int Read(void) = 0;

	/** Read bits from the stream.
	Read multiple bits from the current stream position.
	@param numBits	: No. of bits to read.
	@return					: The code.
	*/
	virtual int Read(int numBits) = 0;

	/** Peek bits in the stream.
	Read multiple bits from the specified stream position 
	without disturbing the current stream position.
	@param bitLoc		: Bit pos in stream.
	@param numBits	: No. of bits to read.
	@return					: The code.
	*/
	virtual int Peek(int bitLoc, int numBits) = 0;

	/** Set the stream to use.
	Set the pointer to write the bits to and its length in
	bits. Resets the current position to zero.
	@param stream		:	Byte stream pointer.
	@param bitSize	: Length in bits of the stream.
	@return					:	none.
	*/
	virtual void SetStream(void* stream, int bitSize) = 0;	

	/** Get the stream used.
	@return	:	stream pointer.
	*/
  virtual void* GetStream(void) = 0;
  
	/** Reset position.
	Reset the bit position to the front of the stream.
	@return	:	none.
	*/
	virtual void Reset(void) = 0;

	/** Seek to a position.
	Seek the desired bit position from the front of
	the stream.
	@param streamBitPos	: Position to set.
	@return							: Success = 1, Past the end = 0;
	*/
	virtual int Seek(int streamBitPos) = 0;

	/** Get the current position.
	@return	:	bit/byte position.
	*/
	virtual int GetStreamBitPos(void) = 0;
	virtual int GetStreamBytePos(void) = 0;

	/** Set/change the bit length of the stream.
	@param bitSize	: New size in bits.
	@return					: nooe.
	*/
	virtual void SetStreamBitSize(int bitSize) = 0;
	virtual int	 GetStreamBitSize(void) = 0;

	/** Get the remaining bits left on the stream.
	@return	: Bits left.
	*/
	virtual int GetStreamBitsRemaining(void) = 0;

};// end class IBitStreamReader.

#endif	// _IBITSTREAMREADER_H
