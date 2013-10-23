/** @file

MODULE				: MacroBlockH264

TAG						: MBH264

FILE NAME			: MacroBlockH264.cpp

DESCRIPTION		: A class to hold H.264 macroblock specific data. Only static 
								methods that apply to macroblocks are used because it is 
								expected that there will be one MacroBlockH264() instantiation
								per overlay position on the picture.

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
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include <string.h>
#include "MacroBlockH264.h"
#include "MeasurementTable.h"

/// It is simpler to derive neighbourhoods of macroblocks if they are left in 
/// rater scanning order right up until their encodings are written onto the
/// bit stream. Therefore a coding order mapping is defined here to reorder
/// the final write as defined by H.264 standard.
const int MacroBlockH264::mbCodingOrderY[4][4] =
	{ { 0, 0, 1, 1 }, 
		{ 0, 0, 1, 1 }, 
		{ 2, 2, 3, 3 }, 
		{ 2, 2, 3, 3 } 
	};
const int MacroBlockH264::mbCodingOrderX[4][4] =
	{ { 0, 1, 0, 1 }, 
		{ 2, 3, 2, 3 }, 
		{ 0, 1, 0, 1 }, 
		{ 2, 3, 2, 3 } 
	};

/// The chr coeffs are less harshly quantised then the lum. This table is
/// the mapping structure as defined in ITU-T Rec. H.264 (03/2005) page 167
///	Table 8-15.
const int MacroBlockH264::QPltoQPcMap[52] = 
	{	0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
	 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
	 29, 30, 31, 32, 32, 33, 34, 34, 35, 35,
	 36, 36, 37, 37, 37, 38, 38, 38, 39, 39,
	 39, 39
	};

/// Macroblock types for I_Slices and Intra_16x16 prediction mode.
const int MacroBlockH264::Intra16x16ModeTable[24][3] =
	{ /// _intra16x16PredMode,  _codedBlkPatternChr, _codedBlkPatternLum
												{ 0,										0,										0 },	/// 1
												{ 1,										0,										0 },	/// 2
												{ 2,										0,										0 },	/// 3
												{ 3,										0,										0 },	/// 4
												{ 0,										1,										0 },	/// 5
												{ 1,										1,										0 },	/// 6
												{ 2,										1,										0 },	/// 7
												{ 3,										1,										0 },	/// 8
												{ 0,										2,										0 },	/// 9
												{ 1,										2,										0 },	/// 10
												{ 2,										2,										0 },	/// 11
												{ 3,										2,										0 },	/// 12
												{ 0,										0,									 15 },	/// 13
												{ 1,										0,									 15 },	/// 14
												{ 2,										0,									 15 },	/// 15
												{ 3,										0,									 15 },	/// 16
												{ 0,										1,									 15 },	/// 17
												{ 1,										1,									 15 },	/// 18
												{ 2,										1,									 15 },	/// 19
												{ 3,										1,									 15 },	/// 20
												{ 0,										2,									 15 },	/// 21
												{ 1,										2,									 15 },	/// 22
												{ 2,										2,									 15 },	/// 23
												{ 3,										2,									 15 } 	/// 24
	};

/*
---------------------------------------------------------------------------
	Construction and destruction.
---------------------------------------------------------------------------
*/
MacroBlockH264::MacroBlockH264(void)
{
	int i,j;

	/// Relative pixel coords (top left) of this macroblock in a contiguous image mem. Each
	/// block in the macroblock describes its relative position from this offset.
	_offLumX = 0;
	_offLumY = 0;
	_offChrX = 0;
	_offChrY = 0;
	_mbIndex = 0;	///< The index of this macroblock in the image space.
	_slice	 = 0;	///< Slice number that this macroblock belongs to.

	/// References to this macroblock's neighbours that will be used for the predictions.
	/// These are related to ITU-T Rec. H.264 (03/2005) page 27 Section 6.4.6 and will
	/// have a NULL value if they are not available. E.g. Top row macroblocks will have
	/// all the _aboveMbxxx = NULL.
	_leftMb				= NULL;		///< mbAddrA.
	_aboveLeftMb	= NULL;		///< mbAddrD.
	_aboveMb			= NULL;		///< mbAddrB.
	_aboveRightMb	= NULL;		///< mbAddrC.

	/// Macroblock quantisation parameter derived from delta qp values.
	_mbQP     = 1;
  _mbEncQP  = 1;  ///< QP used to store the _mbQP value that was used for the encoding.

	/// The macroblock type coded value mb_type is constructed from combinations 
	/// of the following modes.
	_intraFlag					= 1;							///< Indicates the macroblock Intra/Inter status.
	_mbPartPredMode			= Intra_16x16;		///< Macroblock partition prediction mode.
	_mbSubPartPredMode	= 0;							///< ...sub partition mode when partition mode is 8x8.
	_intra16x16PredMode	= Intra_16x16_DC;	///< Only for Intra_16x16 macroblocks.
	_intraChrPredMode		= Intra_Chr_DC;		///< For Intra_4x4 and Intra_16x16 macroblocks = {0..3}.
	_codedBlkPatternChr	= 0;
	_codedBlkPatternLum	= 0;

	/// Distortion-Rate optimisations require storing intermediate values. The arrays
	/// are directly addressable by _mbQP. Use an include flag to help with 
	/// heuristic elimination.
	_mvDistortion	= 0;		///< Total motion compensated distortion.
	_mvRate				= 0;		///< Total motion vector bit count.
	_include			= 1;
	for(i = 0; i < 52; i++)	///< mbQP = {0...51}
	{
		_distortion[i]	= 0;	
		_rate[i]				= 0;
	}//end for i...

	/// A flag to indicate that this macroblock has nothing to code and is skipped.
	_skip = 0;

	/// The macroblock type defines the partitioning, predictionmodes and the coded
	/// block patterns.
	_mb_type						= 1;
	_sub_mb_type				= 0;
	_coded_blk_pattern	= 0;

	/// Differential quantisation parameter for lum coeffs. Chr quant parameter is
	/// derived from the lum qp.
	_mb_qp_delta = 0;

	/// Motion Vectors and their differences. From 1 to 16 vectors depending the 
	/// partitions and sub partition sizes.
	for(i = 0; i < 16; i++)
	{
		_mvX[i]  = 0;
		_mvY[i]  = 0;
		_mvdX[i] = 0;
		_mvdY[i] = 0;
	}//end for i...

	/// The DC Chr blocks are instantiated as 4x4 and must reset
	/// to 2x2.
	_cbDcBlk.SetDim(2,2);
	_crDcBlk.SetDim(2,2);
	_cbDcBlkTmp.SetDim(2,2);
	_crDcBlkTmp.SetDim(2,2);

	/// Load the block parameter structure and the block types.
	/// Block num = -1 (Lum DC block);
	_blkParam[0].pBlk								= &_lumDcBlk;
	_blkParam[0].pBlkTmp						= &_lumDcBlkTmp;
	_blkParam[0].rasterIndex				= 0;
	_blkParam[0].dcSkipFlag					= 0;
	_blkParam[0].neighbourIndicator	= 1;
	_lumDcBlk.SetColour(BlockH264::LUM);
	_lumDcBlk.SetDcFlag(1);
	_lumDcBlkTmp.SetColour(BlockH264::LUM);
	_lumDcBlkTmp.SetDcFlag(1);
	///< 16 Lum blocks without DC value. Block num = 0..15
	int blkNum = 1;
	for(i = 0; i < 4; i++)	
		for(j = 0; j < 4; j++)	
		{
			int blkY = mbCodingOrderY[i][j];
			int blkX = mbCodingOrderX[i][j];
			_blkParam[blkNum].pBlk								= &(_lumBlk[blkY][blkX]);
			_blkParam[blkNum].pBlkTmp							= &(_lumBlkTmp[blkY][blkX]);
			_blkParam[blkNum].rasterIndex					= (4*blkY) + blkX;
			_blkParam[blkNum].dcSkipFlag					= 1;	///< Initialise assuming Intra macroblock.
			_blkParam[blkNum].neighbourIndicator	= 1;
			_lumBlk[blkY][blkX].SetColour(BlockH264::LUM);
			_lumBlk[blkY][blkX].SetDcFlag(0);
			_lumBlkTmp[blkY][blkX].SetColour(BlockH264::LUM);
			_lumBlkTmp[blkY][blkX].SetDcFlag(0);
			blkNum++;
		}//end for i & j...
	/// Chr DC next. Block num = 16 & 17.
	_blkParam[blkNum].pBlk								= &_cbDcBlk;
	_blkParam[blkNum].pBlkTmp							= &_cbDcBlkTmp;
	_blkParam[blkNum].rasterIndex					= 0;
	_blkParam[blkNum].dcSkipFlag					= 0;
	_blkParam[blkNum].neighbourIndicator	= -1;
	_cbDcBlk.SetColour(BlockH264::CB);
	_cbDcBlk.SetDcFlag(1);
	_cbDcBlkTmp.SetColour(BlockH264::CB);
	_cbDcBlkTmp.SetDcFlag(1);
	blkNum++;
	_blkParam[blkNum].pBlk								= &_crDcBlk;
	_blkParam[blkNum].pBlkTmp							= &_crDcBlkTmp;
	_blkParam[blkNum].rasterIndex					= 0;
	_blkParam[blkNum].dcSkipFlag					= 0;
	_blkParam[blkNum].neighbourIndicator	= -1;
	_crDcBlk.SetColour(BlockH264::CR);
	_crDcBlk.SetDcFlag(1);
	_crDcBlkTmp.SetColour(BlockH264::CR);
	_crDcBlkTmp.SetDcFlag(1);
	blkNum++;
	///< 4 Cb Chr blocks without DC value. Block num = 18..21
	for(i = 0; i < 2; i++)	
		for(j = 0; j < 2; j++)	
		{
			_blkParam[blkNum].pBlk								= &(_cbBlk[i][j]);
			_blkParam[blkNum].pBlkTmp							= &(_cbBlkTmp[i][j]);
			_blkParam[blkNum].rasterIndex					= (2*i) + j;
			_blkParam[blkNum].dcSkipFlag					= 1;
			_blkParam[blkNum].neighbourIndicator	= 1;
			_cbBlk[i][j].SetColour(BlockH264::CB);
			_cbBlk[i][j].SetDcFlag(0);
			_cbBlkTmp[i][j].SetColour(BlockH264::CB);
			_cbBlkTmp[i][j].SetDcFlag(0);
			blkNum++;
		}//end for i & j...
	///< 4 Cr Chr blocks without DC value. Block num = 22..25
	for(i = 0; i < 2; i++)	
		for(j = 0; j < 2; j++)	
		{
			_blkParam[blkNum].pBlk								= &(_crBlk[i][j]);
			_blkParam[blkNum].pBlkTmp							= &(_crBlkTmp[i][j]);
			_blkParam[blkNum].rasterIndex					= (2*i) + j;
			_blkParam[blkNum].dcSkipFlag					= 1;
			_blkParam[blkNum].neighbourIndicator	= 1;
			_crBlk[i][j].SetColour(BlockH264::CR);
			_crBlk[i][j].SetDcFlag(0);
			_crBlkTmp[i][j].SetColour(BlockH264::CR);
			_crBlkTmp[i][j].SetDcFlag(0);
			blkNum++;
		}//end for i & j...

  /// Vector Quantisation extensions.
  _vq_flag        = 0;
  _vq_distortion  = 0;     ///< Total VQ distortion (squared error) for the mb.
	for(i = 0; i < 4; i++)	
		for(j = 0; j < 4; j++)	
      _vq_LumCode[i][j] = 0;
	for(i = 0; i < 2; i++)	
		for(j = 0; j < 2; j++)
    {
      _vq_CbCode[i][j] = 0;
      _vq_CrCode[i][j] = 0;
    }//end for i & j...

}//end constructor.

