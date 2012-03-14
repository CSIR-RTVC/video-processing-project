/** @file

MODULE				: ExpGolombUnsignedVlcDecoder

TAG						: EGUVD

FILE NAME			: ExpGolombUnsignedVlcDecoder.h

DESCRIPTION		: An Exp-Golomb Vlc decoder implementation for unsigned 
								integer symbols as used in H.264 Recommendation (03/2005) 
								Table 9.2 page 195. This implementation is implemented 
								with an IVlcDecoder Interface.

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
#ifndef _EXPGOLOMBUNSIGNEDVLCDECODER_H
#define _EXPGOLOMBUNSIGNEDVLCDECODER_H

#pragma once

#include "IVlcDecoder.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class ExpGolombUnsignedVlcDecoder : public IVlcDecoder
{
public:
	ExpGolombUnsignedVlcDecoder();
	virtual ~ExpGolombUnsignedVlcDecoder();

public:
	/// Interface implementation.
	int GetNumDecodedBits(void)	{ return(_numCodeBits); }
	int Marker(void)						{ return(0); }	///< No markers for this decoder.
	/// A single symbol decoding. Returns the sysmbol.
	virtual int Decode(IBitStreamReader* bsr); 

protected:
	int _numCodeBits;	///< Number of coded bits for this symbol.

};// end class ExpGolombUnsignedVlcDecoder.

#endif	// _EXPGOLOMBUNSIGNEDVLCDECODER_H
