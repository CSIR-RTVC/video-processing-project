/** @file

MODULE				: TotalZeros4x4H264VlcDecoder

TAG						: TZ4H264VD

FILE NAME			: TotalZeros4x4H264VlcDecoder.cpp

DESCRIPTION		: A total zeros Vlc decoder implementation as defined in
								H.264 Recommendation (03/2005) Table 9.7 page 204. The 
								total number of coeffs defines which vlc array to use. 
								This implementation is implemented with an IVlcDecoder 
								Interface.

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
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include "TotalZeros4x4H264VlcDecoder.h"

/*
---------------------------------------------------------------------------
	Construction and destruction.
---------------------------------------------------------------------------
*/
TotalZeros4x4H264VlcDecoder::TotalZeros4x4H264VlcDecoder(void)
{
	_numCodeBits	= 0;
}//end constructor.

TotalZeros4x4H264VlcDecoder::~TotalZeros4x4H264VlcDecoder(void)
{
}//end destructor.

/*
---------------------------------------------------------------------------
	Interface Methods.
---------------------------------------------------------------------------
*/
/** Decode total coeff - trailing ones - neighbour total coeffs symbols from the bit stream.
The first 2 param symbols represent total zeros and total coeffs. The total coeffs is an 
input that selects the appropriate vlc array for the total zero decoding. For fast implementation 
the tree splitting is done in the code.
@param bsr			: Bit stream to get from.
@param symbol1	: Returned total zeros for this block
@param symbol2	: Total num of coeffs in this block
@return					: Num of bits extracted.
*/
int	TotalZeros4x4H264VlcDecoder::Decode2(IBitStreamReader* bsr, int* symbol1, int* symbol2)
{
	int totalCoeffs	= *symbol2;	///< Used as an input.

	/// Reset bits extracted.
  _numCodeBits = 0;

	/// Select the method depending on the total number of coeffs.
	switch(totalCoeffs)
	{
		case 1:
			_numCodeBits	= TC1(bsr, symbol1);
			break;
		case 2:
			_numCodeBits	= TC2(bsr, symbol1);
			break;
		case 3:
			_numCodeBits	= TC3(bsr, symbol1);
			break;
		case 4:
			_numCodeBits	= TC4(bsr, symbol1);
			break;
		case 5:
			_numCodeBits	= TC5(bsr, symbol1);
			break;
		case 6:
			_numCodeBits	= TC6(bsr, symbol1);
			break;
		case 7:
			_numCodeBits	= TC7(bsr, symbol1);
			break;
		case 8:
			_numCodeBits	= TC8(bsr, symbol1);
			break;
		case 9:
			_numCodeBits	= TC9(bsr, symbol1);
			break;
		case 10:
			_numCodeBits	= TC10(bsr, symbol1);
			break;
		case 11:
			{
				if(bsr->Read())	///< 1
				{
					_numCodeBits = 1; *symbol1 = 4;
				}//end if 1...
				else						///< 0
				{
					if(bsr->Read())	///< 01
					{
						if(bsr->Read())	///< 011
						{
							_numCodeBits = 3; *symbol1 = 5;
						}//end if 011...
						else						///< 010
						{
							_numCodeBits = 3; *symbol1 = 3;
						}//end else 010...
					}//end if 01...
					else						///< 00
					{
						if(bsr->Read())	///< 001
						{
							_numCodeBits = 3; *symbol1 = 2;
						}//end if 001...
						else						///< 000
						{
							_numCodeBits = 4; *symbol1 = bsr->Read();
						}//end else 000...
					}//end else 00...
				}//end else 0...
			}//end case 11...
			break;
		case 12:
			{
				if(bsr->Read())	///< 1
				{
					_numCodeBits = 1; *symbol1 = 3;
				}//end if 1...
				else						///< 0
				{
					if(bsr->Read())	///< 01
					{
						_numCodeBits = 2; *symbol1 = 2;
					}//end if 01...
					else						///< 00
					{
						if(bsr->Read())	///< 001
						{
							_numCodeBits = 3; *symbol1 = 4;
						}//end if 001...
						else						///< 000
						{
							_numCodeBits = 4; *symbol1 = bsr->Read();
						}//end else 000...
					}//end else 00...
				}//end else 0...
			}//end case 12...
			break;
		case 13:
			{
				if(bsr->Read())	///< 1
				{
					_numCodeBits = 1; *symbol1 = 2;
				}//end if 1...
				else						///< 0
				{
					if(bsr->Read())	///< 01
					{
						_numCodeBits = 2; *symbol1 = 3;
					}//end if 01...
					else						///< 00
					{
						_numCodeBits = 3; *symbol1 = bsr->Read();
					}//end else 00...
				}//end else 0...
			}//end case 13...
			break;
		case 14:
			{
				if(bsr->Read())	///< 1
				{
					_numCodeBits = 1; *symbol1 = 2;
				}//end if 1...
				else						///< 0
				{
					_numCodeBits = 2; *symbol1 = bsr->Read();
				}//end else 0...
			}//end case 14...
			break;
		case 15:
			{
				_numCodeBits = 1; *symbol1	= bsr->Read();
			}//end case 15...
			break;
		default:	///< Invalid input.
			_numCodeBits = 0;	*symbol1 = 0;
			break;
	}//end switch totalCoeffs...

  return(_numCodeBits);
}//end Decode2.

