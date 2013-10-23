/** @file

MODULE				: CoeffTokenH264VlcDecoder

TAG						: CTH264VD

FILE NAME			: CoeffTokenH264VlcDecoder.cpp

DESCRIPTION		: A coeff token Vlc decoder implementation as defined in
								H.264 Recommendation (03/2005) Table 9.2 page 200. The 
								trailing ones and total coeffs are combined in this token 
								and is dependent on the neighbouring blocks' total number
								of coeffs. This implementation is implemented with an
								IVlcDecoder Interface.

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

#include "CoeffTokenH264VlcDecoder.h"

/*
---------------------------------------------------------------------------
	Construction and destruction.
---------------------------------------------------------------------------
*/
CoeffTokenH264VlcDecoder::CoeffTokenH264VlcDecoder(void)
{
	_numCodeBits	= 0;
}//end constructor.

CoeffTokenH264VlcDecoder::~CoeffTokenH264VlcDecoder(void)
{
}//end destructor.

/*
---------------------------------------------------------------------------
	Interface Methods.
---------------------------------------------------------------------------
*/
/** Decode total coeff - trailing ones - neighbour total coeffs symbols from the bit stream.
The first 2 param symbols represent total coeffs and trailing ones are decoded from the stream. The 
3rd symbol is the neighbourhood total coeffs that is an input to this method and is used to select
the appropriate codeword table. For fast implementation the tree splitting is done in the code.
@param bsr			: Bit stream to get from.
@param symbol1	: Returned total coeffs for this block
@param symbol2	: Returned num of trailing ones for this block
@param symbol3	: Total num of coeffs in neighbourhood blocks
@return					: Num of bits extracted.
*/
int	CoeffTokenH264VlcDecoder::Decode3(IBitStreamReader* bsr, int* symbol1, int* symbol2, int* symbol3)
{
	int numTotNeighborCoeff	= *symbol3;	///< Used as an input.

	/// Reset bits extracted.
  _numCodeBits = 0;

	/// Select the table depending on the neighbourhood total number of coeffs.
	switch(numTotNeighborCoeff)
	{
		case 0:
		case 1:
			_numCodeBits	= NC0to1(bsr, symbol1, symbol2);
			break;
		case 2:
		case 3:
			_numCodeBits	= NC2to3(bsr, symbol1, symbol2);
			break;
		case 4:
		case 5:
		case 6:
		case 7:
			_numCodeBits	= NC4to7(bsr, symbol1, symbol2);
			break;
		case -1:
			_numCodeBits	= NCneg1(bsr, symbol1, symbol2);
			break;
		case -2:
			_numCodeBits	= NCneg2(bsr, symbol1, symbol2);
			break;
		default:	///< Greater than or equal to 8.
			_numCodeBits	= NC8up(bsr, symbol1, symbol2);
			break;
	}//end switch numTotNeighborCoeff...

  return(_numCodeBits);
}//end Decode3.

