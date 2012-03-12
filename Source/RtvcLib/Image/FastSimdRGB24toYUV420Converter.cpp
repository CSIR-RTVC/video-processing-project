/** @file

MODULE				: FastSimdRGB24toYUV420Converter

FILE NAME			: FastSimdRGB24toYUV420Converter.cpp

DESCRIPTION			: SIMD RGB 24 bit to YUV420 colour 
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

// SSE
#include <emmintrin.h>

#include "FastSimdRGB24toYUV420Converter.h"

/*
===========================================================================
	Constants.
===========================================================================
*/
const float fRRGB24YUVCI2_00 = 0.299;
const float fRRGB24YUVCI2_01 = 0.587;
const float fRRGB24YUVCI2_02 = 0.114;
const float fRRGB24YUVCI2_10 = -0.147;
const float fRRGB24YUVCI2_11 = -0.289;
const float fRRGB24YUVCI2_12 = 0.436;
const float fRRGB24YUVCI2_20 = 0.615;
const float fRRGB24YUVCI2_21 = -0.515;
const float fRRGB24YUVCI2_22 = -0.100;

// SSE
static const __declspec(align(16)) float fCOEFF_0[4] = {fRRGB24YUVCI2_00, fRRGB24YUVCI2_01, fRRGB24YUVCI2_02, 0};
static const __declspec(align(16)) float fCOEFF_1[4] = {fRRGB24YUVCI2_10, fRRGB24YUVCI2_11, fRRGB24YUVCI2_12, 0};
static const __declspec(align(16)) float fCOEFF_2[4] = {fRRGB24YUVCI2_20, fRRGB24YUVCI2_21, fRRGB24YUVCI2_22, 0};


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
void FastSimdRGB24toYUV420Converter::Convert(void* pRgb, void* pY, void* pU, void* pV)
{
    // Aligned 16 bit
    __m128 xmm_y = _mm_load_ps(fCOEFF_0);
    __m128 xmm_u = _mm_load_ps(fCOEFF_1);
    __m128 xmm_v = _mm_load_ps(fCOEFF_2);

    yuvType*	py = (yuvType *)pY;
    yuvType*	pu = (yuvType *)pU;
    yuvType*	pv = (yuvType *)pV;
    unsigned char* src = (unsigned char *)pRgb;

    /// Y have range 0..255, U & V have range -128..127.
    __declspec(align(16)) float bgr1[4];
    bgr1[3] = 0.0;
    __declspec(align(16)) float bgr2[4];
    bgr2[3] = 0.0;
    __declspec(align(16)) float bgr3[4];
    bgr3[3] = 0.0;
    __declspec(align(16)) float bgr4[4];
    bgr4[3] = 0.0;

    /// Step in 2x2 pel blocks. (4 pels per block).
    int xBlks = _width >> 1;
    int yBlks = _height >> 1;
    for(int yb = 0; yb < yBlks; yb++)
      for(int xb = 0; xb < xBlks; xb++)
      {
        int							chrOff	= yb*xBlks + xb;
        int							lumOff	= (yb*_width + xb) << 1;
        unsigned char*	t				= src + lumOff*3;

        bgr1[2] = (float)*t++;
        bgr1[1] = (float)*t++;
        bgr1[0] = (float)*t++;
        bgr2[2] = (float)*t++;
        bgr2[1] = (float)*t++;
        bgr2[0] = (float)*t++;
        t = t + _width*3 - 6;
        bgr3[2] = (float)*t++;
        bgr3[1] = (float)*t++;
        bgr3[0] = (float)*t++;
        bgr4[2] = (float)*t++;
        bgr4[1] = (float)*t++;
        bgr4[0] = (float)*t++;

        __m128 xmm1 = _mm_load_ps(bgr1);
        __m128 xmm2 = _mm_load_ps(bgr2);
        __m128 xmm3 = _mm_load_ps(bgr3);
        __m128 xmm4 = _mm_load_ps(bgr4);

        // Y
        __m128 xmm_res_y = _mm_mul_ps(xmm1, xmm_y);
        py[lumOff] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255((xmm_res_y.m128_f32[0] + xmm_res_y.m128_f32[1] + xmm_res_y.m128_f32[2] ));
        // Y
        xmm_res_y = _mm_mul_ps(xmm2, xmm_y);
        py[lumOff + 1] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255((xmm_res_y.m128_f32[0] + xmm_res_y.m128_f32[1] + xmm_res_y.m128_f32[2] ));
        lumOff += _width;
        //t = t + _width*3 - 6;
        // Y
        xmm_res_y = _mm_mul_ps(xmm3, xmm_y);
        py[lumOff] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255((xmm_res_y.m128_f32[0] + xmm_res_y.m128_f32[1] + xmm_res_y.m128_f32[2] ));
        // Y
        xmm_res_y = _mm_mul_ps(xmm4, xmm_y);
        py[lumOff+1] = (yuvType)RRGB24YUVCI2_RANGECHECK_0TO255((xmm_res_y.m128_f32[0] + xmm_res_y.m128_f32[1] + xmm_res_y.m128_f32[2] ));


        // U
        __m128 xmm_res = _mm_add_ps(
                              _mm_add_ps(_mm_mul_ps(xmm1, xmm_u), _mm_mul_ps(xmm2, xmm_u)),
                              _mm_add_ps(_mm_mul_ps(xmm3, xmm_u), _mm_mul_ps(xmm4, xmm_u))
                           );

        float fU  = xmm_res.m128_f32[0] + xmm_res.m128_f32[1] + xmm_res.m128_f32[2];

        xmm_res = _mm_add_ps(
          _mm_add_ps(_mm_mul_ps(xmm1, xmm_v), _mm_mul_ps(xmm2, xmm_v)),
          _mm_add_ps(_mm_mul_ps(xmm3, xmm_v), _mm_mul_ps(xmm4, xmm_v))
          );
        float fV  = xmm_res.m128_f32[0] + xmm_res.m128_f32[1] + xmm_res.m128_f32[2];

        /// Average the 4 chr values.
        int iu = (int)fU;
        int iv = (int)fV;
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
}

