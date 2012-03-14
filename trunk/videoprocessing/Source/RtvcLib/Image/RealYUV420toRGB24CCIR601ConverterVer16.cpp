/** @file

MODULE				: RealYUV420toRGB24CCIR601ConverterVer16

TAG						: RYUVRGB24CCV16

FILE NAME			: RealYUV420toRGB24CCIR601ConverterVer16.cpp

DESCRIPTION		: Floating point implementation of YUV420 (16 bpp) to RGB 24 bit 
								colour convertion derived from	the YUV420toRGBConverter base 
								class. Assumes that the colour space of the YUV components
								are defined by the CCIR-601 standard.

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

#include "RealYUV420toRGB24CCIR601ConverterVer16.h"

/*
===========================================================================
	Macros.
===========================================================================
*/
#define RYUVRGB24CCV16_RANGECHECK_0TO255(x) ( (((x) <= 255)&&((x) >= 0))?((x)):( ((x) > 255)?(255):(0) ) )

//	CCIR-601: The defenition assumes limited Y and UV (CbCr) ranges but the inverse
//						conversion will treat it as full range and only compensate for the
//						shift.
//		r = (y-16) + 1.140(v-128)
//		g = (y-16) - 0.394(u-128) - 0.581(v-128)
//		b = (y-16) + 2.033(u-128)

#define RYUVRGB24CCV16_U0		 2.033
#define RYUVRGB24CCV16_U1		-0.394
#define RYUVRGB24CCV16_V1		-0.581
#define RYUVRGB24CCV16_V0		 1.140

/*
===========================================================================
	Private Methods.
===========================================================================
*/
void RealYUV420toRGB24CCIR601ConverterVer16::NonRotateConvert(void* pY, void* pU, void* pV, void* pRgb)
{
	unsigned char* 	optr	=	(unsigned char *)pRgb;
	short*				py		= (short *)pY;
	short*				pu		= (short *)pU;
	short*				pv		= (short *)pV;
	int		lumX	= _width;
	int		lumY	= _height;
	int		uvX		= _width >> 1;

	int x,y;
	int lumposy;
	int rgbposy;
	int uvposy;
  int r,b,g;

	int tworows  = lumX << 1;
	int rgb1row  = (_width * 3);
	int rgb2rows = (_width * 3) << 1;

	for(y = 0,lumposy = 0,uvposy = 0,rgbposy = 0;	y < lumY;	y += 2,lumposy += tworows,uvposy += uvX,rgbposy += rgb2rows)
	{
		int lumpos0 = lumposy;					// Reset to start of rows.
		int lumpos1 = lumposy + lumX;
		int uvpos	 = uvposy;

		int rgbpos0 = rgbposy;
		int rgbpos1 = rgbposy + rgb1row;

		for(x = 0; x < lumX; x += 2)
		{
			double lum00 = (double)( (int)(py[lumpos0++]) - 16);
			double u		 = (double)( (int)(pu[uvpos]) - 128);
			double v		 = (double)( (int)(pv[uvpos++]) - 128);

			// Lum00, u and v.

  		// Fast calculation intermediate variables. 
			double cc = (RYUVRGB24CCV16_U0 * u) + 0.5;
			double cb = (RYUVRGB24CCV16_U1 * u) + (RYUVRGB24CCV16_V1 * v) + 0.5;
			double ca = (RYUVRGB24CCV16_V0 * v) + 0.5;

			b = (int)(lum00 + cc);
			g = (int)(lum00 + cb);
			r = (int)(lum00 + ca);
  
			// R, G & B have range 0..255.
			optr[rgbpos0++] = (unsigned char)(RYUVRGB24CCV16_RANGECHECK_0TO255(b));
			optr[rgbpos0++] = (unsigned char)(RYUVRGB24CCV16_RANGECHECK_0TO255(g));
			optr[rgbpos0++] = (unsigned char)(RYUVRGB24CCV16_RANGECHECK_0TO255(r));

			// Lum01.
			double lum01 = (double)( (int)(py[lumpos0++]) - 16 );

			b = (int)(lum01 + cc);
			g = (int)(lum01 + cb);
			r = (int)(lum01 + ca);
  
			optr[rgbpos0++] = (unsigned char)(RYUVRGB24CCV16_RANGECHECK_0TO255(b));
			optr[rgbpos0++] = (unsigned char)(RYUVRGB24CCV16_RANGECHECK_0TO255(g));
			optr[rgbpos0++] = (unsigned char)(RYUVRGB24CCV16_RANGECHECK_0TO255(r));

			// Lum10.
			double lum10 = (double)( (int)(py[lumpos1++]) - 16 );

			b = (int)(lum10 + cc);
			g = (int)(lum10 + cb);
			r = (int)(lum10 + ca);
  
			optr[rgbpos1++] = (unsigned char)(RYUVRGB24CCV16_RANGECHECK_0TO255(b));
			optr[rgbpos1++] = (unsigned char)(RYUVRGB24CCV16_RANGECHECK_0TO255(g));
			optr[rgbpos1++] = (unsigned char)(RYUVRGB24CCV16_RANGECHECK_0TO255(r));

			// Lum11.
			double lum11 = (double)( (int)(py[lumpos1++]) - 16);

			b = (int)(lum11 + cc);
			g = (int)(lum11 + cb);
			r = (int)(lum11 + ca);
  
			optr[rgbpos1++] = (unsigned char)(RYUVRGB24CCV16_RANGECHECK_0TO255(b));
			optr[rgbpos1++] = (unsigned char)(RYUVRGB24CCV16_RANGECHECK_0TO255(g));
			optr[rgbpos1++] = (unsigned char)(RYUVRGB24CCV16_RANGECHECK_0TO255(r));

		}//end for x...
		
	}//end for y...

}//end NonRotateConvert.

