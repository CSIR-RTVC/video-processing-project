/** @file

MODULE				: MotionCompensatorH264ImplStd

TAG						: MCH264IS

FILE NAME			: MotionCompensatorH264ImplStd.h

DESCRIPTION		: A fast unrestricted motion compensator implementation for 
								Recommendation H.264 (03/2005) with both absolute error 
								difference and square error measure. Access is via an 
								IMotionCompensator interface. The boundary is extended to 
								accomodate the selected motion range.

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
#ifndef _MOTIONCOMPENSATORH264IMPLSTD_H
#define _MOTIONCOMPENSATORH264IMPLSTD_H

#include "IMotionCompensator.h"
#include "VectorStructList.h"
#include "OverlayMem2Dv2.h"
#include "OverlayExtMem2Dv2.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class MotionCompensatorH264ImplStd : public IMotionCompensator
{
	// Construction.
	public:

		MotionCompensatorH264ImplStd(int range);
		virtual ~MotionCompensatorH264ImplStd(void);

	// IMotionCompensator Interface.
	public:
		
		virtual int	Create(	void* ref, 
												int imgWidth,	int imgHeight, 
												int macroBlkWidth, int macroBlkHeight);
		virtual void	Destroy(void);
		virtual void	Reset(void);

		/** Motion compensate to the reference.
		Do the compensation with the block sizes and image sizes defined in
		the implementation and set in Create().
		@param pMotionList	: The list of motion vectors.
		@return							: None.
		*/
		virtual void Compensate(void* pMotionList);

		/** Motion compensate a single vector to the reference.
		Do the compensation with the block sizes and image sizes defined in
		the implementation and set in Create(). The vector coords are in 
		quarter pel units for this implementation. NOTE: The temp image must
		hold a copy of the ref BEFORE using this method and is should be done
		in PrepareForSingleVectorMode().
		@param tlx	: Top left x coord of block.
		@param tly	: Top left y coord of block.
		@param mvx	: X coord of the motion vector.
		@param mvy	: Y coord of the motion vector.
		@return			: None.
		*/
		virtual void Compensate(int tlx, int tly, int mvx, int mvy);

		/** Prepare the ref for single motion vector compensation mode.
		Should be used to copy the ref into a temp location from which to
		do the compensation to the ref. Prevents interference and double
		compensation. Use the Invalidate() command to force compensation
    on a vector by vector basis.
		@return : none.
		*/
		virtual void PrepareForSingleVectorMode(void);
    virtual void Invalidate(void) { _invalid = 1; }

	/// Local methods.
	protected:
	void LoadHalfQuartPelWindow(OverlayMem2Dv2* qPelWin, OverlayMem2Dv2* extRef);
	void LoadQuartPelWindow(OverlayMem2Dv2* qPelWin, int hPelColOff, int hPelRowOff);
	void QuarterRead(OverlayMem2Dv2* dstBlock, OverlayMem2Dv2* qPelWin, int qPelColOff, int qPelRowOff);

	/// Local methods.
	protected:

		/// Parameters must remain const for the life time of this instantiation.
		int	_imgWidth;				///< Width of the ref images. 
		int	_imgHeight;				///< Height of the ref images.
		int _chrWidth;
		int _chrHeight;
		int	_macroBlkWidth;		///< Width of the motion block.
		int	_macroBlkHeight;	///< Height of the motion block.
		int _chrMacroBlkWidth;
		int _chrMacroBlkHeight;
		int _range;						///< Max range of the motion [-_range ... (_range-1)] in full pel units.
    int _invalid;         ///< Invalidate the compensation to force copying on the next compensated vector. Cleared after use.

		int _refSize;					///< Total size of the contiguous Lum, ChrU, ChrV.
		/// Ref image holders.
		short*		_pRefLum;		///< References to the images at creation.
		short*		_pRefChrU;
		short*		_pRefChrV;

		/// Overlays to the ref image.
		OverlayMem2Dv2*	_pRefLumOver;
		OverlayMem2Dv2*	_pRefChrUOver;
		OverlayMem2Dv2*	_pRefChrVOver;

		/// Extended temp ref mem created by ExtendBoundary() calls for each colour
		/// component and an extended boundary overlay for each.
		short*							_pExtTmpLum;
		OverlayExtMem2Dv2*	_pExtTmpLumOver;
		int									_extLumWidth;
		int									_extLumHeight;
		short*							_pExtTmpChrU;
		OverlayExtMem2Dv2*	_pExtTmpChrUOver;
		short*							_pExtTmpChrV;
		OverlayExtMem2Dv2*	_pExtTmpChrVOver;
		int									_extChrWidth;
		int									_extChrHeight;

		/// A work block.
		short*					_pMBlk;
		OverlayMem2Dv2* _pMBlkOver;
};//end MotionCompensatorH264ImplStd.


#endif	// _MOTIONCOMPENSATORH264IMPLSTD_H
