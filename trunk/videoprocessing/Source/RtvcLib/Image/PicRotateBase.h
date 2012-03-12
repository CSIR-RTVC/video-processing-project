/** @file

MODULE                : PicRotateBase

FILE NAME             : PicRotateBase.h

DESCRIPTION           : This class is the base class defining the minimum interface	
						and properties for all derived implementations to rotate an image.
						The instantiation process must set the input dimensions and the rotation mode
						so that the Rotate implementation only requires source image data
						pointers. The Rotate method is pure virtual and must be 
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
#pragma once

enum ROTATE_MODE
{
	ROTATE_NONE						= 0,
	ROTATE_90_DEGREES_CLOCKWISE		= 1,
	ROTATE_180_DEGREES_CLOCKWISE	= 2,
	ROTATE_270_DEGREES_CLOCKWISE	= 3,
	ROTATE_FLIP_VERTICAL			= 4,
	ROTATE_FLIP_HORIZONTAL			= 5,
	ROTATE_FLIP_DIAGONALLY			= 6
};

/**
 * \ingroup ImageLib
 * This class is the base class defining the minimum interface	
 * and properties for all derived implementations to rotate an image.
 * The instantiation process must set the input dimensions and the rotation mode
 * so that the Rotate implementation only requires source image data
 * pointers. The Rotate method is pure virtual and must be 
 * implemented by derived classes.
 */
class PicRotateBase
{
public:

	PicRotateBase(){}
	virtual ~PicRotateBase(){}

	void SetInDimensions(int nWidth, int nHeight)
	{
		m_nWidth = nWidth;
		m_nHeight = nHeight;
	}

	void SetRotateMode(ROTATE_MODE eMode)
	{
		m_eMode = eMode;
	}

	/// Implementation must be overidden by sub class
	virtual bool Rotate(void* pInImg, void* pOutImg) = 0;

protected:
	/// Width of input image
	int m_nWidth;
	/// Height of input image
	int m_nHeight;

	/// Rotation mode
	ROTATE_MODE m_eMode;
};
