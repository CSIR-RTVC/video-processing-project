/** @file

MODULE				: CoeffTokenH264VlcEncoder

TAG						: CTH264VE

FILE NAME			: CoeffTokenH264VlcEncoder.cpp

DESCRIPTION		: A coeff token Vlc encoder implementation as defined in
								H.264 Recommendation (03/2005) Table 9.2 page 200. The 
								trailing ones and total coeffs are combined in this token 
								and is dependent on the neighbouring blocks' total number
								of coeffs. This implementation is implemented with an
								IVlcEncoder Interface.

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

=====================================================================================
*/
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include "CoeffTokenH264VlcEncoder.h"

/*
---------------------------------------------------------------------------
	Construction and destruction.
---------------------------------------------------------------------------
*/
CoeffTokenH264VlcEncoder::CoeffTokenH264VlcEncoder(void)
{
	_numCodeBits	= 0;
	_bitCode			= 0;
}//end constructor.

CoeffTokenH264VlcEncoder::~CoeffTokenH264VlcEncoder(void)
{
}//end destructor.

/*
---------------------------------------------------------------------------
	Interface Methods.
---------------------------------------------------------------------------
*/
/** Encode total coeff - trailing ones - neighbour total coeffs symbols.
The 3 param symbols represent total coeffs, trailing ones and neighbours' 
total coeffs, respectively, and are encoded with the number of bits returned.
The _numCodeBits and _bitCode members are set accordingly.
@param symbol1	: Total coeffs for this block
@param symbol2	: Num of trailing ones for this block
@param symbol3	: Total num of coeffs in neighbourhood blocks
@return					: Num of bits in the codeword.
*/
int	CoeffTokenH264VlcEncoder::Encode3(int symbol1, int symbol2, int symbol3)
{
	int totalCoeffs					= symbol1;
	int trailingOnes				= symbol2;
	int numTotNeighborCoeff = symbol3;

	/// Select the table depending on the neighbourhood total number of coeffs.
	switch(numTotNeighborCoeff)
	{
		case 0:
		case 1:
			_numCodeBits	= nC0to1[trailingOnes][totalCoeffs].numBits;
			_bitCode			= nC0to1[trailingOnes][totalCoeffs].codeWord;
			break;
		case 2:
		case 3:
			_numCodeBits	= nC2to3[trailingOnes][totalCoeffs].numBits;
			_bitCode			= nC2to3[trailingOnes][totalCoeffs].codeWord;
			break;
		case 4:
		case 5:
		case 6:
		case 7:
			_numCodeBits	= nC4to7[trailingOnes][totalCoeffs].numBits;
			_bitCode			= nC4to7[trailingOnes][totalCoeffs].codeWord;
			break;
		case -1:
			_numCodeBits	= nCneg1[trailingOnes][totalCoeffs].numBits;
			_bitCode			= nCneg1[trailingOnes][totalCoeffs].codeWord;
			break;
		case -2:
			_numCodeBits	= nCneg2[trailingOnes][totalCoeffs].numBits;
			_bitCode			= nCneg2[trailingOnes][totalCoeffs].codeWord;
			break;
		default:	///< Greater than or equal to 8.
			_numCodeBits	= nC8up[trailingOnes][totalCoeffs].numBits;
			_bitCode			= nC8up[trailingOnes][totalCoeffs].codeWord;
			break;
	}//end switch numTotNeighborCoeff...

	return(_numCodeBits);
}// end Encode3.


