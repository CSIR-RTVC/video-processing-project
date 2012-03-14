/** @file

MODULE				: BitStreamBaseMSB

TAG						: BSMSB

FILE NAME			: BitStreamBaseMSB.h

DESCRIPTION		: A base class to contain a contiguous mem array to be
								bit accessible. The mem is not owned by the class. 
								Derived classes handle the reading and writing, 
								BitStreamReaderMSB and BitStreamWriterMSB.

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
		{_bitStream = NULL; _bitSize = 0; _bytePos = 0; _bitPos = 7; }
	virtual ~BitStreamBaseMSB()	{ }

	virtual void SetStream(void* stream, int bitSize)	
		{ _bitStream = (unsigned char *)stream; 
			_bitSize = bitSize; 
			_bytePos = 0; _bitPos = 7; 
		}//end SetStream.

  virtual void* GetStream(void) { return( (void *)_bitStream ); }

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
	virtual int		GetStreamBytePos(void)				{ return(_bytePos); }
	virtual void	SetStreamBitSize(int bitSize) { _bitSize = bitSize; }
	virtual int		GetStreamBitSize(void)				{ return(_bitSize); }
	virtual int		GetStreamBitsRemaining(void)	{ return(_bitSize - ((_bytePos << 3) +  (7 - _bitPos))); }

protected:
	unsigned char*	_bitStream;		///< Reference to byte array.
	int							_bitSize;			///< Bits in stream.
	/// Current location that acts like a cursor in the stream.
	int							_bytePos;
	int							_bitPos;

};// end class BitStreamBaseMSB.

#endif	// _BITSTREAMBASEMSB_H
