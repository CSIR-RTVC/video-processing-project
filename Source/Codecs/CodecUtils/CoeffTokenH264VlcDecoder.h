/** @file

MODULE				: CoeffTokenH264VlcDecoder

TAG						: CTH264VD

FILE NAME			: CoeffTokenH264VlcDecoder.h

DESCRIPTION		: A coeff token Vlc decoder implementation as defined in
								H.264 Recommendation (03/2005) Table 9.2 page 200. The 
								trailing ones and total coeffs are combined in this token 
								and is dependent on the neighbouring blocks' total number
								of coeffs. This implementation is implemented with an
								IVlcDecoder Interface.

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
#ifndef _COEFFTOKENH264VLCDECODER_H
#define _COEFFTOKENH264VLCDECODER_H

#pragma once

#include "IVlcDecoder.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class CoeffTokenH264VlcDecoder : public IVlcDecoder
{
public:
	CoeffTokenH264VlcDecoder();
	virtual ~CoeffTokenH264VlcDecoder();

public:
	/// Interface implementation.
	int GetNumDecodedBits(void)	{ return(_numCodeBits); }
	int Marker(void)						{ return(0); }	///< No markers for this decoder.
	/// A single symbol has no meaning for combined multi symbol encoding.
	virtual int Decode(IBitStreamReader* bsr) { _numCodeBits = 0; return(0); } 

	/// Optional interface implementation.
	/// The 3 symbols represent total coeffs, trailing ones and neighbours' total coeffs, respectively.
	virtual int	Decode3(IBitStreamReader* bsr, int* symbol1, int* symbol2, int* symbol3);

protected:
	/// Private methods.
	int	NC0to1(IBitStreamReader* bsr, int* totalCoeffs, int* trailingOnes);
	int	NC2to3(IBitStreamReader* bsr, int* totalCoeffs, int* trailingOnes);
	int	NC4to7(IBitStreamReader* bsr, int* totalCoeffs, int* trailingOnes);
	int	NC8up(IBitStreamReader* bsr, int* totalCoeffs, int* trailingOnes);
	int	NCneg1(IBitStreamReader* bsr, int* totalCoeffs, int* trailingOnes);
	int	NCneg2(IBitStreamReader* bsr, int* totalCoeffs, int* trailingOnes);

protected:
	int _numCodeBits;	///< Number of coded bits for this symbol.

};// end class CoeffTokenH264VlcDecoder.

#endif	// _COEFFTOKENH264VLCDECODER_H