/*
---------------------------------------------------------------------------
	Table Constants.
---------------------------------------------------------------------------
*/
const CTH264VE_VlcCodeType CoeffTokenH264VlcEncoder::nC0to1[4][17] = 
{///<	0				 1				2				 3				4				 5				6				 7				8				 9			 10				11			 12				13			 14				15			 16	= totalCoeffs.
	{ { 1, 1}, { 6, 5}, { 8, 7}, { 9, 7}, {10, 7}, {11, 7}, {13,15}, {13,11}, {13, 8}, {14,15}, {14,11}, {15,15}, {15,11}, {16,15}, {16,11}, {16, 7}, {16, 4} },	///< trailingOnes = 0.
	{ { 0, 0}, { 2, 1}, { 6, 4}, { 8, 6}, { 9, 6}, {10, 6}, {11, 6}, {13,14}, {13,10}, {14,14}, {14,10}, {15,14}, {15,10}, {15, 1}, {16,14}, {16,10}, {16, 6} },	///< trailingOnes = 1.
	{ { 0, 0}, { 0, 0}, { 3, 1}, { 7, 5}, { 8, 5}, { 9, 5}, {10, 5}, {11, 5}, {13,13}, {13, 9}, {14,13}, {14, 9}, {15,13}, {15, 9}, {16,13}, {16, 9}, {16, 5} },	///< trailingOnes = 2.
	{ { 0, 0}, { 0, 0}, { 0, 0}, { 5, 3}, { 6, 3}, { 7, 4}, { 8, 4}, { 9, 4}, {10, 4}, {11, 4}, {13,12}, {14,12}, {14, 8}, {15,12}, {15, 8}, {16,12}, {16, 8} }		///< trailingOnes = 3.
};

const CTH264VE_VlcCodeType CoeffTokenH264VlcEncoder::nC2to3[4][17] = 
{///<	0				 1				2				 3				4				 5				6				 7				8				 9			 10				11			 12				13			 14				15			 16	= totalCoeffs.
	{ { 2, 3}, { 6,11}, { 6, 7}, { 7, 7}, { 8, 7}, { 8, 4}, { 9, 7}, {11,15}, {11,11}, {12,15}, {12,11}, {12, 8}, {13,15}, {13,11}, {13, 7}, {14, 9}, {14, 7} },	///< trailingOnes = 0.
	{ { 0, 0}, { 2, 2}, { 5, 7}, { 6,10}, { 6, 6}, { 7, 6}, { 8, 6}, { 9, 6}, {11,14}, {11,10}, {12,14}, {12,10}, {13,14}, {13,10}, {14,11}, {14, 8}, {14, 6} },	///< trailingOnes = 1.
	{ { 0, 0}, { 0, 0}, { 3, 3}, { 6, 9}, { 6, 5}, { 7, 5}, { 8, 5}, { 9, 5}, {11,13}, {11, 9}, {12,13}, {12, 9}, {13,13}, {13, 9}, {13, 6}, {14,10}, {14, 5} },	///< trailingOnes = 2.
	{ { 0, 0}, { 0, 0}, { 0, 0}, { 4, 5}, { 4, 4}, { 5, 6}, { 6, 8}, { 6, 4}, { 7, 4}, { 9, 4}, {11,12}, {11, 8}, {12,12}, {13,12}, {13, 8}, {13, 1}, {14, 4} }		///< trailingOnes = 3.
};

const CTH264VE_VlcCodeType CoeffTokenH264VlcEncoder::nC4to7[4][17] = 
{///<	0				 1				2				 3				4				 5				6				 7				8				 9			 10				11			 12				13			 14				15			 16	= totalCoeffs.
	{ { 4,15}, { 6,15}, { 6,11}, { 6, 8}, { 7,15}, { 7,11}, { 7, 9}, { 7, 8}, { 8,15}, { 8,11}, { 9,15}, { 9,11}, { 9, 8}, {10,13}, {10, 9}, {10, 5}, {10, 1} },	///< trailingOnes = 0.
	{ { 0, 0}, { 4,14}, { 5,15}, { 5,12}, { 5,10}, { 5, 8}, { 6,14}, { 6,10}, { 7,14}, { 8,14}, { 8,10}, { 9,14}, { 9,10}, { 9, 7}, {10,12}, {10, 8}, {10, 4} },	///< trailingOnes = 1.
	{ { 0, 0}, { 0, 0}, { 4,13}, { 5,14}, { 5,11}, { 5, 9}, { 6,13}, { 6, 9}, { 7,13}, { 7,10}, { 8,13}, { 8, 9}, { 9,13}, { 9, 9}, {10,11}, {10, 7}, {10, 3} },	///< trailingOnes = 2.
	{ { 0, 0}, { 0, 0}, { 0, 0}, { 4,12}, { 4,11}, { 4,10}, { 4, 9}, { 4, 8}, { 5,13}, { 6,12}, { 7,12}, { 8,12}, { 8, 8}, { 9,12}, {10,10}, {10, 6}, {10, 2} }		///< trailingOnes = 3.
};

