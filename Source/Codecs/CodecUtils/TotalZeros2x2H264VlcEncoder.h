/** @file

MODULE				: TotalZeros2x2H264VlcEncoder

TAG						: TZ2H264VE

FILE NAME			: TotalZeros2x2H264VlcEncoder.h

DESCRIPTION		: A total zeros Vlc encoder implementation as defined in
								H.264 Recommendation (03/2005) Table 9.9(a) page 205. The 
								total number of coeffs defines which vlc array to use. 
								This implementation is implemented with an IVlcEncoder 
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
#ifndef _TOTALZEROS2X2H264VLCENCODER_H
#define _TOTALZEROS2X2H264VLCENCODER_H

#include "IVlcEncoder.h"

/*
---------------------------------------------------------------------------
	Type definitions.
---------------------------------------------------------------------------
*/
typedef struct _TZ2H264VE_VlcCodeType
{
  int		numBits;	///< Num of bits in code word.
  int		codeWord;	///< Code word bit representation.
} TZ2H264VE_VlcCodeType;

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class TotalZeros2x2H264VlcEncoder : public IVlcEncoder
{
public:
	TotalZeros2x2H264VlcEncoder(void);
	virtual ~TotalZeros2x2H264VlcEncoder(void);

public:
	/// Interface implementation.
	int GetNumCodedBits(void)	{ return(_numCodeBits); }
	int GetCode(void)					{ return(_bitCode); }
	/// A single symbol has no meaning for combined multi symbol encoding.
	virtual int Encode(int symbol) { return(0); }

	/// Optional interface implementation.
	/// The 2 symbols represent total zeros and total coeffs, respectively.
	virtual int	Encode2(int symbol1, int symbol2);

protected:
	int 	_numCodeBits;	///< Number of coded bits for the encoding.
	int 	_bitCode;			///< The code of length numCodeBits.

	/// Constants of table sets.
	static const TZ2H264VE_VlcCodeType table[4][4];

};// end class TotalZeros2x2H264VlcEncoder.

#endif	// end _TOTALZEROS2X2H264VLCENCODER_H.
