/** @file

MODULE				: IVlcDecoder

TAG						: IVD

FILE NAME			: IVlcDecoder.h

DESCRIPTION		: A IVlcDecoder Interface as an abstract base class. This
								is not all pure virtual and has an extended interface 
								with trivial default implementations.

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
