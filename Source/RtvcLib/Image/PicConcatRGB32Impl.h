/** @file

MODULE						: PicConcatRGB32Impl

TAG								: PCRGB32I

FILE NAME					: PicConcatRGB32Impl.h

DESCRIPTION				: An RGB32 implementation derived from the general
										PicConcatBase() class. Concatenate a packed RGB32 
										image to another packed RGB32 image.

REVISION HISTORY	: 

COPYRIGHT					: (c)CSIR, CSIR 2007 all rights resevered

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of CSIR, CSIR and has been 
										classified as CONFIDENTIAL.
===========================================================================
*/
#ifndef _PICCONCATRGB32IMPL_H
#define _PICCONCATRGB32IMPL_H

#include "PicConcatBase.h"
#include "PicInPicRGB32Impl.h"

/**
 * \ingroup ImageLib
 * An RGB32 implementation derived from the general
 * PicConcatBase() class. Concatenate a packed RGB32 
 * image to another packed RGB32 image.
 */
class PicConcatRGB32Impl: public PicConcatBase
{
public:
	// Construction and destruction.
	PicConcatRGB32Impl(void);
	PicConcatRGB32Impl(int width, int height, int width1st, int height1st, int width2nd, int height2nd, int orient);
	virtual ~PicConcatRGB32Impl(void);

	// Interface.
	virtual int Concat(void* pImg1st, void* pImg2nd, void* pImg);

protected:
	// Support methods.
	virtual int Create(void);

protected:
	// Members.
	PicInPicRGB32Impl* _pPicInPic;	/// Use a pic-in-pic obj to place the images.

};//end PicConcatRGB32Impl.


#endif _PICCONCATRGB32IMPL_H