MacroBlockH264::~MacroBlockH264(void)
{
}//end destructor.

/*
---------------------------------------------------------------------------
	Interface Methods.
---------------------------------------------------------------------------
*/

/** Initialise macroblocks
Set all members to their default values and determine all neighbourhood
references, including those of the blocks. Assume 4:2:0 format. Each 
macroblock is initialised in raster scan order so that the member values 
will be valid for setting the neighbourhood references.
@param numMbRows		: Total num of macroblk rows in the image.
@param numMbCols		: Total num	of macroblk cols in the image.
@param startMbIndex	: Start index in raster scan order of the macroblks to be initialised.
@param endMbIndex		: End index in raster scan order of the macroblks to be initialised.
@param slice				: Slice to allocate macroblocks to.
@param mb						: 2-D array of macroblocks to initialise.
@return							: Success = 1, fail = 0.
*/
int MacroBlockH264::Initialise(int numMbRows, int numMbCols, int startMbIndex, int endMbIndex, int slice, MacroBlockH264** mb)
{
	int i,j,x,y,z,col,row;

	/// Macroblocks are to be processed.
	int startMbCol	= startMbIndex % numMbCols;
	int startMbRow	= startMbIndex / numMbCols;
	int endMbCol		= endMbIndex % numMbCols;
	int endMbRow		= endMbIndex / numMbCols;
	if((startMbRow >= numMbRows)||(endMbRow >= numMbRows))
		return(0);

	/// Parse the slice in raster order only.
	int index = startMbIndex;
	for(row = startMbRow; row <= endMbRow; row++)
		for(col = startMbCol; col <= endMbCol; col++, index++)
		{
			mb[row][col]._mbIndex = index;
			mb[row][col]._slice		= slice;

			/// The top-left corner of this macroblock in pel units assuming 4:2:0 chr dimensions.
			mb[row][col]._offLumY = row * 16;
			mb[row][col]._offLumX = col * 16;
			mb[row][col]._offChrY = row * 8;
			mb[row][col]._offChrX = col * 8;

			/// Find neighbouring macroblocks within the boundaries of the image and within the
			/// same slice group. Mark unavailable macroblocks by a NULL reference.
			x = col - 1;	///< Go left.
			y = row - 1;	///< Go above.
			z = col + 1;	///< Go right.
			mb[row][col]._leftMb = NULL;	///< Left.
			if(x >= 0)	
			{
				if(mb[row][x]._slice == slice)	///< Of the same slice.
					mb[row][col]._leftMb = &(mb[row][x]);
			}//end if x...
			mb[row][col]._aboveLeftMb	= NULL;	///< Above left.
			if((x >= 0)&&(y >= 0))	
			{
				if(mb[y][x]._slice == slice)
					mb[row][col]._aboveLeftMb = &(mb[y][x]);
			}//end if x & y...
			mb[row][col]._aboveMb = NULL;	///< Above.
			if(y >= 0)	
			{
				if(mb[y][col]._slice == slice)
					mb[row][col]._aboveMb = &(mb[y][col]);
			}//end if y...
			mb[row][col]._aboveRightMb	= NULL;	///< Above right.
			if((z >= 0)&&(z < numMbCols)&&(y >= 0))	
			{
				if(mb[y][z]._slice == slice)
					mb[row][col]._aboveRightMb = &(mb[y][z]);
			}//end if z & y...

			/// Set the block offsets within the macroblock with top-left corner
			/// of macroblock as (0,0) coordinate. Set the neighbourhood for
			/// each 4x4 block that is required for the context-aware entropy
			/// encoding.
			MacroBlockH264* pMb = &(mb[row][col]);
			for(i = 0; i < 4; i++)
				for(j = 0; j < 4; j++)
				{
					/// Lum.
					pMb->_lumBlk[i][j]._offX	= j*4;
					pMb->_lumBlk[i][j]._offY	= i*4;

					if(i == 0)	///< Falls into macroblock above and in row 3.
					{
						if(pMb->_aboveMb != NULL)	///< Test that there is an available macroblock above.
							pMb->_lumBlk[i][j]._blkAbove = &((pMb->_aboveMb)->_lumBlk[3][j]);	///< Col aligned.
						else
							pMb->_lumBlk[i][j]._blkAbove = NULL;	///< The block above is not availabe.
					}//end if i...
					else
						pMb->_lumBlk[i][j]._blkAbove = &(pMb->_lumBlk[i-1][j]); ///< In this macroblock.

					if(j == 0)	///< Falls into macroblock to the left and in col 3.
					{
						if(pMb->_leftMb != NULL)	///< Test if the macroblock to the left is available.
							pMb->_lumBlk[i][j]._blkLeft = &((pMb->_leftMb)->_lumBlk[i][3]);	///< Row aligned.
						else
							pMb->_lumBlk[i][j]._blkLeft = NULL;	///< The block to the left is not available.
					}//end if j...
					else
						pMb->_lumBlk[i][j]._blkLeft = &(pMb->_lumBlk[i][j-1]); ///< In this macroblock.
					/// Chr.
					if((i < 2)&&(j < 2))
					{
						pMb->_cbBlk[i][j]._offX	= j*4;
						pMb->_cbBlk[i][j]._offY	= i*4;
						pMb->_crBlk[i][j]._offX	= j*4;
						pMb->_crBlk[i][j]._offY	= i*4;
						if(i == 0)	///< Falls into macroblock above and in row 1.
						{
							if(pMb->_aboveMb != NULL)	///< Test that there is an available macroblock above.
							{
								pMb->_cbBlk[i][j]._blkAbove = &((pMb->_aboveMb)->_cbBlk[1][j]);	///< Col aligned.
								pMb->_crBlk[i][j]._blkAbove = &((pMb->_aboveMb)->_crBlk[1][j]);
							}//end if _aboveMb...
							else	///< The blocks above are not availabe.
							{
								pMb->_cbBlk[i][j]._blkAbove = NULL;	
								pMb->_crBlk[i][j]._blkAbove = NULL;
							}//end else...
						}//end if i...
						else	///< In this macroblock.
						{
							pMb->_cbBlk[i][j]._blkAbove = &(pMb->_cbBlk[i-1][j]); 
							pMb->_crBlk[i][j]._blkAbove = &(pMb->_crBlk[i-1][j]);
						}//end else...

						if(j == 0)	///< Falls into macroblock to the left and in col 1.
						{
							if(pMb->_leftMb != NULL)	///< Test that there is an available macroblock to the left.
							{
								pMb->_cbBlk[i][j]._blkLeft = &((pMb->_leftMb)->_cbBlk[i][1]);	///< Row aligned.
								pMb->_crBlk[i][j]._blkLeft = &((pMb->_leftMb)->_crBlk[i][1]);
							}//end if _leftMb...
							else	///< The blocks above are not availabe.
							{
								pMb->_cbBlk[i][j]._blkLeft = NULL;	
								pMb->_crBlk[i][j]._blkLeft = NULL;
							}//end else...
						}//end if j...
						else	///< In this macroblock.
						{
							pMb->_cbBlk[i][j]._blkLeft = &(pMb->_cbBlk[i][j-1]);
							pMb->_crBlk[i][j]._blkLeft = &(pMb->_crBlk[i][j-1]);
						}//end else...
					}//end if i & j...
				}//end for i & j...

				/// The DC blocks. There is only one DC block per macroblock therefore all
				/// pel offsets are (0,0). Strangely, for coeff counting in the context-aware
				/// process referencing the above and left blocks is required, not other macroblock 
				/// DC blocks, I think.
				pMb->_lumDcBlk._offX	= 0;
				pMb->_lumDcBlk._offY	= 0;
				pMb->_cbDcBlk._offX		= 0;
				pMb->_cbDcBlk._offY		= 0;
				pMb->_crDcBlk._offX		= 0;
				pMb->_crDcBlk._offY		= 0;
				
				if(pMb->_aboveMb != NULL)	///< Test that there is an available macroblock above.
				{
					pMb->_lumDcBlk._blkAbove	= &((pMb->_aboveMb)->_lumBlk[3][0]);	///< Bottom left block of macroblock above.
					pMb->_cbDcBlk._blkAbove		= &((pMb->_aboveMb)->_cbBlk[1][0]);
					pMb->_crDcBlk._blkAbove		= &((pMb->_aboveMb)->_crBlk[1][0]);
				}//end if _aboveMb...
				else
				{
					pMb->_lumDcBlk._blkAbove	= NULL; ///< Macroblock above not available.
					pMb->_cbDcBlk._blkAbove		= NULL;
					pMb->_cbDcBlk._blkAbove		= NULL;
				}//end else...
				if(pMb->_leftMb != NULL)	///< Test that there is an available macroblock to the left.
				{
					pMb->_lumDcBlk._blkLeft	= &((pMb->_leftMb)->_lumBlk[0][3]);	///< Top right block of macroblock to the left.
					pMb->_cbDcBlk._blkLeft	= &((pMb->_leftMb)->_cbBlk[0][1]);
					pMb->_crDcBlk._blkLeft	= &((pMb->_leftMb)->_crBlk[0][1]);
				}//end if _leftMb...
				else
				{
					pMb->_lumDcBlk._blkLeft		= NULL; ///< Macroblock to the left not available.
					pMb->_cbDcBlk._blkLeft		= NULL;
					pMb->_cbDcBlk._blkLeft		= NULL;
				}//end else...

		}//end for row & col...

	return(1);
}//end Initialise.