/*
---------------------------------------------------------------------------
	Private Methods.
---------------------------------------------------------------------------
*/
/** Decode total zeros symbols from the bit stream.
For total coeffs from 1 to 10.
@param bsr				: Bit stream to get from.
@param totalZeros	: Returned total zeros for this block
@return						: Num of bits extracted.
*/
int	TotalZeros4x4H264VlcDecoder::TC1(IBitStreamReader* bsr, int* totalZeros)
{
	int numBits	= 0;
	int tZs			= 0;

	if(bsr->Read())	///< 1
	{
		numBits = 1; tZs = 0;
	}//end if 1...
	else						///< 0
	{
		if(bsr->Read())	///< 01
		{
			if(bsr->Read())	///< 011
			{
				numBits = 3; tZs = 1;
			}//end if 011...
			else						///< 010
			{
				numBits = 3; tZs = 2;
			}//end else 010...
		}//end if 01...
		else						///< 00
		{
			if(bsr->Read())	///< 001
			{
				if(bsr->Read())	///< 0011
				{
					numBits = 4; tZs = 3;
				}//end if 0011...
				else						///< 0010
				{
					numBits = 4; tZs = 4;
				}//end else 0010...
			}//end if 001...
			else						///< 000
			{
				if(bsr->Read())	///< 0001
				{
					if(bsr->Read())	///< 0001 1
					{
						numBits = 5; tZs = 5;
					}//end if 0001 1...
					else						///< 0001 0
					{
						numBits = 5; tZs = 6;
					}//end else 0001 0 ...
				}//end if 0001...
				else						///< 0000
				{
					if(bsr->Read())	///< 0000 1
					{
						if(bsr->Read())	///< 0000 11
						{
							numBits = 6; tZs = 7;
						}//end if 0000 11...
						else						///< 0000 10
						{
							numBits = 6; tZs = 8;
						}//end else 0000 10 ...
					}//end if 0000 1...
					else						///< 0000 0
					{
						if(bsr->Read())	///< 0000 01
						{
							if(bsr->Read())	///< 0000 011
							{
								numBits = 7; tZs = 9;
							}//end if 0000 011...
							else						///< 0000 010
							{
								numBits = 7; tZs = 10;
							}//end else 0000 010 ...
						}//end if 0000 01...
						else						///< 0000 00
						{
							if(bsr->Read())	///< 0000 001
							{
								if(bsr->Read())	///< 0000 0011
								{
									numBits = 8; tZs = 11;
								}//end if 0000 0011...
								else						///< 0000 0010
								{
									numBits = 8; tZs = 12;
								}//end else 0000 0010 ...
							}//end if 0000 001...
							else						///< 0000 000
							{
								if(bsr->Read())	///< 0000 0001
								{
									if(bsr->Read())	///< 0000 0001 1
									{
										numBits = 9; tZs = 13;
									}//end if 0000 0001 1...
									else						///< 0000 0001 0
									{
										numBits = 9; tZs = 14;
									}//end else 0000 0001 0 ...
								}//end if 0000 0001...
								else						///< 0000 0000
								{
									if(bsr->Read())	///< 0000 0000 1
									{
										numBits = 9; tZs = 15;
									}//end if 0000 0000 1...
								}//end else 0000 0000 ...
							}//end else 0000 000 ...
						}//end else 0000 00 ...
					}//end else 0000 0 ...
				}//end else 0000...
			}//end else 000...
		}//end else 00...
	}//end else 0...

	/// Load 'em up.
	*totalZeros	= tZs;

	return(numBits);
}//end TC1.

