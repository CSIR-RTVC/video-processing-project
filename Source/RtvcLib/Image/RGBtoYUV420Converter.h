/** @file

MODULE				: RGBtoYUV420Converter

FILE NAME			: RGBtoYUV420Converter.h

DESCRIPTION			: Colour convertions to YUV420 are required on the input of
					all video codecs. For embedded applications only some
					combinations of colour depths are required. This class is 
					the base class defining the minimum interface	and 
					properties for all derived classes. The conversion method
					is pure virtual and must be implemented by derived classes.

					  
					  
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
#ifndef _RGBTOYUV420CONVERTER_H
#define _RGBTOYUV420CONVERTER_H

#pragma once

#ifdef _BUILD_FOR_SHORT
typedef short yuvType;
#else
typedef signed char yuvType ;
#endif

/*
===========================================================================
  Class definition.
===========================================================================
*/
class RGBtoYUV420Converter
{
public:
	// Construction and destruction.
	RGBtoYUV420Converter(void) {_width = 0; _height = 0; }
	RGBtoYUV420Converter(int width, int height) {_width = width; _height = height;}
	virtual ~RGBtoYUV420Converter(void) {}

	// Interface.
	virtual void Convert(void* pRgb, void* pY, void* pU, void* pV) = 0;

	// Member interface.
	int	GetWidth(void)		{ return(_width); }
	int	GetHeight(void)		{ return(_height); }

	void	SetDimensions(int width, int height)	{_width = width; _height = height;}

protected:
	// Members.
	int	_width;
	int	_height;

};//end RGBtoYUV420Converter.

#endif	// _RGBTOYUV420CONVERTER_H
