/** @file

MODULE				: RealRGB24toYUV420ConverterImpl2

TAG						: RRGB24YUVCI2

FILE NAME			: RealRGB24toYUV420ConverterImpl2.cpp

DESCRIPTION		: Double precision floating point RGB 24 bit to YUV420 colour 
								convertion derived from the RGBtoYUV420Converter base class.
								YUV is represented with 8 or 16 bpp per component. Use this 
								implementation as the reference.

COPYRIGHT			:	(c)CSIR 2007-2010 all rights resevered

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

#include "RealRGB24toYUV420ConverterImpl2.h"

/*
===========================================================================
	Constants.
===========================================================================
*/
#define RRGB24YUVCI2_00		 0.299
#define RRGB24YUVCI2_01		 0.587
#define RRGB24YUVCI2_02		 0.114
#define RRGB24YUVCI2_10		-0.147
#define RRGB24YUVCI2_11		-0.289
#define RRGB24YUVCI2_12		 0.436
#define RRGB24YUVCI2_20		 0.615
#define RRGB24YUVCI2_21		-0.515
#define RRGB24YUVCI2_22		-0.100

#define RRGB24YUVCI2_RANGECHECK_0TO255(x) ( (((x) <= 255)&&((x) >= 0))?((x)):( ((x) > 255)?(255):(0) ) )
#define RRGB24YUVCI2_RANGECHECK_N128TO127(x) ( (((x) <= 127)&&((x) >= -128))?((x)):( ((x) > 127)?(127):(-128) ) )

/*
===========================================================================
	Constructor Methods.
===========================================================================
*/
RealRGB24toYUV420ConverterImpl2::RealRGB24toYUV420ConverterImpl2(int width, int height): RGBtoYUV420Converter(width, height)
{ 
}//end constructor.

RealRGB24toYUV420ConverterImpl2::RealRGB24toYUV420ConverterImpl2(int width, int height, int chrOff): RGBtoYUV420Converter(width, height, chrOff)
{
}//end constructor.

/*
===========================================================================
	Interface Methods.
===========================================================================
*/
/** Double precision reference implementation.
The full real matix equation is used. The YUV output is represented with
8 bits per pel and the UV components are adjusted from their -128..127 range
to 0..255.
@param pRgb	: Packed RGB 888 format.
@param pY		: Lum plane.
@param pU		: Chr U plane.
@param pV		: Chr V plane.
@return			: none.
*/
void RealRGB24toYUV420ConverterImpl2::Convert(void* pRgb, void* pY, void* pU, void* pV)
{
  if (_flip)
    FlipConvert(pRgb, pY, pU, pV);
  else
    NonFlipConvert(pRgb, pY, pU, pV);
}//end Convert.

void RealRGB24toYUV420ConverterImpl2::FlipConvert( void* pRgb, void* pY, void* pU, void* pV )
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
      //unsigned char*	t				= src + lumOff*3;
      unsigned char*	t				= src + ((((_height - (yb * 2) - 1) * _width) + (xb * 2)) * 3);

      /// Top left pel.
      b = (double)(*t++);
      g = (double)(*t++);
      r = (double)(*t++);
      py[lumOff] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255((int)(0.5 + RRGB24YUVCI2_00*r + RRGB24YUVCI2_01*g + RRGB24YUVCI2_02*b));

      u = RRGB24YUVCI2_10*r + RRGB24YUVCI2_11*g + RRGB24YUVCI2_12*b;
      v = RRGB24YUVCI2_20*r + RRGB24YUVCI2_21*g + RRGB24YUVCI2_22*b;

      /// Top right pel.
      b = (double)(*t++);
      g = (double)(*t++);
      r = (double)(*t++);
      py[lumOff+1] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255((int)(0.5 + RRGB24YUVCI2_00*r + RRGB24YUVCI2_01*g + RRGB24YUVCI2_02*b));

      u += RRGB24YUVCI2_10*r + RRGB24YUVCI2_11*g + RRGB24YUVCI2_12*b;
      v += RRGB24YUVCI2_20*r + RRGB24YUVCI2_21*g + RRGB24YUVCI2_22*b;

      lumOff += _width;
      //t = t + _width*3 - 6;
      t = t - _width*3 - 6;

      /// Bottom left pel.
      b = (double)(*t++);
      g = (double)(*t++);
      r = (double)(*t++);
      py[lumOff] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255((int)(0.5 + RRGB24YUVCI2_00*r + RRGB24YUVCI2_01*g + RRGB24YUVCI2_02*b));

      u += RRGB24YUVCI2_10*r + RRGB24YUVCI2_11*g + RRGB24YUVCI2_12*b;
      v += RRGB24YUVCI2_20*r + RRGB24YUVCI2_21*g + RRGB24YUVCI2_22*b;

      /// Bottom right pel.
      b = (double)(*t++);
      g = (double)(*t++);
      r = (double)(*t++);
      py[lumOff+1] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255((int)(0.5 + RRGB24YUVCI2_00*r + RRGB24YUVCI2_01*g + RRGB24YUVCI2_02*b));

      u += RRGB24YUVCI2_10*r + RRGB24YUVCI2_11*g + RRGB24YUVCI2_12*b;
      v += RRGB24YUVCI2_20*r + RRGB24YUVCI2_21*g + RRGB24YUVCI2_22*b;

      /// Average the 4 chr values.
      if(u < 0.0)
        u = (u/4) - 0.5;
      else
        u = (u/4) + 0.5;
      if(v < 0.0)
        v = (v/4) - 0.5;
      else
        v = (v/4) + 0.5;

      pu[chrOff] = (yuvType)( _chrOff + RRGB24YUVCI2_RANGECHECK_N128TO127((int)(u)) );
      pv[chrOff] = (yuvType)( _chrOff + RRGB24YUVCI2_RANGECHECK_N128TO127((int)(v)) );
    }//end for xb & yb...
}

