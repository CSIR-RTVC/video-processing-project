/** @file

MODULE				: CodedBlkPatternH264VlcEncoder

TAG						: CBPH264VE

FILE NAME			: CodedBlkPatternH264VlcEncoder.cpp

DESCRIPTION		: A coded block pattern Vlc encoder implementation used in 
								H.264 Recommendation (03/2005)Table 9.4(a) on page 196. 
								This implementation is implemented with an IVlcEncoder 
								Interface and is an extention of the unsigned Exp-Golomb 
								Vlc encoder.

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

=====================================================================================
*/
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include "CodedBlkPatternH264VlcEncoder.h"

/*
---------------------------------------------------------------------------
	Construction and destruction.
---------------------------------------------------------------------------
*/
CodedBlkPatternH264VlcEncoder::CodedBlkPatternH264VlcEncoder(void)
{
}//end constructor.

CodedBlkPatternH264VlcEncoder::~CodedBlkPatternH264VlcEncoder(void)
{
}//end destructor.

/*
---------------------------------------------------------------------------
	Interface Methods.
---------------------------------------------------------------------------
*/
/** Encode with a macroblock type
The 2 symbols represent the coded block pattern value and the 
macroblock prediction mode (Intra/Inter), respectively.
@param symbol1	: Block pattern to encode.
@param symbol2	: Intra/inter = 0/1.
@return					: Num of bits in the codeword.
*/
int	CodedBlkPatternH264VlcEncoder::Encode2(int symbol1, int symbol2)
{
	_bitCode			= 0;
	_numCodeBits	= 0;
	if(symbol1 < 48)
	{
		int code = 0;
		if(symbol2)	///< if Inter...
			code = InterTable[symbol1];
		else
			code = IntraTable[symbol1];

		return(ExpGolombUnsignedVlcEncoder::Encode(code));
	}//end if symbol1...

	return(_numCodeBits);
}//end Encode2.

/*
---------------------------------------------------------------------------
	Table Constants.
---------------------------------------------------------------------------
*/
const int CodedBlkPatternH264VlcEncoder::IntraTable[48] =
{	///< 0		1		2		3		4		5		6		7
	     3,  29, 30, 17, 31, 18, 37,  8,
	///< 8		9	 10	 11	 12  13  14  15
	    32,  38, 19,  9, 20, 10, 11,  2,
	///<16   17  18  19  20  21  22  23
	    16,  33, 34, 21, 35, 22, 39,  4,
	///<24   25  26  27  28  29  30  31
	    36,  40, 23,  5, 24,  6,  7,  1,
	///<32   33  34  35  36  37  38  39
	    41,  42, 43, 25, 44, 26, 46, 12,
	///<40   41  42  43  44  45  46  47
	    45,  47, 27, 13, 28, 14, 15,  0
};

const int CodedBlkPatternH264VlcEncoder::InterTable[48] = 
{	///< 0		1		2		3		4		5		6		7
			 0,		2,	3,	7,	4,	8, 17, 13,
	///< 8		9	 10	 11	 12  13  14  15
			 5,  18,  9, 14, 10, 15, 16, 11,
	///<16   17  18  19  20  21  22  23
		   1,  32, 33, 36, 34, 37, 44, 40,
	///<24   25  26  27  28  29  30  31
	    35,  45, 38, 41, 39, 42, 43, 19,
	///<32   33  34  35  36  37  38  39
	     6,  24, 25, 20, 26, 21, 46, 28,
	///<40   41  42  43  44  45  46  47
		  27,  47, 22, 29, 23, 30, 31, 12
};



