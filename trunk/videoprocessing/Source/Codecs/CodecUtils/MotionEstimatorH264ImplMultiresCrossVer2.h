/** @file

MODULE				: MotionEstimatorH264ImplMultiresCrossVer2

TAG						: MEH264IMCV2

FILE NAME			: MotionEstimatorH264ImplMultiresCrossVer2.h

DESCRIPTION		: A fast unrestricted motion estimator implementation for 
								Recommendation H.264 (03/2005) with both absolute error 
								difference and square error measure. Access is via an 
								IMotionEstimator interface. There are 2 mode levels of 
								execution speed vs. quality. The boundary is extended to 
								accomodate the selected motion range. The estimation is
								performed on lower resolution representations and at the 
								lowest resolution a cascaded cross search is performed.

LICENSE	: GNU Lesser General Public License

Copyright (c) 2008 - 2013, CSIR
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
#ifndef _MOTIONESTIMATORH264IMPLMULTIRESCROSSVER2_H
#define _MOTIONESTIMATORH264IMPLMULTIRESCROSSVER2_H

#include "IMotionEstimator.h"
#include "IMotionVectorPredictor.h"
#include "VectorStructList.h"
#include "OverlayMem2Dv2.h"
#include "OverlayExtMem2Dv2.h"

/*
---------------------------------------------------------------------------
	Struct definition.
---------------------------------------------------------------------------
*/
typedef struct _MEH264IMCV2_COORD
{
	short int x;
	short int y;
} MEH264IMCV2_COORD;

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class MotionEstimatorH264ImplMultiresCrossVer2 : public IMotionEstimator
{
/// Construction.
public:

	MotionEstimatorH264ImplMultiresCrossVer2(	const void*             pSrc, 
																		        const void*             pRef, 
																		        int					            imgWidth, 
																		        int					            imgHeight,
																		        int					            motionRange,
                                            IMotionVectorPredictor* pMVPred);

	MotionEstimatorH264ImplMultiresCrossVer2(	const void*             pSrc, 
																		        const void*             pRef, 
																		        int					            imgWidth, 
																		        int					            imgHeight,
																		        int					            motionRange,
                                            IMotionVectorPredictor* pMVPred,
																		        void*				            pDistortionIncluded);

	virtual ~MotionEstimatorH264ImplMultiresCrossVer2(void);

/// IMotionEstimator Interface.
public:
	virtual int		Create(void);
	virtual void	Reset(void);
	virtual int		Ready(void)		{ return(_ready); }
	virtual void	SetMode(int mode);
	virtual int		GetMode(void) { return(_mode); }

	/** Motion estimate the source within the reference.
	Do the estimation with the block sizes and image sizes defined in
	the implementation. The returned type holds the vectors.
	@param pSrc		: Input image to estimate.
	@param pRef		: Ref to estimate with.
	@return				: The list of motion vectors.
	*/
	virtual void* Estimate(const void* pSrc, const void* pRef, long* avgDistortion)
		{ return(Estimate(avgDistortion)); }
	virtual void* Estimate(long* avgDistortion);

/// Local methods.
protected:

	/// Used by constructors to reset every member.
	void ResetMembers(void);
	/// Clear alloc mem.
	void Destroy(void);
	/// Get the motion search range in the reference and limit it to the picture extended boundaries.
	void GetMotionRange(int		x,			int		y, 
											int*	xlr,		int*	xrr, 
											int*	yur,		int*	ydr, 
											int		range,	int		level); 
	/// Motion search range in the reference limited to the picture extended boundaries and max vector range.
	void GetMotionRange(int		x,			int		y,
											int		offx,		int		offy,
											int*	xlr,		int*	xrr, 
											int*	yur,		int*	ydr, 
											int		range,	int		level); 

	void LoadHalfQuartPelWindow(OverlayMem2Dv2* qPelWin, OverlayMem2Dv2* extRef);
	void LoadQuartPelWindow(OverlayMem2Dv2* qPelWin, int hPelColOff, int hPelRowOff);
	void QuarterRead(OverlayMem2Dv2* dstBlock, OverlayMem2Dv2* qPelWin, int qPelColOff, int qPelRowOff);

protected:

	int _ready;	///< Ready to estimate.
	int _mode;	///< Speed mode or whatever. [ 0 = auto, 1 = level 1, 2 = level 2.]

	/// Parameters must remain const for the life time of this instantiation.
	int	_imgWidth;				///< Width of the src and ref images. 
	int	_imgHeight;				///< Height of the src and ref images.
	int	_macroBlkWidth;		///< Width of the motion block.
	int	_macroBlkHeight;	///< Height of the motion block.
	int	_motionRange;			///< (4x,4y) range of the motion vectors in 1/4 pel units.

	const void*	_pInput;	///< References to the images at construction.
	const void* _pRef;

	/// Level 0: Input mem overlay members.
	OverlayMem2Dv2*		_pInOver;					///< Input overlay with motion block dim.
	/// Level 1: Subsampled input by 2. [_mode == 1]
	short*						_pInL1;						///< Input mem at (_l1Width * _l1Height).
	int								_l1Width;
	int								_l1Height;
	int								_l1MacroBlkWidth;
	int								_l1MacroBlkHeight;
	int								_l1MotionRange;		///< Full pel level 1 units.
	OverlayMem2Dv2*		_pInL1Over;				///< Input overlay with level 1 motion block dim.
	/// Level 2: Subsampled input by 4.	[_mode == 2]
	short*						_pInL2;						///< Input mem at (_l2Width * _l2Height).
	int								_l2Width;
	int								_l2Height;
	int								_l2MacroBlkWidth;
	int								_l2MacroBlkHeight;
	int								_l2MotionRange;		///< Full pel level 2 units.
	OverlayMem2Dv2*		_pInL2Over;				///< Input overlay with level 2 motion block dim.

	/// Level 0: Ref mem overlay members.
	OverlayMem2Dv2*			_pRefOver;				///< Ref overlay with whole block dim.
	short*							_pExtRef;					///< Extended ref mem created by ExtendBoundary() call.
	int									_extWidth;
	int									_extHeight;
	int									_extBoundary;			///< Extended boundary for left, right, up and down.
	OverlayExtMem2Dv2*	_pExtRefOver;			///< Extended ref overlay with motion block dim.
	/// Level 1: Subsampled ref by 2.		[_mode == 1]
	short*							_pRefL1;					///< Ref mem at (_l1Width * _l1Height).
	OverlayMem2Dv2*			_pRefL1Over; 			///< Ref overlay with whole level 1 block dim.
	short*							_pExtRefL1;				///< Extended ref mem create by ExtendBoundary() call.
	int									_extL1Width;
	int									_extL1Height;
	int									_extL1Boundary;		///< Extended boundary for left, right, up and down.
	OverlayExtMem2Dv2*	_pExtRefL1Over;		///< Ref overlay with level 1 motion block dim.
	/// Level 2: Subsampled ref by 4.		[_mode == 2]
	short*							_pRefL2;					///< Ref mem at (_l2Width * _l2Height).
	OverlayMem2Dv2*			_pRefL2Over; 			///< Ref overlay with whole level 2 block dim.
	short*							_pExtRefL2;				///< Extended ref mem create by ExtendBoundary() call.
	int									_extL2Width;
	int									_extL2Height;
	int									_extL2Boundary;		///< Extended boundary for left, right, up and down.
	OverlayExtMem2Dv2*	_pExtRefL2Over;		///< Ref overlay with level 2 motion block dim.
	/// A 1/4 pel refinement window.
	short*							_pWin;
	OverlayMem2Dv2*			_Win;

	/// Temp working block and its overlay.
	short*							_pMBlk;						///< Motion block temp mem.
	OverlayMem2Dv2*			_pMBlkOver;				///< Motion block overlay of temp mem.

	/// Hold the resulting motion vectors in a byte array.
	VectorStructList*	_pMotionVectorStruct;
  /// Attached motion vector predictor on construction.
  IMotionVectorPredictor* _pMVPred;

	/// A flag per macroblock to include it in the distortion accumulation.
	bool*							_pDistortionIncluded;
};//end MotionEstimatorH264ImplMultiresCrossVer2.


#endif // !_MOTIONESTIMATORH264IMPLMULTIRESCROSSVER2_H

