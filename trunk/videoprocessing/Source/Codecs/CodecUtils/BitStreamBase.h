/** @file

MODULE						: BitStreamBase

TAG								: BS

FILE NAME					: BitStreamBase.h

DESCRIPTION				: A base class to contain a contiguous mem array to be
										bit accessible. The mem is not owned by the class. 
										Derived classes handle the reading and writing, 
										BitStreamReader and BitStreamWriter.

REVISION HISTORY	:

COPYRIGHT					:

RESTRICTIONS			: 
===========================================================================
*/
#ifndef _BITSTREAMBASE_H
#define _BITSTREAMBASE_H

#pragma once

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class BitStreamBase
{
public:
	BitStreamBase()	
		{_bitStream = NULL; _bitSize = 0; _bytePos = 0; _bitPos = 0; }
	virtual ~BitStreamBase()	{ }

	virtual void SetStream(void* stream, int bitSize)	
		{ _bitStream = (unsigned char *)stream; 
			_bitSize = bitSize; 
			_bytePos = 0; _bitPos = 0; 
		}//end SetStream.

  virtual void* GetStream(void) { return( (void *)_bitStream ); }

public:
	/// Interface implementation.
	virtual void Reset(void) 
		{ _bytePos = 0; _bitPos = 0; }

	virtual int Seek(int streamBitPos) 
		{ if(streamBitPos >= _bitSize) 
				return(0);
			_bytePos	= streamBitPos / 8; 
			_bitPos		= streamBitPos % 8; 
			return(1); 
		}//end Seek.

	virtual int		GetStreamBitPos(void)					{ return( (_bytePos << 3) +  _bitPos); }
	virtual int		GetStreamBytePos(void)				{ return(_bytePos); }
	virtual void	SetStreamBitSize(int bitSize) { _bitSize = bitSize; }
	virtual int		GetStreamBitSize(void)				{ return(_bitSize); }

protected:
	unsigned char*	_bitStream;		///< Reference to byte array.
	int							_bitSize;			///< Bits in stream.
	/// Current location.
	int							_bytePos;
	int							_bitPos;

};// end class BitStreamBase.

#endif	// _BITSTREAMBASE_H
