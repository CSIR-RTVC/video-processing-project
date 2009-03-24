/** @file

MODULE				: PicScalerYUV420PImpl

FILE NAME			: PicScalerYUV420PImpl.cpp

DESCRIPTION			: A YUV420P planar implementation derived from the 
					general PicScalerBase() class. Scale a YUV420P
					planar image to the dimensions of another YUV420P
					planar image.
					  
					  
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

#include "PicScalerYUV420PImpl.h"

/*
===========================================================================
	Interface Methods.
===========================================================================
*/
/** Scale the size of the input image.
Scale the input image from its dimensions to that of the output image. The 
lum Y 8 bit pels are followed immediately by the U and then V 8 bit values.
No memory size checking is done and is delegated to the calling process.
@param pOutImg	: Planar YUV420P format output image.
@param pInImg		: Planar YUV420P format input image.
@return					: 0 = failed, 1 = success.
*/
int PicScalerYUV420PImpl::Scale(void* pOutImg, void* pInImg)
{
	if( (pOutImg == NULL) || (pInImg == NULL) )
		return(0);

	scaleType*	pSrc		= (scaleType*)pInImg;
	scaleType*	pDst		= (scaleType*)pOutImg;
	
	double scalex = ((double)_widthIn)/((double)_widthOut);
	double scaley = ((double)_heightIn)/((double)_heightOut);

	int x,y,posx,posy,i,j;

	/// Lum 1st.
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
			int ai = (posy*_widthIn) + posx;
			int lum = 7 * (int)(*(pSrc + ai));
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

					int aii = (row*_widthIn) + col;
					lum += (int)(*(pSrc + aii));

				}//end for j...
			}//end for i...

			/// Round before scaling.
			int ao = (y*_widthOut) + x;
			*(pDst + ao) = (scaleType)((lum + 8) >> 4);

		}//end for x...
		
	}//end for y...

	/// Chr U and V.
	int chrWidthIn		= _widthIn/2;
	int chrHeightIn		= _heightIn/2;
	int chrWidthOut		= _widthOut/2;
	int chrHeightOut	= _heightOut/2;

	scaleType*	pSrcU		= (scaleType*)pInImg + (_widthIn * _heightIn);
	scaleType*	pSrcV		= (scaleType*)pInImg + (_widthIn * _heightIn) + (chrWidthIn * chrHeightIn);
	scaleType*	pDstU		= (scaleType*)pOutImg + (_widthOut * _heightOut);
	scaleType*	pDstV		= (scaleType*)pOutImg + (_widthOut * _heightOut) + (chrWidthOut * chrHeightOut);
	
	for(y = 0; y < chrHeightOut; y++)
	{
		posy = (int)((scaley * (double)y) + 0.5);
		if(posy < 0)	posy = 0;
		else if(posy >= chrHeightIn) posy = chrHeightIn-1;

		for(x = 0; x < chrWidthOut; x++)
		{
			posx = (int)((scalex * (double)x) + 0.5);
			if(posx < 0) posx = 0;
			else if(posx >= chrWidthIn) posx = chrWidthIn-1;

			/// Apply a weighted 3x3 FIR filter.
			int ai = (posy*chrWidthIn) + posx;
			int chrU = 7 * (int)(*(pSrcU + ai));
			int chrV = 7 * (int)(*(pSrcV + ai));
			for(i = -1; i <= 1; i++)
			{
				int row = posy + i;
				if(row < 0)	row = 0;
				else if(row >= chrHeightIn) row = chrHeightIn-1;
				for(j = -1; j <= 1; j++)
				{
					int col = posx + j;
					if(col < 0) col = 0;
					else if(col >= chrWidthIn) col = chrWidthIn-1;

					int aii = (row*chrWidthIn) + col;
					chrU += (int)(*(pSrcU + aii));
					chrV += (int)(*(pSrcV + aii));

				}//end for j...
			}//end for i...

			/// Round before scaling.
			int ao = (y*chrWidthOut) + x;
			*(pDstU + ao) = (scaleType)((chrU + 8) >> 4);
			*(pDstV + ao) = (scaleType)((chrV + 8) >> 4);

		}//end for x...
		
	}//end for y...
	return(1);
}//end Scale.

