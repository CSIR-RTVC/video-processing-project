/** @file

MODULE						: H264v2CodecHeader

TAG								: H264V2CH

FILE NAME					: H264v2CodecHeader.cpp

DESCRIPTION				: This is a utility class to extract the header info of
										a valid H264v2 bit stream.

REVISION HISTORY	:

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
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include "BitStreamReaderMSB.h"
#include "H264v2CodecHeader.h"
#include "ExpGolombUnsignedVlcDecoder.h"
#include "ExpGolombSignedVlcDecoder.h"

H264v2CodecHeader::H264v2CodecHeader()
{
	/// Reset all members.

	/// Internal parameters.
	_pictureCodingType			= H264v2CodecHeader::Intra;

	/// Create a bit stream reader and header vlc decoders to use during bit extraction.
	_pBitStreamReader	      = NULL;
	_pBitStreamReader       = new BitStreamReaderMSB();
  _pHeaderUnsignedVlcDec  = NULL;
	_pHeaderUnsignedVlcDec	= new ExpGolombUnsignedVlcDecoder();
  _pHeaderSignedVlcDec    = NULL;
	_pHeaderSignedVlcDec		= new ExpGolombSignedVlcDecoder();

}//end constructor.

H264v2CodecHeader::~H264v2CodecHeader()
{
	if(_pBitStreamReader != NULL)
		delete _pBitStreamReader;
	_pBitStreamReader = NULL;

	if(_pHeaderUnsignedVlcDec != NULL)
		delete _pHeaderUnsignedVlcDec;
	_pHeaderUnsignedVlcDec = NULL;

	if(_pHeaderSignedVlcDec != NULL)
		delete _pHeaderSignedVlcDec;
	_pHeaderSignedVlcDec = NULL;

}//end destructor.

/** Extract the header from the input stream.
Extract the header and pointers from the input into the
class memebers.
@param pSS		: Stream to extract from.
@param bitLen	: Bit length of input stream.
@return				: 1 = success, 0 = failed.
*/
int H264v2CodecHeader::Extract(void* pSS, int bitLen)
{
	if( (_pBitStreamReader == NULL)||(_pHeaderUnsignedVlcDec == NULL)||(_pHeaderSignedVlcDec == NULL)||(pSS == NULL) )
		return(0);

	_pBitStreamReader->SetStream(pSS, bitLen);

	int bitsSoFar = 0;

	/// The start code is a fixed 32-bit value and the NAL header is a fixed 8 bit sequence.
	if(bitLen < (8+32))
    return(0);
  
  /// Start code.
  int sc = _pBitStreamReader->Read(32);

  /// NAL
	_pBitStreamReader->Read();													///< Discard forbidden zero bit f(1).
	_nal._ref_idc		= 3 & _pBitStreamReader->Read(2);		///< u(2).
	_nal._unit_type = 31 & _pBitStreamReader->Read(5);	///< u(5).
  bitsSoFar += 8;

	switch(_nal._unit_type)
	{
		case NalHeaderH264::IDR_Slice:
			_pictureCodingType = H264v2CodecHeader::Intra;
			break;
		case NalHeaderH264::NonIDR_NoPartition_Slice:
			/// Slice without partitioning and therefore only one set of slice parameters and NAL type = Slice type.
			_pictureCodingType = H264v2CodecHeader::Inter;
			break;
		case NalHeaderH264::SeqParamSet:
			_pictureCodingType = H264v2CodecHeader::SequenceParams;
			break;
		case NalHeaderH264::PicParamSet:
			_pictureCodingType = H264v2CodecHeader::PictureParams;
			break;
		default:
      return(0);
			break;
	}//end switch _unit_type...

  return(1);
}//end Extract.

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
int H264v2CodecHeader::Get(const char* name, int* value)
{
	char* p				= (char *)name;
	int		len			= (int)strlen(p);

	if(_strnicmp(p,"picture coding type",len) == 0)
		*value = _pictureCodingType;
	else
	{
		/// Parameter requested does not exist.
		*value = 0;
		return(0);
	}//end else...

	return(1);
}//end Get.

