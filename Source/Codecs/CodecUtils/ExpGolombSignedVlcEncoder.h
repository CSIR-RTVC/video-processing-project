/** @file

MODULE				: ExpGolombSignedVlcEncoder

TAG						: EGSVE

FILE NAME			: ExpGolombSignedVlcEncoder.h

DESCRIPTION		: An Exp-Golomb Vlc encoder implementation for signed 
								integer symbols as used in H.264 Recommendation (03/2005) 
								Table 9.3 page 196. This implementation is implemented 
								with an IVlcEncoder Interface and is an extention of the
								unsigned Exp-Golomb Vlc encoder.

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
#ifndef _EXPGOLOMBSIGNEDVLCENCODER_H
#define _EXPGOLOMBSIGNEDVLCENCODER_H

#include "ExpGolombUnsignedVlcEncoder.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class ExpGolombSignedVlcEncoder : public ExpGolombUnsignedVlcEncoder
{
public:
	ExpGolombSignedVlcEncoder(void) {}
	virtual ~ExpGolombSignedVlcEncoder(void){}

public:
	/// A single symbol encoding is the only requirement. Convert
	/// the signed input into an unsigned symbol before calling the
	/// base class.
	virtual int Encode(int symbol)
	{
		int x = 2*symbol;
		if(x <= 0)
			return(ExpGolombUnsignedVlcEncoder::Encode(-x));
		else
			return(ExpGolombUnsignedVlcEncoder::Encode(x-1));
	}//end Encode...

};// end class ExpGolombSignedVlcEncoder.

#endif	// end _EXPGOLOMBSIGNEDVLCENCODER_H.
