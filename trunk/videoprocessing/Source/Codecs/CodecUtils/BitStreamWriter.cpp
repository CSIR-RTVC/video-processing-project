/** @file

MODULE						: BitStreamWriter

TAG								: BSW

FILE NAME					: BitStreamWriter.cpp

DESCRIPTION				: A bit stream writer implementation of the BitStreamBase
										base class. Add the functionality to do the writing.

REVISION HISTORY	:
									: 

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
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include "BitStreamWriter.h"

BitStreamWriter::BitStreamWriter()
{
}//end constructor.

BitStreamWriter::~BitStreamWriter()
{
}//end destructor.

/** Write a single bit.
Write to the current bit position in the stream.
@param val	: Bit value to write.
@return			: none.
*/
void BitStreamWriter::Write(int val)
{
  // Strip out the LSB bit of the input and write it to the bit position.
  if(val & 1)
		_bitStream[_bytePos] = _bitStream[_bytePos] | (1 << _bitPos);
  else
    _bitStream[_bytePos] = _bitStream[_bytePos] & ~(1 << _bitPos);

  // Point to next available bit.
  if(_bitPos < 7)
    _bitPos++;
	else
  {
		// Update the bit stream location.
    _bitPos = 0;
    _bytePos++;
  }//end else...
}//end Write.

/** Write bits to the stream.
Write multiple bits from the least significant bit upwards
into the current stream position.
@param numBits	: No. of bits to write.
@param val			: Bit value to write.
@return					: none.
*/
void BitStreamWriter::Write(int numBits, int val)
{
  int pos = _bitPos;
  int b		= val;
	unsigned char cachedByte = _bitStream[_bytePos];

  for(int i = numBits; i > 0; i--)
  {
    // Strip out the LSB bit of the input and write it to bit position pos.
    if(b & 1)
			cachedByte = cachedByte | (1 << pos);
    else
      cachedByte = cachedByte & ~(1 << pos);

    // Point to next available bit.
    if(pos < 7)
			pos++;
		else
    {
			// Update the bit stream from cache.
			_bitStream[_bytePos] = cachedByte;
      pos = 0;
      _bytePos++;
			// Renew the cache.
			cachedByte = _bitStream[_bytePos];
    }//end else...

    // Update the bits still to add.
    b = b >> 1;
  }//end for i...

	// Flush the cache.
	_bitStream[_bytePos] = cachedByte;
  // Update the global next bit position.
  _bitPos = pos;
}//end Write.

/** Poke bits to the stream.
Write multiple bits from the least significant bit upwards
into the specified stream position without disturbing the
current stream position.
@param bitLoc		: Bit pos in stream.
@param numBits	: No. of bits to write.
@param val			: Bit value to write.
@return					: none.
*/
void BitStreamWriter::Poke(int bitLoc, int numBits, int val)
{
	int bytePos = bitLoc / 8;
  int bitPos	= bitLoc % 8;
  int b				= val;

  for(int i = numBits; i > 0; i--)
  {
    // Strip out the LSB bit of the input and write it to bit position.
    if(b & 1)
			_bitStream[bytePos] = _bitStream[bytePos] | (1 << bitPos);
    else
      _bitStream[bytePos] = _bitStream[bytePos] & ~(1 << bitPos);

    // Point to next available bit.
    if(bitPos < 7)
			bitPos++;
		else
    {
      bitPos = 0;
      bytePos++;
    }//end else...

    // Update the bits still to add.
    b = b >> 1;
  }//end for i...

}//end Poke.


