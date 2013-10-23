/** @file

MODULE				: PrefixH264VlcDecoderImpl1

TAG						: PH264VDI1

FILE NAME			: PrefixH264VlcDecoderImpl1.h

DESCRIPTION		: A class to implement a Exp-Golomb prefix Vlc decoder with 
								an IVlcDecoder interface. Trivial decoder that counts leading
								zeros.

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
#ifndef _PREFIXH264VLCDECODERIMPL1_H
#define _PREFIXH264VLCDECODERIMPL1_H

#pragma once

#include "IVlcDecoder.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class PrefixH264VlcDecoderImpl1 : public IVlcDecoder
{
public:
	PrefixH264VlcDecoderImpl1(void)						{ _numCodeBits = 0; }
	virtual ~PrefixH264VlcDecoderImpl1(void)	{ }

public:
	/// Interface implementation.
	int GetNumDecodedBits(void)	{ return(_numCodeBits); }
	int Marker(void)						{ return(0); }

	/** Decode a prefix value from the stream.
	Trivial implementation of symbol number of leading zeros followed by a 1. Read 
	the codeword from the	bit stream and return it. _numCodeBits is set.
	@param bsr	: Bit stream to read from.
	@return			: The decoded symbol.
	*/
	int Decode(IBitStreamReader* bsr) 
	{ 
		int symbol = 0;
		while(0 == bsr->Read())
			symbol++;
		_numCodeBits = symbol + 1;
		return(symbol);
	}//end Decode. 

protected:
	int 	_numCodeBits;	///< Number of coded bits for the last decode.

};// end class PrefixH264VlcDecoderImpl1.

#endif	//_PREFIXH264VLCDECODERIMPL1_H
