/** @file

MODULE				: RunBeforeH264VlcDecoder

TAG						: RBH264VD

FILE NAME			: RunBeforeH264VlcDecoder.h

DESCRIPTION		: A run before Vlc decoder implementation as defined in
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

===========================================================================
*/
#ifndef _RUNBEFOREH264VLCDECODER_H
#define _RUNBEFOREH264VLCDECODER_H

#pragma once

#include "IVlcDecoder.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class RunBeforeH264VlcDecoder : public IVlcDecoder
{
public:
	RunBeforeH264VlcDecoder();
	virtual ~RunBeforeH264VlcDecoder();

public:
	/// Interface implementation.
	int GetNumDecodedBits(void)	{ return(_numCodeBits); }
	int Marker(void)						{ return(0); }	///< No markers for this decoder.
	/// A single symbol has no meaning for combined multi symbol encoding.
	virtual int Decode(IBitStreamReader* bsr) { _numCodeBits = 0; return(0); } 

	/// Optional interface implementation.
	/// The 2 symbols represent run before (output) and zeros left (input), respectively.
	virtual int	Decode2(IBitStreamReader* bsr, int* symbol1, int* symbol2);

protected:
	int _numCodeBits;	///< Number of coded bits for this symbol.

};// end class RunBeforeH264VlcDecoder.

#endif	// _RUNBEFOREH264VLCDECODER_H
