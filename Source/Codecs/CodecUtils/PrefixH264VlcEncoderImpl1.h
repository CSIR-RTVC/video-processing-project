/** @file

MODULE				: PrefixH264VlcEncoderImpl1

TAG						: PH263VEI1

FILE NAME			: PrefixH264VlcEncoderImpl1.h

DESCRIPTION		: An Exp-Goulomb prefix Vlc encoder implementation with an	
								IVlcEncoder Interface. Encode is trivial.

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

==========================================================================================
*/
#ifndef _PREFIXH264VLCENCODERIMPL1_H
#define _PREFIXH264VLCENCODERIMPL1_H

#include "IVlcEncoder.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class PrefixH264VlcEncoderImpl1 : public IVlcEncoder
{
public:
	PrefixH264VlcEncoderImpl1(void)	{ _numCodeBits = 0; _bitCode = 0; }
	virtual ~PrefixH264VlcEncoderImpl1(void)	{ }

public:
	/// Interface implementation.
	int GetNumCodedBits(void)	{ return(_numCodeBits); }
	int GetCode(void)					{ return(_bitCode); }

	/** Encode the prefix symbol.
	Trivial implementation of symbol number of leading zeros followed by a 1. Note that
	only symbol values up to 31 can be encoded i.e. int = 32 bits with the added 1.
	@param	symbol	: Symbol to encode.
	@return					: Num of bits.
	*/
	int Encode(int symbol) { _bitCode = 1; _numCodeBits = symbol + 1; return(_numCodeBits); }

protected:
	int 	_numCodeBits;	///< Number of coded bits for the encoding.
	int 	_bitCode;			///< The code of length numCodeBits.

};// end class PrefixH264VlcEncoderImpl1.

#endif	// end _PREFIXH264VLCENCODERIMPL1_H.
