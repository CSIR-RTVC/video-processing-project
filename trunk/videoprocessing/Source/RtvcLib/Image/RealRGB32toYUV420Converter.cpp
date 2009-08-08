/** @file

MODULE				: RealRGB32toYUV420Converter

FILE NAME			: RealRGB32toYUV420Converter.cpp

DESCRIPTION			: Double precision floating point RGB 32 bit to YUV420 colour 
					convertion derived from the RGBtoYUV420Converter base class.
					YUV is represented with 8 bpp. Use this implementation as 
					the reference.
					  
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

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "RealRGB32toYUV420Converter.h"

/*
===========================================================================
	Constants.
===========================================================================
*/
#define RRGB32YUVC_00		 0.299
#define RRGB32YUVC_01		 0.587
#define RRGB32YUVC_02		 0.114
#define RRGB32YUVC_10		-0.147
#define RRGB32YUVC_11		-0.289
#define RRGB32YUVC_12		 0.436
#define RRGB32YUVC_20		 0.615
#define RRGB32YUVC_21		-0.515
#define RRGB32YUVC_22		-0.100

/*
===========================================================================
	Interface Methods.
===========================================================================
*/
/** Double precision reference implementation.
The full real matix equation is used. The YUV output is represented with
8 bits per pel and the UV components are adjusted from their -128..127 range
to 0..255.
@param pRgb	: Packed RGB 8888 format.
@param pY		: Lum plane.
@param pU		: Chr U plane.
@param pV		: Chr V plane.
@return			: none.
*/
void RealRGB32toYUV420Converter::Convert(void* pRgb, void* pY, void* pU, void* pV)
{
	yuvType*	py = (yuvType *)pY;
	yuvType*	pu = (yuvType *)pU;
	yuvType*	pv = (yuvType *)pV;
	unsigned char* src = (unsigned char *)pRgb;

	/// Y have range 0..255, U & V have range -128..127.
	double	u,v;
	double	r,g,b;

	/// Step in 2x2 pel blocks. (4 pels per block).
	int xBlks = _width >> 1;
	int yBlks = _height >> 1;
	for(int yb = 0; yb < yBlks; yb++)
   for(int xb = 0; xb < xBlks; xb++)
	{
    int							chrOff	= yb*xBlks + xb;
    int							lumOff	= (yb*_width + xb) << 1;
    unsigned char*	t				= src + lumOff*4;

		/// Top left pel.  255->0.999.
		b = (double)(*t++);
		g = (double)(*t++);
		r = (double)(*t++);
		t++;	///< Alpha channel.
		py[lumOff] = (yuvType)((int)(0.5 + RRGB32YUVC_00*r + RRGB32YUVC_01*g + RRGB32YUVC_02*b));

		u = 128.0 + RRGB32YUVC_10*r + RRGB32YUVC_11*g + RRGB32YUVC_12*b;
		v = 128.0 + RRGB32YUVC_20*r + RRGB32YUVC_21*g + RRGB32YUVC_22*b;

		/// Top right pel.
		b = (double)(*t++);
		g = (double)(*t++);
		r = (double)(*t++);
		t++;	///< Alpha channel.
		py[lumOff+1] = (yuvType)((int)(0.5 + RRGB32YUVC_00*r + RRGB32YUVC_01*g + RRGB32YUVC_02*b));

		u += 128.0 + RRGB32YUVC_10*r + RRGB32YUVC_11*g + RRGB32YUVC_12*b;
		v += 128.0 + RRGB32YUVC_20*r + RRGB32YUVC_21*g + RRGB32YUVC_22*b;

    lumOff += _width;
    t = t + _width*4 - 8;
		/// Bottom left pel.  255->0.999.
		b = (double)(*t++);
		g = (double)(*t++);
		r = (double)(*t++);
		t++;	///< Alpha channel.
		py[lumOff] = (yuvType)((int)(0.5 + RRGB32YUVC_00*r + RRGB32YUVC_01*g + RRGB32YUVC_02*b));

		u += 128.0 + RRGB32YUVC_10*r + RRGB32YUVC_11*g + RRGB32YUVC_12*b;
		v += 128.0 + RRGB32YUVC_20*r + RRGB32YUVC_21*g + RRGB32YUVC_22*b;

		/// Bottom right pel.
		b = (double)(*t++);
		g = (double)(*t++);
		r = (double)(*t);
		py[lumOff+1] = (yuvType)((int)(0.5 + RRGB32YUVC_00*r + RRGB32YUVC_01*g + RRGB32YUVC_02*b));

		u += 128.0 + RRGB32YUVC_10*r + RRGB32YUVC_11*g + RRGB32YUVC_12*b;
		v += 128.0 + RRGB32YUVC_20*r + RRGB32YUVC_21*g + RRGB32YUVC_22*b;

		/// Average the 4 chr values.
		pu[chrOff] = (yuvType)( (int)((u + 0.5)/4) );
		pv[chrOff] = (yuvType)( (int)((v + 0.5)/4) );
 	}//end for xb & yb...

}//end Convert.

