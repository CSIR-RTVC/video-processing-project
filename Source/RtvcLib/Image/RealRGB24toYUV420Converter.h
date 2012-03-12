/** @file

MODULE				: RealRGB24toYUV420Converter

FILE NAME			: RealRGB24toYUV420Converter.h

DESCRIPTION			: Double precision floating point RGB 24 bit to YUV420 colour 
					convertion derived from the RGBtoYUV420Converter base class.
					Use this implementation as the reference.
	  
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
#ifndef _REALRGB24TOYUV420CONVERTER_H
#define _REALRGB24TOYUV420CONVERTER_H

#pragma once

#include "RGBtoYUV420Converter.h"

/**
 * \ingroup ImageLib
 * Double precision floating point RGB 24 bit to YUV420 colour 
 * convertion derived from the RGBtoYUV420Converter base class.
 * Use this implementation as the reference.
 */
class RealRGB24toYUV420Converter: public RGBtoYUV420Converter
{
public:
    /// Construction and destruction.
    RealRGB24toYUV420Converter(void) { }
    RealRGB24toYUV420Converter(int width, int height): RGBtoYUV420Converter(width,height) { }
    RealRGB24toYUV420Converter(int width, int height, int chrOff): RGBtoYUV420Converter(width,height, chrOff) { }
    virtual ~RealRGB24toYUV420Converter(void) {}

    /// Interface.
    void Convert(void* pRgb, void* pY, void* pU, void* pV);

protected:
  void FlipConvert(void* pRgb, void* pY, void* pU, void* pV);
  void NonFlipConvert(void* pRgb, void* pY, void* pU, void* pV);

};//end _REALRGB24TOYUV420CONVERTERIMPL2_H.


#endif
