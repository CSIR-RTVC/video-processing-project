/** @file

MODULE				: YUV420toRGBConverter

TAG						: YUVRGBC

FILE NAME			: YUV420toRGBConverter.h

DESCRIPTION		: Colour convertions are required on the output of
								all video codecs. For embedded applications only some
								combinations of colour depths and inversions are required.
								This class is the base class defining the minimum interface
								and properties for all derived classes. 

COPYRIGHT			:	(c)CSIR 2007-2010 all rights resevered

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
#ifndef _YUV420TORGBCONVERTER_H
#define _YUV420TORGBCONVERTER_H


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
class YUV420toRGBConverter
{
public:
	/// Construction and destruction.
	YUV420toRGBConverter(void) {_width = 0; _height = 0; _rotate = 0; /*_chrOff = 0;*/ _flip = false; }
	YUV420toRGBConverter(int width, int height) {_width = width; _height = height; _rotate = 0; /*_chrOff = 0;*/ _flip = false; }
	YUV420toRGBConverter(int width, int height, int chrOff) {_width = width; _height = height; _rotate = 0; _chrOff = chrOff; _flip = false;}
	virtual ~YUV420toRGBConverter(void) {}

	/// Interface.
	virtual void Convert(void* pY, void* pU, void* pV, void* pRgb) = 0;

	/// Member interface.
	int	GetWidth(void)			{ return(_width); }
	int	GetHeight(void)			{ return(_height); }
	int	GetRotate(void)			{ return(_rotate); }
	//int GetChrOffset(void)	{ return(_chrOff); }
  bool GetFlip(void)      { return _flip; }

  void SetDimensions(int width, int height)	{_width = width; _height = height;}
	void SetRotate(int rotate)									{ _rotate = rotate;}
  void SetFlip(bool flip) { _flip = flip; }

protected:
	/// Members.
	int	_width;
	int	_height;
	int	_rotate;
	/// TOREMOVE
  /// Offset added to the chr values. Typically = 128 to shift all values to positive.
	int	_chrOff;
  /// Windows bitmaps are bottom to top, standard YUV formats are top to bottom
  /// for our YUV to be compatible with linux we need to flip the image
  bool _flip;
};//end YUV420toRGBConverter.


#endif	// _YUV420TORGBCONVERTER_H
