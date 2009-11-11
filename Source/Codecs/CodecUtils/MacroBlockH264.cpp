/** @file

MODULE				: MacroBlockH264

TAG						: MBH264

FILE NAME			: MacroBlockH264.cpp

DESCRIPTION		: A class to hold H.264 macroblock specific data. Only static 
								methods that apply to macroblocks are used because it is 
								expected that there will be one MacroBlockH264() instantiation
								per overlay position on the picture.

COPYRIGHT			:	(c)CSIR 2007-2009 all rights resevered

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

#include "MacroBlockH264.h"

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
	_mbQP = 1;

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

	/// Load the block parameter structure and the block types.
	/// Block num = -1 (Lum DC block);
	_blkParam[0].pBlk								= &_lumDcBlk;
	_blkParam[0].dcSkipFlag					= 0;
	_blkParam[0].neighbourIndicator	= 1;
	_lumDcBlk.SetColour(BlockH264::LUM);
	_lumDcBlk.SetDcFlag(1);
	///< 16 Lum blocks without DC value. Block num = 0..15
	int blkNum = 1;
	for(i = 0; i < 4; i++)	
		for(j = 0; j < 4; j++)	
		{
			int blkY = mbCodingOrderY[i][j];
			int blkX = mbCodingOrderX[i][j];
			_blkParam[blkNum].pBlk								= &(_lumBlk[blkY][blkX]);
			_blkParam[blkNum].dcSkipFlag					= 1;	///< Initialise assuming Intra macroblock.
			_blkParam[blkNum].neighbourIndicator	= 1;
			_lumBlk[blkY][blkX].SetColour(BlockH264::LUM);
			_lumBlk[blkY][blkX].SetDcFlag(0);
			blkNum++;
		}//end for i & j...
	/// Chr DC next. Block num = 16 & 17.
	_blkParam[blkNum].pBlk								= &_cbDcBlk;
	_blkParam[blkNum].dcSkipFlag					= 0;
	_blkParam[blkNum].neighbourIndicator	= -1;
	_cbDcBlk.SetColour(BlockH264::CB);
	_cbDcBlk.SetDcFlag(1);
	blkNum++;
	_blkParam[blkNum].pBlk								= &_crDcBlk;
	_blkParam[blkNum].dcSkipFlag					= 0;
	_blkParam[blkNum].neighbourIndicator	= -1;
	_crDcBlk.SetColour(BlockH264::CR);
	_crDcBlk.SetDcFlag(1);
	blkNum++;
	///< 4 Cb Chr blocks without DC value. Block num = 18..21
	for(i = 0; i < 2; i++)	
		for(j = 0; j < 2; j++)	
		{
			_blkParam[blkNum].pBlk								= &(_cbBlk[i][j]);
			_blkParam[blkNum].dcSkipFlag					= 1;
			_blkParam[blkNum].neighbourIndicator	= 1;
			_cbBlk[i][j].SetColour(BlockH264::CB);
			_cbBlk[i][j].SetDcFlag(0);
			blkNum++;
		}//end for i & j...
	///< 4 Cr Chr blocks without DC value. Block num = 22..25
	for(i = 0; i < 2; i++)	
		for(j = 0; j < 2; j++)	
		{
			_blkParam[blkNum].pBlk								= &(_crBlk[i][j]);
			_blkParam[blkNum].dcSkipFlag					= 1;
			_blkParam[blkNum].neighbourIndicator	= 1;
			_crBlk[i][j].SetColour(BlockH264::CR);
			_crBlk[i][j].SetDcFlag(0);
			blkNum++;
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
					mb[row][col]._aboveLeftMb = &(mb[y][col]);
			}//end if y...
			mb[row][col]._aboveRightMb	= NULL;	///< Above right.
			if((z >= 0)&&(y >= 0))	
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
	/// Slice type: 0 = I_Slice, 1 = P_Slice.
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
	
	if(sliceType == 1)	///< P_Slice
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
from the stream. If Intra_16x16 then GetCodedBlockPattern() should be called 
after this method.
@param mb					: Macroblock to set.
@param sliceType	: Slice type that this macroblock belongs to.
return						: None.
*/
void MacroBlockH264::UnpackMbType(MacroBlockH264* mb, int sliceType)
{
	int type = mb->_mb_type;
	/// Slice type: 0 = I_Slice, 1 = P_Slice.
	mb->_intraFlag = 0;
	if(sliceType == 0)	///< I_Slice
	{
		mb->_intraFlag = 1;
	}//end if I_Slice...
	else								///< P_Slice
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

/** Predict the macroblock motion vector.
The prediction for the macroblock motion vector is the median of the 
neighbouring macroblock vectors. Special conditions apply when the
neighbours are outside of the image space or in another slice.
@param mb					: Macroblock to set.
@param mvpx				: Reference to returned predicted horiz component.
@param mvpy				: Reference to returned predicted vert component.
return						: None.
*/
void MacroBlockH264::GetMbMotionPred(MacroBlockH264* mb, int* mvpx, int* mvpy)
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
}//end GetMbMotionPred.

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



