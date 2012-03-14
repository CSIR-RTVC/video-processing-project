/** @file

MODULE						: BitStreamReader

TAG								: BSR

FILE NAME					: BitStreamReader.cpp

DESCRIPTION				: A bit stream reader implementation of the BitStreamBase
										base class. Add the functionality to do the reading.

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

#include "BitStreamReader.h"

BitStreamReader::BitStreamReader()
{
}//end constructor.

BitStreamReader::~BitStreamReader()
{
}//end destructor.

/** Read a single bit.
Read from the current bit position in the stream.
@param val	: Bit value to write.
@return			: The bit [0,1].
*/
int BitStreamReader::Read(void)
{
  // Strip out the bit. 
  int codeBit = (int)(_bitStream[_bytePos] >> _bitPos) & 1;

  // Point to next available bit.
  if(_bitPos < 7)
		_bitPos++;
	else
  {
    _bitPos = 0;
    _bytePos++;
  }//end else...

  return(codeBit);
}//end Read.

/** Read bits from the stream.
Read multiple bits from the least significant bit upwards
from the current stream position.
@param numBits	: No. of bits to read.
@return					: The code.
*/
int BitStreamReader::Read(int numBits)
{
  int pos = _bitPos;
  int b		= 0;

  for(int i = 0; i < numBits; i++)
  {
    // Strip out the next bit and update in the bit position i.
    if((_bitStream[_bytePos] >> pos) & 1)
      b = (int)(b | (1 << i));	// bit=1
    else
      b = (int)(b & ~(1 << i));	// bit=0

    // Point to next available bit.
    if(pos < 7)
      pos++;
		else
    {
      pos = 0;
      _bytePos++;
    }//end else...
  }//end for i...

  // Update the global next bit position.
  _bitPos = pos;

  // Output the result.
  return(b);
}//end Read.

/** Peek bits in the stream.
Read multiple bits from the least significant bit upwards
from the specified stream position without disturbing the
current stream position.
@param bitLoc		: Bit pos in stream.
@param numBits	: No. of bits to read.
@return					: The code.
*/
int BitStreamReader::Peek(int bitLoc, int numBits)
{
  int bytePos = bitLoc / 8;
  int bitPos	= bitLoc % 8;
  int b				= 0;

  for(int i = 0; i < numBits; i++)
  {
    // Strip out the next bit and update in the bit position i.
    if((_bitStream[bytePos] >> bitPos) & 1)
      b = (int)(b | (1 << i));	// bit=1
    else
      b = (int)(b & ~(1 << i));	// bit=0

    // Point to next available bit.
    if(bitPos < 7)
      bitPos++;
		else
    {
      bitPos = 0;
      bytePos++;
    }//end else...
  }//end for i...

  // Output the result.
  return(b);
}//end Peek.



