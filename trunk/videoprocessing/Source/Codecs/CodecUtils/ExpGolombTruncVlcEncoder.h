/** @file

MODULE				: ExpGolombTruncVlcEncoder

TAG						: EGTVE

FILE NAME			: ExpGolombTruncVlcEncoder.h

DESCRIPTION		: An Exp-Golomb Vlc encoder implementation for truncated 
								integer symbols as used in H.264 Recommendation (03/2005) 
								described on page 195. This implementation is implemented 
								with an IVlcEncoder Interface and is an extention of the
								unsigned Exp-Golomb Vlc encoder.

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