int	TotalZeros4x4H264VlcDecoder::TC2(IBitStreamReader* bsr, int* totalZeros)
{
	int numBits	= 0;
	int tZs			= 0;
	int bits;

	if(bsr->Read())	///< 1
	{
		bits = bsr->Read(2);
		switch(bits)
		{
			case 0:		///< 100
				tZs = 3;
				break;
			case 1:		///< 101
				tZs = 2;
				break;
			case 2:		///< 110
				tZs = 1;
				break;
			case 3:		///< 111
				tZs = 0;
				break;
		}//end switch bits...
		numBits = 3;
	}//end if 1...
	else						///< 0
	{
		if(bsr->Read())	///< 01
		{
			if(bsr->Read())	///< 011
			{
				numBits = 3; tZs = 4;
			}//end if 011...
			else						///< 010
			{
				if(bsr->Read())	///< 0101
				{
					numBits = 4; tZs = 5;
				}//end if 0101...
				else						///< 0100
				{
					numBits = 4; tZs = 6;
				}//end else 0100...
			}//end else 010...
		}//end if 01...
		else						///< 00
		{
			if(bsr->Read())	///< 001
			{
				if(bsr->Read())	///< 0011
				{
					numBits = 4; tZs = 7;
				}//end if 0011...
				else						///< 0010
				{
					numBits = 4; tZs = 8;
				}//end else 0010...
			}//end if 001...
			else						///< 000
			{
				if(bsr->Read())	///< 0001
				{
					if(bsr->Read())	///< 0001 1
					{
						numBits = 5; tZs = 9;
					}//end if 0001 1...
					else						///< 0001 0
					{
						numBits = 5; tZs = 10;
					}//end else 0001 0...
				}//end if 0001...
				else						///< 0000
				{
					if(bsr->Read())	///< 0000 1
					{
						if(bsr->Read())	///< 0000 11
						{
							numBits = 6; tZs = 11;
						}//end if 0000 11...
						else						///< 0000 10
						{
							numBits = 6; tZs = 12;
						}//end else 0000 10...
					}//end if 0000 1...
					else						///< 0000 0
					{
						if(bsr->Read())	///< 0000 01
						{
							numBits = 6; tZs = 13;
						}//end if 0000 01...
						else						///< 0000 00
						{
							numBits = 6; tZs = 14;
						}//end else 0000 00...
					}//end else 0000 0...
				}//end else 0000...
			}//end else 000...
		}//end else 00...
	}//end else 0...

	/// Load 'em up.
	*totalZeros	= tZs;

	return(numBits);
}//end TC2.

int	TotalZeros4x4H264VlcDecoder::TC3(IBitStreamReader* bsr, int* totalZeros)
{
	int numBits	= 0;
	int tZs			= 0;
	int bits;

	if(bsr->Read())	///< 1
	{
		bits = bsr->Read(2);
		switch(bits)
		{
			case 0:		///< 100
				tZs = 6;
				break;
			case 1:		///< 101
				tZs = 3;
				break;
			case 2:		///< 110
				tZs = 2;
				break;
			case 3:		///< 111
				tZs = 1;
				break;
		}//end switch bits...
		numBits = 3;
	}//end if 1...
	else						///< 0
	{
		if(bsr->Read())	///< 01
		{
			if(bsr->Read())	///< 011
			{
				numBits = 3; tZs = 7;
			}//end if 011...
			else						///< 010
			{
				if(bsr->Read())	///< 0101
				{
					numBits = 4; tZs = 0;
				}//end if 0101...
				else						///< 0100
				{
					numBits = 4; tZs = 4;
				}//end else 0100...
			}//end else 010...
		}//end if 01...
		else						///< 00
		{
			if(bsr->Read())	///< 001
			{
				if(bsr->Read())	///< 0011
				{
					numBits = 4; tZs = 5;
				}//end if 0011...
				else						///< 0010
				{
					numBits = 4; tZs = 8;
				}//end else 0010...
			}//end if 001...
			else						///< 000
			{
				if(bsr->Read())	///< 0001
				{
					if(bsr->Read())	///< 0001 1
					{
						numBits = 5; tZs = 9;
					}//end if 0001 1...
					else						///< 0001 0
					{
						numBits = 5; tZs = 10;
					}//end else 0001 0...
				}//end if 0001...
				else						///< 0000
				{
					if(bsr->Read())	///< 0000 1
					{
						numBits = 5; tZs = 12;
					}//end if 0000 1...
					else						///< 0000 0
					{
						if(bsr->Read())	///< 0000 01
						{
							numBits = 6; tZs = 11;
						}//end if 0000 01...
						else						///< 0000 00
						{
							numBits = 6; tZs = 13;
						}//end else 0000 00...
					}//end else 0000 0...
				}//end else 0000...
			}//end else 000...
		}//end else 00...
	}//end else 0...

	/// Load 'em up.
	*totalZeros	= tZs;

	return(numBits);
}//end TC3.

