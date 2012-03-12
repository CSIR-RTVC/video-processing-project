/** @file

MODULE				: PicCropperBase

FILE NAME			: PicCropperBase.h

DESCRIPTION			: This class is the base class defining the minimum interface	
					and properties for all derived implementations to crop an
					image and write it to another image. The instantiation process 
					must set the parameters and limits to the images such that
					the Crop() implementation only requires source image data
					pointers. The insertion method is pure virtual and must be 
					implemented by derived classes.
					  
					  
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
#ifndef _PICCROPPERBASE_H
#define _PICCROPPERBASE_H

#pragma once

/**
 * \ingroup ImageLib
 * This class is the base class defining the minimum interface	
 * and properties for all derived implementations to crop an
 * image and write it to another image. The instantiation process 
 * must set the parameters and limits to the images such that
 * the Crop() implementation only requires source image data
 * pointers. The insertion method is pure virtual and must be 
 * implemented by derived classes.
 */
class PicCropperBase
{
public:
	// Construction and destruction.
	PicCropperBase(void) {_widthOut = 0; _heightOut = 0; _widthIn = 0; _heightIn = 0; _byX1 = 0; _byX2 = 0; _byY1 = 0; _byY2 = 0; }
	PicCropperBase(int widthOut, int heightOut, int widthIn, int heightIn, int left, int right, int top, int bottom) 
	{	_widthOut = widthOut; _heightOut = heightOut; _widthIn = widthIn; _heightIn = heightIn; _byX1 = left; _byX2 = right; _byY2 = top; _byY1 = bottom; }
	virtual ~PicCropperBase(void) {}

	// Interface.
	virtual int Crop(void* pInImg, void* pImg) = 0;

	// Member interface.
	int	GetOutWidth(void)		{ return(_widthOut); }
	int	GetOutHeight(void)	{ return(_heightOut); }
	int	GetInWidth(void)		{ return(_widthIn); }
	int	GetInHeight(void)		{ return(_heightIn); }
	int	GetCropLeft(void)		{ return(_byX1); }
	int	GetCropRight(void)	{ return(_byX2); }
	int	GetCropBottom(void)	{ return(_byY1); }
	int	GetCropTop(void)		{ return(_byY2); }
  
	void SetOutDimensions(int widthOut, int heightOut)			{_widthOut = widthOut; _heightOut = heightOut; }
	void SetInDimensions(int widthIn, int heightIn)					{_widthIn = widthIn; _heightIn = heightIn; }
	void SetCrop(int left, int right, int top, int bottom)	{_byX1 = left; _byX2 = right; _byY2 = top; _byY1 = bottom; }

protected:
	// Members.
	int	_widthOut;		/// Of cropped output image.
	int	_heightOut;
	int _widthIn;			/// Of input image to crop.
	int _heightIn;
	int _byX1;				/// Crop pels from the left.
	int _byX2;				/// Crop pels from the right.
	int _byY1;				/// Crop from the bottom.
	int _byY2;				/// Crop from the top.

};//end PicCropperBase.

#endif	// _PICCROPPERBASE_H
