/** @file

MODULE				: ExpGolombTruncVlcDecoder

TAG						: EGTVD

FILE NAME			: ExpGolombTruncVlcDecoder.h

DESCRIPTION		: An Exp-Golomb Vlc decoder implementation for truncated 
								integer symbols as used in H.264 Recommendation (03/2005) 
								described on page 195. This implementation is implemented 
								with an IVlcDecoder Interface and is an extention of the
								unsigned Exp-Golomb Vlc decoder.

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
#ifndef _EXPGOLOMBTRUNCVLCDECODER_H
#define _EXPGOLOMBTRUNCVLCDECODER_H

#include "ExpGolombUnsignedVlcDecoder.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class ExpGolombTruncVlcDecoder : public ExpGolombUnsignedVlcDecoder
{
public:
	ExpGolombTruncVlcDecoder(void) {}
	virtual ~ExpGolombTruncVlcDecoder(void){}

public:
	/// There is no single symbol decoding requirement.
	virtual int Decode(IBitStreamReader* bsr) { return(0); }

	/// Optional interface.
public:

	/** Decode with a range [0...range]
	The 2 symbols represent the unsigned decoded value and the expected
	range of the decoded value, respectively.
	@param bsr			: Bit stream to get from.
	@param symbol1	: Reference to returned value.
	@param symbol2	: Range of the value to be decoded (input).
	@return					: Num of bits extracted.
	*/
	virtual int	Decode2(IBitStreamReader* bsr, int* symbol1, int* symbol2)
	{
		if( *symbol2 == 1)	///< Range = [0...1]
		{
			_numCodeBits = 1;
			*symbol1 = !bsr->Read();
			return(1);
		}//end if symbol2...

		*symbol1 = ExpGolombUnsignedVlcDecoder::Decode(bsr);
		return(_numCodeBits);
	}//end Decode2...

};// end class ExpGolombTruncVlcDecoder.

#endif	// end _EXPGOLOMBTRUNCVLCDECODER_H.
