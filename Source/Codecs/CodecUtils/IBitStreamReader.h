/** @file

MODULE				: IBitStreamReader

TAG						: IBSR

FILE NAME			: IBitStreamReader.h

DESCRIPTION		: A IBitStreamReader interface as an abstract base class 
								to BitStreamReader implementations that operate on 
								bit streams. 

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
