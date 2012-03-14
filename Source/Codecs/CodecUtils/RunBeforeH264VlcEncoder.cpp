/** @file

MODULE				: RunBeforeH264VlcEncoder

TAG						: RBH264VE

FILE NAME			: RunBeforeH264VlcEncoder.cpp

DESCRIPTION		: A run before Vlc encoder implementation as defined in
								H.264 Recommendation (03/2005) Table 9.10 page 206. The 
								zero coeffs left defines which vlc array to use. This 
								implementation is implemented with an IVlcEncoder 
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