/** Set the coded block pattern member.
After quantisation the Lum and Chr block patterns are calculated from
examining the zero/non-zero status of each 4x4 block in their 8x8 block
groups. The _codedBlkPatternLum, _codedBlkPatternChr and _coded_blk_pattern
members are set and each 4x4 block's coded status is synchronized to this
pattern.
@param mb	: Macroblock to set.
return		: None.
*/
void MacroBlockH264::SetCodedBlockPattern(MacroBlockH264* mb)
{
	int i;

	if(mb->_intraFlag)
		mb->_blkParam[MBH264_LUM_DC].pBlk->SetCoded(1);	/// Blk = -1 Lum DC always on.
	else
		mb->_blkParam[MBH264_LUM_DC].pBlk->SetCoded(0);

	/// Iterate through each 4x4 block and set the coded pattern according to the
	/// associated 8x8 block position. One bit represents a coded/not coded flag 
	/// for each 8x8 block. For Intra_16x16 blocks, if any one of the 4x4 Lum blocks 
	/// are non-zero then every block is marked as coded.
	mb->_codedBlkPatternLum = 0;
	if(mb->_intraFlag && (mb->_mbPartPredMode == MacroBlockH264::Intra_16x16))
	{
		for(i = MBH264_LUM_0_0; i <= MBH264_LUM_3_3; i++)	///< 1..16
		{
			if(!mb->_blkParam[i].pBlk->IsZero())
				break;
		}//end for i...
		/// ...then go back and mark all 4x4 blocks as coded within an active 8x8 block.
		if(i <= MBH264_LUM_3_3)	///< At least one block is non-zero.
		{
			for(i = MBH264_LUM_0_0; i <= MBH264_LUM_3_3; i++)	///< 1..16
				mb->_blkParam[i].pBlk->SetCoded(1);
			mb->_codedBlkPatternLum = 15;
		}//end if i...
		else
		{
			for(i = MBH264_LUM_0_0; i <= MBH264_LUM_3_3; i++)	///< 1..16
				mb->_blkParam[i].pBlk->SetCoded(0);
		}//end else...
	}//end if _intraFlag...
	else
	{
		for(i = MBH264_LUM_0_0; i <= MBH264_LUM_3_3; i++)	///< 1..16
		{
			if(!mb->_blkParam[i].pBlk->IsZero())
				mb->_codedBlkPatternLum |= (1 << ((i - 1)/4));
		}//end for i...
		/// ...then go back and mark all 4x4 blocks as coded within an active 8x8 block.
		for(i = MBH264_LUM_0_0; i <= MBH264_LUM_3_3; i++)	///< 1..16
		{
			if( mb->_codedBlkPatternLum & (1 << ((i - 1)/4)) )
				mb->_blkParam[i].pBlk->SetCoded(1);
		}//end for i...
	}//end else...

	/// For Chr, if any AC block is non-zero then they are all set as coded.
	mb->_codedBlkPatternChr = 2;
	for(i = MBH264_CB_0_0; i <= MBH264_CR_1_1; i++)	///< 19...26
	{
		if(!mb->_blkParam[i].pBlk->IsZero())
			break;	///< Non-zero found.
	}//end for i...
	if(i > MBH264_CR_1_1)	///< AC blocks are all zeros.
	{
		if( mb->_cbDcBlk.IsZero() && mb->_crDcBlk.IsZero() )
			mb->_codedBlkPatternChr = 0;
		else
		{
			mb->_codedBlkPatternChr = 1;
			mb->_cbDcBlk.SetCoded(1);	///< Force both DC blocks to be marked as coded.
			mb->_crDcBlk.SetCoded(1);
		}//end else...
	}//end if i...
	else
	{
		/// At least one AC block is non-zero so set all DC and AC blocks as coded.
		for(i = MBH264_CB_DC; i <= MBH264_CR_1_1; i++)	///< 17...26
			mb->_blkParam[i].pBlk->SetCoded(1);
	}//end else...

	/// Assemble the bit pattern to be coded onto the bit stream.
	mb->_coded_blk_pattern = mb->_codedBlkPatternLum | (mb->_codedBlkPatternChr << 4);

}//end SetCodedBlockPattern.

