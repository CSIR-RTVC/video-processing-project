/** @file

MODULE				: ExpGolombUnsignedVlcEncoder

TAG						: EGUVE

FILE NAME			: ExpGolombUnsignedVlcEncoder.cpp

DESCRIPTION		: An Exp-Golomb Vlc encoder implementation for unsigned 
								integer symbols as used in H.264 Recommendation (03/2005) 
								Table 9.2 page 195. This implementation is implemented 
								with an IVlcEncoder Interface.

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




