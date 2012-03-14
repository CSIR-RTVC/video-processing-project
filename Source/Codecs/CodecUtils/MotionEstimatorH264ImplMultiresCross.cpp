/** @file

MODULE				: MotionEstimatorH264ImplMultiresCross

TAG						: MEH264IM

FILE NAME			: MotionEstimatorH264ImplMultiresCross.cpp

DESCRIPTION		: A fast unrestricted motion estimator implementation for 
								Recommendation H.264 (03/2005) with both absolute error 
								difference and square error measure. Access is via an 
								IMotionEstimator interface. There are 2 mode levels of 
								execution speed vs. quality. The boundary is extended to 
								accomodate the selected motion range. The estimation is
								performed on lower resolution representations and at the 
								lowest resolution a cascaded cross search is performed.

LICENSE	: GNU Lesser General Public License

Copyright (c) 2008 - 2012, CSIR
All rights reserved.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

#include	"MotionEstimatorH264ImplMultiresCross.h"

/*
--------------------------------------------------------------------------
  Constants. 
--------------------------------------------------------------------------
*/
/// Calc = ((16[vec dim] * 16[vec dim]) * 2.
#define MEH264IMC_FULL_MOTION_NOISE_FLOOR					512
#define MEH264IMC_MOTION_NOISE_FLOOR							 26 //24 //sqrt 512
#define MEH264IMC_L0_MOTION_VECTOR_REFINED_RANGE		2
#define MEH264IMC_L1_MOTION_VECTOR_REFINED_RANGE		2

/// Boundary padding past the motion vector extremes. Required for calculating
/// sub-pixel interpolations. Different for different levels.
#define MEH264IMC_PADDING													3	
#define MEH264IMC_L1_PADDING											1	
#define MEH264IMC_L2_PADDING											1	

/// Choose between sqr err distortion or abs diff metric.
#undef MEH264IMC_ABS_DIFF

/// Search range coords for centre motion vectors.
#define MEH264IMC_MOTION_SUB_POS_LENGTH 	8
MEH264IMC_COORD MEH264IMC_SubPos[MEH264IMC_MOTION_SUB_POS_LENGTH]	= 
{
	{-1,-1},{0,-1},{1,-1},{-1,0},{1,0},{-1,1},{0,1},{1,1}
};

#define MEH264IMC_MOTION_CROSS_POS_LENGTH 	4
MEH264IMC_COORD MEH264IMC_CrossPos[MEH264IMC_MOTION_CROSS_POS_LENGTH]	= 
{
	{-1,0},{1,0},{0,-1},{0,1}
};

/*
--------------------------------------------------------------------------
  Macros. 
--------------------------------------------------------------------------
*/
#define MEH264IMC_CLIP255(x)	( (((x) <= 255)&&((x) >= 0))? (x) : ( ((x) < 0)? 0:255 ) )

/*
--------------------------------------------------------------------------
  Construction. 
--------------------------------------------------------------------------
*/

MotionEstimatorH264ImplMultiresCross::MotionEstimatorH264ImplMultiresCross(	const void* pSrc, 
																														const void* pRef, 
																														int					imgWidth, 
																														int					imgHeight,
																														int					motionRange)
{
	ResetMembers();

	/// Parameters must remain const for the life time of this instantiation.
	_imgWidth				= imgWidth;					///< Width of the src and ref images. 
	_imgHeight			= imgHeight;				///< Height of the src and ref images.
	_macroBlkWidth	= 16;								///< Width of the motion block = 16 for H.263.
	_macroBlkHeight	= 16;								///< Height of the motion block = 16 for H.263.
	_motionRange		= motionRange;			///< (4x,4y) range of the motion vectors. _motionRange in 1/4 pel units.
	_pInput					= pSrc;
	_pRef						= pRef;

}//end constructor.

MotionEstimatorH264ImplMultiresCross::MotionEstimatorH264ImplMultiresCross(	const void* pSrc, 
																														const void* pRef, 
																														int					imgWidth, 
																														int					imgHeight,
																														int					motionRange,
																														void*				pDistortionIncluded)
{
	ResetMembers();

	/// Parameters must remain const for the life time of this instantiation.
	_imgWidth							= imgWidth;					///< Width of the src and ref images. 
	_imgHeight						= imgHeight;				///< Height of the src and ref images.
	_macroBlkWidth				= 16;								///< Width of the motion block = 16 for H.263.
	_macroBlkHeight				= 16;								///< Height of the motion block = 16 for H.263.
	_motionRange					= motionRange;			///< (4x,4y) range of the motion vectors. _motionRange in 1/4 pel units.
	_pInput								= pSrc;
	_pRef									= pRef;
	_pDistortionIncluded	= (bool *)pDistortionIncluded;

}//end constructor.

void MotionEstimatorH264ImplMultiresCross::ResetMembers(void)
{
	_ready	= 0;	///< Ready to estimate.
	_mode		= 1;	///< Speed mode or whatever. Default to slower speed.

	/// Parameters must remain const for the life time of this instantiation.
	_imgWidth				= 0;					///< Width of the src and ref images. 
	_imgHeight			= 0;					///< Height of the src and ref images.
	_macroBlkWidth	= 16;					///< Width of the motion block = 16 for H.264.
	_macroBlkHeight	= 16;					///< Height of the motion block = 16 for H.264.
	_motionRange		= 64;					///< (4x,4y) range of the motion vectors.
	_pInput					= NULL;
	_pRef						= NULL;

	/// Level 0: Input mem overlay members.
	_pInOver					= NULL;			///< Input overlay with motion block dim.
	/// Level 1: Subsampled input by 2.
	_pInL1						= NULL;			///< Input mem at (_l1Width * _l1Height).
	_l1Width					= 0;
	_l1Height					= 0;
	_l1MacroBlkWidth	= 0;
	_l1MacroBlkHeight = 0;
	_l1MotionRange		= 0;
	_pInL1Over				= NULL;			///< Input overlay with level 1 motion block dim.
	/// Level 2: Subsampled input by 4.
	_pInL2						= NULL;			///< Input mem at (_l2Width * _l2Height).
	_l2Width					= 0;
	_l2Height					= 0;
	_l2MacroBlkWidth	= 0;
	_l2MacroBlkHeight = 0;
	_l2MotionRange		= 0;
	_pInL2Over				= NULL;			///< Input overlay with level 2 motion block dim.

	/// Level 0: Ref mem overlay members.
	_pRefOver					= NULL;			///< Ref overlay with whole block dim.
	_pExtRef					= NULL;			///< Extended ref mem created by ExtendBoundary() call.
	_extWidth					= 0;
	_extHeight				= 0;
	_extBoundary			= 0;
	_pExtRefOver			= NULL;			///< Extended ref overlay with motion block dim.
	/// Level 1: Subsampled ref by 2.
	_pRefL1						= NULL;			///< Ref mem at (_l1Width * _l1Height).
	_pRefL1Over				= NULL; 		///< Ref overlay with whole level 1 block dim.
	_pExtRefL1				= NULL;			///< Extended ref mem create by ExtendBoundary() call.
	_extL1Width				= 0;
	_extL1Height			= 0;
	_extL1Boundary		= 0;
	_pExtRefL1Over		= NULL;			///< Ref overlay with level 1 motion block dim.
	/// Level 2: Subsampled ref by 4.
	_pRefL2						= NULL;			///< Ref mem at (_l2Width * _l2Height).
	_pRefL2Over				= NULL; 		///< Ref overlay with whole level 2 block dim.
	_pExtRefL2				= NULL;			///< Extended ref mem create by ExtendBoundary() call.
	_extL2Width				= 0;
	_extL2Height			= 0;
	_extL2Boundary		= 0;
	_pExtRefL2Over		= NULL;			///< Ref overlay with level 2 motion block dim.
	/// A 1/4 pel refinement window.
	_pWin							= NULL;
	_Win							= NULL;

	/// Temp working block and its overlay.
	_pMBlk						= NULL;			///< Motion block temp mem.
	_pMBlkOver				= NULL;			///< Motion block overlay of temp mem.

	/// Hold the resulting motion vectors in a byte array.
	_pMotionVectorStruct = NULL;

	/// A flag per macroblock to include it in the distortion accumulation.
	_pDistortionIncluded = NULL;
}//end ResetMembers.

