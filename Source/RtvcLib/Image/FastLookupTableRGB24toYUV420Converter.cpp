/** @file

MODULE				: FastLookupTableRGB24toYUV420Converter

FILE NAME			: FastLookupTableRGB24toYUV420Converter.cpp

DESCRIPTION			: Lookup-table-based RGB 24 bit to YUV420 colour 
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

#include "FastLookupTableRGB24toYUV420Converter.h"

/*
===========================================================================
	Constants.
===========================================================================
*/
#define RRGB24YUVCI2_00		 0.299f
#define RRGB24YUVCI2_01		 0.587f
#define RRGB24YUVCI2_02		 0.114f
#define RRGB24YUVCI2_10		-0.147f
#define RRGB24YUVCI2_11		-0.289f
#define RRGB24YUVCI2_12		 0.436f
#define RRGB24YUVCI2_20		 0.615f
#define RRGB24YUVCI2_21		-0.515f
#define RRGB24YUVCI2_22		-0.100f

#define RRGB24YUVCI2_RANGECHECK_0TO255(x) ( (((x) <= 255)&&((x) >= 0))?((x)):( ((x) > 255)?(255):(0) ) )
#define RRGB24YUVCI2_RANGECHECK_N128TO127(x) ( (((x) <= 127)&&((x) >= -128))?((x)):( ((x) > 127)?(127):(-128) ) )

void FastLookupTableRGB24toYUV420Converter::initLookupTable()
{
  for (size_t i = 0; i < 256; ++i)
  {
    m_RRGB24YUVCI2_00[i] = i * RRGB24YUVCI2_00;
    m_RRGB24YUVCI2_01[i] = i * RRGB24YUVCI2_01;
    m_RRGB24YUVCI2_02[i] = i * RRGB24YUVCI2_02;
    m_RRGB24YUVCI2_10[i] = i * RRGB24YUVCI2_10;
    m_RRGB24YUVCI2_11[i] = i * RRGB24YUVCI2_11;
    m_RRGB24YUVCI2_12[i] = i * RRGB24YUVCI2_12;
    m_RRGB24YUVCI2_20[i] = i * RRGB24YUVCI2_20;
    m_RRGB24YUVCI2_21[i] = i * RRGB24YUVCI2_21;
    m_RRGB24YUVCI2_22[i] = i * RRGB24YUVCI2_22;
  }
}

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
void FastLookupTableRGB24toYUV420Converter::Convert(void* pRgb, void* pY, void* pU, void* pV)
{
	yuvType*	py = (yuvType *)pY;
	yuvType*	pu = (yuvType *)pU;
	yuvType*	pv = (yuvType *)pV;
	unsigned char* src = (unsigned char *)pRgb;

  /// Y have range 0..255, U & V have range -128..127.
  double	u,v;
  int	r,g,b;

  /// Step in 2x2 pel blocks. (4 pels per block).
  int xBlks = _width >> 1;
  int yBlks = _height >> 1;
  for(int yb = 0; yb < yBlks; ++yb)
    for(int xb = 0; xb < xBlks; ++xb)
    {
      int							chrOff	= yb*xBlks + xb;
      int							lumOff	= (yb*_width + xb) << 1;
      unsigned char*	t				= src + lumOff*3;

      /// Top left pel.
      b = (*t++);
      g = (*t++);
      r = (*t++);
      py[lumOff] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255(/*(int)*/(0.5 + m_RRGB24YUVCI2_00[r] + m_RRGB24YUVCI2_01[g] + m_RRGB24YUVCI2_02[b]));

      u = m_RRGB24YUVCI2_10[r] + m_RRGB24YUVCI2_11[g] + m_RRGB24YUVCI2_12[b];
      v = m_RRGB24YUVCI2_20[r] + m_RRGB24YUVCI2_21[g] + m_RRGB24YUVCI2_22[b];

      /// Top right pel.
      b = (*t++);
      g = (*t++);
      r = (*t++);
      py[lumOff+1] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255(/*(int)*/(0.5 + m_RRGB24YUVCI2_00[r] + m_RRGB24YUVCI2_01[g] + m_RRGB24YUVCI2_02[b]));

      u += m_RRGB24YUVCI2_10[r] + m_RRGB24YUVCI2_11[g] + m_RRGB24YUVCI2_12[b];
      v += m_RRGB24YUVCI2_20[r] + m_RRGB24YUVCI2_21[g] + m_RRGB24YUVCI2_22[b];

      lumOff += _width;
      t = t + _width*3 - 6;
      /// Bottom left pel.
      b = (*t++);
      g = (*t++);
      r = (*t++);
      py[lumOff] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255(/*(int)*/(0.5 + m_RRGB24YUVCI2_00[r] + m_RRGB24YUVCI2_01[g] + m_RRGB24YUVCI2_02[b]));

      u += m_RRGB24YUVCI2_10[r] + m_RRGB24YUVCI2_11[g] + m_RRGB24YUVCI2_12[b];
      v += m_RRGB24YUVCI2_20[r] + m_RRGB24YUVCI2_21[g] + m_RRGB24YUVCI2_22[b];

      /// Bottom right pel.
      b = (*t++);
      g = (*t++);
      r = (*t++);
      py[lumOff+1] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255(/*(int)*/(0.5 + m_RRGB24YUVCI2_00[r] + m_RRGB24YUVCI2_01[g] + m_RRGB24YUVCI2_02[b]));
      //py[lumOff+1] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255((int)(0.5 + RRGB24YUVCI2_00*r + RRGB24YUVCI2_01*g + RRGB24YUVCI2_02*b));

      u += m_RRGB24YUVCI2_10[r] + m_RRGB24YUVCI2_11[g] + m_RRGB24YUVCI2_12[b];
      v += m_RRGB24YUVCI2_20[r] + m_RRGB24YUVCI2_21[g] + m_RRGB24YUVCI2_22[b];

      /// Average the 4 chr values.
      int iu = (int)u;
      int iv = (int)v;
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

