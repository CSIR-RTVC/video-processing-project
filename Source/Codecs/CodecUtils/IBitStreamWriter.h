/** @file

MODULE				: IBitStreamWriter

TAG						: IBSW

FILE NAME			: IBitStreamWriter.h

DESCRIPTION		: A IBitStreamWriter interface as an abstract base class 
								to BitStreamWriter implementations that operate on 
								bit streams. 

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
#ifndef _IBITSTREAMWRITER_H
#define _IBITSTREAMWRITER_H

#pragma once

class IBitStreamWriter
{
public:
	virtual ~IBitStreamWriter() {}

/// Interface implementation.
public:
	/** Write a single bit.
	Write to the current bit position into the stream.
	@param val	: Bit value to write.
	@return			: none.
	*/
	virtual void Write(int val) = 0;

	/** Write bits to the stream - 32 bits.
	Write multiple bits into the current stream position for 32 bit values.
	@param numBits	: No. of bits to write.
	@param val			: Bit value to write.
	@return					: none.
	*/
	virtual void Write(int numBits, int val) = 0;

	/** Write bits to the stream - 64 bits.
	Write multiple bits into the current stream position for 64 bit values.
	@param numBits	: No. of bits to write.
	@param valH			: Upper 32 bit value to write.
	@param valL			: Lower 32 bit value to write.
	@return					: none.
	*/
  virtual void Write(int numBits, int valH, int valL) = 0;

	/** Poke bits to the stream.
	Write multiple bits from into the specified stream 
	position without disturbing the	current stream position.
	@param bitLoc		: Bit pos in stream.
	@param numBits	: No. of bits to write.
	@param val			: Bit value to write.
	@return					: none.
	*/
	virtual void Poke(int bitLoc, int numBits, int val) = 0;

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
	@return					: none.
	*/
	virtual void SetStreamBitSize(int bitSize) = 0;
	virtual int	 GetStreamBitSize(void) = 0;

	/** Get the remaining bits left on the stream.
	@return	: Bits left.
	*/
	virtual int GetStreamBitsRemaining(void) = 0;

	/** Copy the contents from another bitstream.
  @param pFrom  : Bitstream to copy from.
	@return	      : none.
	*/
  virtual void  Copy(IBitStreamWriter* pFrom)  = 0;

};// end class IBitStreamWriter.

#endif	// _IBITSTREAMWRITER_H
