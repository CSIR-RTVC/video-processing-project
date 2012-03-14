/** @file

MODULE				: ExpGolombTruncVlcEncoder

TAG						: EGTVE

FILE NAME			: ExpGolombTruncVlcEncoder.h

DESCRIPTION		: An Exp-Golomb Vlc encoder implementation for truncated 
								integer symbols as used in H.264 Recommendation (03/2005) 
								described on page 195. This implementation is implemented 
								with an IVlcEncoder Interface and is an extention of the
								unsigned Exp-Golomb Vlc encoder.

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
#ifndef _EXPGOLOMBTRUNCVLCENCODER_H
#define _EXPGOLOMBTRUNCVLCENCODER_H

#include "ExpGolombUnsignedVlcEncoder.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class ExpGolombTruncVlcEncoder : public ExpGolombUnsignedVlcEncoder
{
public:
	ExpGolombTruncVlcEncoder(void) {}
	virtual ~ExpGolombTruncVlcEncoder(void){}

public:
	/// Does not have a single symbol encoding requirement.
	virtual int Encode(int symbol) { return(0); }

	/// Optional interface.
public:

	/** Encode with a range [0...range]
	The 2 symbols represent the input value and the range, respectively.
	@param symbol1	: Unsigned value to encode.
	@param symbol2	: Range of the value to encode.
	@return					: Num of bits in the codeword.
	*/
	virtual int	Encode2(int symbol1, int symbol2)
	{
		int x = symbol1;
		if(symbol2 == 1)	///< range = 0...1
		{
			_numCodeBits	= 1;
			_bitCode			= !x;
			return(1);
		}//end if symbol2...

		return(ExpGolombUnsignedVlcEncoder::Encode(x));
	}//end Encode...

};// end class ExpGolombTruncVlcEncoder.

#endif	// end _EXPGOLOMBTRUNCVLCENCODER_H.
