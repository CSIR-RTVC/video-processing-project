/** @file

MODULE				: RealRGB24toYUV420CCIR601ConverterVer16

TAG						: RRGB24YUVCCV16

FILE NAME			: RealRGB24toYUV420CCIR601ConverterVer16.cpp

DESCRIPTION		: Double precision floating point RGB 24 bit to YUV420 colour 
								convertion derived from the RGBtoYUV420Converter base class.
                YUV colour components are stored as 16-bit shorts per component.
								Implements the CCIR-601 colour conversion process. Use this 
								implementation as the reference.

COPYRIGHT			:	(c)CSIR 2007-2012 all rights resevered

LICENSE				: Software License Agreement (BSD License)

RESTRICTIONS	: Redistribution and use in source and binary forms, with or without 
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

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "RealRGB24toYUV420CCIR601ConverterVer16.h"

/*
===========================================================================
	Constants.
===========================================================================
*/
//	CCIR-601:
//		Y =  0.257r + 0.504g + 0.098b + 16
//		U	= -0.148r - 0.291g + 0.439b + 128		(Cb)
//		V =  0.439r - 0.368g - 0.071b + 128		(Cr)
#define RRGB24YUVCCV16_00		 0.257
#define RRGB24YUVCCV16_01		 0.504
#define RRGB24YUVCCV16_02		 0.098
#define RRGB24YUVCCV16_10		-0.148
#define RRGB24YUVCCV16_11		-0.291
#define RRGB24YUVCCV16_12		 0.439
#define RRGB24YUVCCV16_20		 0.439
#define RRGB24YUVCCV16_21		-0.368
#define RRGB24YUVCCV16_22		-0.071

/*
===========================================================================
	Interface Methods.
===========================================================================
*/
/** Double precision reference implementation.
The full real matix equation is used. The YUV output is represented with
16 bits per pel and the CCIR-601 specifies that the Y range is 16..235 
and the UV (CbCr) range is 16..240 centred on 128.
@param pRgb	: Packed RGB 888 format.
@param pY		: Lum plane.
@param pU		: Chr U plane.
@param pV		: Chr V plane.
@return			: none.
*/
void RealRGB24toYUV420CCIR601ConverterVer16::Convert(void* pRgb, void* pY, void* pU, void* pV)
{
  if (_flip)
    FlipConvert(pRgb, pY, pU, pV);
  else
    NonFlipConvert(pRgb, pY, pU, pV);
}

void RealRGB24toYUV420CCIR601ConverterVer16::FlipConvert( void* pRgb, void* pY, void* pU, void* pV )
{
	short*	py = (short *)pY;
	short*	pu = (short *)pU;
	short*	pv = (short *)pV;
	unsigned char* src = (unsigned char *)pRgb;

	// Y1 & Y2 have range 16..235, U & V have range 16..240.
	double	u,v;
	double	r,g,b;

	// Step in 2x2 pel blocks. (4 pels per block).
	int xBlks = _width >> 1;
	int yBlks = _height >> 1;
	for(int yb = 0; yb < yBlks; yb++)
   for(int xb = 0; xb < xBlks; xb++)
	{
    int							chrOff	= yb*xBlks + xb;
    int							lumOff	= (yb*_width + xb) << 1;
    //unsigned char*	t				= src + lumOff*3;
    unsigned char*	t				= src + ((((_height - (yb * 2) - 1) * _width) + (xb * 2)) * 3);

		// Top left pel.
		b = (double)(*t++);
		g = (double)(*t++);
		r = (double)(*t++);
		py[lumOff] = 16 + (short)(0.5 + RRGB24YUVCCV16_00*r + RRGB24YUVCCV16_01*g + RRGB24YUVCCV16_02*b);

		u = _chrOff + RRGB24YUVCCV16_10*r + RRGB24YUVCCV16_11*g + RRGB24YUVCCV16_12*b;
		v = _chrOff + RRGB24YUVCCV16_20*r + RRGB24YUVCCV16_21*g + RRGB24YUVCCV16_22*b;

		// Top right pel.
		b = (double)(*t++);
		g = (double)(*t++);
		r = (double)(*t++);
		py[lumOff+1] = 16 + (short)(0.5 + RRGB24YUVCCV16_00*r + RRGB24YUVCCV16_01*g + RRGB24YUVCCV16_02*b);

		u += _chrOff + RRGB24YUVCCV16_10*r + RRGB24YUVCCV16_11*g + RRGB24YUVCCV16_12*b;
		v += _chrOff + RRGB24YUVCCV16_20*r + RRGB24YUVCCV16_21*g + RRGB24YUVCCV16_22*b;

    lumOff += _width;
    //t = t + _width*3 - 6;
    t = t - _width*3 - 6;
		// Bottom left pel..
		b = (double)(*t++);
		g = (double)(*t++);
		r = (double)(*t++);
		py[lumOff] = 16 + (short)(0.5 + RRGB24YUVCCV16_00*r + RRGB24YUVCCV16_01*g + RRGB24YUVCCV16_02*b);

		u += _chrOff + RRGB24YUVCCV16_10*r + RRGB24YUVCCV16_11*g + RRGB24YUVCCV16_12*b;
		v += _chrOff + RRGB24YUVCCV16_20*r + RRGB24YUVCCV16_21*g + RRGB24YUVCCV16_22*b;

		// Bottom right pel.
		b = (double)(*t++);
		g = (double)(*t++);
		r = (double)(*t++);
		py[lumOff+1] = 16 + (short)(0.5 + RRGB24YUVCCV16_00*r + RRGB24YUVCCV16_01*g + RRGB24YUVCCV16_02*b);

		u += _chrOff + RRGB24YUVCCV16_10*r + RRGB24YUVCCV16_11*g + RRGB24YUVCCV16_12*b;
		v += _chrOff + RRGB24YUVCCV16_20*r + RRGB24YUVCCV16_21*g + RRGB24YUVCCV16_22*b;

		// Average the 4 chr values.
		pu[chrOff] = (short)( (int)(u + 0.5)/4 );
		pv[chrOff] = (short)( (int)(v + 0.5)/4 );
 	}//end for xb & yb...

}//end Convert.

