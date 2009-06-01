/** @file

MODULE				: PrefixH264VlcDecoderImpl1

TAG						: PH264VDI1

FILE NAME			: PrefixH264VlcDecoderImpl1.h

DESCRIPTION		: A class to implement a Exp-Golomb prefix Vlc decoder with 
								an IVlcDecoder interface. Trivial decoder that counts leading
								zeros.

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
