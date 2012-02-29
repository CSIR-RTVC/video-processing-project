/** @file

MODULE						: IStreamHeaderReader

TAG								: ISHR

FILE NAME					: IStreamHeaderReader.h

DESCRIPTION				: An IStreamHeaderReader Interface is an abstract base class 
										to access stream headers that have bit representations. 
										The access is defined by text (ASCII) strings. Derived 
										classes should include members for each header parameter
										and an associated Get() method.

COPYRIGHT					: (c)CSIR, Meraka Institute 2007-2011 all rights reserved

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of CSIR, Meraka Institute and has been 
										classified as CONFIDENTIAL.
===========================================================================
*/
#ifndef _ISTREAMHEADERREADER_H
#define _ISTREAMHEADERREADER_H

#pragma once

class IStreamHeaderReader
{
public:
	virtual ~IStreamHeaderReader() {}

	/** Extract the header from the input stream.
	Extract the header from the input stream into the derived
	class members.
	@param pSS		: Stream to extract from.
	@param bitLen	: Bit length of input stream.
	@return				: 1 = success, 0 = failed.
	*/
	virtual int Extract(void* pSS, int bitLen) = 0;

  /** Get the header length in bits.
  @return : Length in bits.
  */
	virtual int GetHeaderBitLength(void) = 0;

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
	virtual int Get(const char* name, int* value) = 0;

};// end class IStreamHeaderReader.

#endif	///<_ISTREAMHEADERREADER_H
