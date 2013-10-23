/** @file

MODULE				: H264MotionVectorPredictorImpl1

TAG						: H264MVPI1

FILE NAME			: H264MotionVectorPredictorImpl1.h

DESCRIPTION		: A class to predicting motion vectors from the surrounding motion
                vectors of previously encoded/decoded macroblocks. Implements the 
                IMotionVectorPredictor() interface.

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
#ifndef _H264MOTIONVECTORPREDICTORIMPL1_H
#define _H264MOTIONVECTORPREDICTORIMPL1_H

#include "IMotionVectorPredictor.h"
#include "MacroBlockH264.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class H264MotionVectorPredictorImpl1 : public IMotionVectorPredictor
{
/// Construction.
public:

  H264MotionVectorPredictorImpl1(MacroBlockH264* pMb) { _pMb = pMb; }

  virtual ~H264MotionVectorPredictorImpl1(void) {}

/// IMotionVectorPredictor Interface.
public:
  virtual int	Create(void) { return(1); }

	/** Get the 16x16 2-D prediction for the macroblock/block.
  A NULL pList implies that the mb motion vectors are already valid otherwise
  load them from the pList first. The neighbourhood must also be correct from
  prior calls.
	@param pList	: Input list of motion vectors.
	@param blk		: Macroblock/block number to get the prediction for.
  @param predX  : Output predicted X coordinate
  @param predY  : Output predicted Y coordinate
	@return	      :	1 = success, 0 = failure.
	*/
	int Get16x16Prediction(	void* pList, int blk, int* predX, int* predY)
  {
    if(_pMb == NULL)
      return(0);

    MacroBlockH264::GetMbMotionMedianPred( &(_pMb[blk]), predX, predY);
    return(1);
  }

	/** Force a 16x16 motion vector for the macroblock/block.
  Used to set up future predictions.
	@param blk	: Macroblock/block number to set.
  @param mvX  : X coordinate
  @param mvY  : Y coordinate
	@return	    :	none.
	*/
	void Set16x16MotionVector(int blk, int mvX, int mvY)
  {
    if(_pMb == NULL)
      return;

    _pMb[blk]._mvX[MacroBlockH264::_16x16] = mvX;
    _pMb[blk]._mvY[MacroBlockH264::_16x16] = mvY;
  }

/// Local methods.
protected:

/// Local members.
protected:

	// Parameters must remain const for the life time of this instantiation.
  MacroBlockH264* _pMb; ///< List of the active image mbs.

};//end H264MotionVectorPredictorImpl1.

#endif // !_H264MOTIONVECTORPREDICTORIMPL1_H

