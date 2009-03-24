/** @file

MODULE				: PicScalerRGB24Impl

FILE NAME			: PicScalerRGB24Impl.cpp

DESCRIPTION			: An RGB24 implementation derived from the general
					PicScalerBase() class. Scale a packed RGB24 image
					to the dimensions of another packed RGB24 image.
					Note that the upscaling is limited to 2x either 
					dimension.
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008, Meraka Institute
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of the Meraka Institute nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

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

#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "PicScalerRGB24Impl.h"

/*
===========================================================================
	Interface Methods.
===========================================================================
*/
/** Scale the size of the input image.
Scale the input image from its dimensions to that of the output image. No
memory size checking is done and is delegated to the calling process.
@param pOutImg	: Packed RGB 888 format main base image.
@param pInImg		: Packed RGB 888 format smaller sub image.
@return					: 0 = failed, 1 = success.
*/
int PicScalerRGB24Impl::Scale(void* pOutImg, void* pInImg)
{
	if( (pOutImg == NULL) || (pInImg == NULL) )
		return(0);

	unsigned char*	pSrc		= (unsigned char*)pInImg;
	unsigned char*	pDst		= (unsigned char*)pOutImg;
	
	double scalex = ((double)_widthIn)/((double)_widthOut);
	double scaley = ((double)_heightIn)/((double)_heightOut);

	int x,y,posx,posy,i,j;

	for(y = 0; y < _heightOut; y++)
	{
		posy = (int)((scaley * (double)y) + 0.5);
		if(posy < 0)	posy = 0;
		else if(posy >= _heightIn) posy = _heightIn-1;

		for(x = 0; x < _widthOut; x++)
		{
			posx = (int)((scalex * (double)x) + 0.5);
			if(posx < 0) posx = 0;
			else if(posx >= _widthIn) posx = _widthIn-1;

			/// Apply a weighted 3x3 FIR filter.
			int ai = (posy*_widthIn*3) + (posx*3);
			int b = 7 * (int)(*(pSrc + ai));
			int g = 7 * (int)(*(pSrc + (ai+1)));
			int r = 7 * (int)(*(pSrc + (ai+2)));
			for(i = -1; i <= 1; i++)
			{
				int row = posy + i;
				if(row < 0)	row = 0;
				else if(row >= _heightIn) row = _heightIn-1;
				for(j = -1; j <= 1; j++)
				{
					int col = posx + j;
					if(col < 0) col = 0;
					else if(col >= _widthIn) col = _widthIn-1;

					int aii = (row*_widthIn*3) + (col*3);
					b += (int)(*(pSrc + aii));
					g += (int)(*(pSrc + (aii+1)));
					r += (int)(*(pSrc + (aii+2)));

				}//end for j...
			}//end for i...

			/// Round before scaling.
			int ao = (y*_widthOut*3) + (x*3);
			*(pDst + ao)			= (unsigned char)((b + 8) >> 4);
			*(pDst + (ao+1))	= (unsigned char)((g + 8) >> 4);
			*(pDst + (ao+2))	= (unsigned char)((r + 8) >> 4);

		}//end for x...
		
	}//end for y...

	return(1);
}//end Scale.