/*
---------------------------------------------------------------------------
	Private Methods.
---------------------------------------------------------------------------
*/
/** Decode total coeff - trailing ones symbols from the bit stream.
For neighbourhood total coeffs from 0 to 1, 2 to 3, 4 to 7, above 8, -1
and -2.
@param bsr					: Bit stream to get from.
@param totalCoeffs	: Returned total coeffs for this block
@param trailingOnes	: Returned num of trailing ones for this block
@return							: Num of bits extracted.
*/
int	CoeffTokenH264VlcDecoder::NC0to1(IBitStreamReader* bsr, int* totalCoeffs, int* trailingOnes)
{
	int numBits	= 0;
	int tCs			= 0;
	int tOs			= 0;
	int bits;

	/// Work through the bits from the stream one at a time or in groups if appropriate.
	if(bsr->Read())	///< 1
	{
		numBits	= 1; tOs = 0; tCs = 0;
	}//end if 1...
	else						///< 0
	{
		if(bsr->Read())	///< 01
		{
			numBits	= 2; tOs = 1; tCs = 1;
		}//end if 01...
		else						///< 00
		{
			if(bsr->Read())	///< 001
			{
				numBits	= 3; tOs = 2; tCs = 2;
			}//end if 001...
			else						///< 000
			{
				if(bsr->Read())	///< 0001
				{
					if(bsr->Read())	///< 0001 1
					{
						numBits	= 5; tOs = 3; tCs = 3;
					}//end if 0001 1...
					else						///< 0001 0
					{
						if(bsr->Read())	///< 0001 01
						{
							numBits	= 6; tOs = 0; tCs = 1;
						}//end if 0001 01...
						else						///< 0001 00
						{
							numBits	= 6; tOs = 1; tCs = 2;
						}//end else 0001 00...
					}//end else 0001 0...
				}//end if 0001...
				else						///< 0000
				{
					if(bsr->Read())	///< 0000 1
					{
						if(bsr->Read())	///< 0000 11
						{
							numBits	= 6; tOs = 3; tCs = 4;
						}//end if 0000 11...
						else						///< 0000 10
						{
							if(bsr->Read())	///< 0000 101
							{
								numBits	= 7; tOs = 2; tCs = 3;
							}//end if 0000 101...
							else						///< 0000 100
							{
								numBits	= 7; tOs = 3; tCs = 5;
							}//end else 0000 100...
						}//end else 0000 10...
					}//end if 0000 1...
					else						///< 0000 0
					{
						if(bsr->Read())	///< 0000 01
						{
							bits = bsr->Read(2);
							switch(bits)
							{
								case 0:			///< 0000 0100
									numBits	= 8; tOs = 3; tCs = 6;
									break;
								case 1:			///< 0000 0101
									numBits	= 8; tOs = 2; tCs = 4;
									break;
								case 2:			///< 0000 0110
									numBits	= 8; tOs = 1; tCs = 3;
									break;
								case 3:			///< 0000 0111
									numBits	= 8; tOs = 0; tCs = 2;
									break;
							}//end switch bits...
						}//end if 0000 01...
						else						///< 0000 00
						{
							if(bsr->Read())	///< 0000 001
							{
								bits = bsr->Read(2);
								switch(bits)
								{
									case 0:			///< 0000 0010 0
										numBits	= 9; tOs = 3; tCs = 7;
										break;
									case 1:			///< 0000 0010 1
										numBits	= 9; tOs = 2; tCs = 5;
										break;
									case 2:			///< 0000 0011 0
										numBits	= 9; tOs = 1; tCs = 4;
										break;
									case 3:			///< 0000 0011 1
										numBits	= 9; tOs = 0; tCs = 3;
										break;
								}//end switch bits...
							}//end if 0000 001...
							else						///< 0000 000
							{
								if(bsr->Read())	///< 0000 0001
								{
									bits = bsr->Read(2);
									switch(bits)
									{
										case 0:			///< 0000 0001 00
											numBits	= 10; tOs = 3; tCs = 8;
											break;
										case 1:			///< 0000 0001 01
											numBits	= 10; tOs = 2; tCs = 6;
											break;
										case 2:			///< 0000 0001 10
											numBits	= 10; tOs = 1; tCs = 5;
											break;
										case 3:			///< 0000 0001 11
											numBits	= 10; tOs = 0; tCs = 4;
											break;
									}//end switch bits...
								}//end if 0000 0001...
								else						///< 0000 0000
								{
									if(bsr->Read())	///< 0000 0000 1
									{
										bits = bsr->Read(2);
										switch(bits)
										{
											case 0:			///< 0000 0000 100
												numBits	= 11; tOs = 3; tCs = 9;
												break;
											case 1:			///< 0000 0000 101
												numBits	= 11; tOs = 2; tCs = 7;
												break;
											case 2:			///< 0000 0000 110
												numBits	= 11; tOs = 1; tCs = 6;
												break;
											case 3:			///< 0000 0000 111
												numBits	= 11; tOs = 0; tCs = 5;
												break;
										}//end switch bits...
									}//end if 0000 0000 1...
									else						///< 0000 0000 0
									{
										if(bsr->Read())	///< 0000 0000 01
										{
											bits = bsr->Read(3);
											switch(bits)
											{
												case 0:			///< 0000 0000 0100 0
													numBits	= 13; tOs = 0; tCs = 8;
													break;
												case 1:			///< 0000 0000 0100 1
													numBits	= 13; tOs = 2; tCs = 9;
													break;
												case 2:			///< 0000 0000 0101 0
													numBits	= 13; tOs = 1; tCs = 8;
													break;
												case 3:			///< 0000 0000 0101 1
													numBits	= 13; tOs = 0; tCs = 7;
													break;
												case 4:			///< 0000 0000 0110 0
													numBits	= 13; tOs = 3; tCs = 10;
													break;
												case 5:			///< 0000 0000 0110 1
													numBits	= 13; tOs = 2; tCs = 8;
													break;
												case 6:			///< 0000 0000 0111 0
													numBits	= 13; tOs = 1; tCs = 7;
													break;
												case 7:			///< 0000 0000 0111 1
													numBits	= 13; tOs = 0; tCs = 6;
													break;
											}//end switch bits...
										}//end if 0000 0000 01...
										else						///< 0000 0000 00
										{
											if(bsr->Read())	///< 0000 0000 001
											{
												bits = bsr->Read(3);
												switch(bits)
												{
													case 0:			///< 0000 0000 0010 00
														numBits	= 14; tOs = 3; tCs = 12;
														break;
													case 1:			///< 0000 0000 0010 01
														numBits	= 14; tOs = 2; tCs = 11;
														break;
													case 2:			///< 0000 0000 0010 10
														numBits	= 14; tOs = 1; tCs = 10;
														break;
													case 3:			///< 0000 0000 0010 11
														numBits	= 14; tOs = 0; tCs = 10;
														break;
													case 4:			///< 0000 0000 0011 00
														numBits	= 14; tOs = 3; tCs = 11;
														break;
													case 5:			///< 0000 0000 0011 01
														numBits	= 14; tOs = 2; tCs = 10;
														break;
													case 6:			///< 0000 0000 0011 10
														numBits	= 14; tOs = 1; tCs = 9;
														break;
													case 7:			///< 0000 0000 0011 11
														numBits	= 14; tOs = 0; tCs = 9;
														break;
												}//end switch bits...
											}//end if 0000 0000 001...
											else						///< 0000 0000 000
											{
												if(bsr->Read())	///< 0000 0000 0001
												{
													bits = bsr->Read(3);
													switch(bits)
													{
														case 0:			///< 0000 0000 0001 000
															numBits	= 15; tOs = 3; tCs = 14;
															break;
														case 1:			///< 0000 0000 0001 001
															numBits	= 15; tOs = 2; tCs = 13;
															break;
														case 2:			///< 0000 0000 0001 010
															numBits	= 15; tOs = 1; tCs = 12;
															break;
														case 3:			///< 0000 0000 0001 011
															numBits	= 15; tOs = 0; tCs = 12;
															break;
														case 4:			///< 0000 0000 0001 100
															numBits	= 15; tOs = 3; tCs = 13;
															break;
														case 5:			///< 0000 0000 0001 101
															numBits	= 15; tOs = 2; tCs = 12;
															break;
														case 6:			///< 0000 0000 0001 110
															numBits	= 15; tOs = 1; tCs = 11;
															break;
														case 7:			///< 0000 0000 0001 111
															numBits	= 15; tOs = 0; tCs = 11;
															break;
													}//end switch bits...
												}//end if 0000 0000 0001...
												else						///< 0000 0000 0000
												{
													if(bsr->Read())	///< 0000 0000 0000 1
													{
														bits = bsr->Read(3);
														switch(bits)
														{
															case 0:			///< 0000 0000 0000 1000
																numBits	= 16; tOs = 3; tCs = 16;
																break;
															case 1:			///< 0000 0000 0000 1001
																numBits	= 16; tOs = 2; tCs = 15;
																break;
															case 2:			///< 0000 0000 0000 1010
																numBits	= 16; tOs = 1; tCs = 15;
																break;
															case 3:			///< 0000 0000 0000 1011
																numBits	= 16; tOs = 0; tCs = 14;
																break;
															case 4:			///< 0000 0000 0000 1100
																numBits	= 16; tOs = 3; tCs = 15;
																break;
															case 5:			///< 0000 0000 0000 1101
																numBits	= 16; tOs = 2; tCs = 14;
																break;
															case 6:			///< 0000 0000 0000 1110
																numBits	= 16; tOs = 1; tCs = 14;
																break;
															case 7:			///< 0000 0000 0000 1111
																numBits	= 16; tOs = 0; tCs = 13;
																break;
														}//end switch bits...
													}//end if 0000 0000 0000 1...
													else						///< 0000 0000 0000 0
													{
														if(bsr->Read())	///< 0000 0000 0000 01
														{
															bits = bsr->Read(2);
															switch(bits)
															{
																case 0:			///< 0000 0000 0000 0100
																	numBits	= 16; tOs = 0; tCs = 16;
																	break;
																case 1:			///< 0000 0000 0000 0101
																	numBits	= 16; tOs = 2; tCs = 16;
																	break;
																case 2:			///< 0000 0000 0000 0110
																	numBits	= 16; tOs = 1; tCs = 16;
																	break;
																case 3:			///< 0000 0000 0000 0111
																	numBits	= 16; tOs = 0; tCs = 15;
																	break;
															}//end switch bits...
														}//end if 0000 0000 0000 01...
														else						///< 0000 0000 0000 00
														{
															if(bsr->Read())	///< 0000 0000 0000 001
															{
																numBits	= 15; tOs = 1; tCs = 13;
															}//end if 0000 0000 0000 001...
														}//end else 0000 0000 0000 00...
													}//end else 0000 0000 0000 0...
												}//end else 0000 0000 0000...
											}//end else 0000 0000 000...
										}//end else 0000 0000 00...
									}//end else 0000 0000 0...
								}//end else 0000 0000...
							}//end else 0000 000...
						}//end else 0000 00...
					}//end else 0000 0...
				}//end else 0000...
			}//end else 000...
		}//end else 00...
	}//end else 0...

	/// Load 'em up.
	*totalCoeffs	= tCs;
	*trailingOnes = tOs;

	return(numBits);
}//end NC0to1.