MotionEstimatorH264ImplMultiresCross::~MotionEstimatorH264ImplMultiresCross(void)
{
	Destroy();
}//end destructor.

/*
--------------------------------------------------------------------------
  Public IMotionEstimator Interface. 
--------------------------------------------------------------------------
*/

int MotionEstimatorH264ImplMultiresCross::Create(void)
{
	/// Clean out old mem.
	Destroy();

	/// --------------- Configure input overlays --------------------------------
	/// Level 0: Put an overlay on the input with the block size set to the vector 
	/// dim. This is used to access input vectors.
	_pInOver = new OverlayMem2Dv2((void *)_pInput,_imgWidth,_imgHeight,_macroBlkWidth,_macroBlkHeight);
	if(_pInOver == NULL)
	{
		Destroy();
		return(0);
	}//end _pInOver...

	/// Level 1 is Level 0 sub sampled by 2.
	_l1Width					= _imgWidth/2;
	_l1Height					= _imgHeight/2;
	_l1MacroBlkWidth	= _macroBlkWidth/2;
	_l1MacroBlkHeight = _macroBlkHeight/2;
	_l1MotionRange		= _motionRange/8;	///< _motionRange is in 1/4 pel units. Convert to full level 1 pel units.

	/// Level 1: Input mem at (_l1Width * _l1Height) must be alloc.
	_pInL1 = new short[_l1Width * _l1Height];

	/// Level 1: Input overlay with level 1 motion block dim.
	if(_pInL1 != NULL)
	{
		_pInL1Over = new OverlayMem2Dv2(_pInL1,_l1Width,_l1Height,_l1MacroBlkWidth,_l1MacroBlkHeight);
		if(_pInL1Over == NULL)
		{
			Destroy();
			return(0);
		}//end if !_pInL1Over...
	}//end if ...
	else
	{
		Destroy();
		return(0);
	}//end else...

	/// Level 2 is Level 1 sub sampled by 2.
	_l2Width					= _l1Width/2;
	_l2Height					= _l1Height/2;
	_l2MacroBlkWidth	= _l1MacroBlkWidth/2;
	_l2MacroBlkHeight = _l1MacroBlkHeight/2;
	_l2MotionRange		= _l1MotionRange/2;

	/// Level 2: Input mem at (_l2Width * _l2Height) must be alloc.
	_pInL2 = new short[_l2Width * _l2Height];

	/// Level 2: Input overlay with level 2 motion block dim.
	if(_pInL2 != NULL)
	{
		_pInL2Over = new OverlayMem2Dv2(_pInL2,_l2Width,_l2Height,_l2MacroBlkWidth,_l2MacroBlkHeight);
		if(_pInL2Over == NULL)
		{
			Destroy();
			return(0);
		}//end if !_pInL2Over...
	}//end if ...
	else
	{
		Destroy();
		return(0);
	}//end else...

	/// --------------- Configure ref overlays --------------------------------
	/// Level 0: Overlay the whole reference. The reference will have an extended 
	/// boundary for motion estimation and must therefore have its own mem.
	_pRefOver = new OverlayMem2Dv2((void *)_pRef, _imgWidth, _imgHeight, _imgWidth, _imgHeight);
	if(_pRefOver == NULL)
  {
		Destroy();
	  return(0);
  }//end if !_pRefOver...

	/// Create the new extended boundary ref into _pExtRef. The boundary is extended by
	/// the max dimension of the macroblock.
	_extBoundary = _macroBlkWidth + MEH264IMC_PADDING;
	if(_macroBlkHeight > _macroBlkWidth)
		_extBoundary = _macroBlkHeight + MEH264IMC_PADDING;
	if(!OverlayExtMem2Dv2::ExtendBoundary((void *)_pRef, 
																				_imgWidth,						
																				_imgHeight, 
																				_extBoundary,	///< Extend left and right by...
																				_extBoundary,	///< Extend top and bottom by...
																				(void **)(&_pExtRef)) )	///< Created in the method and returned.
  {
		Destroy();
	  return(0);
  }//end if !ExtendBoundary...
	_extWidth	 = _imgWidth + (2 * _extBoundary);
	_extHeight = _imgHeight + (2 * _extBoundary);

	/// Level 0: Place an overlay on the extended boundary ref with block 
	/// size set to the motion vec dim..
	_pExtRefOver = new OverlayExtMem2Dv2(	_pExtRef,				///< Src description. 
																				_extWidth, 
																				_extHeight,
																				_macroBlkWidth,	///< Block size description.
																				_macroBlkHeight,
																				_extBoundary,		///< Boundary size for both left and right.
																				_extBoundary  );
	if(_pExtRefOver == NULL)
  {
		Destroy();
	  return(0);
  }//end if !_pExtRefOver...

	/// Level 1: Ref mem at (_l1Width * _l1Height).
	_pRefL1 = new short[_l1Width * _l1Height];

	/// Level 1: Ref overlay with whole level 1 block dim.
	if(_pRefL1 != NULL)
	{
		_pRefL1Over = new OverlayMem2Dv2(_pRefL1, _l1Width, _l1Height, _l1Width, _l1Height);
		if(_pRefL1Over == NULL)
		{
			Destroy();
			return(0);
		}//end if _pRefL1Over...
	}//end if _pRefL1...
	else
	{
		Destroy();
		return(0);
	}//end else...

	/// Level 1: Extended ref mem _pExtRefL1 created by ExtendBoundary() call.
	_extL1Boundary = _l1MacroBlkWidth + MEH264IMC_L1_PADDING;
	if(_l1MacroBlkHeight > _l1MacroBlkWidth)
		_extL1Boundary = _l1MacroBlkHeight + MEH264IMC_L1_PADDING;
	if(!OverlayExtMem2Dv2::ExtendBoundary((void *)_pRefL1, 
																				_l1Width,						
																				_l1Height, 
																				_extL1Boundary,
																				_extL1Boundary,
																				(void **)(&_pExtRefL1)) )
  {
    Destroy();
	  return(0);
  }//end if !ExtendBoundary...
	_extL1Width		= _l1Width + (2 * _extL1Boundary);
	_extL1Height	= _l1Height + (2 * _extL1Boundary);

	// Level 1: Ref overlay with level 1 motion block dim.
	_pExtRefL1Over = new OverlayExtMem2Dv2(	_pExtRefL1, 
																					_extL1Width,				///< new mem width.
																					_extL1Height,				///< new mem height.
																					_l1MacroBlkWidth,		///< block width.
																					_l1MacroBlkHeight,	///< block height.
																					_extL1Boundary,			///< boundary width.
																					_extL1Boundary);		///< boundary height.
	if(_pExtRefL1Over == NULL)
  {
    Destroy();
	  return(0);
  }//end if !_pExtRefL1Over...

	/// Level 2: Ref mem at (_l2Width * _l2Height).
	_pRefL2 = new short[_l2Width * _l2Height];

	/// Level 2: Ref overlay with whole level 2 block dim.
	if(_pRefL2 != NULL)
	{
		_pRefL2Over = new OverlayMem2Dv2(_pRefL2, _l2Width, _l2Height, _l2Width, _l2Height);
		if(_pRefL2Over == NULL)
		{
			Destroy();
			return(0);
		}//end if _pRefL2Over...
	}//end if _pRefL2...
	else
	{
		Destroy();
		return(0);
	}//end else...

	/// Level 2: Extended ref mem _pExtRefL2 created by ExtendBoundary() call.
	_extL2Boundary = _l2MacroBlkWidth + MEH264IMC_L2_PADDING;
	if(_l2MacroBlkHeight > _l2MacroBlkWidth)
		_extL2Boundary = _l2MacroBlkHeight + MEH264IMC_L2_PADDING;
	if(!OverlayExtMem2Dv2::ExtendBoundary((void *)_pRefL2, 
																				_l2Width,						
																				_l2Height, 
																				_extL2Boundary,
																				_extL2Boundary,
																				(void **)(&_pExtRefL2)) )
  {
    Destroy();
	  return(0);
  }//end if !ExtendBoundary...
	_extL2Width		= _l2Width + (2 * _extL2Boundary);
	_extL2Height	= _l2Height + (2 * _extL2Boundary);

	/// Level 2: Ref overlay with level 2 motion block dim.
	_pExtRefL2Over = new OverlayExtMem2Dv2(	_pExtRefL2, 
																					_extL2Width,				///< new mem width.
																					_extL2Height,				///< new mem height.
																					_l2MacroBlkWidth,		///< block width.
																					_l2MacroBlkHeight,	///< block height.
																					_extL2Boundary,			///< boundary width.
																					_extL2Boundary);		///< boundary height.
	if(_pExtRefL2Over == NULL)
  {
    Destroy();
	  return(0);
  }//end if !_pExtRefL2Over...

	/// --------------- Configure temp overlays --------------------------------
	/// Alloc some temp mem and overlay it to use for half pel motion estimation and 
	/// compensation. The block size is the same as the mem size.
	_pMBlk = new short[_macroBlkWidth * _macroBlkHeight];
	_pMBlkOver = new OverlayMem2Dv2(_pMBlk, _macroBlkWidth, _macroBlkHeight, 
																					_macroBlkWidth, _macroBlkHeight);
	if( (_pMBlk == NULL)||(_pMBlkOver == NULL) )
  {
		Destroy();
	  return(0);
  }//end if !_pMBlk...

	/// --------------- Configure result ---------------------------------------
	/// The structure container for the motion vectors.
	_pMotionVectorStruct = new VectorStructList(VectorStructList::SIMPLE2D);
	if(_pMotionVectorStruct != NULL)
	{
		/// How many motion vectors will there be at the block dim.
		int numVecs = (_imgWidth/_macroBlkWidth) * (_imgHeight/_macroBlkHeight);
		if(!_pMotionVectorStruct->SetLength(numVecs))
		{
			Destroy();
			return(0);
		}//end _pMotionVectorStruct...
	}//end if _pMotionVectorStruct...
	else
  {
		Destroy();
	  return(0);
  }//end if else...

	/// --------------- Refinement Window ---------------------------------------
	/// Prepare a 1/4 pel search window block for motion estimation refinement.
	int winWidth	= (6 + _macroBlkWidth) * 4;
	int winHeight = (6 + _macroBlkHeight) * 4;
	_pWin					= new short[winWidth * winHeight];
	_Win					= new OverlayMem2Dv2((void *)_pWin, winWidth, winHeight, winWidth, winHeight);
	if( (_pWin == NULL)||(_Win == NULL) )
  {
		Destroy();
	  return(0);
  }//end if !_pWin...

	_ready = 1;
	return(1);
}//end Create.

