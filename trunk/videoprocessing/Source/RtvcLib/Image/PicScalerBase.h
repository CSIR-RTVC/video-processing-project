/** @file

MODULE				: PicScalerBase

FILE NAME			: PicScalerBase.h

DESCRIPTION			: This class is the base class defining the minimum interface	
					and properties for all derived implementations to scale the
					dimensions of an image either up or down. The instantiation 
					process must set the parameters and limits to the images such 
					that the Scale()implementation only requires source image 
					data pointers. All memory new/delete must be handled outside 
					of these objects. The scale method is pure virtual and must be 
					implemented by derived classes.
					  
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008 - 2011, CSIR
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
#ifndef _PICSCALERBASE_H
#define _PICSCALERBASE_H

#pragma once

/**
 * \ingroup ImageLib
 * This class is the base class defining the minimum interface	
 * and properties for all derived implementations to scale the
 * dimensions of an image either up or down. The instantiation 
 * process must set the parameters and limits to the images such 
 * that the Scale()implementation only requires source image 
 * data pointers. All memory new/delete must be handled outside 
 * of these objects. The scale method is pure virtual and must be 
 * implemented by derived classes.
 */
class PicScalerBase
{
public:
	// Construction and destruction.
	PicScalerBase(void) {_widthOut = 1; _heightOut = 1; _widthIn = 1; _heightIn = 1; }
	PicScalerBase(int widthOut, int heightOut, int widthIn, int heightIn) 
	{	_widthOut = widthOut; _heightOut = heightOut; _widthIn = widthIn; _heightIn = heightIn; }
	virtual ~PicScalerBase(void) {}

	// Interface.
	virtual int Scale(void* pOutImg, void* pInImg) = 0;

	// Member interface.
	int	GetOutWidth(void)		{ return(_widthOut); }
	int	GetOutHeight(void)	{ return(_heightOut); }
	int	GetInWidth(void)		{ return(_widthIn); }
	int	GetInHeight(void)		{ return(_heightIn); }
  
	void SetOutDimensions(int widthOut, int heightOut)	{_widthOut = widthOut; _heightOut = heightOut; }
	void SetInDimensions(int widthIn, int heightIn)			{_widthIn = widthIn; _heightIn = heightIn; }

// Private methods.
protected:
	// Members.
	int	_widthOut;	/// Image dimensions to fit to.
	int	_heightOut;
	int _widthIn;		/// Image dimensions to scale from.
	int _heightIn;

};//end PicScalerBase.

#endif	// _PICSCALERBASE_H
