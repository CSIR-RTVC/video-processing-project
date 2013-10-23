/** @file

MODULE				: BitStreamWriterMSB

TAG						: BSWMSB

FILE NAME			: BitStreamWriterMSB.cpp

DESCRIPTION		: A bit stream writer implementation of the BitStreamBase
								base class with the first bit as the MSB of the byte. 
								Add the functionality to do the writing.
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
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include "BitStreamWriterMSB.h"

BitStreamWriterMSB::BitStreamWriterMSB()
{
}//end constructor.

BitStreamWriterMSB::~BitStreamWriterMSB()
{
}//end destructor.

/** Write a single bit.
Write to the current bit position in the stream.
@param val	: Bit value to write.
@return			: none.
*/
void BitStreamWriterMSB::Write(int val)
{
  // Strip out the bit of the input and write it to the bit position.
  if(val & 1)
		_bitStream[_bytePos] = _bitStream[_bytePos] | (1 << _bitPos);
  else
    _bitStream[_bytePos] = _bitStream[_bytePos] & ~(1 << _bitPos);

  // Point to next available bit.
  if(_bitPos > 0)
    _bitPos--;
	else
  {
		// Update the bit stream location.
    _bitPos = 7;
    _bytePos++;
  }//end else...
}//end Write.

/** Write bits to the stream.
Write multiple bits from the most significant bit downwards
into the current stream position.
@param numBits	: No. of bits to write.
@param val			: Bit value to write.
@return					: none.
*/
void BitStreamWriterMSB::Write(int numBits, int val)
{
  int pos = _bitPos;
  int b		= val;
	unsigned char cachedByte = _bitStream[_bytePos];

	unsigned int inMask = 1 << (numBits - 1);	// Align a 1 on the MSB of the input val.
  for(int i = numBits; i > 0; i--)
  {
    // Strip out the bit of the input from the MSB valid bit downwards 
		// and write it to bit position pos.
    if(b & inMask)
			cachedByte = cachedByte | (1 << pos);
    else
      cachedByte = cachedByte & ~(1 << pos);

    // Point to next available bit.
    if(pos > 0)
			pos--;
		else
    {
			// Update the bit stream from cache.
			_bitStream[_bytePos] = cachedByte;
      pos = 7;
      _bytePos++;
			// Renew the cache.
			cachedByte = _bitStream[_bytePos];
    }//end else...

    // Update the input mask to the next MSB.
    inMask = inMask >> 1;
  }//end for i...

	// Flush the cache.
	_bitStream[_bytePos] = cachedByte;
  // Update the global next bit position.
  _bitPos = pos;
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
void BitStreamWriterMSB::Poke(int bitLoc, int numBits, int val)
{
	int bytePos = bitLoc / 8;
  int bitPos	= bitLoc % 8;
  int b				= val;

	int inMask = 1 << (numBits - 1);	// Align a 1 on the MSB of the input val.
  for(int i = numBits; i > 0; i--)
  {
    // Strip out the LSB bit of the input and write it to bit position.
    if(b & inMask)
			_bitStream[bytePos] = _bitStream[bytePos] | (1 << bitPos);
    else
      _bitStream[bytePos] = _bitStream[bytePos] & ~(1 << bitPos);

    // Point to next available bit.
    if(bitPos > 0)
			bitPos--;
		else
    {
      bitPos = 7;
      bytePos++;
    }//end else...

    // Update the input mask to the next MSB.
    inMask = inMask >> 1;
  }//end for i...

}//end Poke.