void	MotionEstimatorH264ImplMultiresCross::Reset(void)
{
}//end Reset.

/** Set the speed mode.
This is a multresolution algorithm and the speed is increased by estimating
at lower levels but it is less accurate. Mode = 1 implies level 1 and mode = 2
for level 2. Mode = 0 is auto mode that selects an appropriate mode depending
on the resolution of the image. The selection is set to 2 if the image has
an area larger than 200x200.

@param pRef		: Ref to estimate with.
@return				: The list of motion vectors.
*/
void	MotionEstimatorH264ImplMultiresCross::SetMode(int mode)
{
	if(mode == 0)	///< Auto mode.
	{
		int area = _imgWidth * _imgHeight;
		if(area > 40000)
			_mode = 2;
		else
			_mode = 1;
	}//end if mode...
	else
		_mode = mode;
}//end SetMode.

/** Motion estimate the source within the reference.
Do the estimation with the block sizes and image sizes defined in
the implementation. The returned type holds the vectors. This is
a multiresolution search algorithm with extended boundaries and a
absolute difference criterion. The mode setting (_mode) determines
the level of resolution.
@param pSrc		: Input image to estimate.
@param pRef		: Ref to estimate with.
@return				: The list of motion vectors.
*/
void* MotionEstimatorH264ImplMultiresCross::Estimate(long* avgDistortion)
{
  int		i,j,m,n,p,q,k,l;
	int		included = 0;
	long	totalDifference = 0;

	/// Set the motion vector struct storage structure.
	int		maxLength	= _pMotionVectorStruct->GetLength();
	int		vecPos		= 0;

	/// Level 1 motion range is adjusted to a refinement if level 2 (_mode = 2)
	/// is to be used.
	int lclL1MotionRange = _l1MotionRange;
	if(_mode == 2)
		lclL1MotionRange = MEH264IMC_L1_MOTION_VECTOR_REFINED_RANGE;

	/// Write the level 0 ref and fill its extended boundary. The centre part of
	/// _pExtRefOver is copied from _pRefOver before filling the boundary.
	_pExtRefOver->SetOrigin(0, 0);
	_pExtRefOver->SetOverlayDim(_imgWidth, _imgHeight);
	_pExtRefOver->Write(*_pRefOver);	///< _pRefOver dimensions are always set to the whole image.
	_pExtRefOver->FillBoundaryProxy();
	_pExtRefOver->SetOverlayDim(_macroBlkWidth, _macroBlkHeight);

	/// Subsample level 0 ref (_pRefOver) to produce level 1 ref (_pRefL1Over).
	OverlayMem2Dv2::Half( (void **)(_pRefOver->Get2DSrcPtr()),			///< Src 2D ptr.
												_imgWidth, 																///< Src width.
												_imgHeight, 															///< Src height.
												(void **)(_pRefL1Over->Get2DSrcPtr()) );	///< Dest 2D ptr.
	/// Write the level 1 extended ref and fill its boundary.
	_pExtRefL1Over->SetOrigin(0, 0);
	_pExtRefL1Over->SetOverlayDim(_l1Width, _l1Height);
	_pExtRefL1Over->Write(*_pRefL1Over);		///< _pRefL1Over is always set to the whole image.
	_pExtRefL1Over->FillBoundaryProxy();
	_pExtRefL1Over->SetOverlayDim(_l1MacroBlkWidth, _l1MacroBlkHeight);

	if(_mode == 2)
	{
		/// Subsample level 1 ref (_pRefL1Over) to produce level 2 ref (_pRefL2Over).
		OverlayMem2Dv2::Half( (void **)(_pRefL1Over->Get2DSrcPtr()),		///< Src 2D ptr.
													_l1Width, 																///< Src width.
													_l1Height, 																///< Src height.
													(void **)(_pRefL2Over->Get2DSrcPtr()) );	///< Dest 2D ptr.
		/// Write the level 2 extended ref and fill its boundary.
		_pExtRefL2Over->SetOrigin(0, 0);
		_pExtRefL2Over->SetOverlayDim(_l2Width, _l2Height);
		_pExtRefL2Over->Write(*_pRefL2Over);	///< _pRefL2Over is always set to the whole image.
		_pExtRefL2Over->FillBoundaryProxy();
		_pExtRefL2Over->SetOverlayDim(_l2MacroBlkWidth, _l2MacroBlkHeight);
	}//end if _mode...

	/// Subsample level 0 input (_pInOver) to produce level 1 input (_pInL1Over).
	OverlayMem2Dv2::Half( (void **)(_pInOver->Get2DSrcPtr()),			///< Src 2D ptr.
												_imgWidth, 															///< Src width.
												_imgHeight, 														///< Src height.
												(void **)(_pInL1Over->Get2DSrcPtr()) ); ///< Dest 2D ptr.

	if(_mode == 2)
	{
		/// Subsample level 1 input (_pInL1Over) to produce level 2 input (_pInL2Over).
		OverlayMem2Dv2::Half( (void **)(_pInL1Over->Get2DSrcPtr()),		///< Src 2D ptr.
													_l1Width, 															///< Src width.
													_l1Height, 															///< Src height.
													(void **)(_pInL2Over->Get2DSrcPtr()) ); ///< Dest 2D ptr.
	}//end if _mode...

	/// Gather the motion vector absolute differnce/square error data and choose the vector.
	/// m,n step level 0 vec dim = _macroBlkHeight, _macroBlkWidth.
	/// p,q step level 1 vec dim = _l1MacroBlkHeight, _l1MacroBlkWidth.
	/// k,l step level 2 vec dim = _l2MacroBlkHeight, _l2MacroBlkWidth.
  for(m = 0, p = 0, k = 0; m < _imgHeight; m += _macroBlkHeight, p += _l1MacroBlkHeight, k += _l2MacroBlkHeight)
		for(n = 0, q = 0, l = 0; n < _imgWidth; n += _macroBlkWidth, q += _l1MacroBlkWidth, l += _l2MacroBlkWidth)
  {
		int mx	= 0;	///< Full pel grid.
		int my	= 0;
		int hmx	= 0;	///< 1/2 pel on 1/4 pel grid.
		int hmy	= 0;
		int qmx	= 0;	///< 1/4 pel grid.
		int qmy	= 0;
		int rmx = 0;	///< Refinement motion vector centre.
		int rmy = 0;

		/// Depending on which img boundary we are on will limit the full search range.
		int xlRng, xrRng, yuRng, ydRng;

		/// The [0,0] motion vector difference between the input and ref blocks is the most
		/// likely candidate and is therefore the starting point.

		/// Level 0: Set the input and ref blocks to work with.
		_pInOver->SetOrigin(n,m);
		_pExtRefOver->SetOrigin(n,m);

		/// Absolute diff comparison method.
#ifdef MEH264IMC_ABS_DIFF
		int zeroVecDiff = _pInOver->Tad16x16(*_pExtRefOver);
#else
		int zeroVecDiff = _pInOver->Tsd16x16(*_pExtRefOver);
#endif
		int minDiff			= zeroVecDiff;	///< Best so far.

		///----------------------- Level 2 full pel search --------------------------------
		if(_mode == 2)
		{
			/// Level 2: Set the input and ref blocks.
			_pInL2Over->SetOrigin(l,k);
			_pExtRefL2Over->SetOrigin(l,k);

			/// Compiler directed distortion comparison method (absolute difference or square difference).
#ifdef MEH264IMC_ABS_DIFF
			int minDiffL2 = _pInL2Over->Tad4x4(*_pExtRefL2Over);
#else
			int minDiffL2 = _pInL2Over->Tsd4x4(*_pExtRefL2Over);
#endif
			
    	/// Level 2: Search on a full pel grid over the defined L2 motion range.

			/// Cross search loop.
			for(int w = _l2MotionRange/2 ;w > 0; w = w/2)	///< Start at half the motion range and then converge by 2 every iteration.
			{
				/// The winning centre is readjusted on every loop therefore the ranges must be recalculated.
				GetMotionRange(l, k, 0, 0, &xlRng, &xrRng, &yuRng, &ydRng, _l2MotionRange-1, 2);

				/// Reset the offset from (mx,my);
				rmx = 0;
				rmy = 0;
				for(int x = 0; x < MEH264IMC_MOTION_CROSS_POS_LENGTH; x++)
				{
                                        int blkDiff = 0; 
					i = w * MEH264IMC_CrossPos[x].y;
					j = w * MEH264IMC_CrossPos[x].x;

					/// Check that this offset is within the range of the image boundaries. If not then skip.
					if( ((i+my) < yuRng)||((i+my) > ydRng)||((j+mx) < xlRng)||((j+mx) > xrRng) )
						goto MEH264IMC_LEVEL2_BREAK;

					/// Set the block to the (j,i) offset motion vector from the (mx,my) motion vector
					/// around the (l,k) reference location.
					_pExtRefL2Over->SetOrigin(j+l+mx, i+k+my);

#ifdef MEH263IMC_ABS_DIFF
					blkDiff = _pInL2Over->Tad4x4LessThan(*_pExtRefL2Over, minDiffL2);
#else
//					blkDiff = _pInL2Over->Tsd4x4PartialLessThan(*_pExtRefL2Over, minDiffL2);
					blkDiff = _pInL2Over->Tsd4x4LessThan(*_pExtRefL2Over, minDiffL2);
#endif

					if(blkDiff <= minDiffL2)
					{
						/// Weight the equal diff with the smallest motion vector magnitude. 
						if(blkDiff == minDiffL2)
						{
							int vecDistDiff = ( (my*my)+(mx*mx) )-( (i*i)+(j*j) );
							if(vecDistDiff < 0)
								goto MEH264IMC_LEVEL2_BREAK;
						}//end if blkDiff...
			
						minDiffL2 = blkDiff;
						rmx = j;
						rmy = i;
					}//end if blkDiff...
			
					MEH264IMC_LEVEL2_BREAK: ; ///< null.

				}//end for x...

				/// Update new centre of winning vector.
				mx += rmx;
				my += rmy;
			}//end for w...
			
			mx = mx << 1; ///< Convert level 2 full pel units to level 1 full pel units ( x2 ).
			my = my << 1;
		}//end if _mode...

		///----------------------- Level 1 full pel search --------------------------------
		/// Level 1: Set the input and ref blocks.
		_pInL1Over->SetOrigin(q,p);
		_pExtRefL1Over->SetOrigin(mx+q,my+p);

		/// Absolute/square diff comparison method.
#ifdef MEH264IMC_ABS_DIFF
		int minDiffL1 = _pInL1Over->Tad8x8(*_pExtRefL1Over);
#else
		int minDiffL1 = _pInL1Over->Tsd8x8(*_pExtRefL1Over);
#endif

    /// Level 1: Search on a cross pel grid over the defined L1 motion range if mode != 2. Otherwise
		///					 the range is a refinement if Level 2 was done (_mode = 2).
		rmx = 0;	///< Refinement motion vector centre.
		rmy = 0;

		/// For mode = 2 (level 2 search has been done) do a refinement search.
		if(_mode == 2)
		{
			GetMotionRange(q, p, mx, my, &xlRng, &xrRng, &yuRng, &ydRng, lclL1MotionRange, 1);

			for(i = yuRng; i <= ydRng; i++)
			{
				for(j = xlRng; j <= xrRng; j++)
				{
                                        int blkDiff = 0; 
					/// Early exit because zero (centre) motion vec already checked.
					if( !(i||j) )	goto MEH264IMC_LEVEL1_BREAK;

					/// Set the block to the [j,i] motion vector around the [mx+p,my+q] reference location.
					_pExtRefL1Over->SetOrigin(j+mx+q, i+my+p);

#ifdef MEH264IMC_ABS_DIFF
					blkDiff = _pInL1Over->Tad8x8LessThan(*_pExtRefL1Over, minDiffL1);
#else
					blkDiff = _pInL1Over->Tsd8x8PartialLessThan(*_pExtRefL1Over, minDiffL1);
//					blkDiff = _pInL1Over->Tsd8x8LessThan(*_pExtRefL1Over, minDiffL1);
#endif
					if(blkDiff <= minDiffL1)
					{
						/// Weight the equal diff with the smallest motion vector magnitude. 
						if(blkDiff == minDiffL1)
						{
							int vecDistDiff = ( ((my+rmy)*(my+rmy))+((mx+rmx)*(mx+rmx)) )-( ((my+i)*(my+i))+((mx+j)*(mx+j)) );
							if(vecDistDiff < 0)
								goto MEH264IMC_LEVEL1_BREAK;
						}//end if blkDiff...

						minDiffL1 = blkDiff;
						rmx = j;
						rmy = i;
					}//end if blkDiff...

					MEH264IMC_LEVEL1_BREAK: ; ///< null.
				}//end for j...
			}//end for i...
		}//end if mode == 2...
		else	///< Do a cross search.
		{
			/// For this case, this is the first entry point of the search i.e. (mx,my) = (0,0).

			for(int w = _l1MotionRange/2 ;w > 0; w = w/2)	///< Start at half the motion range and then converge by 2 every iteration.
			{
				/// The winning centre is readjusted on every loop therefore the ranges must be recalculated.
				GetMotionRange(q, p, mx, my, &xlRng, &xrRng, &yuRng, &ydRng, lclL1MotionRange-1, 1);

				/// Reset the offset from (mx,my);
				rmx = 0;
				rmy = 0;
				for(int x = 0; x < MEH264IMC_MOTION_CROSS_POS_LENGTH; x++)
				{
                                        int blkDiff = 0; 
					i = w * MEH264IMC_CrossPos[x].y;
					j = w * MEH264IMC_CrossPos[x].x;

					/// Check that this offset is within the range of the image boundaries. If not then skip.
					if( ((i+my) < yuRng)||((i+my) > ydRng)||((j+mx) < xlRng)||((j+mx) > xrRng) )
						goto MEH264IMC_LEVEL1_BREAK_C;

					/// Set the block to the (j,i) motion vector around the (mx+p,my+q) reference location.
					_pExtRefL1Over->SetOrigin(j+mx+q, i+my+p);

#ifdef MEH263IMC_ABS_DIFF
					blkDiff = _pInL1Over->Tad8x8LessThan(*_pExtRefL1Over, minDiffL1);
#else
					blkDiff = _pInL1Over->Tsd8x8PartialLessThan(*_pExtRefL1Over, minDiffL1);
//					int blkDiff = _pInL1Over->Tsd8x8LessThan(*_pExtRefL1Over, minDiffL1);
#endif

					if(blkDiff <= minDiffL1)
					{
						/// Weight the equal diff with the smallest motion vector magnitude. 
						if(blkDiff == minDiffL1)
						{
							int vecDistDiff = ( ((my+rmy)*(my+rmy))+((mx+rmx)*(mx+rmx)) )-( ((my+i)*(my+i))+((mx+j)*(mx+j)) );
							if(vecDistDiff < 0)
								goto MEH264IMC_LEVEL1_BREAK_C;
						}//end if blkDiff...

						minDiffL1 = blkDiff;
						rmx = j;
						rmy = i;
					}//end if blkDiff...

					MEH264IMC_LEVEL1_BREAK_C: ; ///< null.

				}//end for x...

				/// Update new centre of winning vector.
				mx += rmx;
				my += rmy;
			}//end for w...

		}//end else...

		mx = (mx+rmx) * 2; ///< Convert level 1 full pel units to level 0 full pel units ( x2 ) after adding the L1 refinement.
		my = (my+rmy) * 2;

		///----------------------- Level 0 full pel refined search ------------------------
    /// Level 0: Search on a full pel grid over the defined L0 refined motion range.

		/// Get the min diff at this location in level 0 grid units.
		_pExtRefOver->SetOrigin(n+mx,m+my);

#ifdef MEH264IMC_ABS_DIFF
		minDiff = _pInOver->Tad16x16(*_pExtRefOver);
#else
		minDiff = _pInOver->Tsd16x16(*_pExtRefOver);
#endif

		/// Look for an improvement on the motion vector calc above within the refined range.
		rmx = 0;	///< Refinement motion vector centre.
		rmy = 0;
		GetMotionRange(n, m, mx, my, &xlRng, &xrRng, &yuRng, &ydRng, MEH264IMC_L0_MOTION_VECTOR_REFINED_RANGE, 0);

    for(i = yuRng; i <= ydRng; i++)
    {
			for(j = xlRng; j <= xrRng; j++)
			{
                                int blkDiff = 0;
				/// Early exit because zero motion vec already checked.
				if( !(i||j) )	goto MEH264IMC_LEVEL0_BREAK;

				/// Set the block to the [j,i] motion vector around the [n+mx,m+my] reference location.
				_pExtRefOver->SetOrigin(n+mx+j, m+my+i);

#ifdef MEH264IMC_ABS_DIFF
				blkDiff = _pInOver->Tad16x16LessThan(*_pExtRefOver, minDiff);
#else
//				blkDiff = _pInOver->Tsd16x16LessThan(*_pExtRefOver, minDiff);
				blkDiff = _pInOver->Tsd16x16PartialLessThan(*_pExtRefOver, minDiff);
#endif
				if(blkDiff <= minDiff)
				{
					/// Weight the equal diff with the smallest global motion vector magnitude. 
					if(blkDiff == minDiff)
					{
						int vecDistDiff = ( ((my+rmy)*(my+rmy))+((mx+rmx)*(mx+rmx)) )-( ((my+i)*(my+i))+((mx+j)*(mx+j)) );
						if(vecDistDiff < 0)
							goto MEH264IMC_LEVEL0_BREAK;
					}//end if blkDiff...

					minDiff = blkDiff;
					rmx = j;
					rmy = i;
				}//end if blkDiff...

				MEH264IMC_LEVEL0_BREAK: ; ///< null.
			}//end for j...
    }//end for i...

		/// Add the refinement in full pixel units.
		mx += rmx;
		my += rmy;

		///----------------------- Level 0 quarter pel refined search ------------------------
    /// Search around the min diff full pel motion vector on a 1/4 pel grid firstly on the
		/// 1/2 pel positions and then refine the winner on the 1/4 pel positions. 
		/// refine to the .

		int mvx = mx << 2;	///< Convert to 1/4 pel units.
		int mvy = my << 2;

		/// Set the location to the min diff motion vector [mx,my].
		_pExtRefOver->SetOrigin(n+mx, m+my);

		/// Fill the 1/4 pel window with valid values only in the 1/2 pel positions.
		LoadHalfQuartPelWindow(_Win, _pExtRefOver); 

    for(int x = 0; x < MEH264IMC_MOTION_SUB_POS_LENGTH; x++)
    {
			int qOffX = 2 * MEH264IMC_SubPos[x].x;
			int qOffY = 2 * MEH264IMC_SubPos[x].y;

			/// Read the half grid pels into temp.
			QuarterRead(_pMBlkOver, _Win, qOffX, qOffY);

#ifdef MEH264IMC_ABS_DIFF
			int blkDiff = _pInOver->Tad16x16LessThan(*_pMBlkOver, minDiff);
#else
//			int blkDiff = _pInOver->Tsd16x16LessThan(*_pMBlkOver, minDiff);
		int blkDiff = _pInOver->Tsd16x16PartialLessThan(*_pMBlkOver, minDiff);
#endif
			if(blkDiff < minDiff)
			{
				minDiff = blkDiff;
				hmx = qOffX;
				hmy = qOffY;
			}//end if blkDiff...
    }//end for x...

		qmx = hmx;
		qmy = hmy;

		/// Fill the 1/4 pel positions around the winning 1/2 pel position (hmx,hmy).
		LoadQuartPelWindow(_Win, hmx, hmy); 

    for(int x = 0; x < MEH264IMC_MOTION_SUB_POS_LENGTH; x++)
    {
			int qOffX = hmx + MEH264IMC_SubPos[x].x;
			int qOffY = hmy + MEH264IMC_SubPos[x].y;

			/// Read the quarter grid pels into temp.
			QuarterRead(_pMBlkOver, _Win, qOffX, qOffY);

#ifdef MEH264IMC_ABS_DIFF
			int blkDiff = _pInOver->Tad16x16LessThan(*_pMBlkOver, minDiff);
#else
//			int blkDiff = _pInOver->Tsd16x16LessThan(*_pMBlkOver, minDiff);
			int blkDiff = _pInOver->Tsd16x16PartialLessThan(*_pMBlkOver, minDiff);
#endif
			if(blkDiff < minDiff)
			{
				minDiff = blkDiff;
				qmx = qOffX;
				qmy = qOffY;
			}//end if blkDiff...
    }//end for x...

		/// Add the refinement in 1/4 pel units.
		mvx += qmx;
		mvy += qmy;

		/// Bounds check used for debugging only as the minDiff will not be correct.
		//if(mvx < -_motionRange) 
		//	mvx = -_motionRange;
		//else if(mvx >= _motionRange)
		//	mvx = (_motionRange-1);
		//if(mvy < -_motionRange) 
		//	mvy = -_motionRange;
		//else if(mvy >= _motionRange)
		//	mvy = (_motionRange-1);

		/// Validity of the motion vector is weighted with non-linear factors.
		int weight							= 0;
		int diffWithZeroDiff	= zeroVecDiff - minDiff;
		int magSqr							= (mvx * mvx) + (mvy * mvy);

#ifdef MEH264IMC_ABS_DIFF
		/// Contribute if motion vector is small.
		if((diffWithZeroDiff * 2) < magSqr)
			weight++;
		/// Contribute if same order as the noise.
		if(zeroVecDiff < MEH264IMC_MOTION_NOISE_FLOOR)
			weight++;
		/// Contribute if the zero vector and min diff vector are similar.
		if((diffWithZeroDiff * 7) < minDiff)
			weight++;
#else
		/// Contribute if motion vector is small.
		if(diffWithZeroDiff < magSqr)
			weight++;
		/// Contribute if same order as the noise.
		if(zeroVecDiff < MEH264IMC_FULL_MOTION_NOISE_FLOOR)
			weight++;
		/// Contribute if the zero vector and min energy vector are similar.
		if((diffWithZeroDiff * 10) < minDiff)
			weight++;
#endif

		/// Check for inclusion in the distortion calculation.
		bool doIt = true;
		if(_pDistortionIncluded != NULL)
			doIt = _pDistortionIncluded[vecPos];
		if(doIt)
			included++;

		/// Decide whether or not to accept the final motion vector or revert to the zero vector.
		if((minDiff < zeroVecDiff)&&(weight < 2))
    {
			if(doIt)
				totalDifference += minDiff;
    }//end if min_energy...
    else
		{
			mvx = 0;
			mvy = 0;
			if(doIt)
				totalDifference += zeroVecDiff;
		}//end else...

		/// Load the selected vector coord.
		if(vecPos < maxLength)
		{
			_pMotionVectorStruct->SetSimpleElement(vecPos, 0, mvx);
			_pMotionVectorStruct->SetSimpleElement(vecPos, 1, mvy);
			vecPos++;
		}//end if vecPos...

  }//end for m & n...

	/// In this context avg distortion is actually avg difference.
//	*avgDistortion = totalDifference/maxLength;
	if(included)	///< Prevent divide by zero error.
		*avgDistortion = totalDifference/included;
	else
		*avgDistortion = 0;
	return((void *)_pMotionVectorStruct);

}//end Estimate.