/** Get the coded block pattern member.
Pull out and set the _codedBlkPatternLum and _codedBlkPatternChr members
from the _coded_blk_pattern member decoded from the stream. Then set each 
4x4 block's coded status synchronized to this pattern.
@param mb	: Macroblock to set.
return		: None.
*/
void MacroBlockH264::GetCodedBlockPattern(MacroBlockH264* mb)
{
	int i;

	/// Disassemble the bit pattern to determine which blocks are to be decoded.
	mb->_codedBlkPatternLum = mb->_coded_blk_pattern & 0x0000000F;
	mb->_codedBlkPatternChr = (mb->_coded_blk_pattern >> 4) & 0x00000003;

	mb->_blkParam[MBH264_LUM_DC].pBlk->SetCoded(1);	/// Blk = -1 Lum DC always on.

	/// Map 4x4 block patterns to 8x8 blocks.
	for(i = MBH264_LUM_0_0; i <= MBH264_LUM_3_3; i++)	///< 1..16
	{
		if( mb->_codedBlkPatternLum & (1 << ((i - 1)/4)) )
			mb->_blkParam[i].pBlk->SetCoded(1);
		else
			mb->_blkParam[i].pBlk->SetCoded(0);
	}//end for i...

	if(mb->_codedBlkPatternChr != 2)	///< Chr AC blks are zero.
	{
		for(i = MBH264_CB_DC; i <= MBH264_CR_1_1; i++)	///< Default all to zero including DC blocks.
			mb->_blkParam[i].pBlk->SetCoded(0);
		if(mb->_codedBlkPatternChr == 1)
		{
			mb->_cbDcBlk.SetCoded(1);	///< Force both DC blocks to be marked as coded.
			mb->_crDcBlk.SetCoded(1);
		}//end if _codedBlkPatternChr...
	}//end if _codedBlkPatternChr...
	else	///< Set all chr blocks as coded.
	{
		for(i = MBH264_CB_DC; i <= MBH264_CR_1_1; i++)
			mb->_blkParam[i].pBlk->SetCoded(1);
	}//end else...

}//end GetCodedBlockPattern.

/** Set the macroblock type member.
Set the type according to the macroblock prediction mode and the Lum and 
Chr coded block patterns. Sets the _mb_type member.
@param mb					: Macroblock to set.
@param sliceType	: Slice type that this macroblock belongs to.
return						: None.
*/
void MacroBlockH264::SetType(MacroBlockH264* mb, int sliceType)
{
	/// Slice type values:
	/// P_Slice		    = 0;
	/// B_Slice		    = 1;
	/// I_Slice		    = 2;
	/// SP_Slice	    = 3;
	/// SI_Slice	    = 4;
	/// P_Slice_All		= 5;
	/// B_Slice_All		= 6;
	/// I_Slice_All		= 7;
	/// SP_Slice_All	= 8;
	/// SI_Slice_All	= 9;
	int type = 0;
	if(mb->_intraFlag)
	{
		if(mb->_mbPartPredMode == MacroBlockH264::Intra_16x16)
		{
			type = 1;
			if(mb->_codedBlkPatternLum)	///< Always either 0 or 15 in 16x16 mode.
				type += 12;
			type += ((4 * mb->_codedBlkPatternChr) + mb->_intra16x16PredMode);
		}//end if Intra_16x16...
	}//end if _intraFlag...
	
	if((sliceType == 0)||(sliceType == 5))	///< P_Slice
	{
		if(!mb->_intraFlag)
			type = mb->_mbPartPredMode;
		else
			type += 5;
	}//end if sliceType...

	mb->_mb_type = type;
}//end SetType.

