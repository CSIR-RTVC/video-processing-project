/** @file

MODULE						: IRunLengthCodec

TAG								: IRLC

FILE NAME					: IRunLengthCodec.h

DESCRIPTION				: A IRunLengthCodec interface as an abstract base class 
										to run-length implementations that promote decoupling 
										of the implementation. 

REVISION HISTORY	:

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
#ifndef _IRUNLENGTHCODEC_H
#define _IRUNLENGTHCODEC_H

#pragma once

class IRunLengthCodec
{
public:
	virtual ~IRunLengthCodec() {}

	/** Encode the input to a run-length structure.
	The input and output rle struct are implementation
	specific and therefore undefined in the interface.
	@param in		:	Input list to encode.
	@param rle	:	Run-length struct generated by the encode.
	@return			: none.
	*/
	virtual void Encode(void* in, void* rle) = 0;

	/** Decode a run-length struct to the output.
	The input rle struct and output are implementation
	specific and therefore undefined in the interface.
	@param rle	:	Run-length struct to decode.
	@param out	:	Output list generated by the decode.
	@return			: none.
	*/
	virtual void Decode(void* rle, void* out) = 0;

	/** Set and get the mode for the implementation.
	Implementations require some flexibility in the interface
	to provide extra modes of operation that are internal. Most
	implementations will have empty methods.
	@param mode	: Mode to set.
	@return			: None (Set) or the mode (Get).
	*/
	virtual void SetMode(int mode) = 0;
	virtual int GetMode(void) = 0;

};// end class IRunLengthCodec.

#endif	//_IRUNLENGTHCODEC_H