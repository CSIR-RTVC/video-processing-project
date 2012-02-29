/** @file

MODULE						: RealRGB24toYUV420CCIR601Converter

TAG								: RRGB24YUVCC

FILE NAME					: RealRGB24toYUV420CCIR601Converter.cpp

DESCRIPTION				: Double precision floating point RGB 24 bit to YUV420 colour 
										convertion derived from the RGBtoYUV420Converter base class.
										YUV is represented with 8 bpp. Implements the CCIR-601 colour 
										conversion process. Use this implementation as the reference.

REVISION HISTORY	:
									: 

COPYRIGHT					:

RESTRICTIONS			: 
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

#include "RealRGB24toYUV420CCIR601Converter.h"

/*
===========================================================================
	Constants.
===========================================================================
*/
//	CCIR-601:
//		Y =  0.257r + 0.504g + 0.098b + 16
//		U	= -0.148r - 0.291g + 0.439b + 128		(Cb)
//		V =  0.439r - 0.368g - 0.071b + 128		(Cr)
#define RRGB24YUVCC_00		 0.257
#define RRGB24YUVCC_01		 0.504
#define RRGB24YUVCC_02		 0.098
#define RRGB24YUVCC_10		-0.148
#define RRGB24YUVCC_11		-0.291
#define RRGB24YUVCC_12		 0.439
#define RRGB24YUVCC_20		 0.439
#define RRGB24YUVCC_21		-0.368
#define RRGB24YUVCC_22		-0.071

/*
===========================================================================
	Interface Methods.
===========================================================================
*/
/** Double precision reference implementation.
The full real matix equation is used. The YUV output is represented with
8/16 bits per pel and the CCIR-601 specifies that the Y range is 16..235 
and the UV (CbCr) range is 16..240 centred on 128.
@param pRgb	: Packed RGB 888 format.
@param pY		: Lum plane.
@param pU		: Chr U plane.
@param pV		: Chr V plane.
@return			: none.
*/
void RealRGB24toYUV420CCIR601Converter::Convert(void* pRgb, void* pY, void* pU, void* pV)
{
  if (_flip)
    FlipConvert(pRgb, pY, pU, pV);
  else
    NonFlipConvert(pRgb, pY, pU, pV);
}

void RealRGB24toYUV420CCIR601Converter::FlipConvert( void* pRgb, void* pY, void* pU, void* pV )
{
	yuvType*	py = (yuvType *)pY;
	yuvType*	pu = (yuvType *)pU;
	yuvType*	pv = (yuvType *)pV;
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
		py[lumOff] = 16 + (yuvType)(0.5 + RRGB24YUVCC_00*r + RRGB24YUVCC_01*g + RRGB24YUVCC_02*b);

		u = _chrOff + RRGB24YUVCC_10*r + RRGB24YUVCC_11*g + RRGB24YUVCC_12*b;
		v = _chrOff + RRGB24YUVCC_20*r + RRGB24YUVCC_21*g + RRGB24YUVCC_22*b;

		// Top right pel.
		b = (double)(*t++);
		g = (double)(*t++);
		r = (double)(*t++);
		py[lumOff+1] = 16 + (yuvType)(0.5 + RRGB24YUVCC_00*r + RRGB24YUVCC_01*g + RRGB24YUVCC_02*b);

		u += _chrOff + RRGB24YUVCC_10*r + RRGB24YUVCC_11*g + RRGB24YUVCC_12*b;
		v += _chrOff + RRGB24YUVCC_20*r + RRGB24YUVCC_21*g + RRGB24YUVCC_22*b;

    lumOff += _width;
    //t = t + _width*3 - 6;
    t = t - _width*3 - 6;
		// Bottom left pel..
		b = (double)(*t++);
		g = (double)(*t++);
		r = (double)(*t++);
		py[lumOff] = 16 + (yuvType)(0.5 + RRGB24YUVCC_00*r + RRGB24YUVCC_01*g + RRGB24YUVCC_02*b);

		u += _chrOff + RRGB24YUVCC_10*r + RRGB24YUVCC_11*g + RRGB24YUVCC_12*b;
		v += _chrOff + RRGB24YUVCC_20*r + RRGB24YUVCC_21*g + RRGB24YUVCC_22*b;

		// Bottom right pel.
		b = (double)(*t++);
		g = (double)(*t++);
		r = (double)(*t++);
		py[lumOff+1] = 16 + (yuvType)(0.5 + RRGB24YUVCC_00*r + RRGB24YUVCC_01*g + RRGB24YUVCC_02*b);

		u += _chrOff + RRGB24YUVCC_10*r + RRGB24YUVCC_11*g + RRGB24YUVCC_12*b;
		v += _chrOff + RRGB24YUVCC_20*r + RRGB24YUVCC_21*g + RRGB24YUVCC_22*b;

		// Average the 4 chr values.
		pu[chrOff] = (yuvType)( (int)(u + 0.5)/4 );
		pv[chrOff] = (yuvType)( (int)(v + 0.5)/4 );
 	}//end for xb & yb...

}//end Convert.

