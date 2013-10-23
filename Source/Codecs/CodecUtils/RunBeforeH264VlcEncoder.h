/** @file

MODULE				: RunBeforeH264VlcEncoder

TAG						: RBH264VE

FILE NAME			: RunBeforeH264VlcEncoder.h

DESCRIPTION		: A run before Vlc encoder implementation as defined in
								H.264 Recommendation (03/2005) Table 9.10 page 206. The 
								zero coeffs left defines which vlc array to use. This 
								implementation is implemented with an IVlcEncoder 
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
#ifndef _RUNBEFOREH264VLCENCODER_H
#define _RUNBEFOREH264VLCENCODER_H

#include "IVlcEncoder.h"

/*
---------------------------------------------------------------------------
	Type definitions.
---------------------------------------------------------------------------
*/
typedef struct _RBH264VE_VlcCodeType
{
  int		numBits;	///< Num of bits in code word.
  int		codeWord;	///< Code word bit representation.
} RBH264VE_VlcCodeType;

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class RunBeforeH264VlcEncoder : public IVlcEncoder
{
public:
	RunBeforeH264VlcEncoder(void);
	virtual ~RunBeforeH264VlcEncoder(void);

public:
	/// Interface implementation.
	int GetNumCodedBits(void)	{ return(_numCodeBits); }
	int GetCode(void)					{ return(_bitCode); }
	/// A single symbol has no meaning for combined multi symbol encoding.
	virtual int Encode(int symbol) { return(0); }

	/// Optional interface implementation.
	/// The 2 symbols represent run before and zeros left, respectively.
	virtual int	Encode2(int symbol1, int symbol2);

protected:
	int 	_numCodeBits;	///< Number of coded bits for the encoding.
	int 	_bitCode;			///< The code of length numCodeBits.

	/// Constants of table sets.
	static const RBH264VE_VlcCodeType table[15][8];

};// end class RunBeforeH264VlcEncoder.

#endif	// end _RUNBEFOREH264VLCENCODER_H.
