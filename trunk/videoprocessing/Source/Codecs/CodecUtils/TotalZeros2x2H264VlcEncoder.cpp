/** @file

MODULE				: TotalZeros2x2H264VlcEncoder

TAG						: TZ2H264VE

FILE NAME			: TotalZeros2x2H264VlcEncoder.cpp

DESCRIPTION		: A total zeros Vlc encoder implementation as defined in
								H.264 Recommendation (03/2005) Table 9.9(a) page 205. The 
								total number of coeffs defines which vlc array to use. 
								This implementation is implemented with an IVlcEncoder 
								Interface.

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

#include "TotalZeros2x2H264VlcEncoder.h"

/*
---------------------------------------------------------------------------
	Construction and destruction.
---------------------------------------------------------------------------
*/
TotalZeros2x2H264VlcEncoder::TotalZeros2x2H264VlcEncoder(void)
{
	_numCodeBits	= 0;
	_bitCode			= 0;
}//end constructor.

TotalZeros2x2H264VlcEncoder::~TotalZeros2x2H264VlcEncoder(void)
{
}//end destructor.

/*
---------------------------------------------------------------------------
	Interface Methods.
---------------------------------------------------------------------------
*/
/** Encode total zeros - total coeffs symbols.
The 2 param symbols represent total zeros and total coeffs, respectively, and 
are encoded with the number of bits returned. The _numCodeBits and _bitCode 
members are set accordingly.
@param symbol1	:	Total zeros in this block 
@param symbol2	: Total coeffs for this block
@return					: Num of bits in the codeword.
*/
int	TotalZeros2x2H264VlcEncoder::Encode2(int symbol1, int symbol2)
{
	int totalZeros	= symbol1;
	int totalCoeffs	= symbol2;

	_bitCode			= table[totalZeros][totalCoeffs].codeWord;
	_numCodeBits	= table[totalZeros][totalCoeffs].numBits;

	return(_numCodeBits);
}// end Encode2.


/*
---------------------------------------------------------------------------
	Table Constants.
---------------------------------------------------------------------------
*/
const TZ2H264VE_VlcCodeType TotalZeros2x2H264VlcEncoder::table[4][4] = 
{///<	0				 1				2				 3	= totalCoeffs.
	{ { 0, 0}, { 1, 1}, { 1, 1}, { 1, 1} }, ///< 0 = totalZeros.
	{ { 0, 0}, { 2, 1}, { 2, 1}, { 1, 0} }, ///< 1
	{ { 0, 0}, { 3, 1}, { 2, 0}, { 0, 0} }, ///< 2
	{ { 0, 0}, { 3, 0}, { 0, 0}, { 0, 0} } 	///< 3
};


