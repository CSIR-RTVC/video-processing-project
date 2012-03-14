/** @file

MODULE				: ExpGolombUnsignedVlcEncoder

TAG						: EGUVE

FILE NAME			: ExpGolombUnsignedVlcEncoder.cpp

DESCRIPTION		: An Exp-Golomb Vlc encoder implementation for unsigned 
								integer symbols as used in H.264 Recommendation (03/2005) 
								Table 9.2 page 195. This implementation is implemented 
								with an IVlcEncoder Interface.

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

=====================================================================================
*/
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include <math.h>
#include "ExpGolombUnsignedVlcEncoder.h"

/*
---------------------------------------------------------------------------
	Construction and destruction.
---------------------------------------------------------------------------
*/
ExpGolombUnsignedVlcEncoder::ExpGolombUnsignedVlcEncoder(void)
{
	_numCodeBits	= 0;
	_bitCode			= 0;
}//end constructor.

ExpGolombUnsignedVlcEncoder::~ExpGolombUnsignedVlcEncoder(void)
{
}//end destructor.

/*
---------------------------------------------------------------------------
	Interface Methods.
---------------------------------------------------------------------------
*/
/** Encode unsigned integer symbol.
The unsigned integer is encoded and the number of bits of the symbol is returned. 
The _numCodeBits and _bitCode members are set accordingly. This implementation
is limited to 32 bit code words.
@param symbol	:	Unsigned integer to code 
@return				: Num of bits in the codeword.
*/
int	ExpGolombUnsignedVlcEncoder::Encode(int symbol)
{
	_bitCode			= 0;
	_numCodeBits	= 0;

	if(symbol == 0)
	{
		_bitCode			= 1;
		_numCodeBits	= 1;
	}//end if symbol...
	else
	{
		int x = symbol + 1;
		int leadingZeros = (int)(log((double)x)/double(0.69314718055994529));	///< log(2.0) = 0.69314718055994529.
		_numCodeBits = 2*leadingZeros + 1;
		if(_numCodeBits <= 32)
		{
			int a = 1 << leadingZeros;
			_bitCode = (x - a) | a;
		}//end if _numCodeBits...
		else
			_numCodeBits = 0;
	}//end else...

	return(_numCodeBits);
}// end Encode.




