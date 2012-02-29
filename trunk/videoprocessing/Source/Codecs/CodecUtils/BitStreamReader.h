/** @file

MODULE				: BitStreamReader

TAG						: BSR

FILE NAME			: BitStreamReader.h

DESCRIPTION		: A bit stream reader implementation of the BitStreamBase
								base class. Add the functionality to do the bit reading.
								Basic operation:
									BitStreamReader* pBsr = new BitStreamReader();
									pBsr->SetStream((void *)pStream, (streamLen * sizeof(pStream[0]) * 8));
									pBsr->Read(5, codeWord);
									.
									.
									delete pBsr;

REVISION HISTORY	:

COPYRIGHT					:

RESTRICTIONS			: 
===========================================================================
*/
#ifndef _BITSTREAMREADER_H
#define _BITSTREAMREADER_H

#pragma once

#include "BitStreamBase.h"
#include "IBitStreamReader.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class BitStreamReader : public BitStreamBase, public IBitStreamReader
{
public:
	BitStreamReader();
	virtual ~BitStreamReader();

// Interface implementation.
public:
	/** Read a single bit.
	Read from the current bit position in the stream.
	@param val	: Bit value to write.
	@return			: The bit [0,1].
	*/
	int Read(void);

	/** Read bits from the stream.
	Read multiple bits from the least significant bit upwards
	from the current stream position.
	@param numBits	: No. of bits to read.
	@return					: The code.
	*/
	int Read(int numBits);

	/** Peek bits in the stream.
	Read multiple bits from the least significant bit upwards
	from the specified stream position without disturbing the
	current stream position.
	@param bitLoc		: Bit pos in stream.
	@param numBits	: No. of bits to read.
	@return					: The code.
	*/
	int Peek(int bitLoc, int numBits);

//	Use base class implementations.
public:
	void SetStream(void* stream, int bitSize) { BitStreamBase::SetStream(stream, bitSize); }	

	void* GetStream(void) { return(BitStreamBase::GetStream()); }	

	void Reset(void) { BitStreamBase::Reset(); }

	int Seek(int streamBitPos) { return(BitStreamBase::Seek(streamBitPos)); }

	int GetStreamBitPos(void) { return(BitStreamBase::GetStreamBitPos()); }

	int GetStreamBytePos(void) { return(BitStreamBase::GetStreamBytePos()); }

	void SetStreamBitSize(int bitSize) { BitStreamBase::SetStreamBitSize(bitSize); }

};// end class BitStreamReader.

#endif	// _BITSTREAMREADER_H
