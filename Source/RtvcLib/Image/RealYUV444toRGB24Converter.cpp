/** @file

MODULE				: RealYUV420toRGB24Converter

FILE NAME			: RealYUV420toRGB24Converter.cpp

DESCRIPTION			: Floating point implementation of YUV420 (8 bpp) to RGB 24 bit 
					colour conversion derived from the YUV420toRGBConverter base 
					class.
					  
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
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else

#endif
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "RealYUV444toRGB24Converter.h"

/*
===========================================================================
	Macros.
===========================================================================
*/
#define RYUVRGB24C_RANGECHECK_0TO255(x) ( (((x) <= 255)&&((x) >= 0))?((x)):( ((x) > 255)?(255):(0) ) )


/*
===========================================================================
	Private Methods.
===========================================================================
*/

void RealYUV444toRGB24Converter::Convert(void* pInputBuffer, void* pRgb)
{
    BYTE* 	optr	=	(unsigned char *)pRgb;
    BYTE*				inBuffer		= (BYTE *)pInputBuffer;

    int numberOfPixels = _width * _height;

    for (int i = 0; i < numberOfPixels; i++ )
    {
        BYTE v = inBuffer[ (i * 4 ) + 0 ];
        BYTE u = inBuffer[ (i * 4 ) + 1 ];
        BYTE y = inBuffer[ (i * 4 ) + 2 ];
        //drop the 4th byte (= alpha-channel which is always 0 in AYUV) to go from 32bit to 24 bit per pixel

        int r = y + 1.402 * (v - 128);
        int g = y - 0.344 * (u - 128) - 0.714 * (v - 128);
        int b = y + 1.772 * (u - 128);

        optr[i * 3] = (BYTE)RYUVRGB24C_RANGECHECK_0TO255(b);
        optr[(i * 3) + 1] = (BYTE)RYUVRGB24C_RANGECHECK_0TO255(g);
        optr[(i *3) + 2] = (BYTE)RYUVRGB24C_RANGECHECK_0TO255(r);

    }



}//end Convert






