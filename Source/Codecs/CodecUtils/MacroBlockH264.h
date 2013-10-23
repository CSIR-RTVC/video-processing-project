/** @file

MODULE				: MacroBlockH264

TAG						: MBH264

FILE NAME			: MacroBlockH264.h

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
	BlockH264*	pBlkTmp;			///< A temporary working block.
	int					rasterIndex;	///< Mapping of this blk to its blk index in raster scan order.
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
	from the stream. If not Intra_16x16 then GetCodedBlockPattern() should be called 
	after this method.
	@param mb					: Macroblock to set.
	@param sliceType	: Slice type that this macroblock belongs to.
	return						: None.
	*/
	static void UnpackMbType(MacroBlockH264* mb, int sliceType);

  /** Test condition where skipped macroblock forces the motion vector to zero.
  The prediction for the macroblock motion vector is either the median of the 
  neighbouring macroblock vectors or (0,0) under some conditions of the macroblock 
  neighbourhood. 
  @param mb					: Macroblock to set.
  return						: Condition for forcing the zero vector.
  */
  static bool SkippedZeroMotionPredCondition(MacroBlockH264* mb);

	/** Predict the macroblock motion vector.
	The prediction for the macroblock motion vector is the median of the 
	neighbouring macroblock vectors. Special conditions apply when the
	neighbours are outside of the image space or in another slice.
	@param mb					: Macroblock to set.
	@param mvpx				: Reference to returned predicted horiz component.
	@param mvpy				: Reference to returned predicted vert component.
	return						: None.
	*/
	static void GetMbMotionMedianPred(MacroBlockH264* mb, int* mvpx, int* mvpy);

	/** Calc the median of 3 numbers.
	@param x	:	1st num.
	@param y	:	2nd num.
	@param z	:	3rd num.
	@return		: The median of x, y and z.
	*/
	static int Median(int x, int y, int z);

	/** Copy the contents of blocks to the temp blocks.
	The block numbers in the parameter list refer to their array position.
	@param mb				: Macroblock to process.
	@param startBlk	: Position of first block to copy.
	@param endBlk		: Position of last block to copy.
	@return					: none.
	*/
	static void CopyBlksToTmpBlks(MacroBlockH264* mb, int startBlk, int endBlk);
	static void CopyBlksToTmpBlksCoeffOnly(MacroBlockH264* mb, int startBlk, int endBlk);

	/** Load macroblock from image.
	Copy the YCbCr values from the image colour components specified in the parameter list
	into the macroblock. It assumes that the image overlay origin is preset to the upper left
	corner for both Lum and Chr.
	@param mb				:	Macroblock to load.
	@param lum			: Lum img overlay.
	@param lumoffx	: X fffset for the Lum overlay.
	@param lumoffy	: Y offset for the Lum overlay.
	@param cb				: Cb img overlay.
	@param cr				: Cb img overlay.
	@param chroffx	: X offset for the Chr overlay.
	@param chroffy	: Y offset for the Chr overlay.
	*/
	static void LoadBlks(MacroBlockH264* mb, OverlayMem2Dv2* lum, int lumoffx, int lumoffy, OverlayMem2Dv2* cb, OverlayMem2Dv2* cr, int chroffx, int chroffy);

	/** Store macroblock to image.
	Copy the YCbCr values from the macroblock into the image colour components specified in the 
	parameter list. It assumes that the image overlay is of appropriate size.
	@param mb				:	Macroblock to store.
	@param lum			: Lum img overlay.
	@param lumoffx	: X fffset for the Lum overlay.
	@param lumoffy	: Y offset for the Lum overlay.
	@param cb				: Cb img overlay.
	@param cr				: Cb img overlay.
	@param chroffx	: X offset for the Chr overlay.
	@param chroffy	: Y offset for the Chr overlay.
	@param tmpBlkFlag	: Store from temp blocks.
	*/
	static void StoreBlks(MacroBlockH264* mb, OverlayMem2Dv2* lum, int lumoffx, int lumoffy, OverlayMem2Dv2* cb, OverlayMem2Dv2* cr, int chroffx, int chroffy, int tmpBlkFlag);

	/** Check for equality with another macroblock.
	Check all key members and main coeff blocks.
	@param me	: Macroblock to compare.
	@param mb	: Macroblock from which to compare.
	@return		: 1/0 = equal/not equal.
	*/
	int Equals(MacroBlockH264* mb) { return(EqualsProxy(this, mb)); }
	static int EqualsProxy(MacroBlockH264* me, MacroBlockH264* mb);

	/** Copy from another macroblock.
	Match the mem size and copy all members and macroblock
	data.
	@param pMb	: Macroblock to copy from.
	@return			: 1/0 = success/failure.
	*/
	int CopyMacroBlock(MacroBlockH264* pMb) { return(CopyMarcoBlockProxy(this, pMb)); }
	static int CopyMarcoBlockProxy(MacroBlockH264* pMbInto, MacroBlockH264* pMbFrom);

	/** Check for coded coeff.
	After SetCodedBlockPattern() method has been called the _coded_blk_pattern
	member reflects the 8x8 block group coded coeff status. However, for Intra
	macroblocks in Intra_16x16 modes, the Lum Dc block is not included and must
	be done seperately. NB: THIS METHOD MUST ONLY BE CALLED AFTER SetCodedBlockPattern()
	HAS BEEN CALLED ON THE MACROBLOCK. I.e. _coded_blk_pattern is valid.
	@param mb	: Macroblock to test.
	return		: 0/1 = All zero coeffs/At least one non-zero coeff.
	*/
	int HasNonZeroCoeffs(void) { return(HasNonZeroCoeffsProxy(this)); }
	static int HasNonZeroCoeffsProxy(MacroBlockH264* mb);

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
  int Distortion(OverlayMem2Dv2* p1Y, OverlayMem2Dv2* p1Cb, OverlayMem2Dv2* p1Cr, OverlayMem2Dv2* p2Y, OverlayMem2Dv2* p2Cb, OverlayMem2Dv2* p2Cr);

  /** Mark this macroblock onto the image.
  For debugging.
  @param  mb  : Macroblock to mark
  @param  lum : Lum image space.
  @param  cb  : Cb image space.
  @param  cr  : Cr image space.
  @return     : none.
  */
  static void Mark(MacroBlockH264* mb, OverlayMem2Dv2* lum, OverlayMem2Dv2* cb, OverlayMem2Dv2* cr);

  /** Dump the state variables of the macroblock to file.
  For debugging.
  @param  mb  : Macroblock to dump
  @param  filename  : File to write into
  @param  title     : Title of stored table data
  @return     : none.
  */
  static void Dump(MacroBlockH264* mb, char* filename, const char* title);

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
	/// Intra _intra4x4PredMode values.
	static const int Intra_4x4_Vert		          = 0;
	static const int Intra_4x4_Horiz	          = 1;
	static const int Intra_4x4_DC			          = 2;
	static const int Intra_4x4_Diag_Down_Left	  = 3;
	static const int Intra_4x4_Diag_Down_Right	= 4;
	static const int Intra_4x4_Vert_Right	      = 5;
	static const int Intra_4x4_Horiz_Down	      = 6;
	static const int Intra_4x4_Vert_Left	      = 7;
	static const int Intra_4x4_Horiz_Up	        = 8;
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
  int     _mbEncQP; ///< QP used to store the _mbQP value that was used for the encoding.

	/// Motion vectors derived from motion vector difference values.
	int			_mvX[16];
	int			_mvY[16];

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
	int			_distortion[87];	///< mbQP = {0...51} + {52..66 (Lum + Chr AC coeff)} + {67..70 (Chr DC)} + {71..86 (Lum DC)} extras.
	int			_rate[87];
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
	/// Temporary working blocks.
	BlockH264	_lumDcBlkTmp;				///< Index  = -1
	BlockH264 _lumBlkTmp[4][4];		///<			  = 0...15
	BlockH264 _cbDcBlkTmp;				///<				= 16	(NB: Chr instantiated as 4x4 and must be 2x2)
	BlockH264 _crDcBlkTmp;				///<				= 17
	BlockH264 _cbBlkTmp[2][2];		///<				= 18...21
	BlockH264 _crBlkTmp[2][2];		///<				= 22...25
	/// Block references and their associated parameters to iterate
	/// through during encoding and decoding.
	MBH264_CODING_STRUCT	_blkParam[MBH264_NUM_BLKS];

  /// Extensions for Vector Quantisation.
  int _vq_flag;
  int _vq_distortion;     ///< Total VQ distortion (squared error) for the mb.
  int _vq_LumCode[4][4];  ///< VQ table address.
  int _vq_CbCode[2][2];
  int _vq_CrCode[2][2];

};// end class MacroBlockH264.

#endif	//_MACROBLOCKH264_H