/** Get the prediction modes.
Pull out and set the _intraFlag, _mbPartPredMode and, if Intra_16x16 mode 
then also set the _coded_blk_pattern member	from the _mb_type member decoded 
from the stream. If not Intra_16x16 then GetCodedBlockPattern() should be called 
after this method.
@param mb					: Macroblock to set.
@param sliceType	: Slice type that this macroblock belongs to.
return						: None.
*/
void MacroBlockH264::UnpackMbType(MacroBlockH264* mb, int sliceType)
{
	/// Slice type values:
	/// P_Slice		    = 0;
	/// B_Slice		    = 1;
	/// I_Slice		    = 2;
	/// SP_Slice	    = 3;
	/// SI_Slice	    = 4;
	/// P_Slice_All		= 5;
	/// B_Slice_All		= 6;
	/// I_Slice_All		= 7;
	/// SP_Slice_All	= 8;
	/// SI_Slice_All	= 9;
	int type = mb->_mb_type;
	mb->_intraFlag = 0;
	if((sliceType == 2)||(sliceType == 7))	///< I_Slice
	{
		mb->_intraFlag = 1;
	}//end if I_Slice...
	else								                    ///< P_Slice
	{
		if(type > 4)
		{
			mb->_intraFlag = 1;
			type -= 5;
		}//end if type...
	}//end else...

	if(mb->_intraFlag)
	{
		if(type == 0)
			mb->_mbPartPredMode = Intra_4x4;
		else
		{
			mb->_mbPartPredMode = Intra_16x16;
			/// Coded block patterns and prediction modes are also embedded.
			mb->_intra16x16PredMode = Intra16x16ModeTable[type-1][0];
			mb->_coded_blk_pattern	= Intra16x16ModeTable[type-1][2] | (Intra16x16ModeTable[type-1][1] << 4);
		}//end else...
	}//end if _intraFlag...
	else
		mb->_mbPartPredMode = type;

}//end UnpackMbType.

/** Test condition where skipped macroblock will force the motion vector to zero.
The prediction for the macroblock motion vector is either the median of the 
neighbouring macroblock vectors or (0,0) under some conditions of the macroblock 
neighbourhood. Defined in ITU-T Recommendation H.264 (03/2005) Section 8.4.1.1 p138.
@param mb					: Macroblock to set.
@return						: Condition for forcing the zero vector.
*/
bool MacroBlockH264::SkippedZeroMotionPredCondition(MacroBlockH264* mb)
{
	bool ret = false;

  if( (mb->_leftMb == NULL)||(mb->_aboveMb == NULL) )
    ret = true;
  else  ///< Both left and above exist.
  {
		if( ( (mb->_leftMb->_mvX[MacroBlockH264::_16x16] == 0) && (mb->_leftMb->_mvY[MacroBlockH264::_16x16] == 0)) ||
        ( (mb->_aboveMb->_mvX[MacroBlockH264::_16x16] == 0) && (mb->_aboveMb->_mvY[MacroBlockH264::_16x16] == 0)) )
        ret = true;
  }//end else...

  return(ret);
}//end SkippedZeroMotionPredCondition.

/** Predict the macroblock motion vector.
The prediction for the macroblock motion vector is the median of the 
neighbouring macroblock vectors. Special conditions apply when the
neighbours are outside of the image space or in another slice. This 
method assumes that all the (_mvX[],_mvY) values are correct before 
being called.
@param mb					: Macroblock to test.
@param mvpx				: Reference to returned predicted horiz component.
@param mvpy				: Reference to returned predicted vert component.
return						: None.
*/
void MacroBlockH264::GetMbMotionMedianPred(MacroBlockH264* mb, int* mvpx, int* mvpy)
{
	// TODO: Only the 16x16 prediction case is considered here. All other cases still to be coded.

	int Ax = 0;
	int Ay = 0;
	int Bx = 0;
	int By = 0;
	int Cx = 0;
	int Cy = 0;

	/// All intra neighbours are set to have zero vectors.
	if(mb->_leftMb != NULL)	///< A
	{
		if( !(mb->_leftMb)->_intraFlag )
		{
			Ax = (mb->_leftMb)->_mvX[MacroBlockH264::_16x16];
			Ay = (mb->_leftMb)->_mvY[MacroBlockH264::_16x16];
		}//end if _intraFlag...
	}//end if _leftMb...
	if(mb->_aboveMb != NULL)	///< B
	{
		if( !(mb->_aboveMb)->_intraFlag )
		{
			Bx = (mb->_aboveMb)->_mvX[MacroBlockH264::_16x16];
			By = (mb->_aboveMb)->_mvY[MacroBlockH264::_16x16];
		}//end if _intraFlag...
	}//end if _aboveMb...
	if(mb->_aboveRightMb != NULL)	///< C
	{
		if( !(mb->_aboveRightMb)->_intraFlag )
		{
			Cx = (mb->_aboveRightMb)->_mvX[MacroBlockH264::_16x16];
			Cy = (mb->_aboveRightMb)->_mvY[MacroBlockH264::_16x16];
		}//end if _intraFlag...
	}//end if _aboveRightMb...
	else	///< Replace C with D (if D exists)
	{
		if(mb->_aboveLeftMb != NULL)	/// D
		{
			if( !(mb->_aboveLeftMb)->_intraFlag )
			{
				Cx = (mb->_aboveLeftMb)->_mvX[MacroBlockH264::_16x16];
				Cy = (mb->_aboveLeftMb)->_mvY[MacroBlockH264::_16x16];
			}//end if _intraFlag...
		}//end if D...
	}//end else !C...

	/// Handle the special case of !B and !C by replacing them with A.
	if( (mb->_aboveMb == NULL) && (mb->_aboveRightMb == NULL) )
	{
		Bx = Ax;
		By = Ay;
		Cx = Ax;
		Cy = Ay;
	}//end !B and !C...

	*mvpx = mb->Median(Ax, Bx, Cx);
	*mvpy = mb->Median(Ay, By, Cy);
}//end GetMbMotionMedianPred.

/** Calc the median of 3 numbers.
@param x	:	1st num.
@param y	:	2nd num.
@param z	:	3rd num.
@return		: The median of x, y and z.
*/
int MacroBlockH264::Median(int x, int y, int z)
{
	int min,max;

	// min = MIN(x,MIN(y,z)) and max = MAX(x,MAX(y,z)).
	if( (y - z) < 0 )	{	min = y;	max = z;	}
	else { min = z; max = y; }
	if(x < min) min = x;
	if(x > max) max = x;
	// Median.
	int result = x + y + z - min - max;

	return(result);
}//end Median.

/** Copy the contents of blocks to the temp blocks.
The block numbers in the parameter list refer to their array position.
@param mb				: Macroblock to process.
@param startBlk	: Position of first block to copy.
@param endBlk		: Position of last block to copy.
@return					: none.
*/
void MacroBlockH264::CopyBlksToTmpBlks(MacroBlockH264* mb, int startBlk, int endBlk)
{
	if( (endBlk < MBH264_NUM_BLKS) && (startBlk >= 0) ) ///< Array range check.
	{
		for(int i = startBlk; i <= endBlk; i++)
			mb->_blkParam[i].pBlkTmp->CopyBlock(mb->_blkParam[i].pBlk);
	}//end if endBlk...
}//end CopyBlksToTmpBlks.

