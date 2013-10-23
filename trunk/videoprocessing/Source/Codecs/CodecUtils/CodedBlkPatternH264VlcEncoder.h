/** @file

MODULE				: CodedBlkPatternH264VlcEncoder

TAG						: CBPH264VE

FILE NAME			: CodedBlkPatternH264VlcEncoder.h

DESCRIPTION		: A coded block pattern Vlc encoder implementation used in 
								H.264 Recommendation (03/2005)Table 9.4(a) on page 196. 
								This implementation is implemented with an IVlcEncoder 
								Interface and is an extention of the unsigned Exp-Golomb 
								Vlc encoder.

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
#ifndef _CODEDBLKPATTERNH264VLCENCODER_H
#define _CODEDBLKPATTERNH264VLCENCODER_H

#include "ExpGolombUnsignedVlcEncoder.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class CodedBlkPatternH264VlcEncoder : public ExpGolombUnsignedVlcEncoder
{
public:
	CodedBlkPatternH264VlcEncoder(void);
	virtual ~CodedBlkPatternH264VlcEncoder(void);

public:
	/// Does not have a single symbol encoding requirement.
	virtual int Encode(int symbol) { return(0); }

	/// Optional interface.
public:

	/** Encode with a macroblock type
	The 2 symbols represent the coded block pattern value and the 
	macroblock prediction mode (Intra/Inter), respectively.
	@param symbol1	: Block pattern to encode.
	@param symbol2	: Intra/inter = 0/1.
	@return					: Num of bits in the codeword.
	*/
	virtual int	Encode2(int symbol1, int symbol2);

protected:
	/// Constants of table sets.
	static const int IntraTable[48];
	static const int InterTable[48];

};// end class CodedBlkPatternH264VlcEncoder.

#endif	// end _CODEDBLKPATTERNH264VLCENCODER_H.