int	TotalZeros4x4H264VlcDecoder::TC4(IBitStreamReader* bsr, int* totalZeros)
{
	int numBits	= 0;
	int tZs			= 0;
	int bits;

	if(bsr->Read())	///< 1
	{
		bits = bsr->Read(2);
		switch(bits)
		{
			case 0:		///< 100
				tZs = 6;
				break;
			case 1:		///< 101
				tZs = 5;
				break;
			case 2:		///< 110
				tZs = 4;
				break;
			case 3:		///< 111
				tZs = 1;
				break;
		}//end switch bits...
		numBits = 3;
	}//end if 1...
	else						///< 0
	{
		if(bsr->Read())	///< 01
		{
			if(bsr->Read())	///< 011
			{
				numBits = 3; tZs = 8;
			}//end if 011...
			else						///< 010
			{
				if(bsr->Read())	///< 0101
				{
					numBits = 4; tZs = 2;
				}//end if 0101...
				else						///< 0100
				{
					numBits = 4; tZs = 3;
				}//end else 0100...
			}//end else 010...
		}//end if 01...
		else						///< 00
		{
			if(bsr->Read())	///< 001
			{
				if(bsr->Read())	///< 0011
				{
					numBits = 4; tZs = 7;
				}//end if 0011...
				else						///< 0010
				{
					numBits = 4; tZs = 9;
				}//end else 0010...
			}//end if 001...
			else						///< 000
			{
				if(bsr->Read())	///< 0001
				{
					if(bsr->Read())	///< 0001 1
					{
						numBits = 5; tZs = 0;
					}//end if 0001 1...
					else						///< 0001 0
					{
						numBits = 5; tZs = 10;
					}//end else 0001 0...
				}//end if 0001...
				else						///< 0000
				{
					if(bsr->Read())	///< 0000 1
					{
						numBits = 5; tZs = 11;
					}//end if 0000 1...
					else						///< 0000 0
					{
						numBits = 5; tZs = 12;
					}//end else 0000 0...
				}//end else 0000...
			}//end else 000...
		}//end else 00...
	}//end else 0...

	/// Load 'em up.
	*totalZeros	= tZs;

	return(numBits);
}//end TC4.

int	TotalZeros4x4H264VlcDecoder::TC5(IBitStreamReader* bsr, int* totalZeros)
{
	int numBits	= 0;
	int tZs			= 0;
	int bits;

	if(bsr->Read())	///< 1
	{
		bits = bsr->Read(2);
		switch(bits)
		{
			case 0:		///< 100
				tZs = 6;
				break;
			case 1:		///< 101
				tZs = 5;
				break;
			case 2:		///< 110
				tZs = 4;
				break;
			case 3:		///< 111
				tZs = 3;
				break;
		}//end switch bits...
		numBits = 3;
	}//end if 1...
	else						///< 0
	{
		if(bsr->Read())	///< 01
		{
			if(bsr->Read())	///< 011
			{
				numBits = 3; tZs = 7;
			}//end if 011...
			else						///< 010
			{
				if(bsr->Read())	///< 0101
				{
					numBits = 4; tZs = 0;
				}//end if 0101...
				else						///< 0100
				{
					numBits = 4; tZs = 1;
				}//end else 0100...
			}//end else 010...
		}//end if 01...
		else						///< 00
		{
			if(bsr->Read())	///< 001
			{
				if(bsr->Read())	///< 0011
				{
					numBits = 4; tZs = 2;
				}//end if 0011...
				else						///< 0010
				{
					numBits = 4; tZs = 8;
				}//end else 0010...
			}//end if 001...
			else						///< 000
			{
				if(bsr->Read())	///< 0001
				{
					numBits = 4; tZs = 10;
				}//end if 0001...
				else						///< 0000
				{
					if(bsr->Read())	///< 0000 1
					{
						numBits = 5; tZs = 9;
					}//end if 0000 1...
					else						///< 0000 0
					{
						numBits = 5; tZs = 11;
					}//end else 0000 0...
				}//end else 0000...
			}//end else 000...
		}//end else 00...
	}//end else 0...

	/// Load 'em up.
	*totalZeros	= tZs;

	return(numBits);
}//end TC5.

