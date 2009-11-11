/** @file

MODULE				: ExpGolombTruncVlcDecoder

TAG						: EGTVD

FILE NAME			: ExpGolombTruncVlcDecoder.h

DESCRIPTION		: An Exp-Golomb Vlc decoder implementation for truncated 
								integer symbols as used in H.264 Recommendation (03/2005) 
								described on page 195. This implementation is implemented 
								with an IVlcDecoder Interface and is an extention of the
								unsigned Exp-Golomb Vlc decoder.

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
