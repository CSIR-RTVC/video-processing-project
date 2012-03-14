/** @file

MODULE				: CodedBlkPatternH264VlcDecoder

TAG						: CBPH264VD

FILE NAME			: CodedBlkPatternH264VlcDecoder.cpp

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
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include "CodedBlkPatternH264VlcDecoder.h"

/*
---------------------------------------------------------------------------
	Construction and destruction.
---------------------------------------------------------------------------
*/
CodedBlkPatternH264VlcDecoder::CodedBlkPatternH264VlcDecoder(void)
{
}//end constructor.

CodedBlkPatternH264VlcDecoder::~CodedBlkPatternH264VlcDecoder(void)
{
}//end destructor.

/*
---------------------------------------------------------------------------
	Interface Methods.
---------------------------------------------------------------------------
*/
/** Decode with a macroblock type
The 2 symbols represent the returned coded block pattern value and the 
macroblock prediction mode (Intra/Inter) is used as an input, respectively.
@param bsr			: Bit stream to get from.
@param symbol1	: Block pattern decoded.
@param symbol2	: Intra/inter = 0/1.
@return					: Num of bits extracted.
*/
int	CodedBlkPatternH264VlcDecoder::Decode2(IBitStreamReader* bsr, int* symbol1, int* symbol2)
{
	/// Reset bits extracted.
  _numCodeBits = 0;

	int code = ExpGolombUnsignedVlcDecoder::Decode(bsr);	///< _numCodeBits is updated in the base class.

	if(code < 48)	///< In the range of the tables.
	{
		if(*symbol2)	///< if Inter...
			*symbol1 = InterTable[code];
		else
			*symbol1 = IntraTable[code];
	}//end if code...

  return(_numCodeBits);
}//end Decode2.

/*
---------------------------------------------------------------------------
	Table Constants.
---------------------------------------------------------------------------
*/
const int CodedBlkPatternH264VlcDecoder::IntraTable[48] =
{	///< 0		1		2		3		4		5		6		7
	    47,  31, 15,  0, 23, 27, 29, 30,
	///< 8		9	 10	 11	 12  13  14  15
	     7,  11, 13, 14, 39, 43, 45, 46,
	///<16   17  18  19  20  21  22  23
	    16,   3,  5, 10, 12, 19, 21, 26,
	///<24   25  26  27  28  29  30  31
	    28,  35, 37, 42, 44,  1,  2,  4,
	///<32   33  34  35  36  37  38  39
	     8,  17, 18, 20, 24,  6,  9, 22,
	///<40   41  42  43  44  45  46  47
	    25,  32, 33, 34, 36, 40, 38, 41
};

const int CodedBlkPatternH264VlcDecoder::InterTable[48] = 
{	///< 0		1		2		3		4		5		6		7
	     0,  16,  1,  2,  4,  8, 32,  3,
	///< 8		9	 10	 11	 12  13  14  15
	     5,  10, 12, 15, 47,  7, 11, 13,
	///<16   17  18  19  20  21  22  23
	    14,   6,  9, 31, 35, 37, 42, 44,
	///<24   25  26  27  28  29  30  31
	    33,  34, 36, 40, 39, 43, 45, 46,
	///<32   33  34  35  36  37  38  39
	    17,  18, 20, 24, 19, 21, 26, 28,
	///<40   41  42  43  44  45  46  47
	    23,  27, 29, 30, 22, 25, 38, 41
};

