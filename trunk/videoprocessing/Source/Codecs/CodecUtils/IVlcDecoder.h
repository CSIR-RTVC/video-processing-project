/** @file

MODULE				: IVlcDecoder

TAG						: IVD

FILE NAME			: IVlcDecoder.h

DESCRIPTION		: A IVlcDecoder Interface as an abstract base class. This
								is not all pure virtual and has an extended interface 
								with trivial default implementations.

COPYRIGHT			:	(c)CSIR 2007-2010 all rights resevered

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
#ifndef _IVLCDECODER_H
#define _IVLCDECODER_H

#include "IBitStreamReader.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/

class IVlcDecoder
{
public:
	virtual ~IVlcDecoder() { }

	/*
	--------------------------------------------------------------------------
	Interface implementation.
	--------------------------------------------------------------------------
	*/
	/** Get the number of decoded bits from the last Decode().
	@return	: Total num bits extracted from the bit stream.
	*/
	virtual int GetNumDecodedBits(void)				= 0;

	/** Is it a marker code.
	Test if the last symbol returned from Decode() was a 
	marker code.
	@return	: 0 = not a marker, 1 = is a marker.
	*/
	virtual int Marker(void)									= 0;

	/** Decode the next symbol from the bit stream.
	Extra info associated with the extraction are stored in 
	the state members that may be accessed after this call.
	e.g. Marker(), GetNumDecodedBits(), etc.
	@param bsr	: Bit stream to read from.
	@return			: The next symbol.
	*/
	virtual int Decode(IBitStreamReader* bsr)	= 0;

	/*
	--------------------------------------------------------------------------
	Optional interface implementation.
	--------------------------------------------------------------------------
	*/
	virtual int		Decode2(IBitStreamReader* bsr, int* symbol1, int* symbol2)									{return(0);}
	virtual int		Decode3(IBitStreamReader* bsr, int* symbol1, int* symbol2, int* symbol3)		{return(0);}
	virtual int		Decode3(int numBits, int codeword, int* symbol1, int* symbol2, int* symbol3){return(0);}
	virtual void	SetEsc(int numEscBits, int escMask)																					{ }

};// end class IVlcDecoder.

#endif	// _IVLCDECODER_H