int	CoeffTokenH264VlcDecoder::NC2to3(IBitStreamReader* bsr, int* totalCoeffs, int* trailingOnes)
{
	int numBits	= 0;
	int tCs			= 0;
	int tOs			= 0;
	int bits;

	/// Work through the bits from the stream one at a time or in groups if appropriate.
	if(bsr->Read())	///< 1
	{
		if(bsr->Read())	///< 11
		{
			numBits	= 2; tOs = 0; tCs = 0;
		}//end if 11...
		else						///< 10
		{
			numBits	= 2; tOs = 1; tCs = 1;
		}//end else 10...
	}//end if 1...
	else						///< 0
	{
		if(bsr->Read())	///< 01
		{
			if(bsr->Read())	///< 011
			{
				numBits	= 3; tOs = 2; tCs = 2;
			}//end if 011...
			else						///< 010
			{
				if(bsr->Read())	///< 0101
				{
					numBits	= 4; tOs = 3; tCs = 3;
				}//end if 0101...
				else						///< 0100
				{
					numBits	= 4; tOs = 3; tCs = 4;
				}//end else 0100...
			}//end else 010...
		}//end if 01...
		else						///< 00
		{
			if(bsr->Read())	///< 001
			{
				if(bsr->Read())	///< 0011
				{
					if(bsr->Read())	///< 0011 1
					{
						numBits	= 5; tOs = 1; tCs = 2;
					}//end if 0011 1...
					else						///< 0011 0
					{
						numBits	= 5; tOs = 3; tCs = 5;
					}//end else 0011 0...
				}//end if 0011...
				else						///< 0010
				{
					bits = bsr->Read(2);
					switch(bits)
					{
						case 0:			///< 0010 00
							numBits	= 6; tOs = 3; tCs = 6;
							break;
						case 1:			///< 0010 01
							numBits	= 6; tOs = 2; tCs = 3;
							break;
						case 2:			///< 0010 10
							numBits	= 6; tOs = 1; tCs = 3;
							break;
						case 3:			///< 0010 11
							numBits	= 6; tOs = 0; tCs = 1;
							break;
					}//end switch bits...
				}//end else 0010...
			}//end if 001...
			else						///< 000
			{
				if(bsr->Read())	///< 0001
				{
					bits = bsr->Read(2);
					switch(bits)
					{
						case 0:			///< 0001 00
							numBits	= 6; tOs = 3; tCs = 7;
							break;
						case 1:			///< 0001 01
							numBits	= 6; tOs = 2; tCs = 4;
							break;
						case 2:			///< 0001 10
							numBits	= 6; tOs = 1; tCs = 4;
							break;
						case 3:			///< 0001 11
							numBits	= 6; tOs = 0; tCs = 2;
							break;
					}//end switch bits...
				}//end if 0001...
				else						///< 0000
				{
					if(bsr->Read())	///< 0000 1
					{
						bits = bsr->Read(2);
						switch(bits)
						{
							case 0:			///< 0000 100
								numBits	= 7; tOs = 3; tCs = 8;
								break;
							case 1:			///< 0000 101
								numBits	= 7; tOs = 2; tCs = 5;
								break;
							case 2:			///< 0000 110
								numBits	= 7; tOs = 1; tCs = 5;
								break;
							case 3:			///< 0000 111
								numBits	= 7; tOs = 0; tCs = 3;
								break;
						}//end switch bits...
					}//end if 0000 1...
					else						///< 0000 0
					{
						if(bsr->Read())	///< 0000 01
						{
							bits = bsr->Read(2);
							switch(bits)
							{
								case 0:			///< 0000 0100
									numBits	= 8; tOs = 0; tCs = 5;
									break;
								case 1:			///< 0000 0101
									numBits	= 8; tOs = 2; tCs = 6;
									break;
								case 2:			///< 0000 0110
									numBits	= 8; tOs = 1; tCs = 6;
									break;
								case 3:			///< 0000 0111
									numBits	= 8; tOs = 0; tCs = 4;
									break;
							}//end switch bits...
						}//end if 0000 01...
						else						///< 0000 00
						{
							if(bsr->Read())	///< 0000 001
							{
								bits = bsr->Read(2);
								switch(bits)
								{
									case 0:			///< 0000 0010 0
										numBits	= 9; tOs = 3; tCs = 9;
										break;
									case 1:			///< 0000 0010 1
										numBits	= 9; tOs = 2; tCs = 7;
										break;
									case 2:			///< 0000 0011 0
										numBits	= 9; tOs = 1; tCs = 7;
										break;
									case 3:			///< 0000 0011 1
										numBits	= 9; tOs = 0; tCs = 6;
										break;
								}//end switch bits...
							}//end if 0000 001...
							else						///< 0000 000
							{
								if(bsr->Read())	///< 0000 0001
								{
									bits = bsr->Read(3);
									switch(bits)
									{
										case 0:			///< 0000 0001 000
											numBits	= 11; tOs = 3; tCs = 11;
											break;
										case 1:			///< 0000 0001 001
											numBits	= 11; tOs = 2; tCs = 9;
											break;
										case 2:			///< 0000 0001 010
											numBits	= 11; tOs = 1; tCs = 9;
											break;
										case 3:			///< 0000 0001 011
											numBits	= 11; tOs = 0; tCs = 8;
											break;
										case 4:			///< 0000 0001 100
											numBits	= 11; tOs = 3; tCs = 10;
											break;
										case 5:			///< 0000 0001 101
											numBits	= 11; tOs = 2; tCs = 8;
											break;
										case 6:			///< 0000 0001 110
											numBits	= 11; tOs = 1; tCs = 8;
											break;
										case 7:			///< 0000 0001 111
											numBits	= 11; tOs = 0; tCs = 7;
											break;
									}//end switch bits...
								}//end if 0000 0001...
								else						///< 0000 0000
								{
									if(bsr->Read())	///< 0000 0000 1
									{
										bits = bsr->Read(3);
										switch(bits)
										{
											case 0:			///< 0000 0000 1000
												numBits	= 12; tOs = 0; tCs = 11;
												break;
											case 1:			///< 0000 0000 1001
												numBits	= 12; tOs = 2; tCs = 11;
												break;
											case 2:			///< 0000 0000 1010
												numBits	= 12; tOs = 1; tCs = 11;
												break;
											case 3:			///< 0000 0000 1011
												numBits	= 12; tOs = 0; tCs = 10;
												break;
											case 4:			///< 0000 0000 1100
												numBits	= 12; tOs = 3; tCs = 12;
												break;
											case 5:			///< 0000 0000 1101
												numBits	= 12; tOs = 2; tCs = 10;
												break;
											case 6:			///< 0000 0000 1110
												numBits	= 12; tOs = 1; tCs = 10;
												break;
											case 7:			///< 0000 0000 1111
												numBits	= 12; tOs = 0; tCs = 9;
												break;
										}//end switch bits...
									}//end if 0000 0000 1...
									else						///< 0000 0000 0
									{
										if(bsr->Read())	///< 0000 0000 01
										{
											bits = bsr->Read(3);
											switch(bits)
											{
												case 0:			///< 0000 0000 0100 0
													numBits	= 13; tOs = 3; tCs = 14;
													break;
												case 1:			///< 0000 0000 0100 1
													numBits	= 13; tOs = 2; tCs = 13;
													break;
												case 2:			///< 0000 0000 0101 0
													numBits	= 13; tOs = 1; tCs = 13;
													break;
												case 3:			///< 0000 0000 0101 1
													numBits	= 13; tOs = 0; tCs = 13;
													break;
												case 4:			///< 0000 0000 0110 0
													numBits	= 13; tOs = 3; tCs = 13;
													break;
												case 5:			///< 0000 0000 0110 1
													numBits	= 13; tOs = 2; tCs = 12;
													break;
												case 6:			///< 0000 0000 0111 0
													numBits	= 13; tOs = 1; tCs = 12;
													break;
												case 7:			///< 0000 0000 0111 1
													numBits	= 13; tOs = 0; tCs = 12;
													break;
											}//end switch bits...
										}//end if 0000 0000 01...
										else						///< 0000 0000 00
										{
											if(bsr->Read())	///< 0000 0000 001
											{
												if(bsr->Read())	///< 0000 0000 0011
												{
													if(bsr->Read())	///< 0000 0000 0011 1
													{
														numBits	= 13; tOs = 0; tCs = 14;
													}//end if 0000 0000 0011 1...
													else						///< 0000 0000 0011 0
													{
														numBits	= 13; tOs = 2; tCs = 14;
													}//end else 0000 0000 0011 0...
												}//end if 0000 0000 0011...
												else						///< 0000 0000 0010
												{
													bits = bsr->Read(2);
													switch(bits)
													{
														case 0:			///< 0000 0000 0010 00
															numBits	= 14; tOs = 1; tCs = 15;
															break;
														case 1:			///< 0000 0000 0010 01
															numBits	= 14; tOs = 0; tCs = 15;
															break;
														case 2:			///< 0000 0000 0010 10
															numBits	= 14; tOs = 2; tCs = 15;
															break;
														case 3:			///< 0000 0000 0010 11
															numBits	= 14; tOs = 1; tCs = 14;
															break;
													}//end switch bits...
												}//end else 0000 0000 0010...
											}//end if 0000 0000 001...
											else						///< 0000 0000 000
											{
												if(bsr->Read())	///< 0000 0000 0001
												{
													bits = bsr->Read(2);
													switch(bits)
													{
														case 0:			///< 0000 0000 0001 00
															numBits	= 14; tOs = 3; tCs = 16;
															break;
														case 1:			///< 0000 0000 0001 01
															numBits	= 14; tOs = 2; tCs = 16;
															break;
														case 2:			///< 0000 0000 0001 10
															numBits	= 14; tOs = 1; tCs = 16;
															break;
														case 3:			///< 0000 0000 0001 11
															numBits	= 14; tOs = 0; tCs = 16;
															break;
													}//end switch bits...
												}//end if 0000 0000 0001...
												else						///< 0000 0000 0000
												{
													if(bsr->Read())	///< 0000 0000 0000 1
													{
														numBits	= 13; tOs = 3; tCs = 15;
													}//end if 0000 0000 0000 1...
												}//end else 0000 0000 0000...
											}//end else 0000 0000 000...
										}//end else 0000 0000 00...
									}//end else 0000 0000 0...
								}//end else 0000 0000...
							}//end else 0000 000...
						}//end else 0000 00...
					}//end else 0000 0...
				}//end else 0000...
			}//end else 000...
		}//end else 00...
	}//end else 0...

	/// Load 'em up.
	*totalCoeffs	= tCs;
	*trailingOnes = tOs;

	return(numBits);
}//end NC2to3.

