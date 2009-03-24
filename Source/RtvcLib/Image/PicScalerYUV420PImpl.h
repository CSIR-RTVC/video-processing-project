/** @file

MODULE				: PicScalerYUV420PImpl

FILE NAME			: PicScalerYUV420PImpl.h

DESCRIPTION			: A YUV420P planar implementation derived from the 
					general PicScalerBase() class. Scale a YUV420P
					planar image to the dimensions of another YUV420P
					planar image.

					  		  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008, Meraka Institute
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of the Meraka Institute nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

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
#ifndef _PICSCALERYUV420PIMPL_H
#define _PICSCALERYUV420PIMPL_H

#pragma once

#include "PicScalerBase.h"

#ifdef _BUILD_FOR_SHORT
typedef short scaleType;
#else
typedef char scaleType ;
#endif

/*
===========================================================================
  Class definition.
===========================================================================
*/
class PicScalerYUV420PImpl: public PicScalerBase
{
public:
	// Construction and destruction.
	PicScalerYUV420PImpl(void) { }
	PicScalerYUV420PImpl(int widthOut, int heightOut, int widthIn, int heightIn): PicScalerBase(widthOut,heightOut,widthIn,heightIn) { }
	virtual ~PicScalerYUV420PImpl(void) {}

	// Interface.
	int Scale(void* pOutImg, void* pInImg);

};//end PicScalerYUV420PImpl.


#endif	// _PICSCALERYUV420PIMPL_H