/** Copy the coeff mem of blocks to the temp blocks.
The block numbers in the parameter list refer to their array position. No bounds range
checking is done.
@param mb				: Macroblock to process.
@param startBlk	: Position of first block to copy.
@param endBlk		: Position of last block to copy.
@return					: none.
*/
void MacroBlockH264::CopyBlksToTmpBlksCoeffOnly(MacroBlockH264* mb, int startBlk, int endBlk)
{
	for(int i = startBlk; i <= endBlk; i++)
    mb->_blkParam[i].pBlk->Copy((void *)(mb->_blkParam[i].pBlkTmp->GetBlk()));
}//end CopyBlksToTmpBlks.

/** Load macroblock from image.
Copy the YCbCr values from the image colour components specified in the parameter list
into the macroblock. It assumes that the image overlay is of appropriate size.
@param mb				:	Macroblock to load.
@param lum			: Lum img overlay.
@param lumoffx	: X fffset for the Lum overlay.
@param lumoffy	: Y offset for the Lum overlay.
@param cb				: Cb img overlay.
@param cr				: Cb img overlay.
@param chroffx	: X offset for the Chr overlay.
@param chroffy	: Y offset for the Chr overlay.
*/
void MacroBlockH264::LoadBlks(MacroBlockH264* mb, OverlayMem2Dv2* lum, int lumoffx, int lumoffy, OverlayMem2Dv2* cb, OverlayMem2Dv2* cr, int chroffx, int chroffy)
{
	int blk;

	/// Fill all the non-DC 4x4 blks (Not blks = -1, 17, 18) of the macroblock blocks with 
	/// each image colour component.

	lum->SetOverlayDim(4, 4);
	for(blk = MBH264_LUM_0_0; blk <= MBH264_LUM_3_3; blk++)
	{
		BlockH264* pBlk = mb->_blkParam[blk].pBlk;
		/// Align the block with the image space.
		lum->SetOrigin(lumoffx + pBlk->_offX, lumoffy + pBlk->_offY);
		/// Read from the image mem into the block.
		lum->Read(*(pBlk->GetBlkOverlay()));
	}//end for blk...

	cb->SetOverlayDim(4, 4);
	for(blk = MBH264_CB_0_0; blk <= MBH264_CB_1_1; blk++)
	{
		BlockH264* pBlk = mb->_blkParam[blk].pBlk;
		/// Align the block with the image space.
		cb->SetOrigin(chroffx + pBlk->_offX, chroffy + pBlk->_offY);
		/// Read from the image mem into the block.
		cb->Read(*(pBlk->GetBlkOverlay()));
	}//end for blk...

	cr->SetOverlayDim(4, 4);
	for(blk = MBH264_CR_0_0; blk <= MBH264_CR_1_1; blk++)
	{
		BlockH264* pBlk = mb->_blkParam[blk].pBlk;
		/// Align the block with the image space.
		cr->SetOrigin(chroffx + pBlk->_offX, chroffy + pBlk->_offY);
		/// Read from the image mem into the block.
		cr->Read(*(pBlk->GetBlkOverlay()));
	}//end for blk...

}//end LoadBlks.

/** Store macroblock to image.
Copy the YCbCr values from the macroblock into the image colour components specified in the 
parameter list. It assumes that the image overlay is of appropriate size.
@param mb					:	Macroblock to store.
@param lum				: Lum img overlay.
@param lumoffx		: X offset for the Lum overlay.
@param lumoffy		: Y offset for the Lum overlay.
@param cb					: Cb img overlay.
@param cr					: Cb img overlay.
@param chroffx		: X offset for the Chr overlay.
@param chroffy		: Y offset for the Chr overlay.
@param tmpBlkFlag	: Store from temp blocks.
*/
void MacroBlockH264::StoreBlks(MacroBlockH264* mb, OverlayMem2Dv2* lum, int lumoffx, int lumoffy, OverlayMem2Dv2* cb, OverlayMem2Dv2* cr, int chroffx, int chroffy, int tmpBlkFlag)
{

	int					i;
	BlockH264*	pLumBlk;
	BlockH264*	pCbBlk;
	BlockH264*	pCrBlk;
	if(tmpBlkFlag)
	{
		pLumBlk = &(mb->_lumBlkTmp[0][0]);
		pCbBlk	= &(mb->_cbBlkTmp[0][0]);
		pCrBlk	= &(mb->_crBlkTmp[0][0]);
	}//end if tmpBlkFlag...
	else
	{
		pLumBlk = &(mb->_lumBlk[0][0]);
		pCbBlk	= &(mb->_cbBlk[0][0]);
		pCrBlk	= &(mb->_crBlk[0][0]);
	}//end else...

	/// Store all the non-DC 4x4 blks (Not blks = -1, 17, 18) of the macroblock blocks into 
	/// each image colour component.

	lum->SetOverlayDim(4, 4);
	for(i = 0; i < 16; i++)
	{
		/// Align the block with the image space.
		lum->SetOrigin(lumoffx + pLumBlk->_offX, lumoffy + pLumBlk->_offY);
		/// Read from the image mem into the block.
		lum->Write4x4(*(pLumBlk->GetBlkOverlay()));
		/// Next block.
		pLumBlk++;
	}//end for i...

	cb->SetOverlayDim(4, 4);
	for(i = 0; i < 4; i++)
	{
		cb->SetOrigin(chroffx + pCbBlk->_offX, chroffy + pCbBlk->_offY);
		cb->Write4x4(*(pCbBlk->GetBlkOverlay()));
		pCbBlk++;
	}//end for i...

	cr->SetOverlayDim(4, 4);
	for(i = 0; i < 4; i++)
	{
		cr->SetOrigin(chroffx + pCrBlk->_offX, chroffy + pCrBlk->_offY);
		cr->Write4x4(*(pCrBlk->GetBlkOverlay()));
		pCrBlk++;
	}//end for i...

}//end StoreBlks.

