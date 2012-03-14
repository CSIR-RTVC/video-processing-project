/** @file

MODULE				: ExpGolombSignedVlcDecoder

TAG						: EGSVD

FILE NAME			: ExpGolombSignedVlcDecoder.h

DESCRIPTION		: An Exp-Golomb Vlc decoder implementation for signed 
								integer symbols as used in H.264 Recommendation (03/2005) 
								Table 9.3 page 196. This implementation is implemented 
								with an IVlcDecoder Interface and is an extention of the
								unsigned Exp-Golomb Vlc decoder.

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
#ifndef _EXPGOLOMBSIGNEDVLCDECODER_H
#define _EXPGOLOMBSIGNEDVLCDECODER_H

#include "ExpGolombUnsignedVlcDecoder.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class ExpGolombSignedVlcDecoder : public ExpGolombUnsignedVlcDecoder
{
public:
	ExpGolombSignedVlcDecoder(void) {}
	virtual ~ExpGolombSignedVlcDecoder(void){}

public:
	/// A single symbol decoding is the only requirement. Convert
	/// the unsigned symbol input from the base class into a signed 
	/// symbol.
	virtual int Decode(IBitStreamReader* bsr)
	{
		int x = ExpGolombUnsignedVlcDecoder::Decode(bsr);
		if(x != 0)
		{
			if(x & 1)	///< Odd are positive.
				x = (x+1) >> 1;
			else			///< Even are negative.
				x = -(x >> 1);
		}//end if !x...

		return(x);
	}//end Decode...

};// end class ExpGolombSignedVlcDecoder.

#endif	// end _EXPGOLOMBSIGNEDVLCDECODER_H.