int	TotalZeros4x4H264VlcDecoder::TC6(IBitStreamReader* bsr, int* totalZeros)
{
	int numBits	= 0;
	int tZs			= 0;
	int bits;

	if(bsr->Read())	///< 1
	{
		bits = bsr->Read(2);
		switch(bits)
		{
			case 0:		///< 100
				tZs = 5;
				break;
			case 1:		///< 101
				tZs = 4;
				break;
			case 2:		///< 110
				tZs = 3;
				break;
			case 3:		///< 111
				tZs = 2;
				break;
		}//end switch bits...
		numBits = 3;
	}//end if 1...
	else						///< 0
	{
		if(bsr->Read())	///< 01
		{
			if(bsr->Read())	///< 011
			{
				numBits = 3; tZs = 6;
			}//end if 011...
			else						///< 010
			{
				numBits = 3; tZs = 7;
			}//end else 010...
		}//end if 01...
		else						///< 00
		{
			if(bsr->Read())	///< 001
			{
				numBits = 3; tZs = 9;
			}//end if 001...
			else						///< 000
			{
				if(bsr->Read())	///< 0001
				{
					numBits = 4; tZs = 8;
				}//end if 0001...
				else						///< 0000
				{
					if(bsr->Read())	///< 0000 1
					{
						numBits = 5; tZs =1 ;
					}//end if 0000 1...
					else						///< 0000 0
					{
						if(bsr->Read())	///< 0000 01
						{
							numBits = 6; tZs = 0;
						}//end if 0000 01...
						else						///< 0000 00
						{
							numBits = 6; tZs = 10;
						}//end else 0000 00...
					}//end else 0000 0...
				}//end else 0000...
			}//end else 000...
		}//end else 00...
	}//end else 0...

	/// Load 'em up.
	*totalZeros	= tZs;

	return(numBits);
}//end TC6.

int	TotalZeros4x4H264VlcDecoder::TC7(IBitStreamReader* bsr, int* totalZeros)
{
	int numBits	= 0;
	int tZs			= 0;

	if(bsr->Read())	///< 1
	{
		if(bsr->Read())	///< 11
		{
			numBits = 2; tZs = 5;
		}//end if 11...
		else						///< 10
		{
			if(bsr->Read())	///< 101
			{
				numBits = 3; tZs = 2;
			}//end if 101...
			else						///< 100
			{
				numBits = 3; tZs = 3;
			}//end else 100...
		}//end else 10...
	}//end if 1...
	else						///< 0
	{
		if(bsr->Read())	///< 01
		{
			if(bsr->Read())	///< 011
			{
				numBits = 3; tZs = 4;
			}//end if 011...
			else						///< 010
			{
				numBits = 3; tZs = 6;
			}//end else 010...
		}//end if 01...
		else						///< 00
		{
			if(bsr->Read())	///< 001
			{
				numBits = 3; tZs = 8;
			}//end if 001...
			else						///< 000
			{
				if(bsr->Read())	///< 0001
				{
					numBits = 4; tZs = 7;
				}//end if 0001...
				else						///< 0000
				{
					if(bsr->Read())	///< 0000 1
					{
						numBits = 5; tZs = 1;
					}//end if 0000 1...
					else						///< 0000 0
					{
						if(bsr->Read())	///< 0000 01
						{
							numBits = 6; tZs = 0;
						}//end if 0000 01...
						else						///< 0000 00
						{
							numBits = 6; tZs = 9;
						}//end else 0000 00...
					}//end else 0000 0...
				}//end else 0000...
			}//end else 000...
		}//end else 00...
	}//end else 0...

	/// Load 'em up.
	*totalZeros	= tZs;

	return(numBits);
}//end TC7.

