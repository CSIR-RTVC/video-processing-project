/** @file

MODULE						: PicInPicRGB24Impl

TAG								: PIPRGB24I

FILE NAME					: PicInPicRGB24Impl.cpp

DESCRIPTION				: An RGB24 implementation derived from the general
										PicInPicBase() class. Insert a packed RGB24 image
										into another packed RGB24 image.

COPYRIGHT					: (c)CSIR 2007-2010 all rights resevered

LICENSE						: Software License Agreement (BSD License)

RESTRICTIONS			: Redistribution and use in source and binary forms, with or without 
										modification, are permitted provided that the following conditions 
										are met:

										* Redistributions of source code must retain the above copyright notice, 
										this list of conditions and the following disclaimer.
										* Redistributions in binary form must reproduce the above copyright notice, 
										this list of conditions and the following disclaimer in the documentation 
										and/or other materials provided with the distribution.
										* Neither the name of the CSIR nor the names of its contributors may be used 
										to endorse or promote products derived from this software without specific 
										prior written permission.

										THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
										"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
										LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
										A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
										CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
										EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
										PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
										PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
										LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
										NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
										SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

===========================================================================
*/
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include <string.h>
#include <stdlib.h>

#include "PicInPicRGB24Impl.h"

/*
===========================================================================
	Interface Methods.
===========================================================================
*/
/** Insert the sub image into the main image.
Write the sub image into the main image at the top-left location of
(_xPos,_yPos). Only write _writeWidth and _writeHeight pixels of the
sub image. These dimensions have been set to stay within the edges
of the main image.
@param pSubImg	: Packed RGB 888 format smaller sub image.
@param pImg			: Packed RGB 888 format main base image.
@return					: none.
*/
void PicInPicRGB24Impl::Insert(void* pSubImg, void* pImg)
{
	unsigned char*	pSrc		= (unsigned char*)pSubImg;
	unsigned char*	pDst		= (unsigned char*)pImg + 3*((_yPos*_width) + _xPos);

	for(int y = 0; y < _writeHeight; y++)
	{
		memcpy((void *)pDst, (const void *)pSrc, 3*_writeWidth);	// Whole row at a time.
		pDst += (3*_width);	// Next row.
		pSrc += (3*_subWidth);
	}//end for y...

}//end Insert.

