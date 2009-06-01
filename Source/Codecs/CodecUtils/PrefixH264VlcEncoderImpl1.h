/** @file

MODULE				: PrefixH264VlcEncoderImpl1

TAG						: PH263VEI1

FILE NAME			: PrefixH264VlcEncoderImpl1.h

DESCRIPTION		: An Exp-Goulomb prefix Vlc encoder implementation with an	
								IVlcEncoder Interface. Encode is trivial.

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
