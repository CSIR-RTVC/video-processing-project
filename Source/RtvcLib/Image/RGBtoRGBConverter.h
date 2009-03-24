/** @file

MODULE				: RGBtoRGBConverter

FILE NAME			: RGBtoRGBConverter.h

DESCRIPTION			: Colour convertions to between differing RGB colour spaces.
					This class is the base class defining the minimum interface	
					and properties for all derived classes. The conversion method
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
#ifndef _RGBTORGBCONVERTER_H
#define _RGBTORGBCONVERTER_H

#pragma once

/*
===========================================================================
  Class definition.
===========================================================================
*/
class RGBtoRGBConverter
{
public:
	// Construction and destruction.
	RGBtoRGBConverter(void) {_width = 0; _height = 0; }
	RGBtoRGBConverter(int width, int height) {_width = width; _height = height;}
	virtual ~RGBtoRGBConverter(void) {}

	// Interface.
	virtual void Convert(void* pRgbIn, void* pRgbOut) = 0;

	// Member interface.
	int	GetWidth(void)		{ return(_width); }
	int	GetHeight(void)		{ return(_height); }

	void	SetDimensions(int width, int height)	{_width = width; _height = height;}

protected:
	// Members.
	int	_width;
	int	_height;

};//end RGBtoRGBConverter.

#endif	// _RGBTORGBCONVERTER_H
