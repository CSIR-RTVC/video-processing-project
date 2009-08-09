/** @file

MODULE						: PicConcatBase

TAG								: PCB

FILE NAME					: PicConcatBase.h

DESCRIPTION				: This class is the base class defining the minimum interface	
										and properties for all derived implementations to concatenate
										a 2nd image on any boundary of the 1st image. The instantiation 
										process must set the parameters and limits to the images such 
										that the Concat()implementation only requires source image 
										data pointers. All memory new/delete must be handled outside 
										of these objects. The concat method is pure virtual and must be 
										implemented by derived classes.

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
#ifndef _PICCONCATBASE_H
#define _PICCONCATBASE_H

/**
 * \ingroup ImageLib
 * This class is the base class defining the minimum interface	
 * and properties for all derived implementations to concatenate
 * a 2nd image on any boundary of the 1st image. The instantiation 
 * process must set the parameters and limits to the images such 
 * that the Concat()implementation only requires source image 
 * data pointers. All memory new/delete must be handled outside 
 * of these objects. The concat method is pure virtual and must be 
 * implemented by derived classes.
 */
class PicConcatBase
{
public:
	// Construction and destruction.
	PicConcatBase(void) {_width = 0; _height = 0; _width1st = 0; _height1st = 0; _width2nd = 0; _height2nd = 0; _orient = PicConcatBase::TOP; }
	PicConcatBase(int width, int height, int width1st, int height1st, int width2nd, int height2nd, int orient) 
	{	_width = width; _height = height; _width1st = width1st; _height1st = height1st; _width2nd = width2nd; _height2nd = height2nd; _orient = orient;}
	virtual ~PicConcatBase(void) {}

	// Interface.
	virtual int Concat(void* pImg1st, void* pImg2nd, void* pImg) = 0;

	// Member interface.
	int	GetOutWidth(void)			{ return(_width); }
	int	GetOutHeight(void)		{ return(_height); }
	int	Get1stWidth(void)			{ return(_width1st); }
	int	Get1stHeight(void)		{ return(_height1st); }
	int	Get2ndWidth(void)			{ return(_width2nd); }
	int	Get2ndHeight(void)		{ return(_height2nd); }
	int GetOrientation(void)	{ return(_orient); }
  
	void SetOutDimensions(int width, int height)				{_width = width; _height = height; }
	void Set1stDimensions(int width1st, int height1st)	{_width1st = width1st; _height1st = height1st; }
	void Set2ndDimensions(int width2nd, int height2nd)	{_width2nd = width2nd; _height2nd = height2nd; }
	void SetOrientation(int orient)											{ _orient = orient; }

	// Private methods.
protected:

// Constants.
protected:
	// Orientation of the concatenation.
	static const int TOP		= 0;
	static const int BOTTOM = 1;
	static const int LEFT		= 2;
	static const int RIGHT	= 3;

protected:
	// Members.
	int	_width;				/// Of total output image that contains both input images.
	int	_height;
	int _width1st;		/// Of 1st base image.
	int _height1st;
	int _width2nd;		/// Of 2nd image to concat to 1st image.
	int _height2nd;
	int _orient;			/// Position to put the 2nd image = {TOP, BOTTOM, LEFT, RIGHT}.

};//end PicConcatBase.

#endif	// _PICCONCATBASE_H