int	CoeffTokenH264VlcDecoder::NC4to7(IBitStreamReader* bsr, int* totalCoeffs, int* trailingOnes)
{
	int numBits	= 0;
	int tCs			= 0;
	int tOs			= 0;
	int bits;

	/// Work through the bits from the stream one at a time or in groups if appropriate.
	if(bsr->Read())	///< 1
	{
		bits = bsr->Read(3);
		switch(bits)
		{
			case 0:			///< 1000
				tOs = 3; tCs = 7;
				break;
			case 1:			///< 1001
				tOs = 3; tCs = 6;
				break;
			case 2:			///< 1010
				tOs = 3; tCs = 5;
				break;
			case 3:			///< 1011
				tOs = 3; tCs = 4;
				break;
			case 4:			///< 1100
				tOs = 3; tCs = 3;
				break;
			case 5:			///< 1101
				tOs = 2; tCs = 2;
				break;
			case 6:			///< 1110
				tOs = 1; tCs = 1;
				break;
			case 7:			///< 1111
				tOs = 0; tCs = 0;
				break;
		}//end switch bits...
		numBits = 4;
	}//end if 1...
	else						///< 0
	{
		if(bsr->Read())	///< 01
		{
			bits = bsr->Read(3);
			switch(bits)
			{
				case 0:			///< 0100 0
					tOs = 1; tCs = 5;
					break;
				case 1:			///< 0100 1
					tOs = 2; tCs = 5;
					break;
				case 2:			///< 0101 0
					tOs = 1; tCs = 4;
					break;
				case 3:			///< 0101 1
					tOs = 2; tCs = 4;
					break;
				case 4:			///< 0110 0
					tOs = 1; tCs = 3;
					break;
				case 5:			///< 0110 1
					tOs = 3; tCs = 8;
					break;
				case 6:			///< 0111 0
					tOs = 2; tCs = 3;
					break;
				case 7:			///< 0111 1
					tOs = 1; tCs = 2;
					break;
			}//end switch bits...
			numBits = 5;
		}//end if 01...
		else						///< 00
		{
			if(bsr->Read())	///< 001
			{
				bits = bsr->Read(3);
				switch(bits)
				{
					case 0:			///< 0010 00
						tOs = 0; tCs = 3;
						break;
					case 1:			///< 0010 01
						tOs = 2; tCs = 7;
						break;
					case 2:			///< 0010 10
						tOs = 1; tCs = 7;
						break;
					case 3:			///< 0010 11
						tOs = 0; tCs = 2;
						break;
					case 4:			///< 0011 00
						tOs = 3; tCs = 9;
						break;
					case 5:			///< 0011 01
						tOs = 2; tCs = 6;
						break;
					case 6:			///< 0011 10
						tOs = 1; tCs = 6;
						break;
					case 7:			///< 0011 11
						tOs = 0; tCs = 1;
						break;
				}//end switch bits...
				numBits = 6;
			}//end if 001...
			else						///< 000
			{
				if(bsr->Read())	///< 0001
				{
					bits = bsr->Read(3);
					switch(bits)
					{
						case 0:			///< 0001 000
							tOs = 0; tCs = 7;
							break;
						case 1:			///< 0001 001
							tOs = 0; tCs = 6;
							break;
						case 2:			///< 0001 010
							tOs = 2; tCs = 9;
							break;
						case 3:			///< 0001 011
							tOs = 0; tCs = 5;
							break;
						case 4:			///< 0001 100
							tOs = 3; tCs = 10;
							break;
						case 5:			///< 0001 101
							tOs = 2; tCs = 8;
							break;
						case 6:			///< 0001 110
							tOs = 1; tCs = 8;
							break;
						case 7:			///< 0001 111
							tOs = 0; tCs = 4;
							break;
					}//end switch bits...
					numBits = 7;
				}//end if 0001...
				else						///< 0000
				{
					if(bsr->Read())	///< 0000 1
					{
						bits = bsr->Read(3);
						switch(bits)
						{
							case 0:			///< 0000 1000
								tOs = 3; tCs = 12;
								break;
							case 1:			///< 0000 1001
								tOs = 2; tCs = 11;
								break;
							case 2:			///< 0000 1010
								tOs = 1; tCs = 10;
								break;
							case 3:			///< 0000 1011
								tOs = 0; tCs = 9;
								break;
							case 4:			///< 0000 1100
								tOs = 3; tCs = 11;
								break;
							case 5:			///< 0000 1101
								tOs = 2; tCs = 10;
								break;
							case 6:			///< 0000 1110
								tOs = 1; tCs = 9;
								break;
							case 7:			///< 0000 1111
								tOs = 0; tCs = 8;
								break;
						}//end switch bits...
						numBits = 8;
					}//end if 0000 1...
					else						///< 0000 0
					{
						if(bsr->Read())	///< 0000 01
						{
							bits = bsr->Read(3);
							switch(bits)
							{
								case 0:			///< 0000 0100 0
									tOs = 0; tCs = 12;
									break;
								case 1:			///< 0000 0100 1
									tOs = 2; tCs = 13;
									break;
								case 2:			///< 0000 0101 0
									tOs = 1; tCs = 12;
									break;
								case 3:			///< 0000 0101 1
									tOs = 0; tCs = 11;
									break;
								case 4:			///< 0000 0110 0
									tOs = 3; tCs = 13;
									break;
								case 5:			///< 0000 0110 1
									tOs = 2; tCs = 12;
									break;
								case 6:			///< 0000 0111 0
									tOs = 1; tCs = 11;
									break;
								case 7:			///< 0000 0111 1
									tOs = 0; tCs = 10;
									break;
							}//end switch bits...
							numBits = 9;
						}//end if 0000 01...
						else						///< 0000 00
						{
							if(bsr->Read())	///< 0000 001
							{
								if(bsr->Read())	///< 0000 0011
								{
									if(bsr->Read())	///< 0000 0011 1
									{
										numBits = 9; tOs = 1; tCs = 13;
									}//end if 0000 0011 1...
									else						///< 0000 0011 0
									{
										if(bsr->Read())	///< 0000 0011 01
										{
											numBits = 10; tOs = 0; tCs = 13;
										}//end if 0000 0011 01...
										else						///< 0000 0011 00
										{
											numBits = 10; tOs = 1; tCs = 14;
										}//end else 0000 0011 00...
									}//end else 0000 0011 0...
								}//end if 0000 0011...
								else						///< 0000 0010
								{
									bits = bsr->Read(2);
									switch(bits)
									{
										case 0:			///< 0000 0010 00
											tOs = 1; tCs = 15;
											break;
										case 1:			///< 0000 0010 01
											tOs = 0; tCs = 14;
											break;
										case 2:			///< 0000 0010 10
											tOs = 3; tCs = 14;
											break;
										case 3:			///< 0000 0010 11
											tOs = 2; tCs = 14;
											break;
									}//end switch bits...
									numBits	= 10;
								}//end else 0000 0010...
							}//end if 0000 001...
							else						///< 0000 000
							{
								if(bsr->Read())	///< 0000 0001
								{
									bits = bsr->Read(2);
									switch(bits)
									{
										case 0:			///< 0000 0001 00
											tOs = 1; tCs = 16;
											break;
										case 1:			///< 0000 0001 01
											tOs = 0; tCs = 15;
											break;
										case 2:			///< 0000 0001 10
											tOs = 3; tCs = 15;
											break;
										case 3:			///< 0000 0001 11
											tOs = 2; tCs = 15;
											break;
									}//end switch bits...
									numBits	= 10;
								}//end if 0000 0001...
								else						///< 0000 0000
								{
									if(bsr->Read())	///< 0000 0000 1
									{
										if(bsr->Read())	///< 0000 0000 11
										{
											numBits = 10; tOs = 2; tCs = 16;
										}//end if 0000 0000 11...
										else						///< 0000 0000 10
										{
											numBits = 10; tOs = 3; tCs = 16;
										}//end else 0000 0000 10...
									}//end if 0000 0000 1...
									else						///< 0000 0000 0
									{
										if(bsr->Read())	///< 0000 0000 01
										{
											numBits = 10; tOs = 0; tCs = 16;
										}//end if 0000 0000 01...
									}//end else 0000 0000 0...
								}//end else 0000 0000...
							}//end else 0000 000...
						}//end else 0000 00...
					}//end else 0000 0...
				}//end else 0000...
			}//end else 000...
		}//end else 00...
	}//end else 0...

	/// Load 'em up.
	*totalCoeffs	= tCs;
	*trailingOnes = tOs;

	return(numBits);
}//end NC4to7.

