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



