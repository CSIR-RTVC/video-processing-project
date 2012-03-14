/** @file

MODULE				: CodedBlkPatternH264VlcDecoder

TAG						: CBPH264VD

FILE NAME			: CodedBlkPatternH264VlcDecoder.h

DESCRIPTION		: A coded block pattern Vlc decoder implementation used in 
								H.264 Recommendation (03/2005)Table 9.4(a) on page 196. 
								This implementation is implemented with an IVlcDecoder 
								Interface and is an extention of the unsigned Exp-Golomb 
								Vlc decoder.

LICENSE	: GNU Lesser General Public License

Copyright (c) 2008 - 2012, CSIR
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
#ifndef _CODEDBLKPATTERNH264VLCDECODER_H
#define _CODEDBLKPATTERNH264VLCDECODER_H

#include "ExpGolombUnsignedVlcDecoder.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class CodedBlkPatternH264VlcDecoder : public ExpGolombUnsignedVlcDecoder
{
public:
	CodedBlkPatternH264VlcDecoder(void);
	virtual ~CodedBlkPatternH264VlcDecoder(void);

public:
	/// There is no single symbol decoding requirement.
	virtual int Decode(IBitStreamReader* bsr) { return(0); }

	/// Optional interface.
public:

	/** Decode with a macroblock type
	The 2 symbols represent the returned coded block pattern value and the 
	macroblock prediction mode (Intra/Inter) is used as an input, respectively.
	@param bsr			: Bit stream to get from.
	@param symbol1	: Block pattern decoded.
	@param symbol2	: Intra/inter = 0/1.
	@return					: Num of bits extracted.
	*/
	virtual int	Decode2(IBitStreamReader* bsr, int* symbol1, int* symbol2);

protected:
	/// Constants of table sets.
	static const int IntraTable[48];
	static const int InterTable[48];

};// end class CodedBlkPatternH264VlcDecoder.

#endif	// end _CODEDBLKPATTERNH264VLCDECODER_H.
