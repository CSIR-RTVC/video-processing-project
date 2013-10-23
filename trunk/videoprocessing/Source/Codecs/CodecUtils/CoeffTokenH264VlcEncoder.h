/** @file

MODULE				: CoeffTokenH264VlcEncoder

TAG						: CTH264VE

FILE NAME			: CoeffTokenH264VlcEncoder.h

DESCRIPTION		: A coeff token Vlc encoder implementation as defined in
								H.264 Recommendation (03/2005) Table 9.2 page 200. The 
								trailing ones and total coeffs are combined in this token 
								and is dependent on the neighbouring blocks' total number
								of coeffs. This implementation is implemented with an
								IVlcEncoder Interface.

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
#ifndef _COEFFTOKENH264VLCENCODER_H
#define _COEFFTOKENH264VLCENCODER_H

#include "IVlcEncoder.h"

/*
---------------------------------------------------------------------------
	Class constants.
---------------------------------------------------------------------------
*/

/*
---------------------------------------------------------------------------
	Type definitions.
---------------------------------------------------------------------------
*/
typedef struct _CTH264VE_VlcCodeType
{
  int		numBits;	///< Num of bits in code word.
  int		codeWord;	///< Code word bit representation.
} CTH264VE_VlcCodeType;

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class CoeffTokenH264VlcEncoder : public IVlcEncoder
{
public:
	CoeffTokenH264VlcEncoder(void);
	virtual ~CoeffTokenH264VlcEncoder(void);

public:
	/// Interface implementation.
	int GetNumCodedBits(void)	{ return(_numCodeBits); }
	int GetCode(void)					{ return(_bitCode); }
	/// A single symbol has no meaning for combined multi symbol encoding.
	virtual int Encode(int symbol) { return(0); }

	/// Optional interface implementation.
	/// The 3 symbols represent total coeffs, trailing ones and neighbours' total coeffs, respectively.
	virtual int	Encode3(int symbol1, int symbol2, int symbol3);

protected:
	int 	_numCodeBits;	///< Number of coded bits for the encoding.
	int 	_bitCode;			///< The code of length numCodeBits.

	/// Constants of table sets.
	static const CTH264VE_VlcCodeType nC0to1[4][17];
	static const CTH264VE_VlcCodeType nC2to3[4][17];
	static const CTH264VE_VlcCodeType nC4to7[4][17];
	static const CTH264VE_VlcCodeType nC8up[4][17];
	static const CTH264VE_VlcCodeType nCneg1[4][5];
	static const CTH264VE_VlcCodeType nCneg2[4][9];

};// end class CoeffTokenH264VlcEncoder.

#endif	// end _COEFFTOKENH264VLCENCODER_H.