/*
--------------------------------------------------------------------------
  Private methods. 
--------------------------------------------------------------------------
*/

void MotionEstimatorH264ImplMultiresCross::Destroy(void)
{
	_ready = 0;

	if(_Win != NULL)
		delete _Win;
	_Win = NULL;
	if(_pWin != NULL)
		delete[] _pWin;
	_pWin = NULL;

	if(_pInOver != NULL)
		delete _pInOver;
	_pInOver = NULL;

	if(_pInL1 != NULL)
		delete[] _pInL1;
	_pInL1 = NULL;

	if(_pInL1Over != NULL)
		delete _pInL1Over;
	_pInL1Over = NULL;

	if(_pInL2 != NULL)
		delete[] _pInL2;
	_pInL2 = NULL;

	if(_pInL2Over != NULL)
		delete _pInL2Over;
	_pInL2Over = NULL;

	if(_pRefOver != NULL)
		delete _pRefOver;
	_pRefOver	= NULL;

	if(_pExtRef != NULL)
		delete[] _pExtRef;
	_pExtRef = NULL;

	if(_pExtRefOver != NULL)
		delete _pExtRefOver;
	_pExtRefOver = NULL;

	if(_pRefL1 != NULL)
		delete[] _pRefL1;
	_pRefL1	= NULL;

	if(_pRefL1Over != NULL)
		delete _pRefL1Over;
	_pRefL1Over	= NULL;

	if(_pExtRefL1 != NULL)
		delete[] _pExtRefL1;
	_pExtRefL1 = NULL;

	if(_pExtRefL1Over != NULL)
		delete _pExtRefL1Over;
	_pExtRefL1Over = NULL;

	if(_pRefL2 != NULL)
		delete[] _pRefL2;
	_pRefL2	= NULL;

	if(_pRefL2Over != NULL)
		delete _pRefL2Over;
	_pRefL2Over	= NULL;

	if(_pExtRefL2 != NULL)
		delete[] _pExtRefL2;
	_pExtRefL2 = NULL;

	if(_pExtRefL2Over != NULL)
		delete _pExtRefL2Over;
	_pExtRefL2Over = NULL;

	if(_pMBlk != NULL)
		delete[] _pMBlk;
	_pMBlk = NULL;

	if(_pMBlkOver != NULL)
		delete _pMBlkOver;
	_pMBlkOver = NULL;

	if(_pMotionVectorStruct != NULL)
		delete _pMotionVectorStruct;
	_pMotionVectorStruct = NULL;

}//end Destroy.