int	CoeffTokenH264VlcDecoder::NC8up(IBitStreamReader* bsr, int* totalCoeffs, int* trailingOnes)
{
	/// Make use of the pattern for the majority (not all) of these codes where the 1st 4 bits
	/// equal trailing coeffs - 1 and the next 2 bits equal the trailing ones. The exception is
	/// when the 1st 4 bits equal zero and the next 2 bits equal 3.

	int tCs	= bsr->Read(4) + 1;
	int tOs	= bsr->Read(2);

	/// Check for the exception.
	if( (tCs == 1)&&(tOs == 3) )	///< Not possible.
	{
		tCs = 0; tOs = 0;
	}//end if tCs...

	/// Load 'em up.
	*totalCoeffs	= tCs;
	*trailingOnes = tOs;

	return(6);
}//end NC8up.

int	CoeffTokenH264VlcDecoder::NCneg1(IBitStreamReader* bsr, int* totalCoeffs, int* trailingOnes)
{
	int numBits = 0;
	int tCs			= 0;
	int tOs			= 0;

	if(bsr->Read())	///< 1
	{
		numBits = 1; tOs = 1; tCs = 1;
	}//end if 1...
	else						///< 0
	{
		if(bsr->Read())	///< 01
		{
			numBits = 2; tOs = 0; tCs = 0;
		}//end if 01...
		else						///< 00
		{
			if(bsr->Read())	///< 001
			{
				numBits = 3; tOs = 2; tCs = 2;
			}//end if 001...
			else						///< 000
			{
				if(bsr->Read())	///< 0001
				{
					if(bsr->Read())	///< 0001 1
					{
						if(bsr->Read())	///< 0001 11
						{
							numBits = 6; tOs = 0; tCs = 1;
						}//end if 0001 11...
						else						///< 0001 10
						{
							numBits = 6; tOs = 1; tCs = 2;
						}//end else 0001 10...
					}//end if 0001 1...
					else						///< 0001 0
					{
						if(bsr->Read())	///< 0001 01
						{
							numBits = 6; tOs = 3; tCs = 3;
						}//end if 0001 01...
						else						///< 0001 00
						{
							numBits = 6; tOs = 0; tCs = 2;
						}//end else 0001 00...
					}//end else 0001 0...
				}//end if 0001...
				else						///< 0000
				{
					if(bsr->Read())	///< 0000 1
					{
						if(bsr->Read())	///< 0000 11
						{
							numBits = 6; tOs = 0; tCs = 3;
						}//end if 0000 11...
						else						///< 0000 10
						{
							numBits = 6; tOs = 0; tCs = 4;
						}//end else 0000 10...
					}//end if 0000 1...
					else						///< 0000 0
					{
						if(bsr->Read())	///< 0000 01
						{
							if(bsr->Read())	///< 0000 011
							{
								numBits = 7; tOs = 1; tCs = 3;
							}//end if 0000 011...
							else						///< 0000 010
							{
								numBits = 7; tOs = 2; tCs = 3;
							}//end else 0000 010...
						}//end if 0000 01...
						else						///< 0000 00
						{
							if(bsr->Read())	///< 0000 001
							{
								if(bsr->Read())	///< 0000 0011
								{
									numBits = 8; tOs = 1; tCs = 4;
								}//end if 0000 0011...
								else						///< 0000 0010
								{
									numBits = 8; tOs = 2; tCs = 4;
								}//end else 0000 0010...
							}//end if 0000 001...
							else						///< 0000 000
							{
								numBits = 7; tOs = 3; tCs = 4;
							}//end else 0000 000...
						}//end else 0000 00...
					}//end else 0000 0...
				}//end else 0000...
			}//end else 000...
		}//end else 00...
	}//end else 0...

	/// Load 'em up.
	*totalCoeffs	= tCs;
	*trailingOnes = tOs;

	return(numBits);
}//end NCneg1.

