/** @file

MODULE				: ExpGolombUnsignedVlcDecoder

TAG						: EGUVD

FILE NAME			: ExpGolombUnsignedVlcDecoder.cpp

DESCRIPTION		: An Exp-Golomb Vlc decoder implementation for unsigned 
								integer symbols as used in H.264 Recommendation (03/2005) 
								Table 9.2 page 195. This implementation is implemented 
								with an IVlcDecoder Interface.

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

#include "ExpGolombUnsignedVlcDecoder.h"

/*
---------------------------------------------------------------------------
	Construction and destruction.
---------------------------------------------------------------------------
*/
ExpGolombUnsignedVlcDecoder::ExpGolombUnsignedVlcDecoder(void)
{
	_numCodeBits	= 0;
}//end constructor.

ExpGolombUnsignedVlcDecoder::~ExpGolombUnsignedVlcDecoder(void)
{
}//end destructor.

/*
---------------------------------------------------------------------------
	Interface Methods.
---------------------------------------------------------------------------
*/
/** Decode an Exp-Golomb code from the bit stream.
The symbol is extracted from the bit stream and returned.
@param bsr			: Bit stream to get from.
@return					: Symbol extracted.
*/
int	ExpGolombUnsignedVlcDecoder::Decode(IBitStreamReader* bsr)
{
	/// Reset bits extracted.
  _numCodeBits = 0;
	int symbol = 0;

	/// Read leading zeros.
	int leadingZeros = 0;
	while(bsr->Read() == 0)
		leadingZeros++;

	symbol = (1 << leadingZeros) - 1 + bsr->Read(leadingZeros);
	_numCodeBits = leadingZeros*2 + 1;

  return(symbol);
}//end Decode.

