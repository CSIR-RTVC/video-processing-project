/** @file

MODULE						: PicInPicBase

TAG								: PIPB

FILE NAME					: PicInPicBase.h

DESCRIPTION				: This class is the base class defining the minimum interface	
										and properties for all derived implementations to insert a
										sub image within another image. The instantiation process 
										must set the parameters and limits to the images such that
										the Insert() implementation only requires source image data
										pointers. The insertion method is pure virtual and must be 
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
#ifndef _PICINPICBASE_H
#define _PICINPICBASE_H

/**
 * \ingroup ImageLib
 * This class is the base class defining the minimum interface	
 * and properties for all derived implementations to insert a
 * sub image within another image. The instantiation process 
 * must set the parameters and limits to the images such that
 * the Insert() implementation only requires source image data
 * pointers. The insertion method is pure virtual and must be 
 * implemented by derived classes.
 */
class PicInPicBase
{
public:
	// Construction and destruction.
	PicInPicBase(void) {_width = 0; _height = 0; _subWidth = 0; _subHeight = 0; _xPos = 0; _yPos = 0; _writeWidth = 0; _writeHeight = 0; }
	PicInPicBase(int width, int height, int subWidth, int subHeight, int xPos, int yPos) 
	{	_width = width; _height = height; _subWidth = subWidth; _subHeight = subHeight; _xPos = xPos; _yPos = yPos; SetActualSubDimensions();}
	virtual ~PicInPicBase(void) {}

	// Interface.
	void Insert(void* pSubImg, void* pImg)
    {
        if (_borderWidth == 0)
            DoInsert(pSubImg, pImg);
        else
            DoInsertWithBorder(pSubImg, pImg);
    }

	// Member interface.
	int	GetWidth(void)						{ return(_width); }
	int	GetHeight(void)						{ return(_height); }
	int	GetSubWidth(void)					{ return(_subWidth); }
	int	GetSubHeight(void)				{ return(_subHeight); }
	int	GetActualSubWidth(void)		{ return(_writeWidth); }
	int	GetActualSubHeight(void)	{ return(_writeHeight); }
	int GetXPos(void)							{ return(_xPos); }
	int GetYPos(void)							{ return(_yPos); }
    int GetBorderWidth() const { return _borderWidth; }
  
	void SetDimensions(int width, int height)						{_width = width; _height = height; SetActualSubDimensions(); }
	void SetSubDimensions(int subWidth, int subHeight)	{_subWidth = subWidth; _subHeight = subHeight; SetActualSubDimensions(); }
	void SetPos(int x, int y)														{ _xPos = x; _yPos = y; SetActualSubDimensions(); }
    void SetBorderWidth(int val) { _borderWidth = val; }

	// Private methods.
protected:
	void SetActualSubDimensions(void)
	{ if((_xPos + _subWidth) > _width) _writeWidth = (_width - _xPos); else _writeWidth = _subWidth;
		if((_yPos + _subHeight) > _height) _writeHeight = (_height - _yPos); else _writeHeight = _subHeight; }

    virtual void DoInsert(void* pSubImg, void* pImg) = 0;
    virtual void DoInsertWithBorder(void* pSubImg, void* pImg) = 0;
protected:
	// Members.
	int	_width;				// Of (larger) image that receives the insertion.
	int	_height;
	int _subWidth;		// Of inserted image.
	int _subHeight;
	int _xPos;				// Top-left position to insert.
	int _yPos;
	int _writeWidth;	// Actual dim to write that prevents outside edge insertion.
	int _writeHeight;
    int _borderWidth;
};//end PicInPicBase.

#endif	// _PICINPICBASE_H
