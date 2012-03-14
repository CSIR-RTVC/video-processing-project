/** @file

MODULE				: RealYUV420toRGB24CCIR601ConverterVer16

TAG						: RYUVRGB24CCV16

FILE NAME			: RealYUV420toRGB24CCIR601ConverterVer16.h

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
#ifndef _REALYUV420TORGB24CCIR601CONVERTERVER16_H
#define _REALYUV420TORGB24CCIR601CONVERTERVER16_H

#include "YUV420toRGBConverter.h"

/*
===========================================================================
  Class definition.
===========================================================================
*/
class RealYUV420toRGB24CCIR601ConverterVer16: public YUV420toRGBConverter
{
	public:
		// Construction and destruction.
		RealYUV420toRGB24CCIR601ConverterVer16(void) { }
		RealYUV420toRGB24CCIR601ConverterVer16(int width, int height): YUV420toRGBConverter(width,height) { }
		virtual ~RealYUV420toRGB24CCIR601ConverterVer16(void) {}

		// Interface.
		virtual void Convert(void* pY, void* pU, void* pV, void* pRgb) 
		{
			if(_rotate) RotateConvert(pY, pU, pV, pRgb);
			else				NonRotateConvert(pY, pU, pV, pRgb);
		};

	protected:
		virtual void NonRotateConvert(void* pY, void* pU, void* pV, void* pRgb);
		virtual void RotateConvert(void* pY, void* pU, void* pV, void* pRgb);

};//end RealYUV420toRGB24CCIR601ConverterVer16.


#endif	// _REALYUV420TORGB24CCIR601CONVERTERVER16_H
