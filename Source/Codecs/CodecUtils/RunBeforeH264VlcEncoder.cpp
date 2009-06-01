/** @file

MODULE				: RunBeforeH264VlcEncoder

TAG						: RBH264VE

FILE NAME			: RunBeforeH264VlcEncoder.cpp

DESCRIPTION		: A run before Vlc encoder implementation as defined in
								H.264 Recommendation (03/2005) Table 9.10 page 206. The 
								zero coeffs left defines which vlc array to use. This 
								implementation is implemented with an IVlcEncoder 
								Interface.

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

=====================================================================================
*/
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include "RunBeforeH264VlcEncoder.h"

/*
---------------------------------------------------------------------------
	Construction and destruction.
---------------------------------------------------------------------------
*/
RunBeforeH264VlcEncoder::RunBeforeH264VlcEncoder(void)
{
	_numCodeBits	= 0;
	_bitCode			= 0;
}//end constructor.

RunBeforeH264VlcEncoder::~RunBeforeH264VlcEncoder(void)
{
}//end destructor.

/*
---------------------------------------------------------------------------
	Interface Methods.
---------------------------------------------------------------------------
*/
/** Encode total zeros - total coeffs symbols.
The 2 param symbols represent run before and zeros left, respectively, and 
are encoded with the number of bits returned. The _numCodeBits and _bitCode 
members are set accordingly.
@param symbol1	:	Run before. 
@param symbol2	: Zeros left.
@return					: Num of bits in the codeword.
*/
int	RunBeforeH264VlcEncoder::Encode2(int symbol1, int symbol2)
{
	int runBefore	= symbol1;
	int zerosLeft	= symbol2;

	if(zerosLeft > 6)	///< Use the last column for all greater than 6.
		zerosLeft = 7;

	_bitCode			= table[runBefore][zerosLeft].codeWord;
	_numCodeBits	= table[runBefore][zerosLeft].numBits;

	return(_numCodeBits);
}// end Encode2.


/*
---------------------------------------------------------------------------
	Table Constants.
---------------------------------------------------------------------------
*/
const RBH264VE_VlcCodeType RunBeforeH264VlcEncoder::table[15][8] = 
{///<	0				 1				2				 3				4				 5				6				 >6 = zerosLeft.
	{ { 0, 0}, { 1, 1}, { 1, 1}, { 2, 3}, { 2, 3}, { 2, 3}, { 2, 3}, { 3, 7} }, ///< 0 = runBefore.
	{ { 0, 0}, { 1, 0}, { 2, 1}, { 2, 2}, { 2, 2}, { 2, 2}, { 3, 0}, { 3, 6} }, ///< 1
	{ { 0, 0}, { 0, 0}, { 2, 0}, { 2, 1}, { 2, 1}, { 3, 3}, { 3, 1}, { 3, 5} }, ///< 2
	{ { 0, 0}, { 0, 0}, { 0, 0}, { 2, 0}, { 3, 1}, { 3, 2}, { 3, 3}, { 3, 4} }, ///< 3
	{ { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 3, 0}, { 3, 1}, { 3, 2}, { 3, 3} }, ///< 4
	{ { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 3, 0}, { 3, 5}, { 3, 2} }, ///< 5
	{ { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 3, 4}, { 3, 1} }, ///< 6
	{ { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 4, 1} }, ///< 7
	{ { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 5, 1} }, ///< 8
	{ { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 6, 1} }, ///< 9
	{ { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 7, 1} }, ///< 10
	{ { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 8, 1} }, ///< 11
	{ { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 9, 1} }, ///< 12
	{ { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, {10, 1} }, ///< 13
	{ { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}, {11, 1} }	///< 14
};


