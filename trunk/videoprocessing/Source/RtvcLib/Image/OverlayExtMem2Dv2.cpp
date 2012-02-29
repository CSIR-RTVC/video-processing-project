/** @file

MODULE				: OverlayExtMem2Dv2

TAG						: OEM2DV2

FILE NAME			: OverlayExtMem2Dv2.cpp

DESCRIPTION		: A class to add extended boundary functionality to an overlay 
								OverlayMem2Dv2 class. The boundary is hidden from the calling 
								functions and is a polymorphism of an OverlayMem2Dv2 class. 

COPYRIGHT			: (c)CSIR 2007-2010 all rights resevered

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
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include <memory.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include	"OverlayExtMem2Dv2.h"

/*
---------------------------------------------------------------------------
	Construction, initialisation and destruction.
---------------------------------------------------------------------------
*/

OverlayExtMem2Dv2::OverlayExtMem2Dv2(void* srcPtr,	int srcWidth, int srcHeight, 
																										int width,		int height,
																										int bWidth,		int bHeight):
		OverlayMem2Dv2(srcPtr, srcWidth, srcHeight, width, height)
{
	_bWidth		= bWidth;
	_bHeight	= bHeight;
	/// Adjust for the boundary in the base class.
	OverlayMem2Dv2::SetOrigin(bWidth, bHeight);
}//end alt constructor.

OverlayExtMem2Dv2::~OverlayExtMem2Dv2()
{
}//end destructor.

/** Set the top left origin of the block.
This can be in the boundary signified by neg input values. The max input
must be checked against the size of the block.
@param x	: x loc.
@param y	: y loc.
@return		: none.
*/
void OverlayExtMem2Dv2::SetOrigin(int x, int y)
{
	x += _bWidth;

	if(x < 0)
		x = 0;
	else if(x > (_srcWidth - _width))
		x = _srcWidth - _width;
	_xPos = x;

	y += _bHeight;
	if(y < 0)
		y = 0;
	else if(y > (_srcHeight - _height))
		y = _srcHeight - _height;
	_yPos = y;

}//end SetOrigin.

/*
---------------------------------------------------------------------------
	Public input/output interface.
---------------------------------------------------------------------------
*/

/*
---------------------------------------------------------------------------
	Static utility methods.
---------------------------------------------------------------------------
*/

/// Extend the boundary by alloc new mem. This static method must
/// be called from the thread that owns the src mem to ensure it is
/// thread safe.
int OverlayExtMem2Dv2::ExtendBoundary(void* srcPtr,
																			int srcWidth,	int srcHeight, 
																			int widthBy,	int heightBy,
																			void** dstPtr)
{
	if(srcPtr == NULL)	///< Nothing to extend.
		return(0);

	int			y;
	short*	lclPtr	= (short *)(srcPtr);
	int			width		= srcWidth + 2*widthBy;
	int			height	= srcHeight + 2*heightBy;

	/// Create a new mem block to copy into.
	short*	newPtr = new short[width * height];
	if(newPtr == NULL)
		return(0);

	/// Copy the src into the centre section of the new mem.
	for(y = 0; y < srcHeight; y++)
		memcpy((void *)(&(newPtr[(heightBy + y)*width + widthBy])),
					 (void *)(&(lclPtr[y * srcWidth])),
					 srcWidth * sizeof(short));

	/// Copy the boundary.
	FillBoundary(newPtr, width, height, widthBy, heightBy);

	*dstPtr = (void *)newPtr;

	return(1);
}//end ExtendBoundary.

/// The inner block is valid and requires re-filling the boundary.
void OverlayExtMem2Dv2::FillBoundary(void* srcPtr,	int srcWidth,	int srcHeight, 
																										int widthBy,	int heightBy)
{
	int	x,y;
	short* lclPtr = (short *)srcPtr;

	/// Fast 2-D extended boundary mem address array.
	short** block = new short*[srcHeight];
	if(block == NULL)
		return;
	/// Fill the row addresses.
	for(y = 0; y < srcHeight; y++)
		block[y] = &(lclPtr[srcWidth * y]);

	/// Top left.
	for(y = 0; y < heightBy; y++)
		for(x = 0; x < widthBy; x++)
			block[y][x] = block[heightBy][widthBy];

	/// Top right.
	for(y = 0; y < heightBy; y++)
		for(x = (srcWidth - widthBy); x < srcWidth; x++)
			block[y][x] = block[heightBy][srcWidth - widthBy - 1];

	/// Bottom left.
	for(y = (srcHeight - heightBy); y < srcHeight; y++)
		for(x = 0; x < widthBy; x++)
			block[y][x] = block[srcHeight - heightBy - 1][widthBy];

	/// Bottom right.
	for(y = (srcHeight - heightBy); y < srcHeight; y++)
		for(x = (srcWidth - widthBy); x < srcWidth; x++)
			block[y][x] = block[srcHeight - heightBy - 1][srcWidth - widthBy - 1];

	/// Top.
	for(y = 0; y < heightBy; y++)
		memcpy((void *)(&(block[y][widthBy])),
					 (void *)(&(block[heightBy][widthBy])),
					 (srcWidth - 2*widthBy) * sizeof(short));

	/// Bottom.
	for(y = (srcHeight - heightBy); y < srcHeight; y++)
		memcpy((void *)(&(block[y][widthBy])),
					 (void *)(&(block[srcHeight - heightBy - 1][widthBy])),
					 (srcWidth - 2*widthBy) * sizeof(short));

	/// Left.
	for(y = heightBy; y < (srcHeight - heightBy); y++)
		for(x = 0; x < widthBy; x++)
			block[y][x] = block[y][widthBy];

	/// Right.
	for(y = heightBy; y < (srcHeight - heightBy); y++)
		for(x = (srcWidth - widthBy); x < srcWidth; x++)
			block[y][x] = block[y][srcWidth - widthBy - 1];

	if(block != NULL)
		delete[] block;

}//end FillBoundary.

void OverlayExtMem2Dv2::FillBoundaryProxy(void)
{
	FillBoundary(_pMem, _srcWidth, _srcHeight, _bWidth, _bHeight);
}//end FillBoundaryProxy.







