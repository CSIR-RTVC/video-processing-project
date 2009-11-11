/** @file

MODULE				: MacroBlockH264

TAG						: MBH264

FILE NAME			: MacroBlockH264.h

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
#ifndef _MACROBLOCKH264_H
#define _MACROBLOCKH264_H

#pragma once

#include "BlockH264.h"

/*
---------------------------------------------------------------------------
	Constants.
---------------------------------------------------------------------------
*/
#define MBH264_LUM_DC				0
#define MBH264_LUM_0_0			1
#define MBH264_LUM_0_1			2
#define MBH264_LUM_1_0			3
#define MBH264_LUM_1_1			4
#define MBH264_LUM_0_2			5
#define MBH264_LUM_0_3			6
#define MBH264_LUM_1_2			7
#define MBH264_LUM_1_3			8
#define MBH264_LUM_2_0			9
#define MBH264_LUM_2_1			10
#define MBH264_LUM_3_0			11
#define MBH264_LUM_3_1			12
#define MBH264_LUM_2_2			13
#define MBH264_LUM_2_3			14
#define MBH264_LUM_3_2			15
#define MBH264_LUM_3_3			16
#define MBH264_CB_DC				17
#define MBH264_CR_DC				18
#define MBH264_CB_0_0				19
#define MBH264_CB_0_1				20
#define MBH264_CB_1_0				21
#define MBH264_CB_1_1				22
#define MBH264_CR_0_0				23
#define MBH264_CR_0_1				24
#define MBH264_CR_1_0				25
#define MBH264_CR_1_1				26

#define MBH264_NUM_BLKS			27

