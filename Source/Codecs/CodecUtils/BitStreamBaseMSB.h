/** @file

MODULE				: BitStreamBaseMSB

TAG						: BSMSB

FILE NAME			: BitStreamBaseMSB.h

DESCRIPTION		: A base class to contain a contiguous mem array to be
								bit accessible. The mem is not owned by the class. 
								Derived classes handle the reading and writing, 
								BitStreamReaderMSB and BitStreamWriterMSB.

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
#ifndef _BITSTREAMBASEMSB_H
#define _BITSTREAMBASEMSB_H

#pragma once

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class BitStreamBaseMSB
{
public:
	BitStreamBaseMSB()	
		{_bitStream = NULL; _bitSize = 0; _bytePos = 7; _bitPos = 0; }
	virtual ~BitStreamBaseMSB()	{ }

	virtual void SetStream(void* stream, int bitSize)	
		{ _bitStream = (unsigned char *)stream; 
			_bitSize = bitSize; 
			_bytePos = 0; _bitPos = 7; 
		}//end SetStream.

public:
	/// Interface implementation.
	virtual void Reset(void) 
		{ _bytePos = 0; _bitPos = 7; }

	virtual int Seek(int streamBitPos) 
		{ if(streamBitPos >= _bitSize) 
				return(0);
			_bytePos	= streamBitPos / 8; 
			_bitPos		= streamBitPos % 8; 
			return(1); 
		}//end Seek.

	virtual int		GetStreamBitPos(void)					{ return( (_bytePos << 3) +  _bitPos ); }
	virtual void	SetStreamBitSize(int bitSize) { _bitSize = bitSize; }
	virtual int		GetStreamBitSize(void)				{ return(_bitSize); }
	virtual int		GetStreamBitsRemaining(void)	{ return(_bitSize - ((_bytePos << 3) +  _bitPos)); }

protected:
	unsigned char*	_bitStream;		///< Reference to byte array.
	int							_bitSize;			///< Bits in stream.
	/// Current location that act like a cursor in the stream.
	int							_bytePos;
	int							_bitPos;

};// end class BitStreamBaseMSB.

#endif	// _BITSTREAMBASEMSB_H