/** Get the allowed motion range for this block.
The search area for unrestricted H.264 is within the bounds of the extended image
dimensions. The range is limited at the corners and edges of the extended
images. The returned values are offset limits from (x,y) image coordinate.
@param x				: X coord of block.
@param y				: Y coord of block.
@param xlr			: Returned allowed left range offset from x.
@param xrr			: Returned allowed right range offset from x.
@param yur			: Returned allowed up range offset from y.
@param ydr			: Returned allowed down range offset from y.
@param range		: Desired range of motion.
@param level		: Resolution level to operate.
@return					: none.
*/
void MotionEstimatorH264ImplMultiresCross::GetMotionRange(int  x,			int  y, 
																													int* xlr,		int* xrr, 
																													int* yur,		int* ydr,
																													int	 range,	int	 level)
{
	/// The level defines the members to use.
	int boundary, width, height;
	switch(level)
	{
		case 1 :
			boundary	= _extL1Boundary - MEH264IMC_L1_PADDING;
			width			= _l1Width;
			height		= _l1Height;
			break;
		case 2 :
			boundary	= _extL2Boundary - MEH264IMC_L2_PADDING;
			width			= _l2Width;
			height		= _l2Height;
			break;
		case 0 :
		default :
			boundary	= _extBoundary - MEH264IMC_PADDING;
			width			= _imgWidth;
			height		= _imgHeight;
			break;
	}//end switch level...

	if( (x - range) >= -boundary )	///< Ok and within left extended boundary.
		*xlr = -range;
	else ///< Bring it into the extended boundary edge.
		*xlr = -(x + boundary);
	if( (x + range) < width )	///< Rest of block extends into the bounday region.
		*xrr = range;
	else
		*xrr = width - x;

	if( (y - range) >= -boundary )	///< Ok and within upper extended boundary.
		*yur = -range;
	else ///< Bring it into the extended boundary edge.
		*yur = -(y + boundary);
	if( (y + range) < height )	///< Rest of block extends into the bounday region.
		*ydr = range;
	else
		*ydr = height - y;

}//end GetMotionRange.