/// A list of these structs give the correct coding order and parameters to 
/// iterate through the blocks during encoding or decoding.
typedef struct _MBH264_CODING_STRUCT
{
	/// Block reference.
	BlockH264*	pBlk;
	/// (< 0) = used directly, (0) = no neighbourhood check, (1) = do a neighbourhood check.
	int					neighbourIndicator;
	/// DC skip flag to use.
	int					dcSkipFlag;
} MBH264_CODING_STRUCT;

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class MacroBlockH264
{
public:
	MacroBlockH264(void);
	virtual ~MacroBlockH264(void);

/// Interface implementation.
public:
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
	static int Initialise(int numMbRows, int numMbCols, int startMbIndex, int endMbIndex, int slice, MacroBlockH264** mb);

	/** Map the lum quant parameter to the Chr quant parameter.
	This mapping is defined in ITU-T Rec. H.264 (03/2005) page 167
	Table 8-15. The chr coeffs are less harshly quantised than the
	lum coeffs.
	@param QPl	: Lum quant param.
	return			: Mapped chr quant param.
	*/
	static int GetQPc(int QPl) { return(QPltoQPcMap[QPl]); }

	/** Set the coded block pattern member.
	After quantisation the Lum and Chr block patterns are calculated from
	examining the zero/non-zero status of each 4x4 block in their 8x8 block
	groups. The _codedBlkPatternLum, _codedBlkPatternChr and _coded_blk_pattern
	members are set and each 4x4 block's coded status is synchronized to this
	pattern.
	@param mb	: Macroblock to set.
	return		: None.
	*/
	static void SetCodedBlockPattern(MacroBlockH264* mb);

	/** Get the coded block pattern member.
	Pull out and set the _codedBlkPatternLum and _codedBlkPatternChr members
	from the _coded_blk_pattern member decoded from the stream. Then set each 
	4x4 block's coded status synchronized to this pattern.
	@param mb	: Macroblock to set.
	return		: None.
	*/
	static void GetCodedBlockPattern(MacroBlockH264* mb);

	/** Set the macroblock type member.
	Set the type according to the macroblock prediction mode and the Lum and 
	Chr coded block patterns. Sets the _mb_type member.
	@param mb					: Macroblock to set.
	@param sliceType	: Slice type that this macroblock belongs to.
	return						: None.
	*/
	static void SetType(MacroBlockH264* mb, int sliceType);

	/** Unpack macroblock type to the prediction modes.
	Pull out and set the _intraFlag, _mbPartPredMode and, if Intra_16x16 mode 
	then also	set the _coded_blk_pattern member	from the _mb_type member decoded 
	from the stream. If Intra_16x16 then GetCodedBlockPattern() should be called 
	after this method.
	@param mb					: Macroblock to set.
	@param sliceType	: Slice type that this macroblock belongs to.
	return						: None.
	*/
	static void UnpackMbType(MacroBlockH264* mb, int sliceType);

	/** Predict the macroblock motion vector.
	The prediction for the macroblock motion vector is the median of the 
	neighbouring macroblock vectors. Special conditions apply when the
	neighbours are outside of the image space or in another slice.
	@param mb					: Macroblock to set.
	@param mvpx				: Reference to returned predicted horiz component.
	@param mvpy				: Reference to returned predicted vert component.
	return						: None.
	*/
	static void GetMbMotionPred(MacroBlockH264* mb, int* mvpx, int* mvpy);

	/** Calc the median of 3 numbers.
	@param x	:	1st num.
	@param y	:	2nd num.
	@param z	:	3rd num.
	@return		: The median of x, y and z.
	*/
	static int Median(int x, int y, int z);

/// Constants.
public:
	static const int mbCodingOrderY[4][4];	///< Row
	static const int mbCodingOrderX[4][4];	///< Col
	static const int QPltoQPcMap[52];
	static const int Intra16x16ModeTable[24][3];

	/// Inter and Intra _mbPartPredMode values. For Inter, the _mbPartPredMode = _mb_type.
	static const int Inter_16x16		= 0;
	static const int Inter_16x8			= 1;
	static const int Inter_8x16			= 2;
	static const int Inter_8x8			= 3;
	static const int Inter_8x8_Ref	= 4;
	static const int Intra_4x4			= 8;
	static const int Intra_8x8			= 9;
	static const int Intra_16x16		= 10;
	/// Intra _intra16x16PredMode values.
	static const int Intra_16x16_Vert		= 0;
	static const int Intra_16x16_Horiz	= 1;
	static const int Intra_16x16_DC			= 2;
	static const int Intra_16x16_Plane	= 3;
	/// Intra _intraChrPredMode values.
	static const int Intra_Chr_DC			= 0;
	static const int Intra_Chr_Horiz	= 1;
	static const int Intra_Chr_Vert		= 2;
	static const int Intra_Chr_Plane	= 3;

	/// Motion vector array indices based on pred type.
	static const int _16x16		= 0;
	static const int _16x8_0	= 0;
	static const int _16x8_1	= 1;
	static const int _8x16_0	= 0;
	static const int _8x16_1	= 1;
	static const int _8x8_0		= 0;
	static const int _8x8_1		= 1;
	static const int _8x8_2		= 2;
	static const int _8x8_3		= 3;

/// Aux data members.
public:
	/// Relative pixel coords (top left) of this macroblock in a contiguous image mem. Each
	/// block in the macroblock describes its relative position from this offset.
	int			_offLumX;
	int			_offLumY;
	int			_offChrX;
	int			_offChrY;
	int			_mbIndex;		///< The index of this macroblock in the image space.
	int			_slice;			///< Slice number that this macroblock belongs to.

	/// References to this macroblock's neighbours that will be used for the predictions.
	/// These are related to ITU-T Rec. H.264 (03/2005) page 27 Section 6.4.6 and will
	/// have a NULL value if they are not available. E.g. Top row macroblocks will have
	/// all the _aboveMbxxx = NULL.
	MacroBlockH264*	_leftMb;				///< mbAddrA.
	MacroBlockH264*	_aboveLeftMb;		///< mbAddrD.
	MacroBlockH264*	_aboveMb;				///< mbAddrB.
	MacroBlockH264*	_aboveRightMb;	///< mbAddrC.

	/// Macroblock quantisation parameter derived from delta qp values.
	int			_mbQP;

	/// Motion vectors derived from motion vector difference values.
	int _mvX[16];
	int _mvY[16];

	/// The macroblock type coded value mb_type is constructed from combinations 
	/// of the following modes.
	int			_intraFlag;							///< Indicates the macroblock Intra/Inter status.
	int			_mbPartPredMode;				///< Macroblock partition prediction mode.
	int			_mbSubPartPredMode;			///< ...sub partition mode when partition mode is 8x8.
	int			_intra16x16PredMode;		///< Only for Intra_16x16 macroblocks = {0..3}.
	int			_intraChrPredMode;			///< For Intra_4x4 and Intra_16x16 macroblocks = {0..3}.
	int			_codedBlkPatternChr;
	int			_codedBlkPatternLum;

	/// Distortion-Rate optimisations require storing intermediate values. The arrays
	/// are directly addressable by _mbQP. Use an include flag to help with 
	/// heuristic elimination.
	int			_mvDistortion;		///< Total motion compensated distortion.
	int			_mvRate;					///< Total motion vector bit count.
	int			_distortion[52];	///< mbQP = {0...51}
	int			_rate[52];
	int			_include;

// Coded data members.
public:
	/// A flag to indicate that this macroblock has nothing to code and is skipped.
	int	_skip;

	/// The macroblock type defines the partitioning, prediction modes and the coded
	/// block patterns.
	int _mb_type;
	int _sub_mb_type;
	int _coded_blk_pattern;

	/// Differential quantisation parameter for lum coeffs. Chr quant parameter is
	/// derived from the lum qp.
	int _mb_qp_delta;

	/// Motion Vector Difference. From 1 to 16 vectors depending the partitions
	/// and sub partition sizes.
	int _mvdX[16];
	int _mvdY[16];

	/// Defined 4x4 blocks. Included are one lum 4x4 DC block and
	/// two chr 2x2 DC blocks. Overlays are created and the default
	/// block dimensions are 4x4 on instantiation.
	BlockH264	_lumDcBlk;				///< Index  = -1
	BlockH264 _lumBlk[4][4];		///<			  = 0...15
	BlockH264 _cbDcBlk;					///<				= 16	(NB: Chr instantiated as 4x4 and must be 2x2)
	BlockH264 _crDcBlk;					///<				= 17
	BlockH264 _cbBlk[2][2];			///<				= 18...21
	BlockH264 _crBlk[2][2];			///<				= 22...25
	/// Block references and their associated parameters to iterate
	/// through during encoding and decoding.
	MBH264_CODING_STRUCT	_blkParam[MBH264_NUM_BLKS];

};// end class MacroBlockH264.

#endif	//_MACROBLOCKH263_H