/** Copy from another macroblock.
Match the mem size and copy all members and macroblock
data.
@param pMbInto	: Macroblock to copy into.
@param pMbFrom	: Macroblock to copy from.
@return					: 1/0 = success/failure.
*/
int MacroBlockH264::CopyMarcoBlockProxy(MacroBlockH264* pMbInto, MacroBlockH264* pMbFrom)
{
	pMbInto->_offLumX				= pMbFrom->_offLumX;
	pMbInto->_offLumY				= pMbFrom->_offLumY;
	pMbInto->_offChrX				= pMbFrom->_offChrX;
	pMbInto->_offChrY				= pMbFrom->_offChrY;
	pMbInto->_mbIndex				= pMbFrom->_mbIndex;
	pMbInto->_slice					= pMbFrom->_slice;
	pMbInto->_leftMb				= pMbFrom->_leftMb;
	pMbInto->_aboveLeftMb		= pMbFrom->_aboveLeftMb;
	pMbInto->_aboveMb				= pMbFrom->_aboveMb;
	pMbInto->_aboveRightMb	= pMbFrom->_aboveRightMb;
	pMbInto->_mbQP					= pMbFrom->_mbQP;
	pMbInto->_mbEncQP				= pMbFrom->_mbEncQP;

	memcpy((void *)(&pMbInto->_mvX[0]), (const void *)(&pMbFrom->_mvX[0]), 16 * sizeof(int));
	memcpy((void *)(&pMbInto->_mvY[0]), (const void *)(&pMbFrom->_mvY[0]), 16 * sizeof(int));

	pMbInto->_intraFlag						= pMbFrom->_intraFlag;
	pMbInto->_mbPartPredMode			= pMbFrom->_mbPartPredMode;
	pMbInto->_mbSubPartPredMode		= pMbFrom->_mbSubPartPredMode;
	pMbInto->_intra16x16PredMode	= pMbFrom->_intra16x16PredMode;
	pMbInto->_intraChrPredMode		= pMbFrom->_intraChrPredMode;
	pMbInto->_codedBlkPatternChr	= pMbFrom->_codedBlkPatternChr;
	pMbInto->_codedBlkPatternLum	= pMbFrom->_codedBlkPatternLum;

	pMbInto->_mvDistortion	= pMbFrom->_mvDistortion;
	pMbInto->_mvRate				= pMbFrom->_mvRate;
	pMbInto->_include				= pMbFrom->_include;
	memcpy((void *)(&pMbInto->_distortion[0]), (const void *)(&pMbFrom->_distortion[0]), 52 * sizeof(int));
	memcpy((void *)(&pMbInto->_rate[0]), (const void *)(&pMbFrom->_rate[0]), 52 * sizeof(int));

	pMbInto->_skip							= pMbFrom->_skip;
	pMbInto->_mb_type						= pMbFrom->_mb_type;
	pMbInto->_sub_mb_type				= pMbFrom->_sub_mb_type;
	pMbInto->_coded_blk_pattern = pMbFrom->_coded_blk_pattern;
	pMbInto->_mb_qp_delta				= pMbFrom->_mb_qp_delta;

	memcpy((void *)(&pMbInto->_mvdX[0]), (const void *)(&pMbFrom->_mvdX[0]), 16 * sizeof(int));
	memcpy((void *)(&pMbInto->_mvdY[0]), (const void *)(&pMbFrom->_mvdY[0]), 16 * sizeof(int));

	for(int i = 0; i < MBH264_NUM_BLKS; i++)
	{
		pMbInto->_blkParam[i].pBlk->CopyBlock(pMbFrom->_blkParam[i].pBlk);
		pMbInto->_blkParam[i].pBlkTmp->CopyBlock(pMbFrom->_blkParam[i].pBlkTmp);
		pMbInto->_blkParam[i].rasterIndex					= pMbFrom->_blkParam[i].rasterIndex;
		pMbInto->_blkParam[i].neighbourIndicator	= pMbFrom->_blkParam[i].neighbourIndicator;
		pMbInto->_blkParam[i].dcSkipFlag					= pMbFrom->_blkParam[i].dcSkipFlag;
	}//end for i...

	return(1);
}//end CopyMarcoBlockProxy;

/** Check for equality with another macroblock.
Check all key members and main coeff blocks.
@param me	: Macroblock to compare.
@param mb	: Macroblock from which to compare.
@return		: 1/0 = equal/not equal.
*/
int MacroBlockH264::EqualsProxy(MacroBlockH264* me, MacroBlockH264* mb)
{
	if( (me->_mbQP != mb->_mbQP) ||
			(me->_mbEncQP != mb->_mbEncQP) ||
			(me->_mb_qp_delta != mb->_mb_qp_delta) ||
			(me->_mb_type != mb->_mb_type) ||
			(me->_sub_mb_type != mb->_sub_mb_type) ||
			(me->_skip != mb->_skip) ||
			(me->_intraFlag != mb->_intraFlag) ||
			(me->_coded_blk_pattern != mb->_coded_blk_pattern) ||
			(me->_codedBlkPatternLum != mb->_codedBlkPatternLum) ||
			(me->_codedBlkPatternChr != mb->_codedBlkPatternChr) ||
			(me->_mbPartPredMode != mb->_mbPartPredMode) ||
			(me->_mbSubPartPredMode != mb->_mbSubPartPredMode) )
		return(0);

	if(me->_intraFlag)
	{
		if(	(me->_intraChrPredMode != mb->_intraChrPredMode) ||
				(me->_intra16x16PredMode != mb->_intra16x16PredMode) )
			return(0);

		if(me->_mbPartPredMode == MacroBlockH264::Intra_16x16)
		{
		}//end if _mbPartPredMode...
	}//end if _intraFlag...
	else
	{
	}//end else...

	for(int i = 0; i < MBH264_NUM_BLKS; i++)
	{
		if( (!me->_blkParam[i].pBlk->Equals(mb->_blkParam[i].pBlk)) ||
				(me->_blkParam[i].rasterIndex					!= mb->_blkParam[i].rasterIndex) ||
				(me->_blkParam[i].neighbourIndicator	!= mb->_blkParam[i].neighbourIndicator) ||
				(me->_blkParam[i].dcSkipFlag					!= mb->_blkParam[i].dcSkipFlag) )
				return(0);
	}//end for i...

	return(1);
}//end EqualsProxy.

/** Check for coded coeff.
After SetCodedBlockPattern() method has been called the _coded_blk_pattern
member reflects the 8x8 block group coded coeff status. However, for Intra
macroblocks in Intra_16x16 modes, the Lum Dc block is not included and must
be done seperately. NB: THIS METHOD MUST ONLY BE CALLED AFTER SetCodedBlockPattern()
HAS BEEN CALLED ON THE MACROBLOCK. I.e. _coded_blk_pattern is valid.
@param mb	: Macroblock to test.
return		: 0/1 = All zero coeffs/At least one non-zero coeff.
*/
int MacroBlockH264::HasNonZeroCoeffsProxy(MacroBlockH264* mb)
{
	if(mb->_intraFlag && (mb->_mbPartPredMode == MacroBlockH264::Intra_16x16))
	{
		if(!mb->_lumDcBlk.IsZero2())
			return(1);
	}//end if _intraFlag...

	if(mb->_coded_blk_pattern)	///< SetCodedBlockPattern() must have been called for _coded_blk_pattern to be valid.
		return(1);

	return(0);
}//end HasNonZeroCoeffsProxy.

