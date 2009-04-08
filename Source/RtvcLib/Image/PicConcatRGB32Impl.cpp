/** @file

MODULE						: PicConcatRGB32Impl

TAG								: PCRGB32I

FILE NAME					: PicConcatRGB32Impl.cpp

DESCRIPTION				: An RGB32 implementation derived from the general
										PicConcatBase() class. Concatenate a packed RGB32 
										image	to another packed RGB32 image.

COPYRIGHT					: (c)CSIR 2007-2010 all rights resevered

LICENSE						: Software License Agreement (BSD License)

RESTRICTIONS			: Redistribution and use in source and binary forms, with or without 
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

#include <string.h>
#include <stdlib.h>

#include "PicConcatRGB32Impl.h"

/*
===========================================================================
	Construction and Destruction.
===========================================================================
*/
PicConcatRGB32Impl::PicConcatRGB32Impl(void) 
{
	Create();
}//end default constuctor.
PicConcatRGB32Impl::PicConcatRGB32Impl(int width, int height, int width1st, int height1st, int width2nd, int height2nd, int orient): PicConcatBase(width,height,width1st,height1st,width2nd,height2nd,orient)
{
	Create();
}//end constructor.

PicConcatRGB32Impl::~PicConcatRGB32Impl(void)
{
	if(_pPicInPic != NULL)
		delete _pPicInPic;
}//end destructor.

int PicConcatRGB32Impl::Create(void)
{
	_pPicInPic = NULL;
	_pPicInPic = new PicInPicRGB32Impl();
	if(_pPicInPic == NULL)
		return(0);
	return(1);
}//end Create.

/*
===========================================================================
	Interface Methods.
===========================================================================
*/
/** Concatenate the 2nd img to the 1st img.
Attach the 2nd image to the 1st image at the set orientation (top,
bottom, left, right) in the output image. There is no memory checking
in this method and must be handled by the calling process.
@param pImg1st	: Packed RGB 888 format base image.
@param pImg2nd	: Packed RGB 888 format image to attach.
@param pImg			: Packed RGB 888 format output image.
@return					: 0 = failed, 1 = success.
*/
int PicConcatRGB32Impl::Concat(void* pImg1st, void* pImg2nd, void* pImg)
{
	if( (pImg == NULL) || (pImg1st == NULL) || (pImg2nd == NULL) || (_pPicInPic == NULL) )
		return(0);

	int posX1, posY1, posX2, posY2;	/// Coords for top-left of images in output image.
	switch(_orient)
	{
		case PicConcatBase::TOP :
			posX1 = 0;	/// Base image position.
			posY1 = 0;
			posX2 = 0;	/// Concat image position.
			posY2 = _height - _height2nd;
			if(posY2 < 0) posY2 = 0;
			break;
		case PicConcatBase::BOTTOM :
			posX1 = 0;	/// Base image position.
			posY1 = _height - _height1st;
			if(posY1 < 0) posY1 = 0;
			posX2 = 0;	/// Concat image position.
			posY2 = 0;
			break;
		case PicConcatBase::LEFT :
			posX1 = _width - _width1st;	/// Base image position.
			if(posX1 < 0) posX1 = 0;
			posY1 = 0;
			posX2 = 0;	/// Concat image position.
			posY2 = 0;
			break;
		case PicConcatBase::RIGHT :
			posX1 = 0;	/// Base image position.
			posY1 = 0;
			posX2 = _width - _width2nd;	/// Concat image position.
			if(posX2 < 0) posX2 = 0;
			posY2 = 0;
			break;
		default:
			posX1 = 0;	/// Base image position.
			posY1 = 0;
			posX2 = 0;	/// Concat image position.
			posY2 = 0;
			break;
	}//end switch _orient...

	_pPicInPic->SetDimensions(_width, _height);						/// Set dimensions of output image.

	/// Concat image written to output 1st.
	_pPicInPic->SetSubDimensions(_width2nd, _height2nd);	/// Set dim of concat image.
	_pPicInPic->SetPos(posX2, posY2);											/// Images are written from bottom to top.
	_pPicInPic->Insert(pImg2nd, pImg);

	/// Base image written last.
	_pPicInPic->SetSubDimensions(_width1st, _height1st);	/// Set dim of base image.
	_pPicInPic->SetPos(posX1, posY1);											/// Images are written from bottom to top.
	_pPicInPic->Insert(pImg1st, pImg);

	return(1);
}//end Concat.

