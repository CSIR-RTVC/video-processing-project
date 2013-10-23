/** @file

MODULE				: IMotionVectorPredictor

TAG						: IMVP

FILE NAME			: IMotionVectorPredictor.h

DESCRIPTION		: An interface to class implementations that predict the motion
                vector for the current macroblock/block from the surrounding
                previously encoded/decoded motion vectors. 

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
#ifndef _IMOTIONVECTORPREDICTOR_H
#define _IMOTIONVECTORPREDICTOR_H

/*
---------------------------------------------------------------------------
	Interface definition.
---------------------------------------------------------------------------
*/
class IMotionVectorPredictor
{
	public:
		virtual ~IMotionVectorPredictor() {}
		
		/** Create any private memory required.
		This is primarily to encourage a two stage construction process where
		mem is not alloc in the implementation constructor.
		@return	:	1 = success, 0 = failure.
		*/
		virtual int Create(void) = 0;

		/** Get the 16x16 2-D prediction for the macroblock/block.
		@param pList	: Input list of motion vectors.
		@param blk		: Macroblock/block number to get the prediction for.
    @param predX  : Output predicted X coordinate
    @param predX  : Output predicted Y coordinate
		@return	      :	1 = success, 0 = failure.
		*/
		virtual int Get16x16Prediction(	void* pList, int blk, int* predX, int* predY) = 0;

	  /** Force a 16x16 motion vector for the macroblock/block.
    Used to set up future predictions.
	  @param blk	: Macroblock/block number to set.
    @param mvX  : X coordinate
    @param mvY  : Y coordinate
	  @return	    :	none.
	  */
    virtual void Set16x16MotionVector(int blk, int mvX, int mvY) {}

};//end IMotionVectorPredictor.

#endif  /// _IMOTIONVECTORPREDICTOR_H
