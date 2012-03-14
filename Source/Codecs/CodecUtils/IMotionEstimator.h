/** @file

MODULE						: IMotionEstimator

TAG								: IME

FILE NAME					: IMotionEstimator.h

DESCRIPTION				: An interface to motion estimator implementations. Results
										are generated as a VectorList object.

REVISION HISTORY	: 29/09/2006 (KF) The reference to VectorList objects is
										depricated to allow any list object structure that is
										to be defined by the implementation.

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
#ifndef _IMOTIONESTIMATOR_H
#define _IMOTIONESTIMATOR_H

/*
---------------------------------------------------------------------------
	Interface definition.
---------------------------------------------------------------------------
*/
class IMotionEstimator
{
	public:
		virtual ~IMotionEstimator() {}
		
		/** Create any private memory required.
		This is primarily to encourage a two stage construction process where
		mem is not alloc in the implementation constructor.
		@return	:	1 = success, 0 = failure.
		*/
		virtual int Create(void) = 0;

		/** Reset any previous frame assumptions.
		But do not change any	mem variables or delete anything.
		@return	: none.
		*/
		virtual void Reset(void) = 0;

		/** Ready to estimate.
		Was the implementation correctly constructed and ready to estimate.
		@return	: 1 = yes, 0 = no.
		*/
		virtual int Ready(void) = 0;

		/** Set decision modes.
		These modes are defined by the implementation and are typically 
		compromises between speed and accuracy.
		@param mode	:	Mode to set. default (0 = auto).
		*/
		virtual void	SetMode(int mode) = 0;
		virtual int		GetMode(void) = 0;

		/** Motion estimate the source within the reference.
		Do the estimation with the block sizes and image sizes defined in
		the implementation. The returned type holds the vectors.
		@param pSrc		: Input image to estimate.
		@param pRef		: Ref to estimate with.
		@return				: The list of motion vectors.
		*/
		virtual void* Estimate(	const void* pSrc, 
														const void* pRef,
														long* avgDistortion) = 0;
		virtual void* Estimate(long* avgDistortion) = 0;

};//end IMotionEstimator.


#endif