/** Get the allowed motion range for this block.
The search area for unrestricted H.264 is within the bounds of the extended image
dimensions. The range is limited at the corners and edges of the extended
images. The range is further checked to ensure that the motion vector is within 
its defined max range. The returned values are offset limits from the (xpos+xoff,ypos+yoff) 
image coordinates.
@param xpos			: X coord of block in the image.
@param ypos			: Y coord of block in the image.
@param xoff			: Current offset (vector) from xpos.
@param yoff			: Current offset (vector) from ypos.
@param xlr			: Returned allowed left range offset from xpos+xoff.
@param xrr			: Returned allowed right range offset from xpos+xoff.
@param yur			: Returned allowed up range offset from ypos+yoff.
@param ydr			: Returned allowed down range offset from ypos+yoff.
@param range		: Desired range of motion.
@param level		: Resolution level to operate.
@return					: none.
*/
void MotionEstimatorH264ImplMultiresCross::GetMotionRange(	int  xpos,	int  ypos,
																														int	 xoff,	int  yoff,
																														int* xlr,		int* xrr, 
																														int* yur,		int* ydr,
																														int	 range,	int	 level)
{
	int x = xpos + xoff;
	int y = ypos + yoff;

	int xLRange, xRRange, yURange, yDRange;

	/// The level defines the members to use.
	int boundary, width, height, vecRange;
	switch(level)
	{
		case 1 :
			boundary	= _extL1Boundary - MEH264IMC_L1_PADDING;
			width			= _l1Width;
			height		= _l1Height;
			vecRange	= _l1MotionRange;
			break;
		case 2 :
			boundary	= _extL2Boundary - MEH264IMC_L2_PADDING;
			width			= _l2Width;
			height		= _l2Height;
			vecRange	= _l2MotionRange;
			break;
		case 0 :
		default :
			boundary	= _extBoundary - MEH264IMC_PADDING;
			width			= _imgWidth;
			height		= _imgHeight;
			vecRange	= _motionRange/4;	///< Convert 1/4 pel range to full pel units.
			break;
	}//end switch level...

	/// Limit the range of the motion vector.
	if( (xoff - range) > -vecRange )
		xLRange = range;
	else
		xLRange = (vecRange-1) + xoff;
	if( (xoff + range) < vecRange )
		xRRange = range;
	else
		xRRange = (vecRange-1) - xoff;
	if( (yoff - range) > -vecRange )
		yURange = range;
	else
		yURange = (vecRange-1) + yoff;
	if( (yoff + range) < vecRange )
		yDRange = range;
	else
		yDRange = (vecRange-1) - yoff;

	if( (x - xLRange) >= -boundary )	///< Ok and within left extended boundary.
		*xlr = -xLRange;
	else ///< Bring it into the extended boundary edge.
		*xlr = -(x + boundary);
	if( (x + xRRange) < width )	///< Rest of block extends into the bounday region.
		*xrr = xRRange;
	else
		*xrr = width - x;

	if( (y - yURange) >= -boundary )	///< Ok and within upper extended boundary.
		*yur = -yURange;
	else ///< Bring it into the extended boundary edge.
		*yur = -(y + boundary);
	if( (y + yDRange) < height )	///< Rest of block extends into the bounday region.
		*ydr = yDRange;
	else
		*ydr = height - y;

}//end GetMotionRange.

