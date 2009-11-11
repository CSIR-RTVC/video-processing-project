/** @file

MODULE				: ExpGolombSignedVlcEncoder

TAG						: EGSVE

FILE NAME			: ExpGolombSignedVlcEncoder.h

DESCRIPTION		: An Exp-Golomb Vlc encoder implementation for signed 
								integer symbols as used in H.264 Recommendation (03/2005) 
								Table 9.3 page 196. This implementation is implemented 
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
