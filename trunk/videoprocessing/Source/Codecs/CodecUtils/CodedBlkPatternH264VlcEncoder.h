/** @file

MODULE				: CodedBlkPatternH264VlcEncoder

TAG						: CBPH264VE

FILE NAME			: CodedBlkPatternH264VlcEncoder.h

DESCRIPTION		: A coded block pattern Vlc encoder implementation used in 
								H.264 Recommendation (03/2005)Table 9.4(a) on page 196. 
								This implementation is implemented with an IVlcEncoder 
								Interface and is an extention of the unsigned Exp-Golomb 
								Vlc encoder.

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
#ifndef _CODEDBLKPATTERNH264VLCENCODER_H
#define _CODEDBLKPATTERNH264VLCENCODER_H

#include "ExpGolombUnsignedVlcEncoder.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class CodedBlkPatternH264VlcEncoder : public ExpGolombUnsignedVlcEncoder
{
public:
	CodedBlkPatternH264VlcEncoder(void);
	virtual ~CodedBlkPatternH264VlcEncoder(void);

public:
	/// Does not have a single symbol encoding requirement.
	virtual int Encode(int symbol) { return(0); }

	/// Optional interface.
public:

	/** Encode with a macroblock type
	The 2 symbols represent the coded block pattern value and the 
	macroblock prediction mode (Intra/Inter), respectively.
	@param symbol1	: Block pattern to encode.
	@param symbol2	: Intra/inter = 0/1.
	@return					: Num of bits in the codeword.
	*/
	virtual int	Encode2(int symbol1, int symbol2);

protected:
	/// Constants of table sets.
	static const int IntraTable[48];
	static const int InterTable[48];

};// end class CodedBlkPatternH264VlcEncoder.

#endif	// end _CODEDBLKPATTERNH264VLCENCODER_H.
