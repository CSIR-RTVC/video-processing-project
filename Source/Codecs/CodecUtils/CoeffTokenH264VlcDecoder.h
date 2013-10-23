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
