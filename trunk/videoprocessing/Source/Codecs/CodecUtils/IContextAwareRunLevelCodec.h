/** @file

MODULE				: IContextAwareRunLevelCodec

TAG						: ICARLC

FILE NAME			: IContextAwareRunLevelCodec.h

DESCRIPTION		: A IContextAwareRunLevelCodec interface as an abstract base 
								class (well, almost) to context-aware run-level implementations
								that promote decoupling from the implementation.

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
#ifndef _ICONTEXTAWARERUNLEVELCODEC_H
#define _ICONTEXTAWARERUNLEVELCODEC_H

#pragma once

class IContextAwareRunLevelCodec
{
public:
	virtual ~IContextAwareRunLevelCodec() {}

	/** Encode the input to a run-level bit stream.
	The input and output structs are implementation	specific and therefore
	undefined in the interface.
	@param in			:	Input list to encode.
	@param stream	:	Run-level encoded stream.
	@return				: Implementation specific return code.
	*/
	virtual int Encode(void* in, void* stream) = 0;

	/** Decode a run-level bit stream to the output.
	The input and output structs are implementation specific and therefore
	undefined in the interface.
	@param stream	:	Run-level stream to decode.
	@param out		:	Output list generated by the decode.
	@return				: Implementation specific return code.
	*/
	virtual int Decode(void* stream, void* out) = 0;

	/** Set and get the mode for the implementation.
	Implementations require some flexibility in the interface	to provide extra
	modes of operation that are internal. Most implementations will have empty 
	methods.
	@param mode	: Mode to set.
	@return			: None (Set) or the mode (Get).
	*/
	virtual void	SetMode(int mode) = 0;
	virtual int		GetMode(void) = 0;

	/** Set and get parameters for the implementation.
	Implementations require some flexibility in the interface	to provide extra
	parameters that are internal. Most implementations will have empty methods.
	@param paramID	: Parameter to set/get.
	@param paramVal	: Parameter value.
	@return					: None (Set) or the param value (Get).
	*/
	virtual void	SetParameter(int paramID, int paramVal) = 0;
	virtual int		GetParameter(int paramID) = 0;

public:
	/// Modes available.
	static const int Mode2x2 = 0;
	static const int Mode4x4 = 1;
	static const int Mode8x8 = 2;

	/// Parameter IDs.
	static const int NUM_TOT_NEIGHBOR_COEFF_ID	= 0;
	static const int DC_SKIP_FLAG_ID						= 1;
	static const int NUM_TOT_COEFF_ID						= 2;

};// end class IContextAwareRunLevelCodec.

#endif	//_ICONTEXTAWARERUNLEVELCODEC_H