/** Load a 1/4 pel window with 1/2 pel values.
The 1/4 pel window must be the macroblock size with a boundary of 3 extra pels on all sides. Only the inner
macroblock size plus 1 extra pel boundary are filled with valid values. This window is used in a cascading 
approach to motion estimation where the best 1/2 pel search around a winning full pel is done first followed 
by the best 1/4 pel around the winning 1/2 pel position. This window is used for the input for the first stage
and therefore only the 1/2 pel positions are valid. This method must be followed by the LoadQuartPelWindow()
method to complete the 1/4 pel values around a winning 1/2 pel position. The reference origin position is 
aligned onto the full pel (3,3) position of the 1/4 pel window.
@param qPelWin	: Window of size (4 * (_macroBlkHeight+6)) x (4 * (_macroBlkWidth+6))
@param extRef		: Reference to derive the 1/4 pel window from.
@return					: none.
*/
void MotionEstimatorH264ImplMultiresCross::LoadHalfQuartPelWindow(OverlayMem2Dv2* qPelWin, OverlayMem2Dv2* extRef)
{
	int fullRow, fullCol, quartRow, quartCol, refRow, refCol;

	int			width		= qPelWin->GetWidth()/4;	///< Convert to full pel units.
	int			height	= qPelWin->GetHeight()/4;
	short** window	= qPelWin->Get2DSrcPtr();

	short** ref			= extRef->Get2DSrcPtr();
	int			refXPos = extRef->GetOriginX();
	int			refYPos = extRef->GetOriginY();

	/// Set all the "h" half pel values in the window only at the positions that will be required for the other calcs. No
	/// scaling or clipping is performed until "j" half pel values are completed.
	for(fullRow = 2, quartRow = 10, refRow = refYPos - 1; fullRow < (_macroBlkHeight + 3); fullRow++, quartRow += 4, refRow++)
	{
		for(fullCol = 0, quartCol = 0, refCol = refXPos - 3; fullCol < width; fullCol++, quartCol += 4, refCol++)
		{
			int h =     (int)ref[refRow-2][refCol] -  5*(int)ref[refRow-1][refCol] + 
							 20*(int)ref[refRow][refCol]   + 20*(int)ref[refRow+1][refCol] - 
							  5*(int)ref[refRow+2][refCol] +    (int)ref[refRow+3][refCol];

			window[quartRow][quartCol] = (short)(h);
		}//end for fullCol...
	}//end fullRow...
	
	/// Set all the "b" half pel values in the window only at the positions that will be required for the other calcs. No
	/// scaling or clipping is performed until "j" half pel values are completed.
	for(fullRow = 0, quartRow = 0, refRow	= refYPos - 3; fullRow < height; fullRow++, quartRow += 4, refRow++)
	{
		for(fullCol = 2, quartCol = 10, refCol = refXPos - 1; fullCol < (_macroBlkWidth + 3); fullCol++, quartCol += 4, refCol++)
		{
			int b =     (int)ref[refRow][refCol-2] -  5*(int)ref[refRow][refCol-1] + 
							 20*(int)ref[refRow][refCol]   + 20*(int)ref[refRow][refCol+1] - 
							  5*(int)ref[refRow][refCol+2] +    (int)ref[refRow][refCol+3];

			window[quartRow][quartCol] = (short)(b);
		}//end for fullCol...
	}//end fullRow...

	/// For the "j" half pel values, use the previously calculated "h" and "b" values only in the positions
	/// surrounding the centre of the reference image origin. Now scaling is included for j.
	for(fullRow = 2, quartRow = 10; fullRow < (_macroBlkHeight + 3); fullRow++, quartRow += 4)
	{
		for(fullCol = 2, quartCol = 10; fullCol < (_macroBlkWidth + 3); fullCol++, quartCol += 4)
		{
			int j = (   (int)window[quartRow][quartCol-10] -  5*(int)window[quartRow][quartCol-6] + 
							 20*(int)window[quartRow][quartCol-2]  + 20*(int)window[quartRow][quartCol+2] - 
							  5*(int)window[quartRow][quartCol+6]  +    (int)window[quartRow][quartCol+10] + 512) >> 10;

			window[quartRow][quartCol] = (short)(MEH264IMC_CLIP255(j));
		}//end for fullCol...
	}//end fullRow...

	/// Scale and clip the useful "h" and "b" half pel values in place.
	for(fullRow = 2, refRow	= refYPos - 1, quartRow = 8; fullRow < (_macroBlkHeight + 3); fullRow++, refRow++, quartRow += 4)
	{
		for(fullCol = 2, refCol = refXPos - 1, quartCol = 8; fullCol < (_macroBlkWidth + 3); fullCol++, refCol++, quartCol += 4)
		{
			/// "h"
			window[quartRow+2][quartCol] = MEH264IMC_CLIP255((window[quartRow+2][quartCol] + 16) >> 5);
			/// "b"
			window[quartRow][quartCol+2] = MEH264IMC_CLIP255((window[quartRow][quartCol+2] + 16) >> 5);
			/// Copy full pel "G"
			window[quartRow][quartCol] = ref[refRow][refCol];
		}//end for fullCol...
	}//end fullRow...

}//end LoadHalfQuartPelWindow.