void RealYUV420toRGB24CCIR601ConverterVer16::RotateConvert(void* pY, void* pU, void* pV, void* pRgb)
{
	unsigned char* 	optr	=	(unsigned char *)pRgb;
	short*				py		= (short *)pY;
	short*				pu		= (short *)pU;
	short*				pv		= (short *)pV;
	int		lumX	= _width;
	int		lumY	= _height;
	int		uvX		= _width >> 1;

	int x,y;
	int lumposy;
	int rgbposx;
	int uvposy;
  int r,b,g;

	int tworows  = lumX << 1;
	//int rgb1row  = (_width * 3);
	int rgb1row  = (_height * 3);

	for(y = 0,lumposy = 0,uvposy = 0,rgbposx = 0;	y < lumY;	y += 2,lumposy += tworows,uvposy += uvX,rgbposx += 6)
	{
		int lumpos0 = lumposy;					// Reset to start of rows.
		int lumpos1 = lumposy + lumX;
		int uvpos	 = uvposy;

		int rgbpos0 = rgbposx;
		int rgbpos1 = rgbposx + 3;

		for(x = 0; x < lumX; x += 2)
		{
			double lum00 = (double)( (int)(py[lumpos0++]) - 16 );
			double u		 = (double)( (int)(pu[uvpos]) - 128 );
			double v		 = (double)( (int)(pv[uvpos++]) - 128 );

			// Lum00, u and v.

  		// Fast calculation intermediate variables. 
			double cc = (RYUVRGB24CCV16_U0 * u) + 0.5;
			double cb = (RYUVRGB24CCV16_U1 * u) + (RYUVRGB24CCV16_V1 * v) + 0.5;
			double ca = (RYUVRGB24CCV16_V0 * v) + 0.5;

			b = (int)(lum00 + cc);
			g = (int)(lum00 + cb);
			r = (int)(lum00 + ca);
  
			// R, G & B have range 0..255.
			optr[rgbpos0]		= (unsigned char)(RYUVRGB24CCV16_RANGECHECK_0TO255(b));
			optr[rgbpos0+1] = (unsigned char)(RYUVRGB24CCV16_RANGECHECK_0TO255(g));
			optr[rgbpos0+2] = (unsigned char)(RYUVRGB24CCV16_RANGECHECK_0TO255(r));
			rgbpos0 += rgb1row;

			// Lum01.
			double lum01 = (double)( (int)(py[lumpos0++]) - 16 );

			b = (int)(lum01 + cc);
			g = (int)(lum01 + cb);
			r = (int)(lum01 + ca);
  
			optr[rgbpos0]		= (unsigned char)(RYUVRGB24CCV16_RANGECHECK_0TO255(b));
			optr[rgbpos0+1] = (unsigned char)(RYUVRGB24CCV16_RANGECHECK_0TO255(g));
			optr[rgbpos0+2] = (unsigned char)(RYUVRGB24CCV16_RANGECHECK_0TO255(r));
			rgbpos0 += rgb1row;

			// Lum10.
			double lum10 = (double)( (int)(py[lumpos1++]) - 16 );

			b = (int)(lum10 + cc);
			g = (int)(lum10 + cb);
			r = (int)(lum10 + ca);
  
			optr[rgbpos1]		= (unsigned char)(RYUVRGB24CCV16_RANGECHECK_0TO255(b));
			optr[rgbpos1+1] = (unsigned char)(RYUVRGB24CCV16_RANGECHECK_0TO255(g));
			optr[rgbpos1+2] = (unsigned char)(RYUVRGB24CCV16_RANGECHECK_0TO255(r));
			rgbpos1 += rgb1row;

			// Lum11.
			double lum11 = (double)( (int)(py[lumpos1++]) - 16 );

			b = (int)(lum11 + cc);
			g = (int)(lum11 + cb);
			r = (int)(lum11 + ca);
  
			optr[rgbpos1]		= (unsigned char)(RYUVRGB24CCV16_RANGECHECK_0TO255(b));
			optr[rgbpos1+1] = (unsigned char)(RYUVRGB24CCV16_RANGECHECK_0TO255(g));
			optr[rgbpos1+2] = (unsigned char)(RYUVRGB24CCV16_RANGECHECK_0TO255(r));
			rgbpos1 += rgb1row;

		}//end for x...
		
	}//end for y...

}//end NonNonRotateConvert.