void RealRGB24toYUV420ConverterImpl2::NonFlipConvert( void* pRgb, void* pY, void* pU, void* pV )
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
      unsigned char*	t				= src + lumOff*3;

      /// Top left pel.
      b = (double)(*t++);
      g = (double)(*t++);
      r = (double)(*t++);
      py[lumOff] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255((int)(0.5 + RRGB24YUVCI2_00*r + RRGB24YUVCI2_01*g + RRGB24YUVCI2_02*b));

      u = RRGB24YUVCI2_10*r + RRGB24YUVCI2_11*g + RRGB24YUVCI2_12*b;
      v = RRGB24YUVCI2_20*r + RRGB24YUVCI2_21*g + RRGB24YUVCI2_22*b;

      /// Top right pel.
      b = (double)(*t++);
      g = (double)(*t++);
      r = (double)(*t++);
      py[lumOff+1] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255((int)(0.5 + RRGB24YUVCI2_00*r + RRGB24YUVCI2_01*g + RRGB24YUVCI2_02*b));

      u += RRGB24YUVCI2_10*r + RRGB24YUVCI2_11*g + RRGB24YUVCI2_12*b;
      v += RRGB24YUVCI2_20*r + RRGB24YUVCI2_21*g + RRGB24YUVCI2_22*b;

      lumOff += _width;
      t = t + _width*3 - 6;
      /// Bottom left pel.
      b = (double)(*t++);
      g = (double)(*t++);
      r = (double)(*t++);
      py[lumOff] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255((int)(0.5 + RRGB24YUVCI2_00*r + RRGB24YUVCI2_01*g + RRGB24YUVCI2_02*b));

      u += RRGB24YUVCI2_10*r + RRGB24YUVCI2_11*g + RRGB24YUVCI2_12*b;
      v += RRGB24YUVCI2_20*r + RRGB24YUVCI2_21*g + RRGB24YUVCI2_22*b;

      /// Bottom right pel.
      b = (double)(*t++);
      g = (double)(*t++);
      r = (double)(*t++);
      py[lumOff+1] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255((int)(0.5 + RRGB24YUVCI2_00*r + RRGB24YUVCI2_01*g + RRGB24YUVCI2_02*b));

      u += RRGB24YUVCI2_10*r + RRGB24YUVCI2_11*g + RRGB24YUVCI2_12*b;
      v += RRGB24YUVCI2_20*r + RRGB24YUVCI2_21*g + RRGB24YUVCI2_22*b;

      /// Average the 4 chr values.
      if(u < 0.0)
        u = (u/4) - 0.5;
      else
        u = (u/4) + 0.5;
      if(v < 0.0)
        v = (v/4) - 0.5;
      else
        v = (v/4) + 0.5;

      pu[chrOff] = (yuvType)( _chrOff + RRGB24YUVCI2_RANGECHECK_N128TO127((int)(u)) );
      pv[chrOff] = (yuvType)( _chrOff + RRGB24YUVCI2_RANGECHECK_N128TO127((int)(v)) );
    }//end for xb & yb...
}

