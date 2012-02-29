/** @file

MODULE				: OverlayExtMem2Dv2

TAG						: OEM2DV2

FILE NAME			: OverlayExtMem2Dv2.h

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
#ifndef _OVERLAYEXTMEM2DV2_H
#define _OVERLAYEXTMEM2DV2_H

/*
---------------------------------------------------------------------------
	Includes.
---------------------------------------------------------------------------
*/

#include	"OverlayMem2Dv2.h"

class OverlayExtMem2Dv2: public OverlayMem2Dv2
{
	/// Construction and destruction.
public:
	OverlayExtMem2Dv2(void* srcPtr, int srcWidth, int srcHeight, 
																	int width,		int height,
																	int bWidth,		int bHeight);

	virtual ~OverlayExtMem2Dv2();

	/// Member access.
public:
	int	GetBoundaryWidth(void)	{ return(_bWidth); }
	int	GetBoundaryHeight(void)	{ return(_bHeight); }

	virtual void	SetOrigin(int x, int y);

	/// Interface: Input/output functions.
public:
	/// Extend the boundary by alloc new mem. These static methods must
	/// be called from the thread that owns the src mem to ensure it is
	/// thread safe.	They are utility functions and are independent of the class. 
	static int ExtendBoundary(void* srcPtr, 
														int srcWidth,int srcHeight, 
														int widthBy, int heightBy,
														void** dstPtr);

	/// Fill an existing boundary.
	static void FillBoundary(void* srcPtr, int srcWidth,int srcHeight, 
																				 int widthBy,	int heightBy);

	/// Proxy class method.
	void 	FillBoundaryProxy(void);

protected:
	int	_bWidth;				///< Boundary width and height.
	int	_bHeight;
};// end class OverlayExtMem2Dv2.

#endif
