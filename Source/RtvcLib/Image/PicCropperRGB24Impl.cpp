/** @file

MODULE				: PicCropperRGB24Impl

FILE NAME			: PicCropperRGB24Impl.cpp

DESCRIPTION			: An RGB24 implementation derived from the general
					PicInPicBase() class. Insert a packed RGB24 image
					into another packed RGB24 image.
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008, CSIR
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of the CSIR nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

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

#include "PicCropperRGB24Impl.h"

/*
===========================================================================
	Interface Methods.
===========================================================================
*/
/** Crop the input image into the output image.
Crop the input image by {_byX1, _byX2, _byY1, _byY2} from the {left, right,
bottom, top} and write it into the output image. Memory management is the
responsibility of the calling process. Very little checking is done to
prevent memory overwriting.
@param pImg		: Packed RGB 888 format output image.
@param pInImg	: Packed RGB 888 format input image.
@return				: 0 = failed, 1 = success.
*/
int PicCropperRGB24Impl::Crop(void* pInImg, void* pImg)
{
	if( (pImg == NULL) || (pInImg == NULL) )
		return(0);

	/// Check the cropping to ensure it will fit and not overlap.
	int writeWidth = _widthIn - _byX1 - _byX2;
	int writeHeight = _heightIn - _byY1 - _byY2;
	if( (writeWidth < 0) || (writeHeight < 0) )
		return(0);
	if(writeWidth > _widthOut) writeWidth = _widthOut;
	if(writeHeight > _heightOut) writeHeight = _heightOut;

	/// Align src and dst pointers. Src pointer is offset into the input image.
	unsigned char*	pSrc	= (unsigned char*)pInImg + 3*((_byY1*_widthIn) + _byX1);
	unsigned char*	pDst	= (unsigned char*)pImg;

	for(int y = 0; y < writeHeight; y++)
	{
		memcpy((void *)pDst, (const void *)pSrc, 3*writeWidth);	/// Whole row at a time.
		pDst += (3*_widthOut);	/// Next row.
		pSrc += (3*_widthIn);
	}//end for y...

	return(1);
}//end Crop.

