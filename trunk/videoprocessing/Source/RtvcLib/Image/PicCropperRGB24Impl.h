/** @file

MODULE				: PicCropperRGB24Impl

FILE NAME			: PicCropperRGB24Impl.h

DESCRIPTION			: An RGB24 implementation derived from the general
					PicCropperBase() class. Crop a packed RGB24 image
					into another packed RGB24 image.
					  					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008 - 2012, CSIR
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
#ifndef _PICCROPPERRGB24IMPL_H
#define _PICCROPPERRGB24IMPL_H

#pragma once

#include "PicCropperBase.h"

/**
 * \ingroup ImageLib
 * An RGB24 implementation derived from the general
 * PicCropperBase() class. Crop a packed RGB24 image
 * into another packed RGB24 image.
 */
class PicCropperRGB24Impl: public PicCropperBase
{
public:
	// Construction and destruction.
	PicCropperRGB24Impl(void) { }
	PicCropperRGB24Impl(int widthOut, int heightOut, int widthIn, int heightIn, int left, int right, int top, int bottom): PicCropperBase(widthOut,heightOut,widthIn,heightIn,left,right,top,bottom) { }
	virtual ~PicCropperRGB24Impl(void) {}

	// Interface.
	int Crop(void* pInImg, void* pImg);

};//end PicCropperRGB24Impl.

#endif	// _PICCROPPERRGB24IMPL_H
