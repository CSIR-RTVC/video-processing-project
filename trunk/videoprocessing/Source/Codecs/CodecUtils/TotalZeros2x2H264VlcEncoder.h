/** @file

MODULE				: TotalZeros2x2H264VlcEncoder

TAG						: TZ2H264VE

FILE NAME			: TotalZeros2x2H264VlcEncoder.h

DESCRIPTION		: A total zeros Vlc encoder implementation as defined in
								H.264 Recommendation (03/2005) Table 9.9(a) page 205. The 
								total number of coeffs defines which vlc array to use. 
								This implementation is implemented with an IVlcEncoder 
								Interface.

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
#ifndef _TOTALZEROS2X2H264VLCENCODER_H
#define _TOTALZEROS2X2H264VLCENCODER_H

#include "IVlcEncoder.h"

/*
---------------------------------------------------------------------------
	Type definitions.
---------------------------------------------------------------------------
*/
typedef struct _TZ2H264VE_VlcCodeType
{
  int		numBits;	///< Num of bits in code word.
  int		codeWord;	///< Code word bit representation.
} TZ2H264VE_VlcCodeType;

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class TotalZeros2x2H264VlcEncoder : public IVlcEncoder
{
public:
	TotalZeros2x2H264VlcEncoder(void);
	virtual ~TotalZeros2x2H264VlcEncoder(void);

public:
	/// Interface implementation.
	int GetNumCodedBits(void)	{ return(_numCodeBits); }
	int GetCode(void)					{ return(_bitCode); }
	/// A single symbol has no meaning for combined multi symbol encoding.
	virtual int Encode(int symbol) { return(0); }

	/// Optional interface implementation.
	/// The 2 symbols represent total zeros and total coeffs, respectively.
	virtual int	Encode2(int symbol1, int symbol2);

protected:
	int 	_numCodeBits;	///< Number of coded bits for the encoding.
	int 	_bitCode;			///< The code of length numCodeBits.

	/// Constants of table sets.
	static const TZ2H264VE_VlcCodeType table[4][4];

};// end class TotalZeros2x2H264VlcEncoder.

#endif	// end _TOTALZEROS2X2H264VLCENCODER_H.
