/** @file

MODULE						: FastRGB24toYUV420Converter

TAG								: FRGB24YUVC

FILE NAME					: FastRGB24toYUV420Converter.h

DESCRIPTION				: Fast RGB 24 bit to YUV420 colour convertion derived from
										the RGBtoYUV420Converter base class.

REVISION HISTORY	:
									: 

COPYRIGHT					: (c)VICS 2000-2006  all rights resevered - info@videocoding.com

RESTRICTIONS			: The information/data/code contained within this file is 
										the property of VICS limited and has been classified as 
										CONFIDENTIAL.
===========================================================================
*/
#ifndef _FASTRGB24TOYUV420CONVERTER_H
#define _FASTRGB24TOYUV420CONVERTER_H

#pragma once

#include "RGBtoYUV420Converter.h"

/*
===========================================================================
  Class definition.
===========================================================================
*/
class FastRGB24toYUV420Converter: public RGBtoYUV420Converter
{
public:
	// Construction and destruction.
	FastRGB24toYUV420Converter(void) { }
	FastRGB24toYUV420Converter(int width, int height): RGBtoYUV420Converter(width,height) { }
	virtual ~FastRGB24toYUV420Converter(void) {}

	// Interface.
	void Convert(void* pRgb, void* pY, void* pU, void* pV);

private:
	// Fast colour look up tables.
	static const int R0[];
	static const int R1[];
	static const int R2[];
	static const int R3[];
	static const int R4[];
	static const int R5[];
	static const int R6[];
	static const int R7[];
	static const int R8[];

};//end _FASTRGB24TOYUV420CONVERTER_H.


#endif
