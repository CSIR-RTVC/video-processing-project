/** @file

MODULE				: CodedBlkPatternH264VlcDecoder

TAG						: CBPH264VD

FILE NAME			: CodedBlkPatternH264VlcDecoder.cpp

DESCRIPTION		: A coded block pattern Vlc decoder implementation used in 
								H.264 Recommendation (03/2005)Table 9.4(a) on page 196. 
								This implementation is implemented with an IVlcDecoder 
								Interface and is an extention of the unsigned Exp-Golomb 
								Vlc decoder.

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