/** Calculate the distortion between two overlays at this mb's postion.
Align the two lum and chr images over this mb and calculate the sum of square error for
the three colour spaces. The image spaces must have identical width and height and match
the mb initialisation settings. The format must be YCbCr 4:2:0 16x16:8x8:8x8.
@param  p1Y   : 1st Image lum overlay
@param  p1Cb  : 1st Image chr overlay
@param  p1Cr  :
@param  p2Y   : 2nd Image lum overlay
@param  p2Cb  : 2nd Image chr overlay
@param  p2Cr  :
@return       : Square error distortion
*/
int MacroBlockH264::Distortion(OverlayMem2Dv2* p1Y, OverlayMem2Dv2* p1Cb, OverlayMem2Dv2* p1Cr, OverlayMem2Dv2* p2Y, OverlayMem2Dv2* p2Cb, OverlayMem2Dv2* p2Cr)
{
  int distortion = 0;
  int tmp1Width, tmp1Height, tmp1OrgX, tmp1OrgY;
  int tmp2Width, tmp2Height, tmp2OrgX, tmp2OrgY;

  /// Lum
  tmp1Width   = p1Y->GetWidth();
  tmp1Height  = p1Y->GetHeight();
  tmp1OrgX    = p1Y->GetOriginX();
  tmp1OrgY    = p1Y->GetOriginY();
  p1Y->SetOverlayDim(16,16);
  p1Y->SetOrigin(_offLumX, _offLumY);
  tmp2Width   = p2Y->GetWidth();
  tmp2Height  = p2Y->GetHeight();
  tmp2OrgX    = p2Y->GetOriginX();
  tmp2OrgY    = p2Y->GetOriginY();
  p2Y->SetOverlayDim(16,16);
  p2Y->SetOrigin(_offLumX, _offLumY);

  distortion = p1Y->Tsd16x16( *p2Y );

  p1Y->SetOverlayDim(tmp1Width,tmp1Height);
  p1Y->SetOrigin(tmp1OrgX, tmp1OrgY);
  p2Y->SetOverlayDim(tmp2Width,tmp2Height);
  p2Y->SetOrigin(tmp2OrgX, tmp2OrgY);

  /// Cb
  tmp1Width   = p1Cb->GetWidth();
  tmp1Height  = p1Cb->GetHeight();
  tmp1OrgX    = p1Cb->GetOriginX();
  tmp1OrgY    = p1Cb->GetOriginY();
  p1Cb->SetOverlayDim(8,8);
  p1Cb->SetOrigin(_offChrX, _offChrY);
  tmp2Width   = p2Cb->GetWidth();
  tmp2Height  = p2Cb->GetHeight();
  tmp2OrgX    = p2Cb->GetOriginX();
  tmp2OrgY    = p2Cb->GetOriginY();
  p2Cb->SetOverlayDim(8,8);
  p2Cb->SetOrigin(_offChrX, _offChrY);

  distortion += p1Cb->Tsd8x8( *p2Cb );

  p1Cb->SetOverlayDim(tmp1Width,tmp1Height);
  p1Cb->SetOrigin(tmp1OrgX, tmp1OrgY);
  p2Cb->SetOverlayDim(tmp2Width,tmp2Height);
  p2Cb->SetOrigin(tmp2OrgX, tmp2OrgY);

  /// Cr
  tmp1Width   = p1Cr->GetWidth();
  tmp1Height  = p1Cr->GetHeight();
  tmp1OrgX    = p1Cr->GetOriginX();
  tmp1OrgY    = p1Cr->GetOriginY();
  p1Cr->SetOverlayDim(8,8);
  p1Cr->SetOrigin(_offChrX, _offChrY);
  tmp2Width   = p2Cr->GetWidth();
  tmp2Height  = p2Cr->GetHeight();
  tmp2OrgX    = p2Cr->GetOriginX();
  tmp2OrgY    = p2Cr->GetOriginY();
  p2Cr->SetOverlayDim(8,8);
  p2Cr->SetOrigin(_offChrX, _offChrY);

  distortion += p1Cr->Tsd8x8( *p2Cr );

  p1Cr->SetOverlayDim(tmp1Width,tmp1Height);
  p1Cr->SetOrigin(tmp1OrgX, tmp1OrgY);
  p2Cr->SetOverlayDim(tmp2Width,tmp2Height);
  p2Cr->SetOrigin(tmp2OrgX, tmp2OrgY);

  return(distortion);
}//end Distortion.

/** Mark this macroblock onto the image.
For debugging.
@param  mb  : Macroblock to mark
@param  lum : Lum image space.
@param  cb  : Cb image space.
@param  cr  : Cr image space.
@return     : none.
*/
void MacroBlockH264::Mark(MacroBlockH264* mb, OverlayMem2Dv2* lum, OverlayMem2Dv2* cb, OverlayMem2Dv2* cr)
{
  int i;

 	lum->SetOverlayDim(16, 16);
  lum->SetOrigin(mb->_offLumX, mb->_offLumY);
  for(i = 0; i < 16; i++)
  {
    lum->Write(i,0,0);      ///< Top row.
    lum->Write(i,15,0);     ///< Bottom row.
    lum->Write(0,i,0);      ///< Left col.
    lum->Write(15,i,0);     ///< Right col.
  }//end for i...

 	cb->SetOverlayDim(8, 8);
  cb->SetOrigin(mb->_offChrX, mb->_offChrY);
 	cr->SetOverlayDim(8, 8);
  cr->SetOrigin(mb->_offChrX, mb->_offChrY);
  for(i = 0; i < 8; i++)
  {
    cb->Write(i,0,127);     ///< Top row.
    cr->Write(i,0,127);     ///< Top row.
    cb->Write(i,7,127);     ///< Bottom row.
    cr->Write(i,7,127);     ///< Bottom row.
    cb->Write(0,i,127);     ///< Left col.
    cr->Write(0,i,127);     ///< Left col.
    cb->Write(7,i,127);     ///< Right col.
    cr->Write(7,i,127);     ///< Right col.
  }//end for i...

}//end Mark.

/** Dump the state variables of the macroblock to file.
For debugging.
@param  mb  : Macroblock to dump
@param  filename  : File to write into
@param  title     : Title of stored table data
@return     : none.
*/
void MacroBlockH264::Dump(MacroBlockH264* mb, char* filename, const char* title)
{
  int j; 

  MeasurementTable* pT = new MeasurementTable();
	pT->Create(18, 1);
	pT->SetTitle(title);
  
  pT->SetHeading(0, "Index");
  pT->SetHeading(1, "Type");
  pT->SetHeading(2, "SubType");
  pT->SetHeading(3, "Skip");
  pT->SetHeading(4, "QP");
  pT->SetHeading(5, "DeltaQP");
  pT->SetHeading(6, "IntraFlg");
  pT->SetHeading(7, "PartPredMode");
  pT->SetHeading(8, "SubPartPredMode");
  pT->SetHeading(9, "Intra16x16PartPredMode");
  pT->SetHeading(10, "IntraChrPredMode");
  pT->SetHeading(11, "mvX[0]");
  pT->SetHeading(12, "mvY[0]");
  pT->SetHeading(13, "mvdX[0]");
  pT->SetHeading(14, "mvdY[0]");
  pT->SetHeading(15, "CodeBlkPattern");
  pT->SetHeading(16, "CodeBlkPatternLum");
  pT->SetHeading(17, "CodeBlkPatternChr");

  for(j = 0; j < 18; j++)
    pT->SetDataType(j, MeasurementTable::INT);

	pT->WriteItem(0, 0, mb->_mbIndex);
	pT->WriteItem(1, 0, mb->_mb_type);
	pT->WriteItem(2, 0, mb->_sub_mb_type);
	pT->WriteItem(3, 0, mb->_skip);
	pT->WriteItem(4, 0, mb->_mbQP);
	pT->WriteItem(5, 0, mb->_mb_qp_delta);
	pT->WriteItem(6, 0, mb->_intraFlag);
	pT->WriteItem(7, 0, mb->_mbPartPredMode);
	pT->WriteItem(8, 0, mb->_mbSubPartPredMode);
	pT->WriteItem(9, 0, mb->_intra16x16PredMode);
	pT->WriteItem(10, 0, mb->_intraChrPredMode);
	pT->WriteItem(11, 0, mb->_mvX[0]);
	pT->WriteItem(12, 0, mb->_mvY[0]);
	pT->WriteItem(13, 0, mb->_mvdX[0]);
	pT->WriteItem(14, 0, mb->_mvdY[0]);
	pT->WriteItem(15, 0, mb->_coded_blk_pattern);
	pT->WriteItem(16, 0, mb->_codedBlkPatternLum);
	pT->WriteItem(17, 0, mb->_codedBlkPatternChr);

  pT->Save(filename, ",", 1);

  delete pT;
}//end Dump.


