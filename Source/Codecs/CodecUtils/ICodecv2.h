/** @file

MODULE				: ICodecv2

TAG						: ICV2

FILE NAME			: ICodecv2.h

DESCRIPTION		: A ICodecv2 Interface as an abstract base class to all 
								video	and audio codecs.

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
#ifndef _ICODECV2_H
#define _ICODECV2_H

#pragma once

class ICodecv2
{
public:
	virtual ~ICodecv2() {}

	/** Codec parameter access.
	This is the primary method of Get/Set for all parameters and caters for
	all variants by using strings. Care must be taken when unicode strings
	are used. Typical usage:
		char		valBuff[20];
		int			len;
		if(!Impl->GetParameter((const char*)("width"), &len, (void *)valBuff))
			errorStr = (char *)Impl->GetErrorStr();

	@param type		:	A string of the parameter required.
	@param length	:	The length of the string returned in value.
	@param value	:	A string representing the value required.
	@return				: 1 = success, 0 = no parameter exists with this name.
	*/
	virtual int GetParameter(const char* type, int* length, void* value) = 0;

	/** Get parameter names.
	After requesting a "parameters" parameter to get the total no. of parameters
	then all the names may be referenced as a list.
	@param ordinal	: The list index.
	@param name			: String name.
	@param length		: The length of the string returned in name.
	@return					: none.
	*/
	virtual void GetParameterName(int ordinal, const char** name, int* length) = 0;

	/** Set codec parameter.
	This is the primary method of Get/Set for all parameters and caters for
	all variants by using strings. Care must be taken when unicode strings
	are used. Typical usage:
		char		Buffer[20];
		int			width = some;
		if(!Impl->SetParameter((char *)("width"), (char *)_itoa(width,Buffer,10));
			errorStr = (char *)Impl->GetErrorStr();

	@param type		:	A string of the parameter required.
	@param value	:	A string representing the value required.
	@return				: 1 = success, 0 = no parameter exists with this name.
	*/
	virtual int SetParameter(const char* type, const char* value) = 0;

	/** Get the last error string.
	@return	: Error string reference.
	*/
	virtual char* GetErrorStr(void) = 0;
	
	/** The codec is ready.
	This is signaled when a successful Open() has occurred before a 
	Close() is called.
	@return	: 1 = Is ready, 0 = not ready.
	*/
	virtual int Ready(void) = 0;

	/** Get the coded bit length.
	This indicates the exact no. of bits/bytes in the coded stream.
	@return	: The length.
	*/
	virtual int GetCompressedBitLength(void) = 0;
	virtual int GetCompressedByteLength(void) = 0;

	/** Access the reference images.
	Get a untyped ptr to the reference list where refNum = 0 is the latest.
	@param refNum	: References back in time (current = 0).
	@return				: Ptr to the reference.
	*/
	virtual void* GetReference(int refNum) = 0;

	/** Restart the codec.
	Used to generate key frames/pages/packets that have no prior
	dependencies on previously coded frames. Resets any prediction
	loops.
	@return	: none.
	*/
	virtual void Restart(void) = 0;

	/** Open the codec.
	Codec is opened with the parameter set described by the result
	of several SetParameter() calls. Defaults must be appropriate.
	@return	: 1 = success, 0 = failure.
	*/
	virtual int Open(void) = 0;

	/** Close an opened codec.
	Clear all memory allocations and reset the parameters to defaults.
	@return	: 1 = success, 0 = failure.
	*/
	virtual int Close(void) = 0;
	
	/** Encode the source.
	@param pSrc						: Source reference whose type is implicitly defined.
	@param pCmp						: Dest of the encoding.
	@param codeParameter	: Codec dependent interpretation.
	@return								: 1 = success, 0 = failure.
	*/
	virtual int Code(void* pSrc, void* pCmp, int codeParameter) = 0;

	/** Decode to the dest.
	@param pCmp				: Source reference of the encoded stream.
	@param bitLength	: Bit length of the source pCmp.
	@param pDst				: Dest reference whose type is implicitly defined.
	@return						: 1 = success, 0 = failure.
	*/
	virtual int Decode(void* pCmp, int bitLength, void* pDst) = 0;

};// end class ICodecv2.

#endif	//_ICODECV2_H