int	CoeffTokenH264VlcDecoder::NCneg2(IBitStreamReader* bsr, int* totalCoeffs, int* trailingOnes)
{
	int numBits = 0;
	int tCs			= 0;
	int tOs			= 0;
	int bits;

	if(bsr->Read())	///< 1
	{
		numBits = 1; tOs = 0; tCs = 0;
	}//end if 1...
	else						///< 0
	{
		if(bsr->Read())	///< 01
		{
			numBits = 2; tOs = 1; tCs = 1;
		}//end if 01...
		else						///< 00
		{
			if(bsr->Read())	///< 001
			{
				numBits = 3; tOs = 2; tCs = 2;
			}//end if 001...
			else						///< 000
			{
				if(bsr->Read())	///< 0001
				{
					bits = bsr->Read(3);
					switch(bits)
					{
						case 0:			///< 0001 000
							tOs = 3; tCs = 6;
							break;
						case 1:			///< 0001 001
							tOs = 3; tCs = 5;
							break;
						case 2:			///< 0001 010
							tOs = 2; tCs = 4;
							break;
						case 3:			///< 0001 011
							tOs = 2; tCs = 3;
							break;
						case 4:			///< 0001 100
							tOs = 1; tCs = 3;
							break;
						case 5:			///< 0001 101
							tOs = 1; tCs = 2;
							break;
						case 6:			///< 0001 110
							tOs = 0; tCs = 2;
							break;
						case 7:			///< 0001 111
							tOs = 0; tCs = 1;
							break;
					}//end switch bits...
					numBits = 7;
				}//end if 0001...
				else						///< 0000
				{
					if(bsr->Read())	///< 0000 1
					{
						numBits = 5; tOs = 3; tCs = 3;
					}//end if 0000 1...
					else						///< 0000 0
					{
						if(bsr->Read())	///< 0000 01
						{
							numBits = 6; tOs = 3; tCs = 4;
						}//end if 0000 01...
						else						///< 0000 00
						{
							if(bsr->Read())	///< 0000 001
							{
								bits = bsr->Read(2);
								switch(bits)
								{
									case 0:			///< 0000 0010 0
										tOs = 2; tCs = 5;
										break;
									case 1:			///< 0000 0010 1
										tOs = 1; tCs = 4;
										break;
									case 2:			///< 0000 0011 0
										tOs = 0; tCs = 4;
										break;
									case 3:			///< 0000 0011 1
										tOs = 0; tCs = 3;
										break;
								}//end switch bits...
								numBits = 9;
							}//end if 0000 001...
							else						///< 0000 000
							{
								if(bsr->Read())	///< 0000 0001
								{
									bits = bsr->Read(2);
									switch(bits)
									{
										case 0:			///< 0000 0001 00
											tOs = 3; tCs = 7;
											break;
										case 1:			///< 0000 0001 01
											tOs = 2; tCs = 6;
											break;
										case 2:			///< 0000 0001 10
											tOs = 1; tCs = 5;
											break;
										case 3:			///< 0000 0001 11
											tOs = 0; tCs = 5;
											break;
									}//end switch bits...
									numBits = 10;
								}//end if 0000 0001...
								else						///< 0000 0000
								{
									if(bsr->Read())	///< 0000 0000 1
									{
										bits = bsr->Read(2);
										switch(bits)
										{
											case 0:			///< 0000 0000 100
												tOs = 3; tCs = 8;
												break;
											case 1:			///< 0000 0000 101
												tOs = 2; tCs = 7;
												break;
											case 2:			///< 0000 0000 110
												tOs = 1; tCs = 6;
												break;
											case 3:			///< 0000 0000 111
												tOs = 0; tCs = 6;
												break;
										}//end switch bits...
										numBits = 11;
									}//end if 0000 0000 1...
									else						///< 0000 0000 0
									{
										if(bsr->Read())	///< 0000 0000 01
										{
											bits = bsr->Read(2);
											switch(bits)
											{
												case 0:			///< 0000 0000 0100
													tOs = 2; tCs = 8;
													break;
												case 1:			///< 0000 0000 0101
													tOs = 1; tCs = 8;
													break;
												case 2:			///< 0000 0000 0110
													tOs = 1; tCs = 7;
													break;
												case 3:			///< 0000 0000 0111
													tOs = 0; tCs = 7;
													break;
											}//end switch bits...
											numBits = 12;
										}//end if 0000 0000 01...
										else						///< 0000 0000 00
										{
											bits = bsr->Read(3);
											if(bits == 7)	///< 0000 0000 0011 1
											{
												numBits = 13; tOs = 0; tCs = 8;
											}//end if 0000 0000 00011 1...
										}//end else 0000 0000 00...
									}//end else 0000 0000 0...
								}//end else 0000 0000...
							}//end else 0000 000...
						}//end else 0000 00...
					}//end else 0000 0...
				}//end else 0000...
			}//end else 000...
		}//end else 00...
	}//end else 0...

	/// Load 'em up.
	*totalCoeffs	= tCs;
	*trailingOnes = tOs;

	return(numBits);
}//end NCneg2.
