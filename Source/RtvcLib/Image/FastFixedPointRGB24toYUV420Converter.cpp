/** @file

MODULE				: FastFixedPointRGB24toYUV420Converter

FILE NAME			: FastFixedPointRGB24toYUV420Converter.cpp

DESCRIPTION			: Fixed-point RGB 24 bit to YUV420 colour 
conversion derived from the RGBtoYUV420Converter base class.
YUV is represented with 16 bpp. Use this implementation as 
the reference.

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
#include <stdio.h>
#endif

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "FastFixedPointRGB24toYUV420Converter.h"

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

const int iPrecision = 12;
const int iRRGB24YUVCI2_00 = static_cast<int>(0.5 + RRGB24YUVCI2_00 * (2 << iPrecision));
const int iRRGB24YUVCI2_01 = static_cast<int>(0.5 + RRGB24YUVCI2_01 * (2 << iPrecision));
const int iRRGB24YUVCI2_02 = static_cast<int>(0.5 + RRGB24YUVCI2_02 * (2 << iPrecision));
const int iRRGB24YUVCI2_10 = static_cast<int>(0.5 + RRGB24YUVCI2_10 * (2 << iPrecision));
const int iRRGB24YUVCI2_11 = static_cast<int>(0.5 + RRGB24YUVCI2_11 * (2 << iPrecision));
const int iRRGB24YUVCI2_12 = static_cast<int>(0.5 + RRGB24YUVCI2_12 * (2 << iPrecision));
const int iRRGB24YUVCI2_20 = static_cast<int>(0.5 + RRGB24YUVCI2_20 * (2 << iPrecision));
const int iRRGB24YUVCI2_21 = static_cast<int>(0.5 + RRGB24YUVCI2_21 * (2 << iPrecision));
const int iRRGB24YUVCI2_22 = static_cast<int>(0.5 + RRGB24YUVCI2_22 * (2 << iPrecision));

#define RRGB24YUVCI2_RANGECHECK_0TO255(x) ( (((x) <= 255)&&((x) >= 0))?((x)):( ((x) > 255)?(255):(0) ) )
#define RRGB24YUVCI2_RANGECHECK_N128TO127(x) ( (((x) <= 127)&&((x) >= -128))?((x)):( ((x) > 127)?(127):(-128) ) )

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
void FastFixedPointRGB24toYUV420Converter::Convert(void* pRgb, void* pY, void* pU, void* pV)
{
  yuvType*	py = (yuvType *)pY;
  yuvType*	pu = (yuvType *)pU;
  yuvType*	pv = (yuvType *)pV;
  unsigned char* src = (unsigned char *)pRgb;

  /// Y have range 0..255, U & V have range -128..127.
  int	u,v;
  int	r,g,b;

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
      b = (int)(*t++);
      g = (int)(*t++);
      r = (int)(*t++);
      py[lumOff] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255(((iRRGB24YUVCI2_00*r + iRRGB24YUVCI2_01*g + iRRGB24YUVCI2_02*b)>> (iPrecision + 1)));

      u = iRRGB24YUVCI2_10*r + iRRGB24YUVCI2_11*g + iRRGB24YUVCI2_12*b;
      v = iRRGB24YUVCI2_20*r + iRRGB24YUVCI2_21*g + iRRGB24YUVCI2_22*b;

      /// Top right pel.
      b = (int)(*t++);
      g = (int)(*t++);
      r = (int)(*t++);
      py[lumOff+1] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255(((iRRGB24YUVCI2_00*r + iRRGB24YUVCI2_01*g + iRRGB24YUVCI2_02*b)>> (iPrecision + 1)));

      u += iRRGB24YUVCI2_10*r + iRRGB24YUVCI2_11*g + iRRGB24YUVCI2_12*b;
      v += iRRGB24YUVCI2_20*r + iRRGB24YUVCI2_21*g + iRRGB24YUVCI2_22*b;

      lumOff += _width;
      t = t + _width*3 - 6;
      /// Bottom left pel.
      b = (int)(*t++);
      g = (int)(*t++);
      r = (int)(*t++);
      py[lumOff] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255(((iRRGB24YUVCI2_00*r + iRRGB24YUVCI2_01*g + iRRGB24YUVCI2_02*b)>> (iPrecision + 1)));

      u += iRRGB24YUVCI2_10*r + iRRGB24YUVCI2_11*g + iRRGB24YUVCI2_12*b;
      v += iRRGB24YUVCI2_20*r + iRRGB24YUVCI2_21*g + iRRGB24YUVCI2_22*b;

      /// Bottom right pel.
      b = (int)(*t++);
      g = (int)(*t++);
      r = (int)(*t++);
      py[lumOff+1] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255(((iRRGB24YUVCI2_00*r + iRRGB24YUVCI2_01*g + iRRGB24YUVCI2_02*b)>> (iPrecision + 1)));

      u += iRRGB24YUVCI2_10*r + iRRGB24YUVCI2_11*g + iRRGB24YUVCI2_12*b;
      v += iRRGB24YUVCI2_20*r + iRRGB24YUVCI2_21*g + iRRGB24YUVCI2_22*b;

      /// Average the 4 chr values.
      int iu = u >> (iPrecision + 1);
      int iv = v >> (iPrecision + 1);
      if(iu < 0)	///< Rounding.
        iu -= 2;
      else
        iu += 2;
      if(iv < 0)	///< Rounding.
        iv -= 2;
      else
        iv += 2;

      pu[chrOff] = (yuvType)( _chrOff + RRGB24YUVCI2_RANGECHECK_N128TO127(iu >> 2) );
      pv[chrOff] = (yuvType)( _chrOff + RRGB24YUVCI2_RANGECHECK_N128TO127(iv >> 2) );
    }//end for xb & yb...
}//end Convert.

