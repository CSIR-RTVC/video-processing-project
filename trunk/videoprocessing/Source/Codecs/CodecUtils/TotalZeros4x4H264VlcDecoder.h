/** @file

MODULE				: TotalZeros4x4H264VlcDecoder

TAG						: TZ4H264VD

FILE NAME			: TotalZeros4x4H264VlcDecoder.h

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
#ifndef _TOTALZEROS4X4H264VLCDECODER_H
#define _TOTALZEROS4X4H264VLCDECODER_H

#pragma once

#include "IVlcDecoder.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class TotalZeros4x4H264VlcDecoder : public IVlcDecoder
{
public:
	TotalZeros4x4H264VlcDecoder();
	virtual ~TotalZeros4x4H264VlcDecoder();

public:
	/// Interface implementation.
	int GetNumDecodedBits(void)	{ return(_numCodeBits); }
	int Marker(void)						{ return(0); }	///< No markers for this decoder.
	/// A single symbol has no meaning for combined multi symbol encoding.
	virtual int Decode(IBitStreamReader* bsr) { _numCodeBits = 0; return(0); } 

	/// Optional interface implementation.
	/// The 2 symbols represent total zeros (output) and total coeffs (input), respectively.
	virtual int	Decode2(IBitStreamReader* bsr, int* symbol1, int* symbol2);

protected:
	/// Private methods.
	int	TC1(IBitStreamReader* bsr, int* totalZeros);
	int	TC2(IBitStreamReader* bsr, int* totalZeros);
	int	TC3(IBitStreamReader* bsr, int* totalZeros);
	int	TC4(IBitStreamReader* bsr, int* totalZeros);
	int	TC5(IBitStreamReader* bsr, int* totalZeros);
	int	TC6(IBitStreamReader* bsr, int* totalZeros);
	int	TC7(IBitStreamReader* bsr, int* totalZeros);
	int	TC8(IBitStreamReader* bsr, int* totalZeros);
	int	TC9(IBitStreamReader* bsr, int* totalZeros);
	int	TC10(IBitStreamReader* bsr, int* totalZeros);

protected:
	int _numCodeBits;	///< Number of coded bits for this symbol.

};// end class TotalZeros4x4H264VlcDecoder.

#endif	// _TOTALZEROS4X4H264VLCDECODER_H
