/** @file

MODULE				: TotalZeros2x2H264VlcDecoder

TAG						: TZ2H264VD

FILE NAME			: TotalZeros2x2H264VlcDecoder.cpp

DESCRIPTION		: A total zeros Vlc decoder implementation as defined in
								H.264 Recommendation (03/2005) Table 9.9(a) page 205. The 
								total number of coeffs defines which vlc array to use. 
								This implementation is implemented with an IVlcDecoder 
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

===========================================================================
*/
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include "TotalZeros2x2H264VlcDecoder.h"

/*
---------------------------------------------------------------------------
	Construction and destruction.
---------------------------------------------------------------------------
*/
TotalZeros2x2H264VlcDecoder::TotalZeros2x2H264VlcDecoder(void)
{
	_numCodeBits	= 0;
}//end constructor.

TotalZeros2x2H264VlcDecoder::~TotalZeros2x2H264VlcDecoder(void)
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
int	TotalZeros2x2H264VlcDecoder::Decode2(IBitStreamReader* bsr, int* symbol1, int* symbol2)
{
	int totalCoeffs	= *symbol2;	///< Used as an input.

	/// Reset bits extracted.
  _numCodeBits	= 0;
	int tZ				= 0;

	/// Select the method depending on the total number of coeffs.
	switch(totalCoeffs)
	{
		case 1:
			{
				if(bsr->Read())	///< 1
				{
					_numCodeBits = 1; tZ = 0;
				}//end if 1...
				else						///< 0
				{
					if(bsr->Read())	///< 01
					{
						_numCodeBits = 2; tZ = 1;
					}//end if 01...
					else						///< 00
					{
						if(bsr->Read())	///< 001
						{
							_numCodeBits = 3; tZ = 2;
						}//end if 001...
						else						///< 000
						{
							_numCodeBits = 3; tZ = 3;
						}//end else 000...
					}//end else 00...
				}//end else 0...
			}//end case 1...
			break;
		case 2:
			{
				if(bsr->Read())	///< 1
				{
					_numCodeBits = 1; tZ = 0;
				}//end if 1...
				else						///< 0
				{
					if(bsr->Read())	///< 01
					{
						_numCodeBits = 2; tZ = 1;
					}//end if 01...
					else						///< 00
					{
						_numCodeBits = 2; tZ = 2;
					}//end else 00...
				}//end else 0...
			}//end case 2...
			break;
		case 3:
			{
				if(bsr->Read())	///< 1
				{
					_numCodeBits = 1; tZ = 0;
				}//end if 1...
				else						///< 0
				{
					_numCodeBits = 1; tZ = 1;
				}//end else 0...
			}//end case 3...
			break;
		default:	///< Invalid input.
			_numCodeBits = 0;	tZ = 0;
			break;
	}//end switch totalCoeffs...

	/// Load 'em up.
	*symbol1 = tZ;

  return(_numCodeBits);
}//end Decode2.


