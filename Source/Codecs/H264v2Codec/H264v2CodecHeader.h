/** @file

MODULE						: H264v2CodecHeader

TAG								: H264V2CH

FILE NAME					: H264v2CodecHeader.h

DESCRIPTION				: This is a utility class to extract the header info of
										a valid H264v2 bit stream.

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
#ifndef _H264V2CODECHEADER_H
#define _H264V2CODECHEADER_H

#pragma once

#include "IStreamHeaderReader.h"
#include "IBitStreamReader.h"
#include "IVlcDecoder.h"
#include "NalHeaderH264.h"

/**
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class H264v2CodecHeader: public IStreamHeaderReader
{
public:
	H264v2CodecHeader();
	virtual ~H264v2CodecHeader();

/// Interface implementation.
public:
	/** Extract the header from the input stream.
	Extract the header and pointers from the input into the
	class members.
	@param pSS		: Stream to extract from.
	@param bitLen	: Bit length of input stream.
	@return				: 1 = success, 0 = failed.
	*/
	int Extract(void* pSS, int bitLen);

  /** Get the header length in bits.
  @return : Length in bits.
  */
  int GetHeaderBitLength(void) { return(0); } ///< Not implemented.

	/** Get the header value by name.
	This is the primary method of getting all header variables and caters 
	for all variants by using strings. Care must be taken when unicode strings
	are used. Typical usage:
		int	val;
		if( !Impl->Get((const unsigned char*)("width"), &val) )
			errorStr = "Header value does not exist";

	@param name		:	A string of the header variable required.
	@param value	:	The header value required.
	@return				: 1 = found, 0 = no header exists with this name.
	*/
	int Get(const char* name, int* value);

	/// Implementation specific access methods.
public:
	/** Get the header info after extraction.
	The data returned is that from the last call to Extract() into the current 
	stream position.
	@return	: Requested header value.
	*/
	int GetPictureCodingType(void)								{ return(_pictureCodingType); }

/// Constants.
public:
  /// Constants for _pictureCodingType.
  static const int Intra          = 0;
  static const int Inter          = 1;
  static const int SequenceParams = 2;
  static const int PictureParams  = 3;

///	Persistant stream header data valid after an Extract() call.
protected:
	/// Internal picture properties. (For now)

	/// Main picture type of I/P/PB-frame or sequence and picture parameter sets.
	int _pictureCodingType;

	/// Global stream reader.
	IBitStreamReader*	_pBitStreamReader;
  /// Hedaer vlc decoders.
	IVlcDecoder*	    _pHeaderUnsignedVlcDec;
	IVlcDecoder*	    _pHeaderSignedVlcDec;

	/// NAL unit definition.
	NalHeaderH264		_nal;

};// end class H264v2CodecHeader.

#endif	// _H264V2CODECHEADER_H