void RealRGB24toYUV420CCIR601ConverterVer16::NonFlipConvert( void* pRgb, void* pY, void* pU, void* pV )
{
	short*	py = (short *)pY;
	short*	pu = (short *)pU;
	short*	pv = (short *)pV;
	unsigned char* src = (unsigned char *)pRgb;

	// Y1 & Y2 have range 16..235, U & V have range 16..240.
	double	u,v;
	double	r,g,b;

	// Step in 2x2 pel blocks. (4 pels per block).
	int xBlks = _width >> 1;
	int yBlks = _height >> 1;
	for(int yb = 0; yb < yBlks; yb++)
   for(int xb = 0; xb < xBlks; xb++)
	{
    int							chrOff	= yb*xBlks + xb;
    int							lumOff	= (yb*_width + xb) << 1;
    unsigned char*	t				= src + lumOff*3;

		// Top left pel.
		b = (double)(*t++);
		g = (double)(*t++);
		r = (double)(*t++);
		py[lumOff] = 16 + (short)(0.5 + RRGB24YUVCCV16_00*r + RRGB24YUVCCV16_01*g + RRGB24YUVCCV16_02*b);

		u = _chrOff + RRGB24YUVCCV16_10*r + RRGB24YUVCCV16_11*g + RRGB24YUVCCV16_12*b;
		v = _chrOff + RRGB24YUVCCV16_20*r + RRGB24YUVCCV16_21*g + RRGB24YUVCCV16_22*b;

		// Top right pel.
		b = (double)(*t++);
		g = (double)(*t++);
		r = (double)(*t++);
		py[lumOff+1] = 16 + (short)(0.5 + RRGB24YUVCCV16_00*r + RRGB24YUVCCV16_01*g + RRGB24YUVCCV16_02*b);

		u += _chrOff + RRGB24YUVCCV16_10*r + RRGB24YUVCCV16_11*g + RRGB24YUVCCV16_12*b;
		v += _chrOff + RRGB24YUVCCV16_20*r + RRGB24YUVCCV16_21*g + RRGB24YUVCCV16_22*b;

    lumOff += _width;
    t = t + _width*3 - 6;
		// Bottom left pel..
		b = (double)(*t++);
		g = (double)(*t++);
		r = (double)(*t++);
		py[lumOff] = 16 + (short)(0.5 + RRGB24YUVCCV16_00*r + RRGB24YUVCCV16_01*g + RRGB24YUVCCV16_02*b);

		u += _chrOff + RRGB24YUVCCV16_10*r + RRGB24YUVCCV16_11*g + RRGB24YUVCCV16_12*b;
		v += _chrOff + RRGB24YUVCCV16_20*r + RRGB24YUVCCV16_21*g + RRGB24YUVCCV16_22*b;

		// Bottom right pel.
		b = (double)(*t++);
		g = (double)(*t++);
		r = (double)(*t++);
		py[lumOff+1] = 16 + (short)(0.5 + RRGB24YUVCCV16_00*r + RRGB24YUVCCV16_01*g + RRGB24YUVCCV16_02*b);

		u += _chrOff + RRGB24YUVCCV16_10*r + RRGB24YUVCCV16_11*g + RRGB24YUVCCV16_12*b;
		v += _chrOff + RRGB24YUVCCV16_20*r + RRGB24YUVCCV16_21*g + RRGB24YUVCCV16_22*b;

		// Average the 4 chr values.
		pu[chrOff] = (short)( (int)(u + 0.5)/4 );
		pv[chrOff] = (short)( (int)(v + 0.5)/4 );
 	}//end for xb & yb...

}//end Convert.