int	TotalZeros4x4H264VlcDecoder::TC8(IBitStreamReader* bsr, int* totalZeros)
{
	int numBits	= 0;
	int tZs			= 0;

	if(bsr->Read())	///< 1
	{
		if(bsr->Read())	///< 11
		{
			numBits = 2; tZs = 4;
		}//end if 11...
		else						///< 10
		{
			numBits = 2; tZs = 5;
		}//end else 10...
	}//end if 1...
	else						///< 0
	{
		if(bsr->Read())	///< 01
		{
			if(bsr->Read())	///< 011
			{
				numBits = 3; tZs = 3;
			}//end if 011...
			else						///< 010
			{
				numBits = 3; tZs = 6;
			}//end else 010...
		}//end if 01...
		else						///< 00
		{
			if(bsr->Read())	///< 001
			{
				numBits = 3; tZs = 7;
			}//end if 001...
			else						///< 000
			{
				if(bsr->Read())	///< 0001
				{
					numBits = 4; tZs = 1;
				}//end if 0001...
				else						///< 0000
				{
					if(bsr->Read())	///< 0000 1
					{
						numBits = 5; tZs = 2;
					}//end if 0000 1...
					else						///< 0000 0
					{
						if(bsr->Read())	///< 0000 01
						{
							numBits = 6; tZs = 0;
						}//end if 0000 01...
						else						///< 0000 00
						{
							numBits = 6; tZs = 8;
						}//end else 0000 00...
					}//end else 0000 0...
				}//end else 0000...
			}//end else 000...
		}//end else 00...
	}//end else 0...

	/// Load 'em up.
	*totalZeros	= tZs;

	return(numBits);
}//end TC8.

int	TotalZeros4x4H264VlcDecoder::TC9(IBitStreamReader* bsr, int* totalZeros)
{
	int numBits	= 0;
	int tZs			= 0;

	if(bsr->Read())	///< 1
	{
		if(bsr->Read())	///< 11
		{
			numBits = 2; tZs = 3;
		}//end if 11...
		else						///< 10
		{
			numBits = 2; tZs = 4;
		}//end else 10...
	}//end if 1...
	else						///< 0
	{
		if(bsr->Read())	///< 01
		{
			numBits = 2; tZs = 6;
		}//end if 01...
		else						///< 00
		{
			if(bsr->Read())	///< 001
			{
				numBits = 3; tZs = 5;
			}//end if 001...
			else						///< 000
			{
				if(bsr->Read())	///< 0001
				{
					numBits = 4; tZs = 2;
				}//end if 0001...
				else						///< 0000
				{
					if(bsr->Read())	///< 0000 1
					{
						numBits = 5; tZs = 7;
					}//end if 0000 1...
					else						///< 0000 0
					{
						if(bsr->Read())	///< 0000 01
						{
							numBits = 6; tZs = 0;
						}//end if 0000 01...
						else						///< 0000 00
						{
							numBits = 6; tZs = 1;
						}//end else 0000 00...
					}//end else 0000 0...
				}//end else 0000...
			}//end else 000...
		}//end else 00...
	}//end else 0...

	/// Load 'em up.
	*totalZeros	= tZs;

	return(numBits);
}//end TC9.

int	TotalZeros4x4H264VlcDecoder::TC10(IBitStreamReader* bsr, int* totalZeros)
{
	int numBits	= 0;
	int tZs			= 0;

	if(bsr->Read())	///< 1
	{
		if(bsr->Read())	///< 11
		{
			numBits = 2; tZs = 3;
		}//end if 11...
		else						///< 10
		{
			numBits = 2; tZs = 4;
		}//end else 10...
	}//end if 1...
	else						///< 0
	{
		if(bsr->Read())	///< 01
		{
			numBits = 2; tZs = 5;
		}//end if 01...
		else						///< 00
		{
			if(bsr->Read())	///< 001
			{
				numBits = 3; tZs = 2;
			}//end if 001...
			else						///< 000
			{
				if(bsr->Read())	///< 0001
				{
					numBits = 4; tZs = 6;
				}//end if 0001...
				else						///< 0000
				{
					if(bsr->Read())	///< 0000 1
					{
						numBits = 5; tZs = 0;
					}//end if 0000 1...
					else						///< 0000 0
					{
						numBits = 5; tZs = 1;
					}//end else 0000 0...
				}//end else 0000...
			}//end else 000...
		}//end else 00...
	}//end else 0...

	/// Load 'em up.
	*totalZeros	= tZs;

	return(numBits);
}//end TC10.

