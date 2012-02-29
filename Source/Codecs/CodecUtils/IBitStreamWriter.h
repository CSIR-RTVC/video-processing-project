/** @file

MODULE				: IBitStreamWriter

TAG						: IBSW

FILE NAME			: IBitStreamWriter.h

DESCRIPTION		: A IBitStreamWriter interface as an abstract base class 
								to BitStreamWriter implementations that operate on 
								bit streams. 

COPYRIGHT			: (c)CSIR 2007-2011 all rights resevered

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

	/** Write bits to the stream.
	Write multiple bits into the current stream position.
	@param numBits	: No. of bits to write.
	@param val			: Bit value to write.
	@return					: none.
	*/
	virtual void Write(int numBits, int val) = 0;

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

};// end class IBitStreamWriter.

#endif	// _IBITSTREAMWRITER_H
