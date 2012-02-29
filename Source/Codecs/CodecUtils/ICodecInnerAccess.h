/** @file

MODULE						: ICodecInnerAccess

TAG								: ICIA

FILE NAME					: ICodecInnerAccess.h

DESCRIPTION				: A ICodecInnerAccess Interface as an optional base class 
										to expose the inner members of all video and audio codecs.
										The destructor is protected to prevent deletion	of the 
										associated super class. NOTE: These methods should only
										be called after an Open() call on the ICodecv2 interface.

REVISION HISTORY	:	

COPYRIGHT					:	(c)CSIR, Meraka Institute 2007 - 2008 all rights resevered

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of CSIR, Meraka Institute and has been 
										classified as CONFIDENTIAL.
===========================================================================
*/
#ifndef _ICODECINNERACCESS_H
#define _ICODECINNERACCESS_H

#pragma once

class ICodecInnerAccess
{
protected:
	virtual ~ICodecInnerAccess() {}

public:
	/** Codec member access.
	This is the primary method of Get/Set for all members and caters for
	all variants by using strings and list pointers. Care must be taken 
	when unicode strings are used. Typical usage:
		int		len;
		int*	pMember = (int *)Impl->GetMember((const char*)("macroblocks"), &len);
		if(pMember == NULL)
			errorStr = (char *)Impl->GetErrorStr();

	On acquiring a handle to the member, any changes made to it will be reflected
	in the next call to Code/Decode of the codec. Interferring with the members
	could have disasterous side effects so care must be taken.

	@param type			:	A string of the member required.
	@param length		:	The length of the member list returned.
	@return					: success = member pointer, NULL = no member exists with this name.
	*/
	virtual void* GetMember(const char* type, int* length) = 0;

	/** Get member names.
	After requesting a "members" type to get the total no. of accessible members
	then all the names may be referenced as a list.
	@param ordinal	: The list index.
	@param name			: String name.
	@param length		: The length of the string returned in name.
	@return					: none.
	*/
	virtual void GetMemberName(int ordinal, const char** name, int* length) = 0;

	/** Set codec member.
	This is the primary method of Get/Set for all member and caters for
	all variants by using strings. Care must be taken when unicode strings
	are used. Typical usage:
		int	width = some;
		if(!Impl->SetMember((char *)("width"), (void *)(&width));
			errorStr = (char *)Impl->GetErrorStr();

	@param type		:	A string name of the member to set.
	@param pValue	:	A reference to the value to set.
	@return				: 1 = success, 0 = no member exists with this name.
	*/
	virtual int SetMember(const char* type, void* pValue) = 0;

};// end class ICodecInnerAccess.

#endif	//_ICODECINNERACCESS_H
