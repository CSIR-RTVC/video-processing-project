/** @file

MODULE				: BitStreamReaderMSB

TAG						: BSRMSB

FILE NAME			: BitStreamReaderMSB.cpp

DESCRIPTION		: A bit stream reader implementation of the BitStreamBase
								base class with the first bit as the MSB of the byte. 
								Add the functionality to do the reading.
									BitStreamReaderMSB* pBsr = new BitStreamReaderMSB();
									pBsr->SetStream((void *)pStream, (streamLen * sizeof(pStream[0]) * 8));
									pBsr->Read(5, codeWord);
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

#include "BitStreamReaderMSB.h"

BitStreamReaderMSB::BitStreamReaderMSB()
{
}//end constructor.

BitStreamReaderMSB::~BitStreamReaderMSB()
{
}//end destructor.

/** Read a single bit.
Read from the current bit position in the stream.
@return			: The bit [0,1].
*/
int BitStreamReaderMSB::Read(void)
{
  // Strip out the bit. 
  int codeBit = (int)(_bitStream[_bytePos] >> _bitPos) & 1;

  // Point to next available bit.
  if(_bitPos > 0)
		_bitPos--;
	else
  {
    _bitPos = 7;
    _bytePos++;
  }//end else...

  return(codeBit);
}//end Read.

/** Read bits from the stream.
Read multiple bits from the most significant bit downwards
from the current stream position.
@param numBits	: No. of bits to read.
@return					: The code.
*/
int BitStreamReaderMSB::Read(int numBits)
{
  int pos = _bitPos;
  int b		= 0;

  for(int i = numBits; i > 0; i--)
  {
    // Strip out the next bit and update in the bit position i.
    if((_bitStream[_bytePos] >> pos) & 1)
      b = (int)(b | (1 << (i-1)));	// bit=1
    else
      b = (int)(b & ~(1 << (i-1)));	// bit=0

    // Point to next available bit.
    if(pos > 0)
      pos--;
		else
    {
      pos = 7;
      _bytePos++;
    }//end else...
  }//end for i...

  // Update the global next bit position.
  _bitPos = pos;

  // Output the result.
  return(b);
}//end Read.

/** Peek bits in the stream.
Read multiple bits from the most significant bit downwards
from the specified stream position without disturbing the
current stream position.
@param bitLoc		: Bit pos in stream.
@param numBits	: No. of bits to read.
@return					: The code.
*/
int BitStreamReaderMSB::Peek(int bitLoc, int numBits)
{
  int bytePos = bitLoc / 8;
  int bitPos	= bitLoc % 8;
  int b				= 0;

  for(int i = numBits; i > 0; i--)
  {
    // Strip out the next bit and update in the bit position i.
    if((_bitStream[bytePos] >> bitPos) & 1)
      b = (int)(b | (1 << (i-1)));	// bit=1
    else
      b = (int)(b & ~(1 << (i-1)));	// bit=0

    // Point to next available bit.
    if(bitPos > 0)
      bitPos--;
		else
    {
      bitPos = 7;
      bytePos++;
    }//end else...
  }//end for i...

  // Output the result.
  return(b);
}//end Peek.



