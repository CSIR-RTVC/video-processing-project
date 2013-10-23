/** @file

MODULE				: ExpGolombUnsignedVlcEncoder

TAG						: EGUVE

FILE NAME			: ExpGolombUnsignedVlcEncoder.h

DESCRIPTION		: An Exp-Golomb Vlc encoder implementation for unsigned 
								integer symbols as used in H.264 Recommendation (03/2005) 
								Table 9.2 page 195. This implementation is implemented 
								with an IVlcEncoder Interface.

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
#ifndef _EXPGOLOMBUNSIGNEDVLCENCODER_H
#define _EXPGOLOMBUNSIGNEDVLCENCODER_H

#include "IVlcEncoder.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class ExpGolombUnsignedVlcEncoder : public IVlcEncoder
{
public:
	ExpGolombUnsignedVlcEncoder(void);
	virtual ~ExpGolombUnsignedVlcEncoder(void);

public:
	/// Interface implementation.
	int GetNumCodedBits(void)	{ return(_numCodeBits); }
	int GetCode(void)					{ return(_bitCode); }
  /// For extended codes of greater than 32 bits but less than 64 bits. _bitCode
  /// holds the lower 32 bits and _extBitCode the upper 32 bits.
  int GetExtCode(void)      { return(_extBitCode); }
	/// A single symbol encoding is the only requirement.
	virtual int Encode(int symbol);

protected:
	int 	_numCodeBits;	///< Number of coded bits for the encoding.
	int 	_bitCode;			///< The lower 32 bits of code of length numCodeBits or 32.
  int   _extBitCode;  ///< The upper 32 bits of code of length 0 or numCodeBits - 32.

};// end class ExpGolombUnsignedVlcEncoder.

#endif	// end _EXPGOLOMBUNSIGNEDVLCENCODER_H.