const CTH264VE_VlcCodeType CoeffTokenH264VlcEncoder::nC8up[4][17] = 
{///<	0				 1				2				 3				4				 5				6				 7				8				 9			 10				11			 12				13			 14				15			 16	= totalCoeffs.
	{ { 6, 3}, { 6, 0}, { 6, 4}, { 6, 8}, { 6,12}, { 6,16}, { 6,20}, { 6,24}, { 6,28}, { 6,32}, { 6,36}, { 6,40}, { 6,44}, { 6,48}, { 6,52}, { 6,56}, { 6,60} },	///< trailingOnes = 0.
	{ { 0, 0}, { 6, 1}, { 6, 5}, { 6, 9}, { 6,13}, { 6,17}, { 6,21}, { 6,25}, { 6,29}, { 6,33}, { 6,37}, { 6,41}, { 6,45}, { 6,49}, { 6,53}, { 6,57}, { 6,61} },	///< trailingOnes = 1.
	{ { 0, 0}, { 0, 0}, { 6, 6}, { 6,10}, { 6,14}, { 6,18}, { 6,22}, { 6,26}, { 6,30}, { 6,34}, { 6,38}, { 6,42}, { 6,46}, { 6,50}, { 6,54}, { 6,58}, { 6,62} },	///< trailingOnes = 2.
	{ { 0, 0}, { 0, 0}, { 0, 0}, { 6,11}, { 6,15}, { 6,19}, { 6,23}, { 6,27}, { 6,31}, { 6,35}, { 6,39}, { 6,43}, { 6,47}, { 6,51}, { 6,55}, { 6,59}, { 6,63} }		///< trailingOnes = 3.
};

const CTH264VE_VlcCodeType CoeffTokenH264VlcEncoder::nCneg1[4][5] = 
{///<	0				 1				2				 3				4	= totalCoeffs.
	{ { 2, 1}, { 6, 7}, { 6, 4}, { 6, 3}, { 6, 2} },	///< trailingOnes = 0.
	{ { 0, 0}, { 1, 1}, { 6, 6}, { 7, 3}, { 8, 3} },	///< trailingOnes = 1.
	{ { 0, 0}, { 0, 0}, { 3, 1}, { 7, 2}, { 8, 2} },	///< trailingOnes = 2.
	{ { 0, 0}, { 0, 0}, { 0, 0}, { 6, 5}, { 7, 0} }		///< trailingOnes = 3.
};

const CTH264VE_VlcCodeType CoeffTokenH264VlcEncoder::nCneg2[4][9] = 
{///<	0				 1				2				 3				4				 5				6				 7				8	= totalCoeffs.
	{ { 1, 1}, { 7,15}, { 7,14}, { 9, 7}, { 9, 6}, {10, 7}, {11, 7}, {12, 7}, {13, 7} },	///< trailingOnes = 0.
	{ { 0, 0}, { 2, 1}, { 7,13}, { 7,12}, { 9, 5}, {10, 6}, {11, 6}, {12, 6}, {12, 5} },	///< trailingOnes = 1.
	{ { 0, 0}, { 0, 0}, { 3, 1}, { 7,11}, { 7,10}, { 9, 4}, {10, 5}, {11, 5}, {12, 4} },	///< trailingOnes = 2.
	{ { 0, 0}, { 0, 0}, { 0, 0}, { 5, 1}, { 6, 1}, { 7, 9}, { 7, 8}, {10, 4}, {11, 4} }		///< trailingOnes = 3.
};