void RealRGB24toYUV420CCIR601Converter::NonFlipConvert( void* pRgb, void* pY, void* pU, void* pV )
{
	yuvType*	py = (yuvType *)pY;
	yuvType*	pu = (yuvType *)pU;
	yuvType*	pv = (yuvType *)pV;
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
		py[lumOff] = 16 + (yuvType)(0.5 + RRGB24YUVCC_00*r + RRGB24YUVCC_01*g + RRGB24YUVCC_02*b);

		u = _chrOff + RRGB24YUVCC_10*r + RRGB24YUVCC_11*g + RRGB24YUVCC_12*b;
		v = _chrOff + RRGB24YUVCC_20*r + RRGB24YUVCC_21*g + RRGB24YUVCC_22*b;

		// Top right pel.
		b = (double)(*t++);
		g = (double)(*t++);
		r = (double)(*t++);
		py[lumOff+1] = 16 + (yuvType)(0.5 + RRGB24YUVCC_00*r + RRGB24YUVCC_01*g + RRGB24YUVCC_02*b);

		u += _chrOff + RRGB24YUVCC_10*r + RRGB24YUVCC_11*g + RRGB24YUVCC_12*b;
		v += _chrOff + RRGB24YUVCC_20*r + RRGB24YUVCC_21*g + RRGB24YUVCC_22*b;

    lumOff += _width;
    t = t + _width*3 - 6;
		// Bottom left pel..
		b = (double)(*t++);
		g = (double)(*t++);
		r = (double)(*t++);
		py[lumOff] = 16 + (yuvType)(0.5 + RRGB24YUVCC_00*r + RRGB24YUVCC_01*g + RRGB24YUVCC_02*b);

		u += _chrOff + RRGB24YUVCC_10*r + RRGB24YUVCC_11*g + RRGB24YUVCC_12*b;
		v += _chrOff + RRGB24YUVCC_20*r + RRGB24YUVCC_21*g + RRGB24YUVCC_22*b;

		// Bottom right pel.
		b = (double)(*t++);
		g = (double)(*t++);
		r = (double)(*t++);
		py[lumOff+1] = 16 + (yuvType)(0.5 + RRGB24YUVCC_00*r + RRGB24YUVCC_01*g + RRGB24YUVCC_02*b);

		u += _chrOff + RRGB24YUVCC_10*r + RRGB24YUVCC_11*g + RRGB24YUVCC_12*b;
		v += _chrOff + RRGB24YUVCC_20*r + RRGB24YUVCC_21*g + RRGB24YUVCC_22*b;

		// Average the 4 chr values.
		pu[chrOff] = (yuvType)( (int)(u + 0.5)/4 );
		pv[chrOff] = (yuvType)( (int)(v + 0.5)/4 );
 	}//end for xb & yb...

}//end Convert.