/** Load a 1/4 pel window with 1/4 pel values not in 1/2 pel positions.
The 1/4 pel window must be the macroblock size with a boundary of 3 extra pels on all sides. Only the inner
macroblock size plus 1 extra pel boundary are filled with valid values. This window is used in a cascading 
approach to motion estimation where the best 1/2 pel search around a winning full pel is done first followed 
by the best 1/4 pel around the winning 1/2 pel position. This window is used for the input for the second stage
and therefore the 1/2 pel positions are valid from a previous (first stage) call to the LoadHalfQuartPelWindow() 
method. This method will complete the 1/4 pel values around a winning 1/2 pel position. The reference origin 
position is aligned onto the full pel (3,3) position of the 1/4 pel window. The 1/4 pel values are processed
from the values already in the window.
@param qPelWin		: Window of size (4 * (_macroBlkHeight+6)) x (4 * (_macroBlkWidth+6))
@param hPelColOff	: The winning 1/2 pel X offset from the (3,3) window position in 1/4 pel units.
@param hPelRowOff	: The winning 1/2 pel Y offset from the (3,3) window position in 1/4 pel units.
@return						: none.
*/
void MotionEstimatorH264ImplMultiresCross::LoadQuartPelWindow(OverlayMem2Dv2* qPelWin, int hPelColOff, int hPelRowOff)
{
	int fullRow, fullCol, quartRow, quartCol;

	/// The 1/4 pels are to calculated for the 8 positions surrounding the 1/2 location at (hPelRowOff,hPelColOff). Note
	/// that hPelRowOff and hPelColOff are still in 1/4 pel units i.e. = range [-2,0,2][-2,0,2].

	short** window	= qPelWin->Get2DSrcPtr();

	for(fullRow = 0, quartRow = (12 + hPelRowOff); fullRow < _macroBlkHeight; fullRow++, quartRow += 4)
	{
		for(fullCol = 0, quartCol = (12 + hPelColOff); fullCol < _macroBlkWidth; fullCol++, quartCol += 4)
		{
			window[quartRow-1][quartCol-1]	= (window[quartRow-2][quartCol-2] + window[quartRow][quartCol] + 1) >> 1;
			window[quartRow-1][quartCol]		= (window[quartRow-2][quartCol]		+ window[quartRow][quartCol] + 1) >> 1;
			window[quartRow-1][quartCol+1]	= (window[quartRow-2][quartCol]		+ window[quartRow][quartCol+2] + 1) >> 1;
			window[quartRow][quartCol-1]		= (window[quartRow][quartCol-2]		+ window[quartRow][quartCol] + 1) >> 1;
			window[quartRow][quartCol+1]		= (window[quartRow][quartCol+2]		+ window[quartRow][quartCol] + 1) >> 1;
			window[quartRow+1][quartCol-1]	= (window[quartRow][quartCol-2]		+ window[quartRow+2][quartCol] + 1) >> 1;
			window[quartRow+1][quartCol]		= (window[quartRow+2][quartCol]		+ window[quartRow][quartCol] + 1) >> 1;
			window[quartRow+1][quartCol+1]	= (window[quartRow+2][quartCol]		+ window[quartRow][quartCol+2] + 1) >> 1;
		}//end for fullCol...
	}//end fullRow...

}//end LoadQuartPelWindow.

/** Read 1/4 pels from window.
The 1/4 pel window must be the macroblock size with a boundary of 3 extra pels on all sides. Only the inner
macroblock size plus 1 extra pel boundary are filled with valid values. Read a block from the window centred
on the (3,3) position with a 1/4 pel offset given by the input params into the destination block.
@param dstBlock		: Destination block.
@param qPelWin		: Window of size (4 * (_macroBlkHeight+6)) x (4 * (_macroBlkWidth+6))
@param qPelColOff	: The 1/4 pel X offset from the (3,3) window position in 1/4 pel units.
@param qPelRowOff	: The 1/4 pel Y offset from the (3,3) window position in 1/4 pel units.
@return						: none.
*/
void MotionEstimatorH264ImplMultiresCross::QuarterRead(OverlayMem2Dv2* dstBlock, OverlayMem2Dv2* qPelWin, int qPelColOff, int qPelRowOff)
{
	int fullRow, fullCol, quartRow, quartCol, dstX;

	short** window	= qPelWin->Get2DSrcPtr();	/// The origin of the window is around full pel position (3,3).

	short** dst			= dstBlock->Get2DSrcPtr();
	int			width		= dstBlock->GetWidth();
	int			height	= dstBlock->GetHeight();
	int			dstXPos = dstBlock->GetOriginX();
	int			dstYPos = dstBlock->GetOriginY();

	/// 1/4 pel rows and cols increment in quarter pel units with (3,3) full pel offset + input offset. As
	/// in: quartRow	= ((fullRow + 3) * 4) + qPelRowOff; and quartCol	= ((fullCol + 3) * 4) + qPelColOff;

	for(fullRow = 0, quartRow = (12 + qPelRowOff); fullRow < height; fullRow++, dstYPos++, quartRow += 4)
	{
		for(fullCol = 0, quartCol = (12 + qPelColOff), dstX = dstXPos; fullCol < width; fullCol++, quartCol += 4, dstX++)
			dst[dstYPos][dstX] = window[quartRow][quartCol];
	}//end fullRow...

}//end QuarterRead.




