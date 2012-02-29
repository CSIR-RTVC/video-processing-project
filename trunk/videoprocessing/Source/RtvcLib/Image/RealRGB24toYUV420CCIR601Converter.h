/** @file

MODULE						: RealRGB24toYUV420CCIR601Converter

TAG								: RRGB24YUVCC

FILE NAME					: RealRGB24toYUV420CCIR601Converter.h

DESCRIPTION				: Double precision floating point RGB 24 bit to YUV420 colour 
										convertion derived from the RGBtoYUV420Converter base class.
										Implements the CCIR-601 colour conversion process. Use this 
										implementation as the reference.

REVISION HISTORY	:
									: 

COPYRIGHT					: 

RESTRICTIONS			: 
===========================================================================
*/
#ifndef _REALRGB24TOYUV420CCIR601CONVERTER_H
#define _REALRGB24TOYUV420CCIR601CONVERTER_H

#pragma once

#include "RGBtoYUV420Converter.h"

/*
===========================================================================
  Class definition.
===========================================================================
*/
class RealRGB24toYUV420CCIR601Converter: public RGBtoYUV420Converter
{
public:
	// Construction and destruction.
	RealRGB24toYUV420CCIR601Converter(void) { }
	RealRGB24toYUV420CCIR601Converter(int width, int height): RGBtoYUV420Converter(width,height) { }
	RealRGB24toYUV420CCIR601Converter(int width, int height, int chrOff): RGBtoYUV420Converter(width,height, chrOff) { }
	virtual ~RealRGB24toYUV420CCIR601Converter(void) {}

	// Interface.
	void Convert(void* pRgb, void* pY, void* pU, void* pV);

private:
  void FlipConvert(void* pRgb, void* pY, void* pU, void* pV);
  void NonFlipConvert(void* pRgb, void* pY, void* pU, void* pV);

};//end _REALRGB24TOYUV420CCIR601CONVERTER_H.


#endif
