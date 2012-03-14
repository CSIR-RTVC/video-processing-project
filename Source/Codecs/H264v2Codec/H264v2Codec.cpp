/** @file

MODULE        : H264v2Codec

TAG           : H264V2

FILE NAME     : H264v2Codec.cpp

DESCRIPTIONS  : A 2nd generation of video codecs based on the H264 standard
								implementation. The primary interface is ICodecv2 for access 
								and configuration.
                  
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

#include <cstdio>
#include <memory.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "H264v2Codec.h"

/// Implementations.
#include "BitStreamWriterMSB.h"
#include "BitStreamReaderMSB.h"

#include "RealRGB24toYUV420CCIR601ConverterVer16.h"
#include "RealYUV420toRGB24CCIR601ConverterVer16.h"	
#include "RealRGB24toYUV420ConverterImpl2Ver16.h"
#include "RealYUV420toRGB24ConverterImpl2Ver16.h"

#include "FastForward4x4ITImpl2.h"
#include "FastForward4x4ITImpl1.h"
#include "FastInverse4x4ITImpl1.h"
#include "FastForwardDC4x4ITImpl1.h"
#include "FastInverseDC4x4ITImpl1.h"
#include "FastForwardDC2x2ITImpl1.h"
#include "FastInverseDC2x2ITImpl1.h"
#include "FastForward4x4On16x16ITImpl1.h"
#include "FastInverse4x4On16x16ITImpl1.h"
#include "CAVLCH264Impl.h"
#include "CAVLCH264Impl2.h"

#include "MotionEstimatorH264ImplMultires.h"
#include "MotionEstimatorH264ImplMultiresCross.h"
#include "MotionCompensatorH264ImplStd.h"

#include "PrefixH264VlcEncoderImpl1.h"
#include "PrefixH264VlcDecoderImpl1.h"
#include "CoeffTokenH264VlcEncoder.h"
#include "CoeffTokenH264VlcDecoder.h"
#include "TotalZeros4x4H264VlcEncoder.h"
#include "TotalZeros4x4H264VlcDecoder.h"
#include "TotalZeros2x2H264VlcEncoder.h"
#include "TotalZeros2x2H264VlcDecoder.h"
#include "TotalZeros2x4H264VlcEncoder.h"
#include "TotalZeros2x4H264VlcDecoder.h"
#include "RunBeforeH264VlcEncoder.h"
#include "RunBeforeH264VlcDecoder.h"
#include "ExpGolombUnsignedVlcEncoder.h"
#include "ExpGolombUnsignedVlcDecoder.h"
#include "ExpGolombSignedVlcEncoder.h"
#include "ExpGolombSignedVlcDecoder.h"
#include "ExpGolombTruncVlcEncoder.h"
#include "ExpGolombTruncVlcDecoder.h"
#include "CodedBlkPatternH264VlcEncoder.h"
#include "CodedBlkPatternH264VlcDecoder.h"

/// For storing measurements during testing.
#include "MeasurementTable.h"

/*
---------------------------------------------------------------------------
  Codec parameter constants. 
---------------------------------------------------------------------------
*/
/// Picture type definitions. Values for _pictureCodingType member.
#define H264V2_INTRA				0
#define H264V2_INTER				1
#define H264V2_SEQ_PARAM		2
#define H264V2_PIC_PARAM		3

#define H264V2_I_PIC				0
#define H264V2_P_PIC				1
#define H264V2_PB_PIC				2
#define H264V2_B_PIC				3
#define H264V2_EI_PIC				4
#define H264V2_EP_PIC				5

#define H264V2_MAX_QP							51	///< Maximum QP value
/*
---------------------------------------------------------------------------
  Colour space constants. 
---------------------------------------------------------------------------
*/
#define H264V2_RGB24							0		///< 888 (Default).
#define H264V2_RGB32							1		///< 8888.
#define H264V2_RGB16							2		///< 565.

#define H264V2_YUV420P16					16	///< Planar Y, U then V (16 bits/component).
#define H264V2_YUV420P8						17	///< Planar Y, U then V (8 bits/component).

/*
--------------------------------------------------------------------------
  Macros. 
--------------------------------------------------------------------------
*/
#define H264V2_CLIP31(x)	( (((x) <= 31)&&((x) >= 1))? (x) : ( ((x) < 1)? 1:31 ) )
#define H264V2_CLIP51(x)	( (((x) <= 31)&&((x) >= 0))? (x) : ( ((x) < 0)? 0:51 ) )
#define H264V2_CLIP255(x)	( (((x) <= 255)&&((x) >= 0))? (x) : ( ((x) < 0)? 0:255 ) )

#define	H264V2_FAST_MINMAX
#undef	H264V2_EXACT_DMAX

#define H264V2_FAST_ABS32(x) ( ((x)^((x)>>31))-((x)>>31) )
#define H264V2_FAST_ABS16(x) ( ((x)^((x)>>15))-((x)>>15) )

/*
--------------------------------------------------------------------------
  Local constants. 
--------------------------------------------------------------------------
*/
const int		H264v2Codec::PARAMETER_LEN = 18;
const char*	H264v2Codec::PARAMETER_LIST[] = 
{
	"parameters",								        // 0
	"codecid",									        // 1
	"width",										        // 2
	"height",										        // 3
	"incolour",									        // 4
	"outcolour",								        // 5
	"picture coding type",			        // 6
	"last pic coding type",			        // 7
	"quality",									        // 8
	"autoipicture",							        // 9 
	"ipicturemultiplier",				        // 10
	"ipicturefraction",					        // 11
	"mode of operation",				        // 12
	"seq param set",						        // 13
	"pic param set",						        // 14
	"gen param set on open",		        // 15
  "prepend param sets to i-pictures", // 16
	"start code emulation prevention"	  // 17
};

const int		H264v2Codec::MEMBER_LEN = 4;
const char*	H264v2Codec::MEMBER_LIST[] = 
{
	"members",									// 0
	"macroblocks",							// 1
  "reference",                // 2
	"autoiframedetectflag"			// 3
};

/// Scaling is required for the DC coeffs to match the 4x4 
/// inverse IT scaling of the AC coeffs. Standard defines 
/// default Flat_4x4_16 for DC scaling.
const int H264v2Codec::dc4x4Scale[16] =
	{ 16, 16, 16, 16, 
		16, 16, 16, 16, 
		16, 16, 16, 16,
		16, 16, 16, 16 
	};
const int H264v2Codec::dc2x2Scale[4] =
	{ 16, 16, 
		16, 16 
	};

/// Test sampling point coordinates for Intra_16x16 prediction mode selection.
const int H264v2Codec::test16X[256] = { 3, 11,  3, 11, 
									1,  5,  9, 13,  1,  5,  9, 13,  1,  5,  9, 13,  1,  5,  9, 13, 
									3,  7, 11, 15,  0,  7, 15,  0,  3,  7, 11, 15,  0,  7, 15,  0,  3,  7, 11, 15,
									0,  2,  4,  6,  8, 10, 12, 14,  2,  4,  6,  8, 10, 12, 14,  2,  4,  6,  8, 10, 12, 14,  2,  4,  6,  8, 10, 12, 14,
									5,  9, 13,  1,  5,  9, 13,  2,  4,  6,  8, 10, 12, 14,  0,  3,  7, 11, 15,  1,  5,  9, 13,  2,  4,  6,  8, 10, 12, 14,  0,  3,  7, 11, 15,  1,  5,  9, 13,  2,  4,  6,  8, 10, 12, 14,  0,  3,  7, 11, 15,  5,  9, 13,
									1,  2,  4,  6,  8, 10, 12, 14,  0,  3,  7, 11, 15,  0,  3, 11, 15,  6, 14,  0,  1,  9, 15,  0,  3, 11, 15,  0,  3, 11, 15,  0,  1, 15,  6, 14,  0,  9, 15,  0,  3, 11, 15,  1,  2,  4,  6,  8, 10, 12, 14,
									2,  4,  6,  8, 10, 12, 14,  2,  4,  8, 10, 12,  2,  4,  6,  8, 10, 12, 14,  2,  4,  6,  8, 10, 12, 14,  2,  4,  6,  8, 10, 12, 14,  2,  4,  8, 10, 12,  2,  4,  6,  8, 10, 12, 14,
									1,  5,  7,  9, 13,  3,  5,  7, 11, 13,  1,  5,  7,  9, 13,  1,  5,  7,  9, 13,  3,  5,  7,  9, 11, 13,  1,  3,  5,  7, 11, 13,  1,  5,  7,  9, 13 };
const int H264v2Codec::test16Y[256] = { 3,  3, 11, 11, 
									1,  1,  1,  1,  5,  5,  5,  5,  9,  9,  9,  9, 13, 13, 13, 13, 
									0,  0,  0,  0,  3,  3,  3,  7,  7,  7,  7,  7, 11, 11, 11, 15, 15, 15, 15, 15,
									0,  2,  2,  2,  2,  2,  2,  2,  6,  6,  6,  6,  6,  6,  6, 10, 10, 10, 10, 10, 10, 10, 14, 14, 14, 14, 14, 14, 14,
									0,  0,  0,  3,  3,  3,  3,  4,  4,  4,  4,  4,  4,  4,  5,  5,  5,  5,  5,  7,  7,  7,  7,  8,  8,  8,  8,  8,  8,  8,  9,  9,  9,  9,  9, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 15, 15, 15,
									0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  2,  2,  2,  2,  3,  3,  4,  4,  4,  4,  6,  6,  6,  6,  8,  8,  8,  8, 10, 10, 10, 11, 11, 12, 12, 12, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 
									1,  1,  1,  1,  1,  1,  1,  3,  3,  3,  3,  3,  5,  5,  5,  5,  5,  5,  5,  7,  7,  7,  7,  7,  7,  7,  9,  9,  9,  9,  9,  9,  9, 11, 11, 11, 11, 11, 13, 13, 13, 13, 13, 13, 13,
									2,  2,  2,  2,  2,  4,  4,  4,  4,  4,  6,  6,  6,  6,  6,  8,  8,  8,  8,  8, 10, 10, 10, 10, 10, 10, 12, 12, 12, 12, 12, 12, 14, 14, 14, 14, 14 }; 
const int H264v2Codec::test16Limit[8] = { 4, 20, 40, 69, 123, 174, 219, 256 };

/// Test sampling point coordinates for Intra_8x8 prediction mode selection.			/// New H.264
const int H264v2Codec::test8X[64] = { 2, 5, 2, 5,
																			0, 4, 7, 0, 7, 0, 4, 7,
																			2, 5, 6, 1, 6, 0, 7, 3, 4, 3, 4, 0, 7, 1, 6, 2, 5,
																			1, 3, 0, 2, 3, 4, 5, 7, 1, 3, 4, 6, 0, 1, 2, 5, 6, 7, 1, 2, 5, 6, 1, 3, 4, 6, 0, 2, 3, 4, 5, 7, 1, 3, 6 };
const int H264v2Codec::test8Y[64] = { 2, 2, 5, 5,
																			0, 0, 0, 4, 4, 7, 7, 7,
																			0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7,
																			0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 7, 7, 7 };
const int H264v2Codec::test8Limit[4] = { 4, 12, 29, 64 };

/// Loop filter constants.
const int H264v2Codec::alpha[52]	= { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 5, 6, 7, 8, 9, 10, 12, 13, 15, 17, 20, 22, 25, 28, 32, 36, 40, 45, 50, 56, 63, 71, 80, 90, 101, 113, 127, 144, 162, 182, 203, 226, 255, 255 };
const int H264v2Codec::beta[52]		= { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 3, 3, 3, 3,  4,  4,  4,  6,  6,  7,  7,  8,  8,  9,  9, 10, 10, 11, 11, 12, 12, 13, 13,  14,  14,  15,  15,  16,  16,  17,  17,  18,  18 }; 
///                                           0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51
const int H264v2Codec::indexAbS[3][52]	= { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 6, 6, 7, 8, 9,10,11,13 }, 
																						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 5, 5, 6, 7, 8, 8,10,11,12,13,15,17 }, 
																						{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 6, 6, 7, 8, 9,10,11,13,14,16,18,20,23,25} };

/// Fast search on the macroblock distortion vs. QP curve changes the step size
/// depending on the QP value.
#ifdef H264V2_EXACT_DMAX
	const int H264v2Codec::MbStepSize[H264V2_MAX_QP+1] = 
		{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
#else
		const int H264v2Codec::MbStepSize[H264V2_MAX_QP+1] = 
			{//	0		1		2		3		4		5		6		7		8		9		10	11	12	13	14	15
					1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	 1,	 1,	 1,	 2,	 2,	 2,
			//	16	17	18	19	20	21	22	23	24	25	26	27	28	29	30	31
					2,	 2,	 2,	 2,	 2,	 2,	 2,	 2,	 2,	 3,	 3,	 3,	 3,	 3,	 3,	 3,
			//	32	33	34	35	36	37	38	39	40	41	42	43	44	45	46	47
					3,	 3,	 3,	 3,	 3,	 4,	 4,	 4,	 4,	 4,	 4,	 4,	 4,	 4,	 4,	 4,
			//	48	49	50	51  	
					4,	 4,	 4,	 4
			};
#endif

/*
--------------------------------------------------------------------------
  Construction and destruction. 
--------------------------------------------------------------------------
*/

H264v2Codec::H264v2Codec()
{
  ResetMembers();
}//end constructor.

void H264v2Codec::ResetMembers(void)
{
	_errorStr			 = "[H264v2Codec::ResetMembers] No error";
	_codecIsOpen	 = 0;
	_bitStreamSize = 0;

	/// Set default parameter values.
  _idCode														= (int)H264V2_ID;
  _width														= 176;
  _height														= 144;
	_inColour													= H264V2_RGB24;	///< Default to 24 bit rgb.
	_outColour												= H264V2_RGB24;
  _flip                             = false;
	_pictureCodingType								= H264V2_INTRA;
	_lastPicCodingType								= H264V2_INTRA;
	_pQuant														= 16;		///< Mid range [1..51].
	_autoIPicture											= 1;
	_iPictureMultiplier								= 1;
	_iPictureFraction									= 0;

	_modeOfOperation									= H264V2_OPEN;

	_currSeqParam											= 0;	///< Index reference into _seqParam[32] array.
	_currPicParam											= 0;	///< Index reference into _picParam[2] array.
	_genParamSetOnOpen								= 1;	///< Generate the seq/pic parameter sets on the Open() call.
  _prependParamSetsToIPic           = 1;  ///< Prepend a SPS and PPS NAL unit to the front of every I-Picture.

  _startCodeEmulationPrevention     = 1;  ///< Enable/disable start code emulation prevention in bit stream.

  /// Work input image.
  _lumWidth			= 0;
  _lumHeight		= 0;
  _pLum					= NULL;
	_chrWidth			= 0;
	_chrHeight		= 0;
  _pChrU				= NULL; 
  _pChrV				= NULL; 
	_Lum					= NULL;
	_Cb						= NULL;
	_Cr						= NULL;	
  /// Reference images.
  _pRLum				= NULL;
  _pRChrU				= NULL;
  _pRChrV				= NULL;
	_RefLum				= NULL;
	_RefCb				= NULL;
	_RefCr				= NULL;

	/// Temp work mem.
	_p16x16				= NULL;
	_16x16				= NULL;
	_p8x8_0				= NULL;
	_8x8_0				= NULL;
	_p8x8_1				= NULL;
	_8x8_1				= NULL;

	/// Colour converters.
	_pInColourConverter		= NULL;
	_pOutColourConverter	= NULL;
	/// Stream access.
	_pBitStreamWriter	= NULL;
	_pBitStreamReader	= NULL;
	/// IT transform filters.
	_pF4x4TLum		= NULL;
	_pF4x4TChr		= NULL;
	_pFDC4x4T			= NULL;
	_pFDC2x2T			= NULL;
	_pI4x4TLum		= NULL;
	_pI4x4TChr		= NULL;
	_pIDC4x4T			= NULL;
	_pIDC2x2T			= NULL;

	/// For motion estimation (and compensation).
	_motionFactor							= 2; ///< Default for abs diff algorithms.
	_prevMotionDistortion			= -1;
	_autoIFrameIncluded				= NULL;
	_pMotionEstimator					= NULL;
	_pMotionEstimationResult	= NULL;
	_pMotionCompensator				= NULL;
	_pMotionVectors						= NULL;

	/// Vlc encoders and decoders for use with CAVLC.
	_pPrefixVlcEnc						= NULL;
	_pPrefixVlcDec						= NULL;
	_pCoeffTokenVlcEnc				= NULL;		///< New H.264
	_pCoeffTokenVlcDec				= NULL;		///< New H.264
	_pTotalZeros4x4VlcEnc			= NULL;		///< New H.264
	_pTotalZeros4x4VlcDec			= NULL;		///< New H.264
	_pTotalZeros2x2VlcEnc			= NULL;		///< New H.264
	_pTotalZeros2x2VlcDec			= NULL;		///< New H.264
	_pRunBeforeVlcEnc					= NULL;		///< New H.264
	_pRunBeforeVlcDec					= NULL;		///< New H.264
	/// ... and the remaining vlc encoders and decoders.
	_pBlkPattVlcEnc						= NULL;		///< New H.264
	_pBlkPattVlcDec						= NULL;		///< New H.264
	_pDeltaQPVlcEnc						= NULL;		///< New H.264
	_pDeltaQPVlcDec						= NULL;		///< New H.264
	_pMbTypeVlcEnc						= NULL;		///< New H.264
	_pMbTypeVlcDec						= NULL;		///< New H.264
	_pMbIChrPredModeVlcEnc		= NULL;		///< New H.264
	_pMbIChrPredModeVlcDec		= NULL;		///< New H.264
	_pMbMotionVecDiffVlcEnc		= NULL;		///< New H.264
	_pMbMotionVecDiffVlcDec		= NULL;		///< New H.264
	/// The CAVLC codecs.
	_pCAVLC4x4								= NULL;		///< New H.264
	_pCAVLC2x2								= NULL;		///< New H.264
	/// General header vlc encoders and decoders.
	_pHeaderUnsignedVlcEnc		= NULL;		///< New H.264
	_pHeaderUnsignedVlcDec		= NULL;		///< New H.264
	_pHeaderSignedVlcEnc			= NULL;		///< New H.264
	_pHeaderSignedVlcDec			= NULL;		///< New H.264

	/// Macroblock data objects.
	_pMb				= NULL;
	_Mb					= NULL;

  /// Internal parameters.

	/// H.264 has inter prediction mechanisms that permit multiple refrence pictures/frames and
	/// control parameters to maintain a list of these references. However, in this implementation
	/// the GOP are IPPPPPP..... and all P-frames use only the previous frame for reference. No
	/// reference lists (List0) are maintained and frame number counting is consecutively incremented
	/// from the previous IDR-frame.
	_frameNum					= 0;				///< Used for SliceHeader._frame_num
	_maxFrameNum			= 1 << 16;	///< Used for and derived from SeqParamSet._log2_max_frame_num_minus4
	_idrFrameNum			= 0;				///< Used for SliceHeader._idr_pic_id

	/// Slice without partitioning and therefore the slice parameters are simple. (Only one slice/frame.)
	_slice._type			= SliceHeaderH264::I_Slice_All;	///< I-Slice/P-Slice.
	_slice._qp				= _pQuant;	                    ///< Same as _pQuant for this implementation.
	_slice._qp_delta	= 0;				                    ///< = 0 for this implementation.
	_slice._disable_deblocking_filter_idc = 0;
	_mb_skip_run			= 0;				                    ///< For P-Slices a skip run preceeds each macroblock.

	/// Image plane encoders/decoders.
	_pIntraImgPlaneEncoder	= NULL;
	_pInterImgPlaneEncoder	= NULL;
	_pIntraImgPlaneDecoder	= NULL;
	_pInterImgPlaneDecoder	= NULL;

}//end ResetMembers.

H264v2Codec::~H264v2Codec()
{
  // If open then close first before exiting.
  if(_codecIsOpen)
    Close();
}//end destructor.

/*
-----------------------------------------------------------------------
  Parameter interface functions.                                
-----------------------------------------------------------------------
*/
int	H264v2Codec::GetParameter(const char* type, int*	length, void*	value)
{

	char *p = (char *)type;
	int len	= (int)strlen(p);

	if( _strnicmp(p,"width",len) == 0 )
		_itoa(_width,(char *)value,10);
	else if( _strnicmp(p,"height",len) == 0 )
		_itoa(_height,(char *)value,10);
	else if( _strnicmp(p,"incolour",len) == 0 )
		_itoa(_inColour,(char *)value,10);
	else if( _strnicmp(p,"outcolour",len) == 0 )
		_itoa(_outColour,(char *)value,10);
  else if( strncmp(p,"flip",len) == 0 )
  {
    int i = _flip ? 1 : 0;
    sprintf((char *)value, "%d", i);
  }
	else if( _strnicmp(p,"picture coding type",len) == 0 )
		_itoa(_pictureCodingType,(char *)value,10);
	else if( _strnicmp(p,"last pic coding type",len) == 0 )
		_itoa(_lastPicCodingType,(char *)value,10);
	else if( _strnicmp(p,"quality",len) == 0 )
		_itoa(_pQuant,(char *)value,10);
	else if( _strnicmp(p,"autoipicture",len) == 0 )
		_itoa(_autoIPicture,(char *)value,10);
	else if( _strnicmp(p,"ipicturemultiplier",len) == 0 )
		_itoa(_iPictureMultiplier,(char *)value,10);
	else if( _strnicmp(p,"ipicturefraction",len) == 0 )
		_itoa(_iPictureFraction,(char *)value,10);
	else if( _strnicmp(p,"codecid",len) == 0 )
		_itoa(_idCode,(char *)value,10);
	else if( _strnicmp(p,"mode of operation",len) == 0 )
		_itoa(_modeOfOperation,(char *)value,10);
	else if( _strnicmp(p,"seq param set",len) == 0 )
		_itoa(_currSeqParam,(char *)value,10);
	else if( _strnicmp(p,"pic param set",len) == 0 )
		_itoa(_currPicParam,(char *)value,10);
	else if( _strnicmp(p,"generate param set on open",len) == 0 )
		_itoa(_genParamSetOnOpen,(char *)value,10);
	else if( _strnicmp(p,"prepend param sets to i-pictures",len) == 0 )
		_itoa(_prependParamSetsToIPic,(char *)value,10);
	else if( _strnicmp(p,"start code emulation prevention",len) == 0 )
		_itoa(_startCodeEmulationPrevention,(char *)value,10);
	else if( _strnicmp(p,"parameters",len) == 0 )
		_itoa(PARAMETER_LEN,(char *)value,10);
	else
	{
		_errorStr = "[H264v2Codec::GetParameter] Read parameter not supported";
		return(0);
	}//end else...

	*length = (int)strlen((char *)value);
	return(1);

	return(0);
}//end GetParameter.

void H264v2Codec::GetParameterName(int ordinal, const char** name, int* length)
{

	if( (ordinal < 0)||(ordinal >= PARAMETER_LEN) )
		return;
	*name		= (const char *)PARAMETER_LIST[ordinal];
	*length = (int)strlen(PARAMETER_LIST[ordinal]);

}//end GetParameterName.

int H264v2Codec::SetParameter(const char* type, const char* value)
{

	char *p = (char *)type;
	char *v = (char *)value;
	int  len	= (int)strlen(p);

	if( _strnicmp(p,"width",len) == 0 )
		_width = (int)(atoi(v));
	else if( _strnicmp(p,"height",len) == 0 )
		_height = (int)(atoi(v));
	else if( _strnicmp(p,"incolour",len) == 0 )
		_inColour = (int)(atoi(v));
	else if( _strnicmp(p,"outcolour",len) == 0 )
		_outColour = (int)(atoi(v));
  else if( strncmp(p,"flip",len) == 0 )
	{
		int i = (atoi)(v);
		_flip = (i != 0);
	}
	else if( _strnicmp(p,"picture coding type",len) == 0 )
		_pictureCodingType = (int)(atoi(v));
	else if( _strnicmp(p,"quality",len) == 0 )
		_pQuant = (int)(atoi(v));
	else if( _strnicmp(p,"autoipicture",len) == 0 )
		_autoIPicture = (int)(atoi(v));
	else if( _strnicmp(p,"ipicturemultiplier",len) == 0 )
		_iPictureMultiplier = (int)(atoi(v));
	else if( _strnicmp(p,"ipicturefraction",len) == 0 )
		_iPictureFraction = (int)(atoi(v));
	else if( _strnicmp(p,"mode of operation",len) == 0 )
		_modeOfOperation = (int)(atoi(v));
	else if( _strnicmp(p,"seq param set",len) == 0 )
		_currSeqParam = (int)(atoi(v));
	else if( _strnicmp(p,"pic param set",len) == 0 )
		_currPicParam = (int)(atoi(v));
	else if( _strnicmp(p,"generate param set on open",len) == 0 )
		_genParamSetOnOpen = (int)(atoi(v));
	else if( _strnicmp(p,"prepend param sets to i-pictures",len) == 0 )
		_prependParamSetsToIPic = (int)(atoi(v));
	else if( _strnicmp(p,"start code emulation prevention",len) == 0 )
		_startCodeEmulationPrevention = (int)(atoi(v));
	else
	{
		_errorStr = "[H264v2Codec::SetParameter] Write parameter not supported";
		return(0);
	}//end else...

	return(1);
}//end SetParameter.

/**
-----------------------------------------------------------------------
  Member interface functions.                                
-----------------------------------------------------------------------
*/
void* H264v2Codec::GetMember(const char* type, int* length)
{
	char*	p			= (char *)type;
	int		len		= (int)strlen(p);
	void* pRet	= NULL;
	*length			= 0;

	if( _strnicmp(p,"macroblocks",len) == 0 )
	{
		*length = _mbLength;
		pRet		= (void *)_pMb;
	}
	else if( _strnicmp(p,"autoiframedetectflag",len) == 0 )
	{
		*length = _mbLength;
		pRet		= (void *)_autoIFrameIncluded;
	}
	else if( _strnicmp(p,"reference",len) == 0 )
	{
		*length = (_lumWidth * _lumHeight) + 2*(_chrWidth * _chrHeight);
		pRet		= (void *)_pRLum;
	}
	else if( _strnicmp(p,"members",len) == 0 )
	{
		int numMembers	= (int)MEMBER_LEN;
		*length = 1;
		pRet		= (void *)(numMembers);	///< Note that MEMBER_LEN is converted to a pointer type.
	}//end else if members...
	else
		_errorStr = "[H264v2Codec::GetMember] Read member not supported";

	return(pRet);
}//end GetMember.

void H264v2Codec::GetMemberName(int ordinal, const char** name, int* length)
{

	if( (ordinal < 0)||(ordinal >= MEMBER_LEN) )
		return;
	*name		= (const char *)MEMBER_LIST[ordinal];
	*length = (int)strlen(MEMBER_LIST[ordinal]);

}//end GetMemberName.

int H264v2Codec::SetMember(const char* type, void* pValue)
{
	int	 i;
	char *p		= (char *)type;
	int  len	= (int)strlen(p);

	if( _strnicmp(p,"autoiframedetectflag",len) == 0 )
	{
		bool* pV = (bool *)pValue;
		for(i = 0; i < _mbLength; i++)
			_autoIFrameIncluded[i] = pV[i];
	}
	else
	{
		_errorStr = "[H264v2Codec::SetMember] Write member not supported";
		return(0);
	}//end else...

	return(1);
}//end SetMember.

/**
-----------------------------------------------------------------------
  Public Implementation.                                
-----------------------------------------------------------------------
*/
/** Open the codec for encoding/decoding.
The state of the codec and memory allocations are made in this method and are 
entirely dependent on the codec parameters. Using SetParameter() all parmeters
must be set prior to calling this methods.
@return : 1 = success, 0 = failure.
*/
int H264v2Codec::Open(void)
{
  int i;

  /// If already open then close first before continuing.
  if(_codecIsOpen)
    Close();

	/// --------------- Configure Sequence & Picture parameter sets -----------------
	/// The _genParamSetOnOpen parameter determines whether or not the seq/pic params 
	/// are generated and set in this call to Open(). If the param sets are to be generated 
	/// then the SetParameters() method calls must have set the appropriate codec params 
	/// prior to calling Open()as these settings are dependent on the codec parameters.
	/// If the param sets are not to be generated then the codec params must be extracted
	/// from the seq/pic param sets.
	if(_genParamSetOnOpen)
	{
		/// Sequence parameter set for Baseline profile.
		if(!SetSeqParamSet(_currSeqParam))
		{
			_errorStr = "[H264Codec::Open] Cannot set sequence parameter set";
			Close();
			return(0);
		}//end if !SetSeqParamSet...

		/// Picture parameter set.
		if(!SetPicParamSet(_currPicParam, _currSeqParam))
		{
			_errorStr = "[H264Codec::Open] Cannot set picture parameter set";
			Close();
			return(0);
		}//end if !SetPicParamSet...
	}//end if _genParamSetOnOpen...
	else
	{
		if(!GetCodecParams(_currPicParam))
		{
			/// Error string is set in the method.
			Close();
			return(0);
		}//end if !GetCodecParams...
	}//end else...

	/// --------------- Create encoded SPS and PPS streams ----------------------------------------
  /// Every I-Pic will have an SPS NAL, PPS NAL and an IDR NAL concatenated together in a single 
  /// stream. Create a cached param set for the current selection to use in the Code() method.
  if(_prependParamSetsToIPic)
  {
    int tempPicCodingType = _pictureCodingType;

    _pictureCodingType = H264V2_SEQ_PARAM;
    if(!CodeNonPicNALTypes((void *)_pEncSeqParam, H264V2_ENC_PARAM_LEN * 8))
		{
			/// Error string is set in the method.
			Close();
			return(0);
		}//end if !CodeNonPicNALTypes...
    _encSeqParamByteLen = GetCompressedByteLength();

    _pictureCodingType = H264V2_PIC_PARAM;
    if(!CodeNonPicNALTypes((void *)_pEncPicParam, H264V2_ENC_PARAM_LEN * 8))
		{
			/// Error string is set in the method.
			Close();
			return(0);
		}//end if !CodeNonPicNALTypes...
    _encPicParamByteLen = GetCompressedByteLength();

    /// Restore state.
    _pictureCodingType = tempPicCodingType;

  }//end if _prependParamSetsToIPic...

	/// --------------- Alloc image memory ----------------------------------------------
  /// Create an image memory space.
  _lumWidth		= _width;
  _lumHeight	= _height;
	_chrWidth		= _width/2;
	_chrHeight	= _height/2;

	/// Alloc a large contiguous block with Lum first followed by ChrU then ChrV
	/// for both image blocks of input and reference.
	int lumSize = _lumWidth * _lumHeight;
	int chrSize = _chrWidth * _chrHeight;
	int imgSize = lumSize + 2*chrSize;
	/// In/Out and ref images with primary lum at the head.
  _pLum = new short[2 * imgSize];
  if(!_pLum)
  {
    _errorStr = "[H264Codec::Open] Image memory unavailable";
    Close();
	  return(0);
  }//end if !_pLum...
	/// Place each image and colour component head pointer.
	_pChrU		= &(_pLum[lumSize]);												///< End of _pLum.
	_pChrV		= &(_pLum[lumSize + chrSize]);							///< End of _pChrU.
	_pRLum		= &(_pLum[imgSize]);												///< End of input image.
	_pRChrU		= &(_pLum[imgSize + lumSize]);							///< End of image and _pRLum.
	_pRChrV		= &(_pLum[imgSize + lumSize + chrSize]);		///< End of _pRChrU.

  /// Zero the reference and the previous input image spaces. Note that the mem
	/// is contiguous.
	memset((void *)_pLum, 0, 2 * imgSize * sizeof(short));

	/// --------------- Configure the overlays to the img mem -------------------------
	/// The encoding/decoding of the residual image is performed on 4x4 blocks within
	/// each macroblock of the in/out and ref images.
	_Lum		= new OverlayMem2Dv2(_pLum, _lumWidth, _lumHeight, 16, 16);
	_RefLum	= new OverlayMem2Dv2(_pRLum, _lumWidth, _lumHeight, 16, 16);
	_Cb			= new OverlayMem2Dv2(_pChrU, _chrWidth, _chrHeight, 8, 8);
	_RefCb	= new OverlayMem2Dv2(_pRChrU, _chrWidth, _chrHeight, 8, 8);
	_Cr			= new OverlayMem2Dv2(_pChrV, _chrWidth, _chrHeight, 8, 8);
	_RefCr	= new OverlayMem2Dv2(_pRChrV, _chrWidth, _chrHeight, 8, 8);
	if( (_Lum == NULL)||(_RefLum == NULL)||(_Cb == NULL)||							
			(_RefCb == NULL)||(_Cr == NULL)||(_RefCr == NULL) )
  {
    _errorStr = "[H264Codec::Open] Cannot instantiate image and reference overlay objects";
    Close();
	  return(0);
  }//end if !_Lum...

	/// --------------- Alloc and configure prediction mem data objects ---------------------
	_p16x16 = new short[256];	///< 16x16 block for predicition operations.
	_16x16	= new OverlayMem2Dv2(_p16x16, 16, 16, 16, 16);
	_p8x8_0 = new short[64];	///< 8x8 blocks for predicition operations.
	_8x8_0	= new OverlayMem2Dv2(_p8x8_0, 8, 8, 8, 8);
	_p8x8_1 = new short[64];
	_8x8_1	= new OverlayMem2Dv2(_p8x8_1, 8, 8, 8, 8);

  if( (_p16x16 == NULL)||(_16x16 == NULL) ||																
			(_p8x8_0 == NULL)||(_8x8_0 == NULL) || 
			(_p8x8_1 == NULL)||(_8x8_1 == NULL) )
  {
    _errorStr = "[H264Codec::Open] Cannot create prediction memory objects";
    Close();
	  return(0);
  }//end if !_p16x16...

	/// --------------- Configure Macroblock data objects -----------------------------
	int mbWidth		= _lumWidth/16;		///< Note: Assuming mod 16 image dimensions (potential problem).
	int mbHeight	= _lumHeight/16;
	_mbLength			= mbWidth * mbHeight;

	_pMb = new MacroBlockH264[_mbLength];
	_Mb  = new MacroBlockH264*[mbHeight];	///< Address array.

	/// Only specified macroblocks are included in the detection of an I-frame. This
	/// flag list is used to indicate that inclusion.
	_autoIFrameIncluded = new bool[_mbLength];

	if( (_pMb == NULL)||(_Mb == NULL)||(_autoIFrameIncluded == NULL) )
  {
    _errorStr = "[H264Codec::Open] Cannot instantiate macroblock data objects";
    Close();
	  return(0);
  }//end if !_pMB...

	for(i = 0; i < mbHeight; i++)	///< Load the address array.
		_Mb[i] = &(_pMb[i * mbWidth]);

	/// Load the macroblock image mem 2-D offsets, indices and neighbourhood variables. There
	/// is only one slice (slice num = 0) and therefore extends from macroblock index 0..._mbLength-1.
	MacroBlockH264::Initialise(mbHeight, mbWidth, 0, _mbLength-1, 0, _Mb);

	/// Load the flag for each macroblock that includes/excludes it from the 
	/// auto I-frame test during motion estimation. Default to include all.
	for(i = 0; i < _mbLength; i++)
		_autoIFrameIncluded[i] = 1;

	/// --------------- Configure colour converters ---------------------------------
	if(_inColour == H264V2_RGB24)
	{
		/// Encoder input.
#ifdef _CCIR601
		_pInColourConverter = new RealRGB24toYUV420CCIR601ConverterVer16(_width, _height, 128);
#else
		_pInColourConverter = new RealRGB24toYUV420ConverterImpl2Ver16(_width, _height, 128);
#endif

		if(_pInColourConverter == NULL)
		{
			_errorStr = "[H264Codec::Open] Cannot instantiate input colour converter";
			Close();
			return(0);
		}//end if !_pInColourConverter...

    /// the calling code is responsible for the flipping of the image
		_pInColourConverter->SetFlip(_flip);
	}//end if _inColour...

	if(_outColour == H264V2_RGB24)
	{
		/// Decoder output.
#ifdef _CCIR601
			_pOutColourConverter = new RealYUV420toRGB24CCIR601ConverterVer16(_width, _height);
#else
			_pOutColourConverter = new RealYUV420toRGB24ConverterImpl2Ver16(_width, _height, 128);
#endif

		if(_pOutColourConverter == NULL)
		{
			_errorStr = "[H264Codec::Open] Cannot instantiate output colour converter";
			Close();
			return(0);
		}//end if !_pOutColourConverter...

    /// the calling code is responsible for the flipping of the image
		_pOutColourConverter->SetFlip(_flip);
	
  }//end if _outColour...

	/// --------------- Instantiate IT filters ------------------------------------
	/// Create Integer Transformers (IT) and their inverses for AC and DC coeffs. The
	/// quantisers are included in the IT transform classes.
	_pF4x4TLum  = new FastForward4x4ITImpl2();
	_pF4x4TChr  = new FastForward4x4ITImpl2();
	_pFDC4x4T   = new FastForwardDC4x4ITImpl1();
	_pFDC2x2T   = new FastForwardDC2x2ITImpl1();
	_pI4x4TLum  = new FastInverse4x4ITImpl1();
	_pI4x4TChr  = new FastInverse4x4ITImpl1();
	_pIDC4x4T   = new FastInverseDC4x4ITImpl1();
	_pIDC2x2T   = new FastInverseDC2x2ITImpl1();

	if( (_pF4x4TLum == NULL)||(_pF4x4TChr == NULL)||(_pFDC4x4T == NULL)||(_pFDC2x2T == NULL)||(_pI4x4TLum == NULL)||(_pI4x4TChr == NULL)||(_pIDC4x4T == NULL)||(_pIDC2x2T == NULL) )
  {
    _errorStr = "[H264Codec::Open] Cannot instantiate Integer Transform filter objects";
    Close();
	  return(0);
  }//end if !_pF4x4TLum...

	/// Set default modes and added scaling for IT filters.
	_pF4x4TLum->SetMode(IForwardTransform::TransformOnly);
	_pF4x4TChr->SetMode(IForwardTransform::TransformOnly);
	_pFDC4x4T->SetMode(IForwardTransform::TransformAndQuant);
	_pFDC2x2T->SetMode(IForwardTransform::TransformAndQuant);
	_pI4x4TLum->SetMode(IInverseTransform::TransformOnly);
	_pI4x4TChr->SetMode(IInverseTransform::TransformOnly);
	_pIDC4x4T->SetMode(IInverseTransform::TransformAndQuant);
	_pIDC2x2T->SetMode(IInverseTransform::TransformAndQuant);

	// --------------- Create the Vlc encoders and decoders --------------------------
	/// Create the vlc encoders and decoders for use with CAVLC.
	_pPrefixVlcEnc					= new PrefixH264VlcEncoderImpl1();
	_pPrefixVlcDec					= new PrefixH264VlcDecoderImpl1();
	_pCoeffTokenVlcEnc			= new CoeffTokenH264VlcEncoder();
	_pCoeffTokenVlcDec			= new CoeffTokenH264VlcDecoder();
	_pTotalZeros4x4VlcEnc		= new TotalZeros4x4H264VlcEncoder();
	_pTotalZeros4x4VlcDec		= new TotalZeros4x4H264VlcDecoder();
	_pTotalZeros2x2VlcEnc		= new TotalZeros2x2H264VlcEncoder();
	_pTotalZeros2x2VlcDec		= new TotalZeros2x2H264VlcDecoder();
	_pRunBeforeVlcEnc				= new RunBeforeH264VlcEncoder();
	_pRunBeforeVlcDec				= new RunBeforeH264VlcDecoder();

	/// Vlc encoder and decoder for the coded block pattern.
	_pBlkPattVlcEnc	= new CodedBlkPatternH264VlcEncoder();
	_pBlkPattVlcDec	= new CodedBlkPatternH264VlcDecoder();

	/// Vlc encoder and decoder for the delta QP.
	_pDeltaQPVlcEnc	= new ExpGolombSignedVlcEncoder();
	_pDeltaQPVlcDec	= new ExpGolombSignedVlcDecoder();

	/// Vlc encoder and decoder for the macroblock type.
	_pMbTypeVlcEnc	= new ExpGolombUnsignedVlcEncoder();
	_pMbTypeVlcDec	= new ExpGolombUnsignedVlcDecoder();

	/// Vlc encoder and decoder for intra chr pred mode. ExpGolomb codecs
	/// are stateless therefore they can be reused.
	_pMbIChrPredModeVlcEnc	= _pMbTypeVlcEnc;
	_pMbIChrPredModeVlcDec	= _pMbTypeVlcDec;

	/// Vlc encoder and decoder for motion vector differences. ExpGolomb codecs
	/// are stateless therefore they can be reused.
	_pMbMotionVecDiffVlcEnc	= _pDeltaQPVlcEnc;
	_pMbMotionVecDiffVlcDec	= _pDeltaQPVlcDec;

	/// Vlc encoder and decoder for general headers. ExpGolomb codecs
	/// are stateless therefore they can be reused.
	_pHeaderUnsignedVlcEnc	= _pMbTypeVlcEnc;	
	_pHeaderUnsignedVlcDec	= _pMbTypeVlcDec;
	_pHeaderSignedVlcEnc		= _pDeltaQPVlcEnc;
	_pHeaderSignedVlcDec		= _pDeltaQPVlcDec;

	if( (_pPrefixVlcEnc == NULL)||(_pPrefixVlcDec == NULL)||
			(_pCoeffTokenVlcEnc == NULL)||(_pCoeffTokenVlcDec == NULL) ||
			(_pTotalZeros4x4VlcEnc == NULL)||(_pTotalZeros4x4VlcDec == NULL) ||
			(_pTotalZeros2x2VlcEnc == NULL)||(_pTotalZeros2x2VlcDec == NULL) ||
			(_pRunBeforeVlcEnc == NULL)||(_pRunBeforeVlcDec == NULL) ||
			(_pBlkPattVlcEnc == NULL)||(_pBlkPattVlcDec == NULL) ||
			(_pDeltaQPVlcEnc == NULL)||(_pDeltaQPVlcDec == NULL) ||
			(_pMbTypeVlcEnc == NULL)||(_pMbTypeVlcDec == NULL) )
  {
		_errorStr = "[H264Codec::Open] Cannot instantiate Vlc codec objects";
    Close();
	  return(0);
  }//end if !_pPrefixVlcEnc...

	/// Create a CAVLC encoder and decoder for each coeff block size.
	_pCAVLC4x4 = new CAVLCH264Impl();
	_pCAVLC2x2 = new CAVLCH264Impl();
	if( (_pCAVLC4x4 == NULL)||(_pCAVLC2x2 == NULL) )
  {
    _errorStr = "[H264Codec::Open] Cannot instantiate CAVLC codec objects";
    Close();
	  return(0);
  }//end if !_pCAVLC4x4...

	/// Attach the vlc encoders and decoders to the associated CAVLC.
	_pCAVLC4x4->SetMode(CAVLCH264Impl::Mode4x4);
	((CAVLCH264Impl *)_pCAVLC4x4)->SetTokenCoeffVlcEncoder(_pCoeffTokenVlcEnc);
	((CAVLCH264Impl *)_pCAVLC4x4)->SetTokenCoeffVlcDecoder(_pCoeffTokenVlcDec);
	((CAVLCH264Impl *)_pCAVLC4x4)->SetPrefixVlcEncoder(_pPrefixVlcEnc);
	((CAVLCH264Impl *)_pCAVLC4x4)->SetPrefixVlcDecoder(_pPrefixVlcDec);
	((CAVLCH264Impl *)_pCAVLC4x4)->SetRunBeforeVlcEncoder(_pRunBeforeVlcEnc);
	((CAVLCH264Impl *)_pCAVLC4x4)->SetRunBeforeVlcDecoder(_pRunBeforeVlcDec);
	((CAVLCH264Impl *)_pCAVLC4x4)->SetTotalZerosVlcEncoder(_pTotalZeros4x4VlcEnc);
	((CAVLCH264Impl *)_pCAVLC4x4)->SetTotalZerosVlcDecoder(_pTotalZeros4x4VlcDec);

	_pCAVLC2x2->SetMode(CAVLCH264Impl::Mode2x2);
	((CAVLCH264Impl *)_pCAVLC2x2)->SetTokenCoeffVlcEncoder(_pCoeffTokenVlcEnc);
	((CAVLCH264Impl *)_pCAVLC2x2)->SetTokenCoeffVlcDecoder(_pCoeffTokenVlcDec);
	((CAVLCH264Impl *)_pCAVLC2x2)->SetPrefixVlcEncoder(_pPrefixVlcEnc);
	((CAVLCH264Impl *)_pCAVLC2x2)->SetPrefixVlcDecoder(_pPrefixVlcDec);
	((CAVLCH264Impl *)_pCAVLC2x2)->SetRunBeforeVlcEncoder(_pRunBeforeVlcEnc);
	((CAVLCH264Impl *)_pCAVLC2x2)->SetRunBeforeVlcDecoder(_pRunBeforeVlcDec);
	((CAVLCH264Impl *)_pCAVLC2x2)->SetTotalZerosVlcEncoder(_pTotalZeros2x2VlcEnc);
	((CAVLCH264Impl *)_pCAVLC2x2)->SetTotalZerosVlcDecoder(_pTotalZeros2x2VlcDec);

	// --------------- Configure bit stream access -----------------------------------
	_pBitStreamWriter = new BitStreamWriterMSB();
	_pBitStreamReader = new BitStreamReaderMSB();
	if( (_pBitStreamWriter == NULL)||(_pBitStreamReader == NULL) )
  {
    _errorStr = "[H264Codec::Open] Cannot instantiate bit stream access objects";
    Close();
	  return(0);
  }//end if !_pBitStreamWriter...

	/// --------------- Configure motion estimators -----------------------------------
	
	/// Select an appropriate motion estimator.
	int motionVectorRange;	///< In 1/4 pel units but also required for motion compensator in full pel units.
	if( (_width <= 352)&&(_height <= 288) )
		motionVectorRange = 256;	///< [-64.00 ... 63.75]
//		motionVectorRange = 128;	///< [-32.00 ... 31.75]
	else if( (_width <= 704)&&(_height <= 576) )
		motionVectorRange = 256;	///< [-64.00 ... 63.75]
	else if( (_width <= 1408)&&(_height <= 1152) )	///< These next 2 are ridiculous!
		motionVectorRange = 512;	///< [-128.00 ... 127.75]
	else
		motionVectorRange = 1024;	///< [-256.00 ... 255.75]

	/// Fast less accurate estimator.
	_pMotionEstimator = new MotionEstimatorH264ImplMultiresCross(	(const void *)_pLum,	///< Multi res estimation.
																																(const void *)_pRLum,
																																_lumWidth,
																																_lumHeight,
																																motionVectorRange, ///< In 1/4 pel units.
																																_autoIFrameIncluded);

	/// Slow more accurate multiresolution estimator.
//	_pMotionEstimator = new MotionEstimatorH264ImplMultires((const void *)_pLum,	///< Multi res estimation.
//																													(const void *)_pRLum,
//																													_lumWidth,
//																													_lumHeight,
//																													motionVectorRange, ///< In 1/4 pel units
//																													_autoIFrameIncluded);

	if(_pMotionEstimator != NULL)
	{
		/// Implementation specific modes.
		_pMotionEstimator->SetMode(0);	///< Auto mode.

		//_motionFactor = 2;	///< Abs diff algorithm.
		_motionFactor = 4;	///< Sqr err algorithm.

		if(!_pMotionEstimator->Create())
		{
			_errorStr = "[H264Codec::Open] Cannot create motion estimator";
			Close();
			return(0);
		}//end if !Create...
	}//end if _pMotionEstimator...
	else
  {
    _errorStr = "[H264Codec::Open] Cannot instantiate motion estimator object";
    Close();
	  return(0);
  }//end if else...

	/// --------------- Configure motion compensator -----------------------------------
	/// The motion compensator requires the range param to be in full pel units.	
	_pMotionCompensator = new MotionCompensatorH264ImplStd(motionVectorRange/4);

	if(_pMotionCompensator != NULL)
	{
		if(!_pMotionCompensator->Create((void *)_pRLum, _lumWidth, _lumHeight, 16, 16)) 
		{
			_errorStr = "[H264Codec::Open] Cannot create motion compensator";
			Close();
			return(0);
		}//end if !Create...
	}//end if _pMotionCompensator...
	else
  {
    _errorStr = "[H264Codec::Open] Cannot instantiate motion compensator object";
    Close();
	  return(0);
  }//end if else...

	/// Create a motion vector list to hold the decoded vectors for the compensation process.
	_pMotionVectors = new VectorStructList(VectorStructList::SIMPLE2D);
	if(!_pMotionVectors)
  {
    _errorStr = "[H264Codec::Open] Cannot create motion vector list object";
    Close();
    return(0);
  }//end if !_pMotionVectors...
	if(!_pMotionVectors->SetLength(mbWidth * mbHeight)) ///< One per Mb for 16x16 vectors only.
  {
    _errorStr = "[H264Codec::Open] Insufficient mem for motion vector list";
    Close();
    return(0);
  }//end if !SetLength...

	/// --------------- Create image plane encoders and decoders ----------------------
	/// Select an Intra Decoder.
	_pIntraImgPlaneDecoder = new IntraImgPlaneDecoderImplStdVer1(this);
	/// Set the Inter decoder.
	_pInterImgPlaneDecoder = new InterImgPlaneDecoderImplStdVer1(this);

	/// Select an Intra and Inter Encoder depending on the mode.
	if(_modeOfOperation == H264V2_OPEN)
	{
		_pIntraImgPlaneEncoder = new IntraImgPlaneEncoderImplStdVer1(this);
		_pInterImgPlaneEncoder = new InterImgPlaneEncoderImplStdVer1(this);
	}//end if _modeOfOperation...
	else		///< if(_modeOfOperation == H264V2_MB_QP_MINMAX_ADAPTIVE)
	{
    _pQuant = 26; ///< Start in the middle before adaptation of each mb QP value.
		_pIntraImgPlaneEncoder = new IntraImgPlaneEncoderImplMinMax(this);
		_pInterImgPlaneEncoder = new InterImgPlaneEncoderImplMinMax(this);
	}//end else...

	if( (_pIntraImgPlaneEncoder == NULL)||(_pIntraImgPlaneDecoder == NULL)||
			(_pInterImgPlaneEncoder == NULL)||(_pInterImgPlaneDecoder == NULL) )
  {
    _errorStr = "[H264Codec::Open] Cannot instantiate image plane encoder and decoder objects";
    Close();
	  return(0);
  }//end if !_pIntraImgPlaneEncoder...
	if( (!_pIntraImgPlaneEncoder->Create(_mbLength))||
			(!_pInterImgPlaneEncoder->Create(_mbLength)) )
  {
    _errorStr = "[H264Codec::Open] Cannot create image plane encoders";
    Close();
	  return(0);
  }//end if !_pIntraImgPlaneEncoder...
	
	/// Start at the beginning.
	_lastPicCodingType		= H264V2_INTRA;
	_prevMotionDistortion = -1;
	_maxFrameNum					= 1 << (_seqParam[_currSeqParam]._log2_max_frame_num_minus4 + 4);	///< Max limit (modulus) for _frameNum.
	_idrFrameNum					= 0;	///< The starting I-frame counter incremented with every consecutive I-frame.
	Restart();

  _errorStr						= "[H264Codec::Open] No Erorr";
  _codecIsOpen				= 1;
  return(1);
}//end Open.

/** Encode one frame of pels to a compressed stream.
The input source pels in the format specified by "incolour" parameter are compressed
to the output stream. The codeParameter specifies either the max size in bits of the 
pCmp buffer or the number of bits to target during the compression process. The interpretation
is determined by the "mode of operation" parameter. For SPS and PPS encoding the input 
pSrc is ignored.
@param  pSrc          : Input raw pels of one complete frame.
@param  pCmp          : Output compressed stream buffer.
@param  codeParameter : Mode of operation based bit size limits.
@return               : 1 = success, 0 = failure.
*/
int	H264v2Codec::Code(void* pSrc, void* pCmp, int codeParameter)
{
	int allowedBits, bitsUsed;

	if((_pictureCodingType != H264V2_INTRA)&&(_pictureCodingType != H264V2_INTER))
		return(CodeNonPicNALTypes(pCmp, codeParameter));

  if(!_codecIsOpen)
  {
		_errorStr = "[H264V2::Code] Codec is not open";
    return(0);
  }//end if !_codecIsOpen...

  /// Interpret the code parameter as a frame bit limit.
  int frameBitLimit	= codeParameter;
  int runOutOfBits	= 0;
  int bitLimit			= frameBitLimit - 0; ///< Allow some slack for the expected trailing picture bits.
  _bitStreamSize		= 0;

	/// Reset the stream writer with the input param interpreted as the frame bit limit. 
	_pBitStreamWriter->SetStream(pCmp, codeParameter);

	/// Only IDR and P pictures are supported.
	if((_pictureCodingType != H264V2_INTRA)&&(_pictureCodingType != H264V2_INTER))
	{
		_errorStr = "[H264v2Codec::Code] Picture coding type not supported";
		return(0);
	}//end if !H264V2_INTRA...

	/// Convert the colour space of the input image.
	if(_inColour == H264V2_YUV420P16)	      ///< The natural colour space of the encoder with type = short.
		memcpy( (void *)_pLum, (const void *)pSrc, ((_lumWidth * _lumHeight) + 2*(_chrWidth * _chrHeight)) * sizeof(short));
	else if(_inColour == H264V2_YUV420P8)  ///< ...type = byte.
  {
    int colLen = (_lumWidth * _lumHeight) + 2*(_chrWidth * _chrHeight);
    for(int i = 0; i < colLen; i++)
      _pLum[i] = (short) ((unsigned char *)pSrc)[i];
  }//end if H264V2_YUV420P8...
	else
		_pInColourConverter->Convert((void *)pSrc, (void *)_pLum, (void *)_pChrU, (void *)_pChrV);

	/// Motion estimation is used to determine if an IDR frame should be inserted.
	if(_pictureCodingType == H264V2_INTER)
	{
		/// Motion estimation.
		long motionDistortion  = 0;

		/// The estimator was chosen in Open() depending on the mode selected.
		_pMotionEstimationResult = (VectorStructList *)(_pMotionEstimator->Estimate(&motionDistortion));

		/// The estimation results are processed into an encoded structure list. A 
		/// decision is made on the type of encoding as predictive or basic and 
		/// returns the selection. The _Motion member reflects the choice.
			
		/// Auto I-picture detection.
		if(_autoIPicture && (_prevMotionDistortion != -1)) ///< Previous frame was not an I-Picture.
		{
			/// Test for an I-picture.
			if( motionDistortion > (_motionFactor * _prevMotionDistortion) )
				_pictureCodingType = H264V2_INTRA;
		}///end if _prevMotionDistortion...

		_prevMotionDistortion = motionDistortion;

	}//end if H264V2_INTER...

	/// The motion estimation process is used to determine if this picture
	/// should rather be coded as an IDR picture. Therefore the INTRA 
	/// picture type coding is done afterwards. 
	
	/// For INTRA pictures and before any encoding begins readjust the 
	/// allowable bits based on the I-picture multiplier parameters.
	if(_pictureCodingType == H264V2_INTRA)
	{
		/// Continuously divide by 10 until in the range 0..9.
		while(_iPictureFraction > 10)
			_iPictureFraction = (_iPictureFraction/10);
		/// Readjust frame bits with multiplier.fraction factor.
		frameBitLimit = (frameBitLimit * _iPictureMultiplier) + ((frameBitLimit * _iPictureFraction)/10);
		_pBitStreamWriter->SetStreamBitSize(frameBitLimit);
		/// New bit limit setting.
		bitLimit = frameBitLimit - 0; ///< Allow some slack for the expected trailing picture bits.
    /// Reset the frame number for I-pics.
    _frameNum = 0;

    /// Prepend SPS and PPS to the I-picture.
    if(_prependParamSetsToIPic)
    {
	    allowedBits	= bitLimit - _bitStreamSize;
      int paramTotBitLen = 8 * (_encSeqParamByteLen + _encPicParamByteLen);
      if(allowedBits < paramTotBitLen)
      {
		    _errorStr = "[H264V2Codec::Code] Cannot prepend SPS and PPS to I-Picture stream";
        return(0);
      }//end if allowedBits...

      /// Write the pre-encoded SPS to the stream.
      int i;
      for(i = 0; i < _encSeqParamByteLen; i++)  ///< One byte at a time.
        _pBitStreamWriter->Write(8,_pEncSeqParam[i]);

      /// Write the pre-encoded PPS to the stream.
      for(i = 0; i < _encPicParamByteLen; i++)  ///< One byte at a time.
        _pBitStreamWriter->Write(8,_pEncPicParam[i]);

      _bitStreamSize += paramTotBitLen;

    }//end if _prependParamSetsToIPic...
	}//end if H264V2_INTRA...

  /// Write the 32-bit start code 0x00000001 to the stream.
	allowedBits		= bitLimit - _bitStreamSize;
  if(allowedBits < 32)
  {
		_errorStr = "[H264V2Codec::Code] Cannot write start code to stream";
    return(0);
  }//end if allowedBits...
  _pBitStreamWriter->Write(32,1);
  _bitStreamSize += 32;

	/// Define the NAL unit header based on the seleceted picture coding type. The final NAL type
	/// is only known at this point in the process.
	if(_pictureCodingType == H264V2_INTER)
  {
	  _nal._ref_idc	= 2;  ///< Non-zero for all referenced frames P and I.
		_nal._unit_type = NalHeaderH264::NonIDR_NoPartition_Slice;
  }//end if H264V2_INTER...
	else	///< if(_pictureCodingType == H264V2_INTRA)
  {
	  _nal._ref_idc	= 3;  ///< Non-zero for all referenced frames P and I.
		_nal._unit_type = NalHeaderH264::IDR_Slice;
  }//end else...

	/// Write the NAL header to the stream. 
	allowedBits		= bitLimit - _bitStreamSize;
	runOutOfBits	= WriteNALHeader(_pBitStreamWriter, allowedBits, &bitsUsed);
	_bitStreamSize += bitsUsed;
  if(runOutOfBits) ///< or if(== 2) An error has occured.
    return(0);

	/// Write (concatinate) the slice header layer with its header flags to
	/// the stream. There is only one slice so the header, macroblocks (slice
	/// data) and tail may be coded in a linear order.
	_slice._frame_num		= _frameNum;
	_slice._idr_pic_id	= _idrFrameNum;
	/// Force the image plane encoders to use _pQuant as the slice qp and therefore the
	/// slice header is determined before encoding and may be added to the bit stream here.
	_slice._qp										= _pQuant;
	_slice._qp_delta							= _slice._qp - (_picParam[_currPicParam]._pic_init_qp_minus26 + 26);
	_slice._pic_parameter_set_id	= _currPicParam;
	/// Baseline profile only has two slice types and in this implementation they are
	/// used for the entire frame/picture.
	if(_pictureCodingType == H264V2_INTER)
		_slice._type		= SliceHeaderH264::P_Slice_All;
	else
		_slice._type		= SliceHeaderH264::I_Slice_All;

	allowedBits		= bitLimit - _bitStreamSize;
	runOutOfBits	= WriteSliceLayerHeader(_pBitStreamWriter, allowedBits, &bitsUsed);
	_bitStreamSize += bitsUsed;
  if(runOutOfBits) ///< or if(== 2) An error has occured.
    return(0);

	/// Encode the entire picture. The plane encoders do not write to the stream
	/// but do require to know the available bits. Allowance is made for the single
	/// trailing bit.
	allowedBits	= bitLimit - _bitStreamSize - 1;
	if(_pictureCodingType == H264V2_INTRA)
	{
		_prevMotionDistortion = -1;
    Restart(); ///< Reset the loop for I-picture.

		/// The encoder was chosen in Open() depending on the mode selected. It
		/// operates on the full list of slices and their macroblocks.
		if(!_pIntraImgPlaneEncoder->Encode(allowedBits, &bitsUsed, 1))
			return(0);	///< An error has occured.
	}//end if H264V2_INTRA...
	else if(_pictureCodingType == H264V2_INTER)
	{
		/// The encoder was chosen in Open() depending on the mode selected. It
		/// operates on the list of macroblocks. Motion compensation is included.
		if(!_pInterImgPlaneEncoder->Encode(allowedBits, &bitsUsed, 3))
			return(0);	///< An error has occured.
	}//end else H264V2_INTER...

	/// Write (concatinate) the macroblock layer (slice data) with its header 
	/// flags to the stream.
	runOutOfBits	= WriteSliceDataLayer(_pBitStreamWriter, allowedBits, &bitsUsed);
	_bitStreamSize += bitsUsed;
  if(runOutOfBits) ///< or if(== 2) An error has occured.
    return(0);

	/// Write (concatinate) the slice trailing bits to the stream. This is a min
  /// of 1 bit + zero bits to the end of the byte boundary.
	allowedBits		= bitLimit - _bitStreamSize;
	runOutOfBits	= WriteTrailingBits(_pBitStreamWriter, allowedBits, &bitsUsed);
	_bitStreamSize += bitsUsed;
  if(runOutOfBits) ///< or if(== 2) An error has occured.
    return(0);

	/// In-loop filter for 4x4 block boundaries to remove blocking artefacts.
	if(_slice._disable_deblocking_filter_idc != 1)
		ApplyLoopFilter();

  /// Prevent start code emulation within the coded bit stream. The extra byte added
  /// to prevent the emulation is not counted as part of the bit written.
  if(_startCodeEmulationPrevention)
  {
    int offset = 0;
    if( (_pictureCodingType == H264V2_INTRA)&&(_prependParamSetsToIPic) )
      offset = _encSeqParamByteLen + _encPicParamByteLen;

    _bitStreamSize += InsertEmulationPrevention(_pBitStreamWriter, offset);
  }//end if _startCodeEmulationPrevention...

	/// Any param changes required for the next picture encoding are done here.
  /// INTER pictures always follow INTRA pictures.
	_lastPicCodingType = _pictureCodingType;
  if(_pictureCodingType == H264V2_INTRA)
	{
    _pictureCodingType	= H264V2_INTER;
		_idrFrameNum++;	///< The entire I-frame is encoded in one I-slice therefore this can be incremented.
	}//end if H264V2_INTRA...

	///< Increment the frame number relative to the last I-frame.
	_frameNum = (_frameNum + 1) % _maxFrameNum;

	return(1);
}//end Code.

/** Decode the compresed frame into raw pel samples.
The input types are a compressed picture IDR or P NAL unit, a SPS, a PPS or a concatenated 
SPS, PPS and compressed picture. The output is the raw picture pels in the format specified 
by the "outcolour" codec parameter.
@param  pCmp      : Compressed stream.
@param  bitLength : Length in bits of pCmp.
@param  pDst      : Raw pel output.
@return           : 1 = success, 0 = failure.
*/
int	H264v2Codec::Decode(void* pCmp, int bitLength, void* pDst)
{
  int	runOutOfBits	= 0;
	int frameBitSize	= bitLength;
	int bitsUsed			= 0;
	int ret						= 1;

	/// Set the bit stream access. The bit stream reader and related objects are instantiated within 
	/// Open() and is therefore not available for non-picture NAL types. They are temporarily created 
	/// for these other NAL types.
  _bitStreamSize	= bitLength;
	if(!_codecIsOpen)
  {
		_pBitStreamReader				= new BitStreamReaderMSB();
		_pHeaderUnsignedVlcDec	= new ExpGolombUnsignedVlcDecoder();
		_pHeaderSignedVlcDec		= new ExpGolombSignedVlcDecoder();
		if( (_pBitStreamReader == NULL)||(_pHeaderUnsignedVlcDec == NULL)||(_pHeaderSignedVlcDec == NULL) )
		{
			_errorStr = "[H264Codec::Decode] Cannot instantiate stream reader and vlc objects";
			ret				= 0;
			goto H264V2_D_CLEAN_MEM;
		}//end if !_pBitStreamReader...
  }//end if !_codecIsOpen...
	/// Set the stream reader.
	_pBitStreamReader->SetStream(pCmp, bitLength);

  /// Typically in-band SPS and PPS are prepended to IDR frames. Therefore keep decoding until all
  /// non-picture NAL types are decoded. If the frameBitSize indicates at least 4 more bytes (32 bits)
  /// then we assume there is another NAL to be decoded.
  int moreNonPicNALUnits = 1;
  while(moreNonPicNALUnits)
  {
    /// Extract the start code 0x00000001 from the stream.
    int startCode = _pBitStreamReader->Read(32);
    if(startCode != 1)
    {
		  _errorStr = "[H264Codec::Decode] Cannot extract start code from stream";
 		  ret	= 0;
		  goto H264V2_D_CLEAN_MEM;
    }//end if frameBitSize...
    frameBitSize -= 32;

    /// Get the NAL header encodings off the bit stream to determine the picture coding type..
	  runOutOfBits = ReadNALHeader(_pBitStreamReader, frameBitSize, &bitsUsed);
	  frameBitSize -= bitsUsed;
    if(runOutOfBits > 0) ///< An error has occurred. 1 = run out of bits, 2 = vlc decode error.
	  {
 		  ret	= 0;
		  goto H264V2_D_CLEAN_MEM;
	  }//end if runOutOfBits...

	  /// NAL to coding type conversion.
	  switch(_nal._unit_type)
	  {
		  case NalHeaderH264::IDR_Slice:
			  _pictureCodingType = H264V2_INTRA;
        moreNonPicNALUnits = 0;
			  break;
		  case NalHeaderH264::NonIDR_NoPartition_Slice:
			  /// Slice without partitioning and therefore only one set of slice parameters and NAL type = Slice type.
			  _pictureCodingType = H264V2_INTER;
        moreNonPicNALUnits = 0;
			  break;
		  case NalHeaderH264::SeqParamSet:
		  case NalHeaderH264::PicParamSet:
			  {
          int changed = 1;  ///< Indicate if the decoded params for this index have changed. Assume they have changed as the default.
				  if(_nal._unit_type == NalHeaderH264::SeqParamSet)
				  {
					  _pictureCodingType = H264V2_SEQ_PARAM;
					  /// Read the seq param set off the stream.
					  runOutOfBits = ReadSeqParamSet(_pBitStreamReader, frameBitSize, &bitsUsed, &_currSeqParam, &changed);
				  }//end if SeqParamSet...
				  else ///< if(_nal._unit_type == NalHeaderH264::PicParamSet)
				  {
					  _pictureCodingType = H264V2_PIC_PARAM;
					  /// Read the pic param set off the stream.
					  runOutOfBits = ReadPicParamSet(_pBitStreamReader, frameBitSize, &bitsUsed, &_currPicParam, &changed);
					  if(!runOutOfBits)
						  _currSeqParam = _picParam[_currPicParam]._seq_parameter_set_id;
				  }//end else...
				  frameBitSize -= bitsUsed;
				  if(runOutOfBits > 0) ///< An error has occurred. 1 = run out of bits, 2 = vlc decode error.
				  {
 					  ret	= 0;
					  goto H264V2_D_CLEAN_MEM;
				  }//end if runOutOfBits...

				  /// Read the trailing bits off the stream.
				  runOutOfBits = ReadTrailingBits(_pBitStreamReader, frameBitSize, &bitsUsed);
				  frameBitSize -= bitsUsed;
				  if(runOutOfBits > 0) ///< An error has occurred. 1 = run out of bits, 2 = vlc decode error.
				  {
 					  ret	= 0;
					  goto H264V2_D_CLEAN_MEM;
				  }//end if runOutOfBits...

          /// If the codec is in the open state then any change in sequence or picture param set requires the
          /// codec to be restarted with these new sets.
          if(_codecIsOpen)
          {
            if(changed)
            {
              int tmpPicCodingType = _pictureCodingType; ///< Store the coding type.

              _genParamSetOnOpen = 0; ///< Must be off if the new param sets are to be used.
              if(!Open())
              {
 					      ret	= 0;
					      goto H264V2_D_CLEAN_MEM;
              }//end if !Open...

              /// Restore the picture coding type.
              _pictureCodingType = tmpPicCodingType;
              }//end if changed...

          }//end if _codecIsOpen...

          if(frameBitSize < 32) ///< No more units.
          {
            moreNonPicNALUnits = 0;
            if(_codecIsOpen)
              return(1);
            else
            {
				      /// Clean up the mem.
				      ret = 1;
				      goto H264V2_D_CLEAN_MEM;
            }//end else...
          }//end if frameBitSize...

			  };//end SeqParamSet and PicParamSet block...
			  break;
		  default:
			  _errorStr = "[H264v2Codec::Decode] NAL unit type not supported";
			  ret = 0;
			  goto H264V2_D_CLEAN_MEM;
			  break;
	  }//end switch _unit_type...
  }//end while moreNonPicNALUnits...

	/// Only IDR and P pictures are decoded from here and the codec must be open.
	if(!_codecIsOpen)
  {
		_errorStr = "[H264v2Codec::Decode] Codec is not open";
		ret = 0;
		goto H264V2_D_CLEAN_MEM;
  }//end if !_codecIsOpen...

  /// Remove prevention of start code emulation codes within the coded bit stream.
  if(_startCodeEmulationPrevention)
    frameBitSize -= RemoveEmulationPrevention(_pBitStreamReader);

	/// Get the slice header encodings off the bit stream. As this implementation 
	/// has only one slice, the slice header, slice data (macroblocks) and the 
	/// slice trailing bits can be decoded in linear order.
	runOutOfBits = ReadSliceLayerHeader(_pBitStreamReader, frameBitSize, &bitsUsed);
	frameBitSize -= bitsUsed;
  if(runOutOfBits > 0) ///< An error has occurred. 1 = run out of bits, 2 = vlc decode error.
    return(0);
	/// Load frame counter members from the decoded slice header.
	_frameNum			= _slice._frame_num;
	_idrFrameNum	= _slice._idr_pic_id;
	/// Load the picture and sequence parameter set references.
	_currPicParam = _slice._pic_parameter_set_id;
	_currSeqParam = _picParam[_currPicParam]._seq_parameter_set_id;
	/// There is only one slice so the quant parameter is picture quant + the delta slice quant.
	_slice._qp		= _picParam[_currPicParam]._pic_init_qp_minus26 + 26 + _slice._qp_delta;
	_pQuant				= _slice._qp;

	/// Get the macroblock (slice data) encodings off the bit stream.
	runOutOfBits = ReadSliceDataLayer(_pBitStreamReader, frameBitSize, &bitsUsed);
	frameBitSize -= bitsUsed;
  if(runOutOfBits > 0) ///< An error has occurred. 1 = run out of bits, 2 = vlc decode error.
    return(0);

	/// Get the slice trailing bits off the bit stream. 
	runOutOfBits = ReadTrailingBits(_pBitStreamReader, frameBitSize, &bitsUsed);
	frameBitSize -= bitsUsed;
  if(runOutOfBits > 0) ///< An error has occurred. 1 = run out of bits, 2 = vlc decode error.
    return(0);

  /// INTRA frames require the reference images to be zeroed.
	if(_pictureCodingType == H264V2_INTRA)
	{
    Restart();	///< Reset the loop and ref img.

		/// The decoder was chosen in Open() depending on the mode selected. It
		/// operates on the full list of macroblocks.
		if(!_pIntraImgPlaneDecoder->Decode())
			return(0);	///< An error has occured.
	}//end if INTRA...
	else
	{
		/// INTER picture decode.
		/// The decoder was chosen in Open() depending on the mode selected. It
		/// operates on the list of macroblocks. Motion compensation is included.
		if(!_pInterImgPlaneDecoder->Decode())
			return(0);	///< An error has occured.
	}//end else INTER...

	/// In-loop filter for 4x4 block boundaries to remove blocking artefacts.
	if(_slice._disable_deblocking_filter_idc != 1)
		ApplyLoopFilter();

  /// Convert to the output image depending on the output dimension settings. Set
	/// up in the Open() method for the correctly selected converter.
	if(_outColour == H264V2_YUV420P16)      /// The natural colour space of the decoder with type = short.
		memcpy((void *)pDst, (const void *)_pLum, ((_lumWidth * _lumHeight) + 2*(_chrWidth * _chrHeight)) * sizeof(short));
	else if(_outColour == H264V2_YUV420P8)  ///< ...type = byte.
  {
    int colLen = (_lumWidth * _lumHeight) + 2*(_chrWidth * _chrHeight);
    for(int i = 0; i < colLen; i++)
      ((unsigned char *)pDst)[i] = (unsigned char)_pLum[i];
  }//end if H264V2_YUV420P8...
	else
		_pOutColourConverter->Convert(_pRLum, _pRChrU, _pRChrV, pDst);

  return(1);

	/// Clean up memory objects.
	H264V2_D_CLEAN_MEM:
		if(_pHeaderUnsignedVlcDec != NULL)
		{
			delete _pHeaderUnsignedVlcDec;
			_pHeaderUnsignedVlcDec = NULL;
		}//end if _pHeaderUnsignedVlcDec...
		if(_pHeaderSignedVlcDec != NULL)
		{
			delete _pHeaderSignedVlcDec;
			_pHeaderSignedVlcDec = NULL;
		}//end if _pHeaderSignedVlcDec...
		if(_pBitStreamReader != NULL)
		{
			delete _pBitStreamReader;
			_pBitStreamReader = NULL;
		}//end if _pBitStreamReader...

		return(ret);
}//end Decode.

int H264v2Codec::Close(void)
{
	/// Free the image memory and associated overlays.
	if(_Lum != NULL)
		delete _Lum;
	_Lum	= NULL;
	if(_Cb != NULL)
		delete _Cb;
	_Cb	= NULL;
	if(_Cr != NULL)
		delete _Cr;
	_Cr	= NULL;

  if(_RefLum != NULL)
		delete _RefLum;
	_RefLum	= NULL;
	if(_RefCb != NULL)
		delete _RefCb;
	_RefCb	= NULL;
	if(_RefCr != NULL)
		delete _RefCr;
	_RefCr	= NULL;

	if(_pLum != NULL)
		delete[] _pLum;
	_pLum			= NULL;
	_pChrU		= NULL;
	_pChrV		= NULL;
	_pRLum		= NULL;
	_pRChrU		= NULL;
	_pRChrV		= NULL;

	if(_16x16 != NULL)
		delete _16x16;
	_16x16	= NULL;
	if(_p16x16 != NULL)
		delete[] _p16x16;
	_p16x16	= NULL;
	if(_8x8_0 != NULL)
		delete _8x8_0;
	_8x8_0	= NULL;
	if(_p8x8_0 != NULL)
		delete[] _p8x8_0;
	_p8x8_0	= NULL;
	if(_8x8_1 != NULL)
		delete _8x8_1;
	_8x8_1	= NULL;
	if(_8x8_1 != NULL)
		delete[] _8x8_1;
	_8x8_1	= NULL;

	/// Colour converters.
	if(_pInColourConverter != NULL)
		delete _pInColourConverter;
	_pInColourConverter = NULL;

	if(_pOutColourConverter != NULL)
		delete _pOutColourConverter;
	_pOutColourConverter = NULL;

	/// IT transform filters.
	if(_pF4x4TLum != NULL)
		delete _pF4x4TLum;
	_pF4x4TLum = NULL;
	if(_pF4x4TChr != NULL)
		delete _pF4x4TChr;
	_pF4x4TChr = NULL;
	if(_pFDC4x4T != NULL)
		delete _pFDC4x4T;
	_pFDC4x4T = NULL;
	if(_pFDC2x2T != NULL)
		delete _pFDC2x2T;
	_pFDC2x2T = NULL;
	if(_pI4x4TLum != NULL)
		delete _pI4x4TLum;
	_pI4x4TLum = NULL;
	if(_pI4x4TChr != NULL)
		delete _pI4x4TChr;
	_pI4x4TChr = NULL;
	if(_pIDC4x4T != NULL)
		delete _pIDC4x4T;
	_pIDC4x4T = NULL;
	if(_pIDC2x2T != NULL)
		delete _pIDC2x2T;
	_pIDC2x2T = NULL;

	/// Vlc encoders and decoders.
	if(_pPrefixVlcEnc != NULL)
		delete _pPrefixVlcEnc;
	_pPrefixVlcEnc = NULL;
	if(_pPrefixVlcDec != NULL)
		delete _pPrefixVlcDec;
	_pPrefixVlcDec = NULL;

	if(_pCoeffTokenVlcEnc != NULL)
		delete _pCoeffTokenVlcEnc;
	_pCoeffTokenVlcEnc = NULL;
	if(_pCoeffTokenVlcDec != NULL)
		delete _pCoeffTokenVlcDec;
	_pCoeffTokenVlcDec = NULL;

	if(_pTotalZeros4x4VlcEnc != NULL)
		delete _pTotalZeros4x4VlcEnc;
	_pTotalZeros4x4VlcEnc = NULL;
	if(_pTotalZeros4x4VlcDec != NULL)
		delete _pTotalZeros4x4VlcDec;
	_pTotalZeros4x4VlcDec = NULL;

	if(_pTotalZeros2x2VlcEnc != NULL)
		delete _pTotalZeros2x2VlcEnc;
	_pTotalZeros2x2VlcEnc = NULL;
	if(_pTotalZeros2x2VlcDec != NULL)
		delete _pTotalZeros2x2VlcDec;
	_pTotalZeros2x2VlcDec = NULL;

	if(_pRunBeforeVlcEnc != NULL)
		delete _pRunBeforeVlcEnc;
	_pRunBeforeVlcEnc = NULL;
	if(_pRunBeforeVlcDec != NULL)
		delete _pRunBeforeVlcDec;
	_pRunBeforeVlcDec = NULL;

	if(_pBlkPattVlcEnc != NULL)
		delete _pBlkPattVlcEnc;
	_pBlkPattVlcEnc = NULL;
	if(_pBlkPattVlcDec != NULL)
		delete _pBlkPattVlcDec;
	_pBlkPattVlcDec = NULL;

	if(_pDeltaQPVlcEnc != NULL)
		delete _pDeltaQPVlcEnc;
	_pDeltaQPVlcEnc = NULL;
	if(_pDeltaQPVlcDec != NULL)
		delete _pDeltaQPVlcDec;
	_pDeltaQPVlcDec = NULL;

	if(_pMbTypeVlcEnc != NULL)
		delete _pMbTypeVlcEnc;
	_pMbTypeVlcEnc = NULL;
	if(_pMbTypeVlcDec != NULL)
		delete _pMbTypeVlcDec;
	_pMbTypeVlcDec = NULL;

	/// Vlc codecs that were used as references and not instantiated.
	_pMbIChrPredModeVlcEnc		= NULL;
	_pMbIChrPredModeVlcDec		= NULL;
	_pMbMotionVecDiffVlcEnc		= NULL;
	_pMbMotionVecDiffVlcDec		= NULL;
	_pHeaderUnsignedVlcEnc		= NULL;
	_pHeaderUnsignedVlcDec		= NULL;
	_pHeaderSignedVlcEnc			= NULL;
	_pHeaderSignedVlcDec			= NULL;

	/// The CAVLC codecs.
	if(_pCAVLC4x4 != NULL)
		delete _pCAVLC4x4;
	_pCAVLC4x4 = NULL;
	if(_pCAVLC2x2 != NULL)
		delete _pCAVLC2x2;
	_pCAVLC2x2 = NULL;

	/// Macroblock data objects.
	if(_pMb != NULL)
		delete[] _pMb;
	_pMb = NULL;
	if(_Mb != NULL)
		delete[] _Mb;
	_Mb = NULL;

	if(_autoIFrameIncluded != NULL)
		delete[] _autoIFrameIncluded;
	_autoIFrameIncluded = NULL;

	/// Stream access.
	if(_pBitStreamWriter != NULL)
		delete _pBitStreamWriter;
	_pBitStreamWriter = NULL;

	if(_pBitStreamReader != NULL)
		delete _pBitStreamReader;
	_pBitStreamReader = NULL;

	/// Motion estimator and compensators.
	if(_pMotionEstimator != NULL)
		delete _pMotionEstimator;
	_pMotionEstimator = NULL;

	/// Motion compensation vectors.
	if(_pMotionVectors != NULL)
		delete _pMotionVectors;
	_pMotionVectors = NULL;

	if(_pMotionCompensator != NULL)
		delete _pMotionCompensator;
	_pMotionCompensator = NULL;

	/// Image plane encoders/decoders.
	if(_pIntraImgPlaneEncoder != NULL)
		delete _pIntraImgPlaneEncoder;
	_pIntraImgPlaneEncoder = NULL;

	if(_pInterImgPlaneEncoder != NULL)
		delete _pInterImgPlaneEncoder;
	_pInterImgPlaneEncoder	= NULL;

	if(_pIntraImgPlaneDecoder != NULL)
		delete _pIntraImgPlaneDecoder;
	_pIntraImgPlaneDecoder	= NULL;

	if(_pInterImgPlaneDecoder != NULL)
		delete _pInterImgPlaneDecoder;
	_pInterImgPlaneDecoder	= NULL;

  _codecIsOpen = 0;
  return(1);
}//end Close.

void H264v2Codec::Restart(void)
{
  _pictureCodingType	= H264V2_INTRA;
	_frameNum						= 0;				///< Reset the frame counter.

  /// Zero the reference image. Note that the colour components are 
	/// held in contiguous mem.
	int imgSize = (_lumWidth * _lumHeight) + 2*(_chrWidth * _chrHeight);
	memset(_pRLum, 0, imgSize * sizeof(short));

}//end Restart.

/*
-----------------------------------------------------------------------
  Private Implementation.                                
-----------------------------------------------------------------------
*/
/** Code non-picture nal types.
This method operates independently and therefore all the coding objects must be
instantiated and destroyed before and after the coding process. This is typically an 
out-of-band process and therefore speed is not critical. The codec must NOT be open
for calls to this method i.e. _codecIsOpen = 0.
@param pCmp						: The stream memory to write to.
@param frameBitLimit	: The max number of bits to use.
@return								: 1 = success, 0 = failure.
*/
int H264v2Codec::CodeNonPicNALTypes(void* pCmp, int frameBitLimit)
{
	int allowedBits, bitsUsed;

  int runOutOfBits	= 0;
  int bitLimit			= frameBitLimit;
	int ret						= 1;	///< Return value default to success.
  _bitStreamSize		= 0;

	/// This method uses the class members and therefore Open() must not be called before
	/// coding non-pic nal-types.
  if(_codecIsOpen)
  {
		_errorStr = "[H264V2::CodeNonPicNALTypes] Codec must not be open for non-picture nal types";
    return(0);
  }//end if _codecIsOpen...

	/// Check which types are supported with this implementation.
	if((_pictureCodingType != H264V2_SEQ_PARAM)&&(_pictureCodingType != H264V2_PIC_PARAM))
	{
		_errorStr = "[H264Codec::CodeNonPicNALTypes] Non-Nal picture type not supported";
		return(0);
	}//end if !H264V2_SEQ_PARAM...

	///----------------------------------------------------------------------------------------------
	/// Instantiate the mem objects required for the encoding.

	/// A stream writer.
	_pBitStreamWriter = new BitStreamWriterMSB();
	if(_pBitStreamWriter == NULL)
	{
		_errorStr = "[H264Codec::CodeNonPicNALTypes] Cannot instantiate bit stream writer object";
		ret				= 0;
		goto H264V2_CNPNT_CLEAN_MEM;
	}//end if !_pBitStreamWriter...
	/// Set the stream writer to the compressed input parameter reference.
	_pBitStreamWriter->SetStream(pCmp, frameBitLimit);

	/// Vlc encoder and decoder for general headers. ExpGolomb codecs
	/// are stateless therefore they can be reused.
	_pHeaderUnsignedVlcEnc	= new ExpGolombUnsignedVlcEncoder();
	_pHeaderSignedVlcEnc		= new ExpGolombSignedVlcEncoder();
	if( (_pHeaderUnsignedVlcEnc == NULL)||(_pHeaderSignedVlcEnc == NULL) )
	{
		_errorStr = "[H264Codec::CodeNonPicNALTypes] Cannot instantiate vlc encoder objects";
		ret				= 0;
		goto H264V2_CNPNT_CLEAN_MEM;
	}//end if _pHeaderUnsignedVlcEnc...

	///----------------------------------------------------------------------------------------------
	/// Parameter set coding. The _currSeqParam and _currPicParam members determine the param sets
	/// to encode. From here the picture coding type is only H264V2_SEQ_PARAM or H264V2_PIC_PARAM.

	_nal._ref_idc = 3;

	if(_pictureCodingType == H264V2_SEQ_PARAM)
	{
		_nal._unit_type = NalHeaderH264::SeqParamSet;
		/// Sequence parameter set for Baseline profile.
		if(!SetSeqParamSet(_currSeqParam))
		{
			_errorStr = "[H264Codec::CodeNonPicNALTypes] Cannot set sequence parameter set";
			ret				= 0;
			goto H264V2_CNPNT_CLEAN_MEM;
		}//end if !SetSeqParamSet...
	}//end if H264V2_SEQ_PARAM...
	else ///< if(_pictureCodingType == H264V2_PIC_PARAM)
	{
		_nal._unit_type = NalHeaderH264::PicParamSet;
		/// Picture parameter set.
		if(!SetPicParamSet(_currPicParam, _currSeqParam))
		{
			_errorStr = "[H264Codec::CodeNonPicNALTypes] Cannot set picture parameter set";
			ret				= 0;
			goto H264V2_CNPNT_CLEAN_MEM;
		}//end if !SetPicParamSet...
	}//end else...

  /// Write the 32-bit start code 0x00000001 to the stream.
	allowedBits		= bitLimit - _bitStreamSize;
  if(allowedBits < 32)
  {
	  _errorStr = "[H264V2Codec::CodeNonPicNALTypes] Cannot write start code to stream";
    ret				= 0;
		goto H264V2_CNPNT_CLEAN_MEM;
  }//end if allowedBits...
  _pBitStreamWriter->Write(32,1);
  _bitStreamSize += 32;

	/// Write the NAL header to the stream. 
	allowedBits		= bitLimit - _bitStreamSize;
	runOutOfBits	= WriteNALHeader(_pBitStreamWriter, allowedBits, &bitsUsed);
	_bitStreamSize += bitsUsed;
	if(runOutOfBits) ///< or if(== 2) An error has occured.
	{
		ret	= 0;
		goto H264V2_CNPNT_CLEAN_MEM;
	}//end if runOutOfBits...

	/// Write the parameter set to the stream.
	allowedBits		= bitLimit - _bitStreamSize;
	if(_pictureCodingType == H264V2_SEQ_PARAM)
		runOutOfBits	= WriteSeqParamSet(_pBitStreamWriter, allowedBits, &bitsUsed, _currSeqParam);
	else ///< if(_pictureCodingType == H264V2_PIC_PARAM)
		runOutOfBits	= WritePicParamSet(_pBitStreamWriter, allowedBits, &bitsUsed, _currPicParam);
	_bitStreamSize += bitsUsed;
	if(runOutOfBits) ///< or if(== 2) An error has occured.
	{
		ret	= 0;
		goto H264V2_CNPNT_CLEAN_MEM;
	}//end if runOutOfBits...

	/// Write trailing bits to the stream.
	allowedBits		= bitLimit - _bitStreamSize;
	runOutOfBits	= WriteTrailingBits(_pBitStreamWriter, allowedBits, &bitsUsed);
	_bitStreamSize += bitsUsed;
	if(runOutOfBits) ///< or if(== 2) An error has occured.
	{
		ret	= 0;
		goto H264V2_CNPNT_CLEAN_MEM;
	}//end if runOutOfBits...

	///----------------------------------------------------------------------------------------------
	/// Clean up memory objects.
	H264V2_CNPNT_CLEAN_MEM:
		if(_pHeaderUnsignedVlcEnc != NULL)
		{
			delete _pHeaderUnsignedVlcEnc;
			_pHeaderUnsignedVlcEnc = NULL;
		}//end if _pHeaderUnsignedVlcEnc...
		if(_pHeaderSignedVlcEnc != NULL)
		{
			delete _pHeaderSignedVlcEnc;
			_pHeaderSignedVlcEnc = NULL;
		}//end if _pHeaderSignedVlcEnc...
		if(_pBitStreamWriter != NULL)
		{
			delete _pBitStreamWriter;
			_pBitStreamWriter = NULL;
		}//end if _pBitStreamWriter...

	return(ret);
}//end CodeNonPicNALTypes.

/** Set the Sequence parameter set.
Set one of the seq param sets defined by the index parameter where the 
values are determined by interpreting the codec parameters.
@param index	:	Seq parameter set to modify [0..31].
@return				: Success = 1, failure = 0.
*/
int H264v2Codec::SetSeqParamSet(int index)
{
	if(index > 31)
		return(0);

	_seqParam[index]._seq_parameter_set_id									= index;					///< The index for this definition of sequence params [0..31].
	_seqParam[index]._profile_idc														= 66;							///< Baseline profile = 66.
	_seqParam[index]._constraint_set0_flag									= 0;							///< = 0.
	_seqParam[index]._constraint_set1_flag									= 0;							///< = 0.
	_seqParam[index]._constraint_set2_flag									= 0;							///< = 0.
	_seqParam[index]._constraint_set3_flag									= 0;							///< = 0.
	_seqParam[index]._level_idc															= 20;							///< = 20 (Level num = 2, calculated as 10 x level num).
	_seqParam[index]._chroma_format_idc											= 1;							///< Range = [0..3]. When not present = 1 (4:2:0).
	_seqParam[index]._residual_colour_transform_flag				= 0;							///< = 0.
	_seqParam[index]._bit_depth_luma_minus8									= 0;							///< Range = [0..4]. When not present = 0 (8 bits/lum component).
	_seqParam[index]._bit_depth_chroma_minus8								= 0;							///< Range = [0..4]. When not present = 0 (8 bits/chr component).
	_seqParam[index]._qpprime_y_zero_transform_bypass_flag	= 0;							///< If QP = 0, this flag indicates bypassing the transform process. When not present = 0.
	_seqParam[index]._seq_scaling_matrix_present_flag				= 0;							///< = 0. No scaling matrices for baseline profile. Indicates existence of _seq_scaling_list_present_flag flags.
	/// Not initialised _seq_scaling_list_present_flag[8]			///< if _seq_scaling_matrix_present_flag, these indicate lists not present/present (0/1).
	_seqParam[index]._log2_max_frame_num_minus4							= 12;							///< Range = [0..12]. MaxFrameNum = 2^(_log2_max_frame_num_minus4 + 4) for slice _frame_num derivations.
	_seqParam[index]._pic_order_cnt_type										= 2;							///< Range = [0..2]. 2 = 1-to-1 mapping. (Picture order count = frame num).
	_seqParam[index]._log2_max_pic_order_cnt_lsb_minus4			= 0;							///< For B-Slice decoding.  There are no B-Slices in the baseline profile.
	_seqParam[index]._delta_pic_order_always_zero_flag			= 0;							///< For B-Slice decoding.
	_seqParam[index]._offset_for_non_ref_pic								= 0;							///< For B-Slice decoding.
	_seqParam[index]._offset_for_top_to_bottom_field				= 0;							///< For B-Slice decoding.
	_seqParam[index]._num_ref_frames_in_pic_order_cnt_cycle	= 0;							///< For B-Slice decoding.
	/// Not initialised _offset_for_ref_frame[256];						///< For B-Slice decoding.
	_seqParam[index]._num_ref_frames												= 1;							///< Max num of short-term and long-term ref frames.
	_seqParam[index]._gaps_in_frame_num_value_allowed_flag	= 0;							///< Indicates allowed values of slice _frame_num.
	_seqParam[index]._frame_mbs_only_flag										= 1;							///< = 1 (baseline profile). fields/frames = 0/1.
	_seqParam[index]._mb_adaptive_frame_field_flag					= 0;							///< Indicates switching between frame and field macroblocks. When not present = 0.
	_seqParam[index]._direct_8x8_inference_flag							= 0;							///< Methods for B-Slice motion vector decoding. There are no B-Slices in the baseline profile.
	_seqParam[index]._frame_cropping_flag										= 0;							///< Indicates that the cropping params below follow in the stream.
	_seqParam[index]._frame_crop_left_offset								= 0;							///< In frame coordinate units.
	_seqParam[index]._frame_crop_right_offset								= 0;							///< In frame coordinate units.
	_seqParam[index]._frame_crop_top_offset									= 0;							///< In frame coordinate units.
	_seqParam[index]._frame_crop_bottom_offset							= 0;							///< In frame coordinate units.
	_seqParam[index]._vui_parameters_present_flag						= 0;							///< vui parameters follow.

	if((_width < 16)||(_height < 16))
		return(0);

	_seqParam[index]._pic_width_in_mbs_minus1								= (_width/16) - 1;	///< Width of decoded pic in units of macroblocks. Derive lum and chr pic width from this variable.
	_seqParam[index]._pic_height_in_map_units_minus1				= (_height/16) - 1;	///< Map units are 2 macroblocks for fields and 1 for frames. Derive lum and chr pic height from this variable.

	return(1);
}//end SetSeqParamSet.

/** Write the sequence parameter set to a bit stream.
The seq param set must be correctly defined before this method is called. The 
vlc encoding is performed first before writing. Each write checks if a bit 
overflow will occur before writing. The check is sufficiently frequent to 
warrant an early exit GOTO statement. If the input stream param is NULL then 
this method is used to count the bits only.
@param bsw					: Stream to write into.
@param allowedBits	: Upper limit to the writable bits.
@param bitsUsed			: Return the actual bits used.
@param index				: Sequence parameter set index to write.
@return							: Run out of bits = 1, more bits available = 0, Vlc error = 2, unsupported parameter set = 3.
*/
int H264v2Codec::WriteSeqParamSet(IBitStreamWriter* bsw, int allowedBits, int* bitsUsed, int index)
{
	int	bitCount, i;
	int bitsUsedSoFar = 0;

	/// Only baseline profile is supported: _profile_idc = 66.
	if(_seqParam[index]._profile_idc != 66)
	{
		_errorStr = "H264V2:[H264Codec::WriteSeqParamSet] This implementation only supports the baseline profile";
		*bitsUsed = 0;
		return(3);
	}//end if _profile_idc not baseline...

	/// The first set of bit fields are fixed with a total of 24 bits.
	if(allowedBits < 24)
		goto H264V2_WSPS_RUNOUTOFBITS_WRITE;
	if(bsw)
	{
		bsw->Write(8, _seqParam[index]._profile_idc);						///< u(8)
		bsw->Write(_seqParam[index]._constraint_set0_flag);			///< u(1)
		bsw->Write(_seqParam[index]._constraint_set1_flag);			///< u(1)
		bsw->Write(_seqParam[index]._constraint_set2_flag);			///< u(1)
		bsw->Write(_seqParam[index]._constraint_set3_flag);			///< u(1)
		bsw->Write(4, 0);																				///< u(4), reserved_zero_4bits.
		bsw->Write(8, _seqParam[index]._level_idc);							///< u(8)
	}//end if bsw...
	bitsUsedSoFar += 24;

	/// _seq_parameter_set_id
	bitCount = _pHeaderUnsignedVlcEnc->Encode(_seqParam[index]._seq_parameter_set_id);
	if(bitCount <= 0)	///< Vlc codec errors are detected from a zero or negative return value.
		goto H264V2_WSPS_VLCERROR_WRITE;
	if( (bitsUsedSoFar + bitCount) > allowedBits )
		goto H264V2_WSPS_RUNOUTOFBITS_WRITE;
	if(bsw)
		bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
	bitsUsedSoFar += bitCount;

	// TODO: Add support for other profiles here
	/// _chroma_format_idc, _residual_colour_transform_flag, _bit_depth_luma_minus8, _bit_depth_chroma_minus8,
	/// _qpprime_y_zero_transform_bypass_flag, _seq_scaling_matrix_present_flag, _seq_scaling_list_present_flag[8].

	/// _log2_max_frame_num_minus4
	bitCount = _pHeaderUnsignedVlcEnc->Encode(_seqParam[index]._log2_max_frame_num_minus4);
	if(bitCount <= 0)	///< Vlc codec errors are detected from a zero or negative return value.
		goto H264V2_WSPS_VLCERROR_WRITE;
	if( (bitsUsedSoFar + bitCount) > allowedBits )
		goto H264V2_WSPS_RUNOUTOFBITS_WRITE;
	if(bsw)
		bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
	bitsUsedSoFar += bitCount;

	/// _pic_order_cnt_type
	bitCount = _pHeaderUnsignedVlcEnc->Encode(_seqParam[index]._pic_order_cnt_type);
	if(bitCount <= 0)	///< Vlc codec errors are detected from a zero or negative return value.
		goto H264V2_WSPS_VLCERROR_WRITE;
	if( (bitsUsedSoFar + bitCount) > allowedBits )
		goto H264V2_WSPS_RUNOUTOFBITS_WRITE;
	if(bsw)
		bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
	bitsUsedSoFar += bitCount;

	if(_seqParam[index]._pic_order_cnt_type == 0)
	{
		/// _log2_max_pic_order_cnt_lsb_minus4
		bitCount = _pHeaderUnsignedVlcEnc->Encode(_seqParam[index]._log2_max_pic_order_cnt_lsb_minus4);
		if(bitCount <= 0)	///< Vlc codec errors are detected from a zero or negative return value.
			goto H264V2_WSPS_VLCERROR_WRITE;
		if( (bitsUsedSoFar + bitCount) > allowedBits )
			goto H264V2_WSPS_RUNOUTOFBITS_WRITE;
		if(bsw)
			bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
		bitsUsedSoFar += bitCount;
	}//end if _pic_order_cnt_type...
	else if(_seqParam[index]._pic_order_cnt_type == 1)
	{
		/// _delta_pic_order_always_zero_flag
		if( (bitsUsedSoFar + 1) > allowedBits )
			goto H264V2_WSPS_RUNOUTOFBITS_WRITE;
		if(bsw)
			bsw->Write(_seqParam[index]._delta_pic_order_always_zero_flag);
		bitsUsedSoFar++;

		/// _offset_for_non_ref_pic
		bitCount = _pHeaderSignedVlcEnc->Encode(_seqParam[index]._offset_for_non_ref_pic);
		if(bitCount <= 0)	///< Vlc codec errors are detected from a zero or negative return value.
			goto H264V2_WSPS_VLCERROR_WRITE;
		if( (bitsUsedSoFar + bitCount) > allowedBits )
			goto H264V2_WSPS_RUNOUTOFBITS_WRITE;
		if(bsw)
			bsw->Write(bitCount, _pHeaderSignedVlcEnc->GetCode());
		bitsUsedSoFar += bitCount;

		/// _offset_for_top_to_bottom_field
		bitCount = _pHeaderSignedVlcEnc->Encode(_seqParam[index]._offset_for_top_to_bottom_field);
		if(bitCount <= 0)	///< Vlc codec errors are detected from a zero or negative return value.
			goto H264V2_WSPS_VLCERROR_WRITE;
		if( (bitsUsedSoFar + bitCount) > allowedBits )
			goto H264V2_WSPS_RUNOUTOFBITS_WRITE;
		if(bsw)
			bsw->Write(bitCount, _pHeaderSignedVlcEnc->GetCode());
		bitsUsedSoFar += bitCount;

		/// _num_ref_frames_in_pic_order_cnt_cycle
		bitCount = _pHeaderUnsignedVlcEnc->Encode(_seqParam[index]._num_ref_frames_in_pic_order_cnt_cycle);
		if(bitCount <= 0)	///< Vlc codec errors are detected from a zero or negative return value.
			goto H264V2_WSPS_VLCERROR_WRITE;
		if( (bitsUsedSoFar + bitCount) > allowedBits )
			goto H264V2_WSPS_RUNOUTOFBITS_WRITE;
		if(bsw)
			bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
		bitsUsedSoFar += bitCount;

		for(i = 0; i < _seqParam[index]._num_ref_frames_in_pic_order_cnt_cycle; i++)
		{
			/// _offset_for_ref_frame[256]
			bitCount = _pHeaderSignedVlcEnc->Encode(_seqParam[index]._offset_for_ref_frame[i]);
			if(bitCount <= 0)	///< Vlc codec errors are detected from a zero or negative return value.
				goto H264V2_WSPS_VLCERROR_WRITE;
			if( (bitsUsedSoFar + bitCount) > allowedBits )
				goto H264V2_WSPS_RUNOUTOFBITS_WRITE;
			if(bsw)
				bsw->Write(bitCount, _pHeaderSignedVlcEnc->GetCode());
			bitsUsedSoFar += bitCount;
		}//end for i...

	}//end else if _pic_order_cnt_type...

	/// _num_ref_frames
	bitCount = _pHeaderUnsignedVlcEnc->Encode(_seqParam[index]._num_ref_frames);
	if(bitCount <= 0)	///< Vlc codec errors are detected from a zero or negative return value.
		goto H264V2_WSPS_VLCERROR_WRITE;
	if( (bitsUsedSoFar + bitCount) > allowedBits )
		goto H264V2_WSPS_RUNOUTOFBITS_WRITE;
	if(bsw)
		bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
	bitsUsedSoFar += bitCount;

	/// _gaps_in_frame_num_value_allowed_flag
	if( (bitsUsedSoFar + 1) > allowedBits )
		goto H264V2_WSPS_RUNOUTOFBITS_WRITE;
	if(bsw)
		bsw->Write(_seqParam[index]._gaps_in_frame_num_value_allowed_flag);
	bitsUsedSoFar++;

	/// _pic_width_in_mbs_minus1
	bitCount = _pHeaderUnsignedVlcEnc->Encode(_seqParam[index]._pic_width_in_mbs_minus1);
	if(bitCount <= 0)	///< Vlc codec errors are detected from a zero or negative return value.
		goto H264V2_WSPS_VLCERROR_WRITE;
	if( (bitsUsedSoFar + bitCount) > allowedBits )
		goto H264V2_WSPS_RUNOUTOFBITS_WRITE;
	if(bsw)
		bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
	bitsUsedSoFar += bitCount;

	/// _pic_height_in_map_units_minus1
	bitCount = _pHeaderUnsignedVlcEnc->Encode(_seqParam[index]._pic_height_in_map_units_minus1);
	if(bitCount <= 0)	///< Vlc codec errors are detected from a zero or negative return value.
		goto H264V2_WSPS_VLCERROR_WRITE;
	if( (bitsUsedSoFar + bitCount) > allowedBits )
		goto H264V2_WSPS_RUNOUTOFBITS_WRITE;
	if(bsw)
		bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
	bitsUsedSoFar += bitCount;

	/// _frame_mbs_only_flag
	if( (bitsUsedSoFar + 1) > allowedBits )
		goto H264V2_WSPS_RUNOUTOFBITS_WRITE;
	if(bsw)
		bsw->Write(_seqParam[index]._frame_mbs_only_flag);
	bitsUsedSoFar++;

	if(!_seqParam[index]._frame_mbs_only_flag)
	{
		/// _mb_adaptive_frame_field_flag
		if( (bitsUsedSoFar + 1) > allowedBits )
			goto H264V2_WSPS_RUNOUTOFBITS_WRITE;
		if(bsw)
			bsw->Write(_seqParam[index]._mb_adaptive_frame_field_flag);
		bitsUsedSoFar++;
	}//end if !_frame_mbs_only_flag...

	/// _direct_8x8_inference_flag
	if( (bitsUsedSoFar + 1) > allowedBits )
		goto H264V2_WSPS_RUNOUTOFBITS_WRITE;
	if(bsw)
		bsw->Write(_seqParam[index]._direct_8x8_inference_flag);
	bitsUsedSoFar++;

	/// _frame_cropping_flag
	if( (bitsUsedSoFar + 1) > allowedBits )
		goto H264V2_WSPS_RUNOUTOFBITS_WRITE;
	if(bsw)
		bsw->Write(_seqParam[index]._frame_cropping_flag);
	bitsUsedSoFar++;

	if(_seqParam[index]._frame_cropping_flag)
	{
		/// _frame_crop_left_offset
		bitCount = _pHeaderUnsignedVlcEnc->Encode(_seqParam[index]._frame_crop_left_offset);
		if(bitCount <= 0)	///< Vlc codec errors are detected from a zero or negative return value.
			goto H264V2_WSPS_VLCERROR_WRITE;
		if( (bitsUsedSoFar + bitCount) > allowedBits )
			goto H264V2_WSPS_RUNOUTOFBITS_WRITE;
		if(bsw)
			bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
		bitsUsedSoFar += bitCount;
		/// _frame_crop_right_offset
		bitCount = _pHeaderUnsignedVlcEnc->Encode(_seqParam[index]._frame_crop_right_offset);
		if(bitCount <= 0)	///< Vlc codec errors are detected from a zero or negative return value.
			goto H264V2_WSPS_VLCERROR_WRITE;
		if( (bitsUsedSoFar + bitCount) > allowedBits )
			goto H264V2_WSPS_RUNOUTOFBITS_WRITE;
		if(bsw)
			bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
		bitsUsedSoFar += bitCount;
		/// _frame_crop_top_offset
		bitCount = _pHeaderUnsignedVlcEnc->Encode(_seqParam[index]._frame_crop_top_offset);
		if(bitCount <= 0)	///< Vlc codec errors are detected from a zero or negative return value.
			goto H264V2_WSPS_VLCERROR_WRITE;
		if( (bitsUsedSoFar + bitCount) > allowedBits )
			goto H264V2_WSPS_RUNOUTOFBITS_WRITE;
		if(bsw)
			bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
		bitsUsedSoFar += bitCount;
		/// _frame_crop_bottom_offset
		bitCount = _pHeaderUnsignedVlcEnc->Encode(_seqParam[index]._frame_crop_bottom_offset);
		if(bitCount <= 0)	///< Vlc codec errors are detected from a zero or negative return value.
			goto H264V2_WSPS_VLCERROR_WRITE;
		if( (bitsUsedSoFar + bitCount) > allowedBits )
			goto H264V2_WSPS_RUNOUTOFBITS_WRITE;
		if(bsw)
			bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
		bitsUsedSoFar += bitCount;
	}//end if _frame_cropping_flag...

	/// _vui_parameters_present_flag
	if( (bitsUsedSoFar + 1) > allowedBits )
		goto H264V2_WSPS_RUNOUTOFBITS_WRITE;
	if(bsw)
		bsw->Write(_seqParam[index]._vui_parameters_present_flag);
	bitsUsedSoFar++;

	if(_seqParam[index]._vui_parameters_present_flag)
	{
		//TODO: Implement VUI encoding.
	}//end if _vui_parameters_present_flag...

	*bitsUsed = bitsUsedSoFar;
	return(0);

	H264V2_WSPS_RUNOUTOFBITS_WRITE:
		_errorStr = "H264V2:[H264Codec::WriteSeqParamSet] Bits required exceeds max available for picture";
		*bitsUsed = bitsUsedSoFar;
		return(1);

	H264V2_WSPS_VLCERROR_WRITE:
		_errorStr = "H264V2:[H264Codec::WriteSeqParamSet] Vlc encoder error";
		*bitsUsed = bitsUsedSoFar;
		return(2);

}//end WriteSeqParamSet.

/** Read the sequence parameter set from the bit stream.
This impementation reads sequence parameter set encodings. Each read checks if a 
bit underflow will occur after reading. Check for loss of vlc sync wherever possible. 
The checks are sufficiently frequent to warrant an early exit GOTO statement.
@param bsr						: Stream to read from.
@param remainingBits	: Upper limit to the readable bits.
@param bitsUsed				: Return the actual bits extracted.
@param idx						: Reference to seq param set index read.
@param changedFlag    : Flag to indicate that param set has changed to a new one.
@return								: Run out of bits = 1, more bits available = 0, error = 2.
*/
int H264v2Codec::ReadSeqParamSet(IBitStreamReader* bsr, int remainingBits, int* bitsUsed, int* idx, int* changedFlag)
{
	int bitsUsedSoFar = 0;
	int numBits, i;
  SeqParamSetH264 tmpParamSet;

	/// Check existance of stream reader.
	if(bsr == NULL)
	{
		_errorStr = "H264V2:[H264Codec::ReadSeqParamSet] No bit stream reader available";
		*bitsUsed = 0;
		return(2);
	}//end if bsr...

	/// The first set of bit fields are fixed with a total of 24 bits.
	if(remainingBits < 24)
		goto H264V2_RSPS_RUNOUTOFBITS_READ;

	/// Temporarily read up to the _seq_parameter_set_id that defines the index into the
	/// _seqParam[] array.

	int pi = bsr->Read(8);																		///< u(8):_profile_idc
	/// Only baseline profile is supported: _profile_idc = 66.
	if(pi != 66)
	{
		_errorStr = "[H264Codec::ReadSeqParamSet] This implementation only supports the baseline profile";
		*bitsUsed = 8;
		return(2);
	}//end if pi not baseline...
	int set0_flag = bsr->Read();															///< u(1):_constraint_set0_flag
	int set1_flag = bsr->Read();															///< u(1):_constraint_set1_flag
	int set2_flag = bsr->Read();															///< u(1):_constraint_set2_flag
	int set3_flag = bsr->Read();															///< u(1):_constraint_set3_flag
	bsr->Read(4);																							///< u(4), reserved_zero_4bits ignored.
	int li = bsr->Read(8);																		///< u(8):_level_idc
	bitsUsedSoFar += 24;

	/// _seq_parameter_set_id
	int index = _pHeaderUnsignedVlcDec->Decode(bsr);
	numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
	if(numBits == 0)	///< Return = 0 implies no valid vlc code.
		goto H264V2_RSPS_NOVLC_READ;
	bitsUsedSoFar += numBits;
	/// Vlc length unknown and so only checked after the fact but assumes non-destructive read.
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RSPS_RUNOUTOFBITS_READ;
	*idx = index;

	/// Range check on the array index.
	if( (index < 0)||(index > 31) )
	{
		_errorStr = "[H264Codec::ReadSeqParamSet] Sequence parameter set index out of range";
		*bitsUsed = bitsUsedSoFar;
		return(2);
	}//end if index...

  /// Store the current set.
  tmpParamSet.Copy(&(_seqParam[index]));

	/// The array index is now known and within range so the structure can be checked and loaded from the
	/// temporary variables.
	_seqParam[index]._profile_idc						= pi;
	_seqParam[index]._constraint_set0_flag	= set0_flag;
	_seqParam[index]._constraint_set1_flag	= set1_flag;
	_seqParam[index]._constraint_set2_flag	= set2_flag;
	_seqParam[index]._constraint_set3_flag	= set3_flag;
	_seqParam[index]._level_idc							= li;
	_seqParam[index]._seq_parameter_set_id	= index;

	// TODO: Add support for other profiles here
	/// _chroma_format_idc, _residual_colour_transform_flag, _bit_depth_luma_minus8, _bit_depth_chroma_minus8,
	/// _qpprime_y_zero_transform_bypass_flag, _seq_scaling_matrix_present_flag, _seq_scaling_list_present_flag[8].

	/// _log2_max_frame_num_minus4
	_seqParam[index]._log2_max_frame_num_minus4 = _pHeaderUnsignedVlcDec->Decode(bsr);
	numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
	if(numBits == 0)	///< Return = 0 implies no valid vlc code.
		goto H264V2_RSPS_NOVLC_READ;
	bitsUsedSoFar += numBits;
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RSPS_RUNOUTOFBITS_READ;

	/// _pic_order_cnt_type
	_seqParam[index]._pic_order_cnt_type = _pHeaderUnsignedVlcDec->Decode(bsr);
	numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
	if(numBits == 0)	///< Return = 0 implies no valid vlc code.
		goto H264V2_RSPS_NOVLC_READ;
	bitsUsedSoFar += numBits;
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RSPS_RUNOUTOFBITS_READ;

	if(_seqParam[index]._pic_order_cnt_type == 0)
	{
		/// _log2_max_pic_order_cnt_lsb_minus4
		_seqParam[index]._log2_max_pic_order_cnt_lsb_minus4 = _pHeaderUnsignedVlcDec->Decode(bsr);
		numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
		if(numBits == 0)	///< Return = 0 implies no valid vlc code.
			goto H264V2_RSPS_NOVLC_READ;
		bitsUsedSoFar += numBits;
		if( bitsUsedSoFar > remainingBits )
			goto H264V2_RSPS_RUNOUTOFBITS_READ;
	}//end if _pic_order_cnt_type...
	else if(_seqParam[index]._pic_order_cnt_type == 1)
	{
		/// _delta_pic_order_always_zero_flag
		_seqParam[index]._delta_pic_order_always_zero_flag = bsr->Read();
		bitsUsedSoFar++;
		if( bitsUsedSoFar > remainingBits )
			goto H264V2_RSPS_RUNOUTOFBITS_READ;

		/// _offset_for_non_ref_pic
		_seqParam[index]._offset_for_non_ref_pic = _pHeaderSignedVlcDec->Decode(bsr);
		numBits = _pHeaderSignedVlcDec->GetNumDecodedBits();
		if(numBits == 0)	///< Return = 0 implies no valid vlc code.
			goto H264V2_RSPS_NOVLC_READ;
		bitsUsedSoFar += numBits;
		if( bitsUsedSoFar > remainingBits )
			goto H264V2_RSPS_RUNOUTOFBITS_READ;

		/// _offset_for_top_to_bottom_field
		_seqParam[index]._offset_for_top_to_bottom_field = _pHeaderSignedVlcDec->Decode(bsr);
		numBits = _pHeaderSignedVlcDec->GetNumDecodedBits();
		if(numBits == 0)	///< Return = 0 implies no valid vlc code.
			goto H264V2_RSPS_NOVLC_READ;
		bitsUsedSoFar += numBits;
		if( bitsUsedSoFar > remainingBits )
			goto H264V2_RSPS_RUNOUTOFBITS_READ;

		/// _num_ref_frames_in_pic_order_cnt_cycle
		_seqParam[index]._num_ref_frames_in_pic_order_cnt_cycle = _pHeaderUnsignedVlcDec->Decode(bsr);
		numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
		if(numBits == 0)	///< Return = 0 implies no valid vlc code.
			goto H264V2_RSPS_NOVLC_READ;
		bitsUsedSoFar += numBits;
		if( bitsUsedSoFar > remainingBits )
			goto H264V2_RSPS_RUNOUTOFBITS_READ;

		for(i = 0; i < _seqParam[index]._num_ref_frames_in_pic_order_cnt_cycle; i++)
		{
			/// _offset_for_ref_frame[256]
			_seqParam[index]._offset_for_ref_frame[i] = _pHeaderSignedVlcDec->Decode(bsr);
			numBits = _pHeaderSignedVlcDec->GetNumDecodedBits();
			if(numBits == 0)	///< Return = 0 implies no valid vlc code.
				goto H264V2_RSPS_NOVLC_READ;
			bitsUsedSoFar += numBits;
			if( bitsUsedSoFar > remainingBits )
				goto H264V2_RSPS_RUNOUTOFBITS_READ;
		}//end for i...

	}//end else if _pic_order_cnt_type...

	/// _num_ref_frames
	_seqParam[index]._num_ref_frames = _pHeaderUnsignedVlcDec->Decode(bsr);
	numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
	if(numBits == 0)	///< Return = 0 implies no valid vlc code.
		goto H264V2_RSPS_NOVLC_READ;
	bitsUsedSoFar += numBits;
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RSPS_RUNOUTOFBITS_READ;

	/// _gaps_in_frame_num_value_allowed_flag
	_seqParam[index]._gaps_in_frame_num_value_allowed_flag = bsr->Read();
	bitsUsedSoFar++;
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RSPS_RUNOUTOFBITS_READ;

	/// _pic_width_in_mbs_minus1
	_seqParam[index]._pic_width_in_mbs_minus1 = _pHeaderUnsignedVlcDec->Decode(bsr);
	numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
	if(numBits == 0)	///< Return = 0 implies no valid vlc code.
		goto H264V2_RSPS_NOVLC_READ;
	bitsUsedSoFar += numBits;
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RSPS_RUNOUTOFBITS_READ;

	/// _pic_height_in_map_units_minus1
	_seqParam[index]._pic_height_in_map_units_minus1 = _pHeaderUnsignedVlcDec->Decode(bsr);
	numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
	if(numBits == 0)	///< Return = 0 implies no valid vlc code.
		goto H264V2_RSPS_NOVLC_READ;
	bitsUsedSoFar += numBits;
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RSPS_RUNOUTOFBITS_READ;

	/// _frame_mbs_only_flag
	_seqParam[index]._frame_mbs_only_flag = bsr->Read();
	bitsUsedSoFar++;
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RSPS_RUNOUTOFBITS_READ;

	if(!_seqParam[index]._frame_mbs_only_flag)
	{
		/// _mb_adaptive_frame_field_flag
		_seqParam[index]._mb_adaptive_frame_field_flag = bsr->Read();
		bitsUsedSoFar++;
		if( bitsUsedSoFar > remainingBits )
			goto H264V2_RSPS_RUNOUTOFBITS_READ;
	}//end if !_frame_mbs_only_flag...

	/// _direct_8x8_inference_flag
	_seqParam[index]._direct_8x8_inference_flag = bsr->Read();
	bitsUsedSoFar++;
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RSPS_RUNOUTOFBITS_READ;

	/// _frame_cropping_flag
	_seqParam[index]._frame_cropping_flag = bsr->Read();
	bitsUsedSoFar++;
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RSPS_RUNOUTOFBITS_READ;

	if(_seqParam[index]._frame_cropping_flag)
	{
		/// _frame_crop_left_offset
		_seqParam[index]._frame_crop_left_offset = _pHeaderUnsignedVlcDec->Decode(bsr);
		numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
		if(numBits == 0)	///< Return = 0 implies no valid vlc code.
			goto H264V2_RSPS_NOVLC_READ;
		bitsUsedSoFar += numBits;
		if( bitsUsedSoFar > remainingBits )
			goto H264V2_RSPS_RUNOUTOFBITS_READ;
		/// _frame_crop_right_offset
		_seqParam[index]._frame_crop_right_offset = _pHeaderUnsignedVlcDec->Decode(bsr);
		numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
		if(numBits == 0)	///< Return = 0 implies no valid vlc code.
			goto H264V2_RSPS_NOVLC_READ;
		bitsUsedSoFar += numBits;
		if( bitsUsedSoFar > remainingBits )
			goto H264V2_RSPS_RUNOUTOFBITS_READ;
		/// _frame_crop_top_offset
		_seqParam[index]._frame_crop_top_offset = _pHeaderUnsignedVlcDec->Decode(bsr);
		numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
		if(numBits == 0)	///< Return = 0 implies no valid vlc code.
			goto H264V2_RSPS_NOVLC_READ;
		bitsUsedSoFar += numBits;
		if( bitsUsedSoFar > remainingBits )
			goto H264V2_RSPS_RUNOUTOFBITS_READ;
		/// _frame_crop_bottom_offset
		_seqParam[index]._frame_crop_bottom_offset = _pHeaderUnsignedVlcDec->Decode(bsr);
		numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
		if(numBits == 0)	///< Return = 0 implies no valid vlc code.
			goto H264V2_RSPS_NOVLC_READ;
		bitsUsedSoFar += numBits;
		if( bitsUsedSoFar > remainingBits )
			goto H264V2_RSPS_RUNOUTOFBITS_READ;
	}//end if _frame_cropping_flag...

	/// _vui_parameters_present_flag
	_seqParam[index]._vui_parameters_present_flag = bsr->Read();
	bitsUsedSoFar++;
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RSPS_RUNOUTOFBITS_READ;

	if(_seqParam[index]._vui_parameters_present_flag)
	{
		//TODO: Implement VUI encoding.
	}//end if _vui_parameters_present_flag...

  /// Check if this new set is different to the old stored at this index.
  *changedFlag = 1;
  if(tmpParamSet.Equals(&(_seqParam[index])))
    *changedFlag = 0;

	*bitsUsed = bitsUsedSoFar;
	return(0);

	H264V2_RSPS_RUNOUTOFBITS_READ:
	_errorStr = "[H264Codec::ReadSeqParamSet] Insufficient bits to decode the picture";
		*bitsUsed = bitsUsedSoFar;
		return(1);

	H264V2_RSPS_NOVLC_READ:
		_errorStr = "[H264Codec::ReadSeqParamSet] No valid vlc in bit stream";
		*bitsUsed = bitsUsedSoFar;
		return(2);

}//end ReadSeqParamSet.

/** Set the Picture parameter set.
Set one of the pic param sets defined by the index parameters where the 
values are determined by interpreting the codec parameters.
@param index	:	Pic parameter set to modify [0..255].
@param seqRef	: Seq param set to reference.
@return				: Success = 1, failure = 0.
*/
int H264v2Codec::SetPicParamSet(int index, int seqRef)
{
	if((index > 255)||(seqRef > 31))
		return(0);

	_picParam[index]._pic_parameter_set_id										= index;	///< The index for this definition of picture params [0..255].
	_picParam[index]._seq_parameter_set_id										= seqRef;	///< This pic param set refers to this seq param set. Range = [0..31].
	_picParam[index]._entropy_coding_mode_flag								= 0;			///< Baseline profile = 0. CAVLC/CABAC = 0/1.
	_picParam[index]._pic_order_present_flag									= 0;			///< Pic order count syntax elements are present in the slice header.
	_picParam[index]._num_slice_groups_minus1									= 0;			///< = 0 (baseline profile). Num of slice groups for a pic.
	_picParam[index]._slice_group_map_type										= 0;			///< Defines map units to slice groups. Range = [0..6].
	/// Not initialised: _run_length_minus1[7];									///< Slice group mapping.
	/// Not initialised: _top_left[7];													///< Slice group mapping.
	/// Not initialised: _bottom_right[7];											///< Slice group mapping.
	_picParam[index]._slice_group_change_direction_flag				= 0;			///< Slice group mapping.
	_picParam[index]._slice_group_change_rate_minus1					= 0;			///< Slice group mapping.
	/// Not initialised: _slice_group_id[7];										///< Slice group mapping.
	_picParam[index]._num_ref_idx_l0_active_minus1						= 0;			///< Max ref index for ref pic list0.
	_picParam[index]._num_ref_idx_l1_active_minus1						= 0;			///< Max ref index for ref pic list1. (For B-Slices)
	_picParam[index]._weighted_pred_flag											= 0;			///< Indicates weighted prediction applied to P-Slices.
	_picParam[index]._weighted_bipred_idc											= 0;			///< Weighted prediction type applied to B-Slices.
	_picParam[index]._pic_init_qp_minus26											= 0;			///< Initial value of lum SliceQP - 26 for each slice. The slice_qp_delta modifies this value. Rng = [-26..25].
	_picParam[index]._pic_init_qs_minus26											= 0;			///< For SP and SI slices.
	_picParam[index]._chroma_qp_index_offset									= 0;			///< Offset added to lum QP (and QS) for Cb chr QP values. Rng = [-12..12].
	_picParam[index]._second_chroma_qp_index_offset						= 0;			///< For Cr chr QP. When not present = _chroma_qp_index_offset above.
	_picParam[index]._deblocking_filter_control_present_flag	= 0;			///< Indicates presence of elements in slice header to change the characteristics of the deblocking filter.
	_picParam[index]._constrained_intra_pred_flag							= 1;			///< = 1. Indicates that intra macroblock prediction can only be done from other intra macroblocks. 
	_picParam[index]._redundant_pic_cnt_present_flag					= 0;			///< Indicates that redundant pic count elements are in the slice header.
	_picParam[index]._transform_8x8_mode_flag									= 0;			///< = 0. Indicates 8x8 transform is used. When not present = 0.
	_picParam[index]._pic_scaling_matrix_present_flag					= 0;			///< = 0. Indicates params are present to modify the scaling lists specified in the seq param sets.
	/// Not initialised:_pic_scaling_list_present_flag[8];			///< Scaling list syntax structure present for scaling list i = 0..7.

	_picParam[index]._pic_size_in_map_units_minus1	= ((_width/16)*(_height/16)) - 1;	///< Num of map units (macroblocks for frames) in the pic. Macroblock width * height.

	return(1);
}//end SetPicParamSet.

/** Write the picture parameter set to a bit stream.
The pic param set must be correctly defined before this method is called. The 
vlc encoding is performed first before writing. Each write checks if a bit 
overflow will occur before writing. The check is sufficiently frequent to 
warrant an early exit GOTO statement. If the input stream param is NULL then 
this method is used to count the bits only.
@param bsw					: Stream to write into.
@param allowedBits	: Upper limit to the writable bits.
@param bitsUsed			: Return the actual bits used.
@param index				: Picture parameter set index to write.
@return							: Run out of bits = 1, more bits available = 0, Vlc error = 2, unsupported parameter set = 3.
*/
int H264v2Codec::WritePicParamSet(IBitStreamWriter* bsw, int allowedBits, int* bitsUsed, int index)
{
	int	bitCount;
	int bitsUsedSoFar = 0;

	/// _pic_parameter_set_id
	bitCount = _pHeaderUnsignedVlcEnc->Encode(_picParam[index]._pic_parameter_set_id);
	if(bitCount <= 0)	///< Vlc codec errors are detected from a zero or negative return value.
		goto H264V2_WPPS_VLCERROR_WRITE;
	if( (bitsUsedSoFar + bitCount) > allowedBits )
		goto H264V2_WPPS_RUNOUTOFBITS_WRITE;
	if(bsw)
		bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
	bitsUsedSoFar += bitCount;

	/// _seq_parameter_set_id
	bitCount = _pHeaderUnsignedVlcEnc->Encode(_picParam[index]._seq_parameter_set_id);
	if(bitCount <= 0)	///< Vlc codec errors are detected from a zero or negative return value.
		goto H264V2_WPPS_VLCERROR_WRITE;
	if( (bitsUsedSoFar + bitCount) > allowedBits )
		goto H264V2_WPPS_RUNOUTOFBITS_WRITE;
	if(bsw)
		bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
	bitsUsedSoFar += bitCount;

	/// _entropy_coding_mode_flag
	if( (bitsUsedSoFar + 1) > allowedBits )
		goto H264V2_WPPS_RUNOUTOFBITS_WRITE;
	if(bsw)
		bsw->Write(_picParam[index]._entropy_coding_mode_flag);
	bitsUsedSoFar++;

	/// _pic_order_present_flag
	if( (bitsUsedSoFar + 1) > allowedBits )
		goto H264V2_WPPS_RUNOUTOFBITS_WRITE;
	if(bsw)
		bsw->Write(_picParam[index]._pic_order_present_flag);
	bitsUsedSoFar++;

	/// _num_slice_groups_minus1
	bitCount = _pHeaderUnsignedVlcEnc->Encode(_picParam[index]._num_slice_groups_minus1);
	if(bitCount <= 0)	///< Vlc codec errors are detected from a zero or negative return value.
		goto H264V2_WPPS_VLCERROR_WRITE;
	if( (bitsUsedSoFar + bitCount) > allowedBits )
		goto H264V2_WPPS_RUNOUTOFBITS_WRITE;
	if(bsw)
		bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
	bitsUsedSoFar += bitCount;

	if(_picParam[index]._num_slice_groups_minus1 > 0)
	{
		// TODO: Add support for slice groups here
		goto H264V2_WPPS_MODEERROR_WRITE;

		/// _slice_group_map_type, _run_length_minus1[7], _top_left[7], _bottom_right[7], _slice_group_change_direction_flag,
		/// _slice_group_change_rate_minus1, _pic_size_in_map_units_minus1, _slice_group_id[].
	}//end if _num_slice_groups_minus1...

	/// _num_ref_idx_l0_active_minus1
	bitCount = _pHeaderUnsignedVlcEnc->Encode(_picParam[index]._num_ref_idx_l0_active_minus1);
	if(bitCount <= 0)	///< Vlc codec errors are detected from a zero or negative return value.
		goto H264V2_WPPS_VLCERROR_WRITE;
	if( (bitsUsedSoFar + bitCount) > allowedBits )
		goto H264V2_WPPS_RUNOUTOFBITS_WRITE;
	if(bsw)
		bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
	bitsUsedSoFar += bitCount;

	/// _num_ref_idx_l1_active_minus1
	bitCount = _pHeaderUnsignedVlcEnc->Encode(_picParam[index]._num_ref_idx_l1_active_minus1);
	if(bitCount <= 0)	///< Vlc codec errors are detected from a zero or negative return value.
		goto H264V2_WPPS_VLCERROR_WRITE;
	if( (bitsUsedSoFar + bitCount) > allowedBits )
		goto H264V2_WPPS_RUNOUTOFBITS_WRITE;
	if(bsw)
		bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
	bitsUsedSoFar += bitCount;

	/// _weighted_pred_flag
	if( (bitsUsedSoFar + 1) > allowedBits )
		goto H264V2_WPPS_RUNOUTOFBITS_WRITE;
	if(bsw)
		bsw->Write(_picParam[index]._weighted_pred_flag);
	bitsUsedSoFar++;

	/// _weighted_bipred_idc
	if( (bitsUsedSoFar + 2) > allowedBits )
		goto H264V2_WPPS_RUNOUTOFBITS_WRITE;
	if(bsw)
		bsw->Write(2, _picParam[index]._weighted_bipred_idc);
	bitsUsedSoFar += 2;

	/// _pic_init_qp_minus26
	bitCount = _pHeaderSignedVlcEnc->Encode(_picParam[index]._pic_init_qp_minus26);
	if(bitCount <= 0)	///< Vlc codec errors are detected from a zero or negative return value.
		goto H264V2_WPPS_VLCERROR_WRITE;
	if( (bitsUsedSoFar + bitCount) > allowedBits )
		goto H264V2_WPPS_RUNOUTOFBITS_WRITE;
	if(bsw)
		bsw->Write(bitCount, _pHeaderSignedVlcEnc->GetCode());
	bitsUsedSoFar += bitCount;

	/// _pic_init_qs_minus26
	bitCount = _pHeaderSignedVlcEnc->Encode(_picParam[index]._pic_init_qs_minus26);
	if(bitCount <= 0)	///< Vlc codec errors are detected from a zero or negative return value.
		goto H264V2_WPPS_VLCERROR_WRITE;
	if( (bitsUsedSoFar + bitCount) > allowedBits )
		goto H264V2_WPPS_RUNOUTOFBITS_WRITE;
	if(bsw)
		bsw->Write(bitCount, _pHeaderSignedVlcEnc->GetCode());
	bitsUsedSoFar += bitCount;

	/// _chroma_qp_index_offset
	bitCount = _pHeaderSignedVlcEnc->Encode(_picParam[index]._chroma_qp_index_offset);
	if(bitCount <= 0)	///< Vlc codec errors are detected from a zero or negative return value.
		goto H264V2_WPPS_VLCERROR_WRITE;
	if( (bitsUsedSoFar + bitCount) > allowedBits )
		goto H264V2_WPPS_RUNOUTOFBITS_WRITE;
	if(bsw)
		bsw->Write(bitCount, _pHeaderSignedVlcEnc->GetCode());
	bitsUsedSoFar += bitCount;

	/// _deblocking_filter_control_present_flag
	if( (bitsUsedSoFar + 1) > allowedBits )
		goto H264V2_WPPS_RUNOUTOFBITS_WRITE;
	if(bsw)
		bsw->Write(_picParam[index]._deblocking_filter_control_present_flag);
	bitsUsedSoFar++;

	/// _constrained_intra_pred_flag
	if( (bitsUsedSoFar + 1) > allowedBits )
		goto H264V2_WPPS_RUNOUTOFBITS_WRITE;
	if(bsw)
		bsw->Write(_picParam[index]._constrained_intra_pred_flag);
	bitsUsedSoFar++;

	/// _redundant_pic_cnt_present_flag
	if( (bitsUsedSoFar + 1) > allowedBits )
		goto H264V2_WPPS_RUNOUTOFBITS_WRITE;
	if(bsw)
		bsw->Write(_picParam[index]._redundant_pic_cnt_present_flag);
	bitsUsedSoFar++;

	//TODO: Implement if more_rbsp_data() section here.
	/// _transform_8x8_mode_flag, _pic_scaling_matrix_present_flag, _pic_scaling_list_present_flag[8],
	/// _second_chroma_qp_index_offset.

	*bitsUsed = bitsUsedSoFar;
	return(0);

	H264V2_WPPS_RUNOUTOFBITS_WRITE:
		_errorStr = "H264V2:[H264Codec::WritePicParamSet] Bits required exceeds max available for picture";
		*bitsUsed = bitsUsedSoFar;
		return(1);

	H264V2_WPPS_VLCERROR_WRITE:
		_errorStr = "H264V2:[H264Codec::WritePicParamSet] Vlc encoder error";
		*bitsUsed = bitsUsedSoFar;
		return(2);

	H264V2_WPPS_MODEERROR_WRITE:
		_errorStr = "H264V2:[H264Codec::WritePicParamSet] Picture parameter not implemented";
		*bitsUsed = bitsUsedSoFar;
		return(2);

}//end WritePicParamSet.

/** Read the picture parameter set from the bit stream.
This impementation reads picture parameter set encodings. Each read checks if a 
bit underflow will occur after reading. Check for loss of vlc sync wherever possible. 
The checks are sufficiently frequent to warrant an early exit GOTO statement.
@param bsr						: Stream to read from.
@param remainingBits	: Upper limit to the readable bits.
@param bitsUsed				: Return the actual bits extracted.
@param idx						: Reference to pic param set index read.
@param changedFlag    : Flag to indicate that param set has changed to a new one.
@return								: Run out of bits = 1, more bits available = 0, error = 2.
*/
int H264v2Codec::ReadPicParamSet(IBitStreamReader* bsr, int remainingBits, int* bitsUsed, int* idx, int* changedFlag)
{
	int bitsUsedSoFar = 0;
	int numBits;
  PicParamSetH264 tmpParamSet;

	/// Check existance of stream reader.
	if(bsr == NULL)
	{
		_errorStr = "[H264Codec::ReadPicParamSet] No bit stream reader available";
		*bitsUsed = 0;
		return(2);
	}//end if bsr...

	/// _pic_parameter_set_id
	int index = _pHeaderUnsignedVlcDec->Decode(bsr);
	numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
	if(numBits == 0)	///< Return = 0 implies no valid vlc code.
		goto H264V2_RPPS_NOVLC_READ;
	bitsUsedSoFar += numBits;
	/// Vlc length unknown and so only checked after the fact but assumes non-destructive read.
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RPPS_RUNOUTOFBITS_READ;
	*idx = index;

	/// Range check on the pic param array index.
	if( (index < 0)||(index > 255) )
	{
		_errorStr = "[H264Codec::ReadPicParamSet] Picture parameter set index out of range";
		*bitsUsed = bitsUsedSoFar;
		return(2);
	}//end if index...

  /// Store the current set.
  tmpParamSet.Copy(&(_picParam[index]));

	/// The decoded index identifies the param set to use.
	_picParam[index]._pic_parameter_set_id = index;

	/// _seq_parameter_set_id
	_picParam[index]._seq_parameter_set_id = _pHeaderUnsignedVlcDec->Decode(bsr);
	numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
	if(numBits == 0)	///< Return = 0 implies no valid vlc code.
		goto H264V2_RPPS_NOVLC_READ;
	bitsUsedSoFar += numBits;
	/// Vlc length unknown and so only checked after the fact but assumes non-destructive read.
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RPPS_RUNOUTOFBITS_READ;

	/// Range check on the seq param array index. This does not test if the data in the _seqParam[]
	/// array for this id is valid.
	if( (_picParam[index]._seq_parameter_set_id < 0)||(_picParam[index]._seq_parameter_set_id > 31) )
	{
		_errorStr = "H264V2:[H264Codec::ReadPicParamSet] Sequence parameter set index of picture parameter set is out of range";
		*bitsUsed = bitsUsedSoFar;
		return(2);
	}//end if index...

	/// _entropy_coding_mode_flag
	_picParam[index]._entropy_coding_mode_flag = bsr->Read();
	bitsUsedSoFar++;
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RPPS_RUNOUTOFBITS_READ;

	/// _pic_order_present_flag
	_picParam[index]._pic_order_present_flag = bsr->Read();
	bitsUsedSoFar++;
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RPPS_RUNOUTOFBITS_READ;

	/// _num_slice_groups_minus1
	_picParam[index]._num_slice_groups_minus1 = _pHeaderUnsignedVlcDec->Decode(bsr);
	numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
	if(numBits == 0)	///< Return = 0 implies no valid vlc code.
		goto H264V2_RPPS_NOVLC_READ;
	bitsUsedSoFar += numBits;
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RPPS_RUNOUTOFBITS_READ;

	if(_picParam[index]._num_slice_groups_minus1 > 0)
	{
		// TODO: Add support for slice groups here
		goto H264V2_RPPS_NOMODE_READ;

		/// _slice_group_map_type, _run_length_minus1[7], _top_left[7], _bottom_right[7], _slice_group_change_direction_flag,
		/// _slice_group_change_rate_minus1, _pic_size_in_map_units_minus1, _slice_group_id[].
	}//end if _num_slice_groups_minus1...

	/// _num_ref_idx_l0_active_minus1
	_picParam[index]._num_ref_idx_l0_active_minus1 = _pHeaderUnsignedVlcDec->Decode(bsr);
	numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
	if(numBits == 0)	///< Return = 0 implies no valid vlc code.
		goto H264V2_RPPS_NOVLC_READ;
	bitsUsedSoFar += numBits;
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RPPS_RUNOUTOFBITS_READ;

	/// _num_ref_idx_l1_active_minus1
	_picParam[index]._num_ref_idx_l1_active_minus1 = _pHeaderUnsignedVlcDec->Decode(bsr);
	numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
	if(numBits == 0)	///< Return = 0 implies no valid vlc code.
		goto H264V2_RPPS_NOVLC_READ;
	bitsUsedSoFar += numBits;
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RPPS_RUNOUTOFBITS_READ;

	/// _weighted_pred_flag
	_picParam[index]._weighted_pred_flag = bsr->Read();
	bitsUsedSoFar++;
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RPPS_RUNOUTOFBITS_READ;

	/// _weighted_bipred_idc
	_picParam[index]._weighted_bipred_idc = bsr->Read(2);
	bitsUsedSoFar += 2;
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RPPS_RUNOUTOFBITS_READ;

	/// _pic_init_qp_minus26
	_picParam[index]._pic_init_qp_minus26 = _pHeaderSignedVlcDec->Decode(bsr);
	numBits = _pHeaderSignedVlcDec->GetNumDecodedBits();
	if(numBits == 0)	///< Return = 0 implies no valid vlc code.
		goto H264V2_RPPS_NOVLC_READ;
	bitsUsedSoFar += numBits;
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RPPS_RUNOUTOFBITS_READ;

	/// _pic_init_qs_minus26
	_picParam[index]._pic_init_qs_minus26 = _pHeaderSignedVlcDec->Decode(bsr);
	numBits = _pHeaderSignedVlcDec->GetNumDecodedBits();
	if(numBits == 0)	///< Return = 0 implies no valid vlc code.
		goto H264V2_RPPS_NOVLC_READ;
	bitsUsedSoFar += numBits;
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RPPS_RUNOUTOFBITS_READ;

	/// _chroma_qp_index_offset
	_picParam[index]._chroma_qp_index_offset = _pHeaderSignedVlcDec->Decode(bsr);
	numBits = _pHeaderSignedVlcDec->GetNumDecodedBits();
	if(numBits == 0)	///< Return = 0 implies no valid vlc code.
		goto H264V2_RPPS_NOVLC_READ;
	bitsUsedSoFar += numBits;
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RPPS_RUNOUTOFBITS_READ;

	/// _deblocking_filter_control_present_flag
	_picParam[index]._deblocking_filter_control_present_flag = bsr->Read();
	bitsUsedSoFar++;
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RPPS_RUNOUTOFBITS_READ;

	/// _constrained_intra_pred_flag
	_picParam[index]._constrained_intra_pred_flag = bsr->Read();
	bitsUsedSoFar++;
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RPPS_RUNOUTOFBITS_READ;

	/// _redundant_pic_cnt_present_flag
	_picParam[index]._redundant_pic_cnt_present_flag = bsr->Read();
	bitsUsedSoFar++;
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RPPS_RUNOUTOFBITS_READ;

	//TODO: Implement if more_rbsp_data() section here.
	/// _transform_8x8_mode_flag, _pic_scaling_matrix_present_flag, _pic_scaling_list_present_flag[8],
	/// _second_chroma_qp_index_offset.

  /// Check if this new set is different to the old stored at this index.
  *changedFlag = 1;
  if(tmpParamSet.Equals(&(_picParam[index])))
    *changedFlag = 0;

	*bitsUsed = bitsUsedSoFar;
	return(0);

	H264V2_RPPS_RUNOUTOFBITS_READ:
	_errorStr = "[H264Codec::ReadPicParamSet] Insufficient bits to decode the picture";
		*bitsUsed = bitsUsedSoFar;
		return(1);

	H264V2_RPPS_NOVLC_READ:
		_errorStr = "[H264Codec::ReadPicParamSet] No valid vlc in bit stream";
		*bitsUsed = bitsUsedSoFar;
		return(2);

	H264V2_RPPS_NOMODE_READ:
		_errorStr = "[H264Codec::ReadPicParamSet] Picture parameter not implemented";
		*bitsUsed = bitsUsedSoFar;
		return(2);

}//end ReadPicParamSet.

/** Get the codec parameters.
Load the codec params from the picture param set indexed by
the input param. The sequence param set is implied in the referenced
pic param set. The pic/seq param set must be previously defined
prior to calling this method.
@param picParamSet	:	Pic parameter set to get from [0..255].
@return							: Success = 1, failure = 0.
*/
int H264v2Codec::GetCodecParams(int picParamSet)
{
	///----------- Some parameter set range checking ------------------------------
	if( (picParamSet < 0) || (picParamSet > 255) )
	{
		_errorStr = "[H264Codec::GetCodecParams] Picture parameter set out of range";
		return(0);
	}//end if picParamSet...
	if(_picParam[picParamSet]._pic_parameter_set_id != picParamSet)
	{
		_errorStr = "[H264Codec::GetCodecParams] Picture parameter set mismatch";
		return(0);
	}//end if _pic_parameter_set_id...
	int seqParamSet = _picParam[picParamSet]._seq_parameter_set_id;
	if( (seqParamSet < 0) || (seqParamSet > 31) )
	{
		_errorStr = "[H264Codec::GetCodecParams] Sequence parameter set out of range";
		return(0);
	}//end if seqParamSet...
	if(_seqParam[seqParamSet]._seq_parameter_set_id != seqParamSet)
	{
		_errorStr = "[H264Codec::GetCodecParams] Sequence parameter set mismatch";
		return(0);
	}//end if _seq_parameter_set_id...
	/// Modify the codec seq param set to that associated with this pic param set.
	_currSeqParam = seqParamSet;

	///----------- Extract params of interest for codec -----------------------
	/// Get width and height in pels.
	_width	= (_seqParam[seqParamSet]._pic_width_in_mbs_minus1	+ 1) * 16;
	_height = (_seqParam[seqParamSet]._pic_height_in_map_units_minus1	+ 1) * 16;

	///----------- Check params for this implementation -----------------------

	/// Sequence parameter implementation.
	if(	(_seqParam[seqParamSet]._profile_idc != 66) ||
			(_seqParam[seqParamSet]._constraint_set0_flag != 0) ||
			(_seqParam[seqParamSet]._constraint_set1_flag != 0) ||
			(_seqParam[seqParamSet]._constraint_set2_flag != 0) ||
			(_seqParam[seqParamSet]._constraint_set3_flag != 0) ||
			(_seqParam[seqParamSet]._level_idc != 20) ||
			(_seqParam[seqParamSet]._chroma_format_idc != 1) ||
			(_seqParam[seqParamSet]._bit_depth_luma_minus8 != 0) ||
			(_seqParam[seqParamSet]._bit_depth_chroma_minus8 != 0) ||
			(_seqParam[seqParamSet]._qpprime_y_zero_transform_bypass_flag != 0) ||
			(_seqParam[seqParamSet]._seq_scaling_matrix_present_flag != 0) ||
			(_seqParam[seqParamSet]._pic_order_cnt_type != 2) ||
			(_seqParam[seqParamSet]._num_ref_frames != 1) ||
			(_seqParam[seqParamSet]._gaps_in_frame_num_value_allowed_flag != 0) ||
			(_seqParam[seqParamSet]._frame_mbs_only_flag != 1) ||
			(_seqParam[seqParamSet]._mb_adaptive_frame_field_flag != 0) ||
			(_seqParam[seqParamSet]._direct_8x8_inference_flag != 0) ||
			(_seqParam[seqParamSet]._frame_cropping_flag != 0) ||
			(_seqParam[seqParamSet]._vui_parameters_present_flag != 0) )
	{
		_errorStr = "[H264Codec::GetCodecParams] Sequence parameters not implemented";
		return(0);
	}//end if _profile_idc...

	/// Picture parameter implementation.
	if(	(_picParam[picParamSet]._entropy_coding_mode_flag != 0) ||
			(_picParam[picParamSet]._pic_order_present_flag != 0) ||
			(_picParam[picParamSet]._num_slice_groups_minus1 != 0) ||
			(_picParam[picParamSet]._num_ref_idx_l0_active_minus1 != 0) ||
			(_picParam[picParamSet]._num_ref_idx_l1_active_minus1 != 0) ||
			(_picParam[picParamSet]._weighted_pred_flag != 0) ||
			(_picParam[picParamSet]._weighted_bipred_idc != 0) ||
			(_picParam[picParamSet]._constrained_intra_pred_flag != 1) ||
			(_picParam[picParamSet]._redundant_pic_cnt_present_flag != 0) ||
			(_picParam[picParamSet]._transform_8x8_mode_flag != 0) ||
			(_picParam[picParamSet]._pic_scaling_matrix_present_flag != 0) )
	{
		_errorStr = "[H264Codec::GetCodecParams] Picture parameters not implemented";
		return(0);
	}//end if _profile_idc...

	return(1);
}//end GetCodecParams.

/** Write the NAL header to a bit stream.
The NAL header data must be correctly defined before this method is called. The 
vlc encoding is performed first before writing. Each write checks if a bit overflow 
will occur before writing. The check is sufficiently frequent to warrant an early 
exit GOTO statement. If the input stream param is NULL then this method is used 
to count the bits only.
@param bsw					: Stream to write into.
@param allowedBits	: Upper limit to the writable bits.
@param bitsUsed			: Return the actual bits used.
@return							: Run out of bits = 1, more bits available = 0, Vlc error = 2.
*/
int H264v2Codec::WriteNALHeader(IBitStreamWriter* bsw, int allowedBits, int* bitsUsed)
{
	/// The NAL header is a fixed 8 bit sequence.
	if(allowedBits >= 8)
	{
		if(bsw)
		{
			bsw->Write(1, 0);												///< Forbidden zero bit f(1).
			bsw->Write(2, (_nal._ref_idc & 3));			///< u(2).		
			bsw->Write(5, (_nal._unit_type & 31));	///< u(5).		
		}//end if bsw != NULL...
	}//end if allowedBits...
	else
	{
		_errorStr = "H264V2:[WriteNALHeader] Bits required exceeds max available for NAL header";
		*bitsUsed = 0;
		return(1);
	}///end else...

	*bitsUsed = 8;
	return(0);
}//end WriteNALHeader.

/** Read the NAL header from the bit stream.
This impementation reads the NAL header encoding. Each read checks if a 
bit underflow will occur after reading. Check for loss of vlc sync 
wherever possible. The checks are sufficiently frequent to warrant 
an early exit GOTO statement.
@param bsr						: Stream to read from.
@param remainingBits	: Upper limit to the readable bits.
@param bitsUsed				: Return the actual bits extracted.
@return								: Run out of bits = 1, more bits available = 0, error = 2.
*/
int H264v2Codec::ReadNALHeader(IBitStreamReader* bsr, int remainingBits, int* bitsUsed)
{
	/// The NAL header is a fixed 8 bit sequence.
	if(remainingBits >= 8)
	{
		bsr->Read();													///< Discard forbidden zero bit f(1).
		_nal._ref_idc		= 3 & bsr->Read(2);		///< u(2).
		_nal._unit_type = 31 & bsr->Read(5);	///< u(5).
	}//end if remainingBits...
	else
	{
		_errorStr = "H264V2:[ReadNALHeader] Insufficient bits to decode the NAl header";
		*bitsUsed = 0;
		return(1);
	}//end else...

	*bitsUsed = 8;
	return(0);
}//end ReadNALHeader.

/** Write the slice layer header to a bit stream.
The encodings of all the slice data must be correctly defined before 
this method is called. The vlc encoding is performed first before
writing. Each write checks if a bit overflow will occur before writing. 
The check is sufficiently frequent to warrant an early exit GOTO 
statement. If the input stream param is NULL then this method is used 
to count the bits only.
@param bsw					: Stream to write into.
@param allowedBits	: Upper limit to the writable bits.
@param bitsUsed			: Return the actual bits used.
@return							: Run out of bits = 1, more bits available = 0, Vlc error = 2.
*/
int H264v2Codec::WriteSliceLayerHeader(IBitStreamWriter* bsw, int allowedBits, int* bitsUsed)
{
	int	bitCount;
	int bitsUsedSoFar = 0;

	/// Limited implementation of modes and types for Baseline profile.
	if((_slice._type != SliceHeaderH264::I_Slice) && (_slice._type != SliceHeaderH264::I_Slice_All) && 
     (_slice._type != SliceHeaderH264::P_Slice) && (_slice._type != SliceHeaderH264::P_Slice_All))
		goto H264V2_WSLH_MODEERROR_WRITE;

	///-------------------------- Addr of first macroblock -----------------------------------
	bitCount = _pHeaderUnsignedVlcEnc->Encode(_slice._first_mb_in_slice);
	if(bitCount > 0)	///< Vlc codec errors are detected from a zero or negative return value.
	{
		if( (bitsUsedSoFar + bitCount) <= allowedBits )
		{
			if(bsw)
				bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
		}//end if bitsUsedSoFar...
		else
			goto H264V2_WSLH_RUNOUTOFBITS_WRITE;
	}//end if bitCount...
	else
		goto H264V2_WSLH_VLCERROR_WRITE;
	bitsUsedSoFar += bitCount;

	///-------------------------- Slice Type -------------------------------------------------
	bitCount = _pHeaderUnsignedVlcEnc->Encode(_slice._type);
	if(bitCount > 0)	///< Vlc codec errors are detected from a zero or negative return value.
	{
		if( (bitsUsedSoFar + bitCount) <= allowedBits )
		{
			if(bsw)
				bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
		}//end if bitsUsedSoFar...
		else
			goto H264V2_WSLH_RUNOUTOFBITS_WRITE;
	}//end if bitCount...
	else
		goto H264V2_WSLH_VLCERROR_WRITE;
	bitsUsedSoFar += bitCount;

	///-------------------------- Pic Param Set Id --------------------------------------------
	bitCount = _pHeaderUnsignedVlcEnc->Encode(_slice._pic_parameter_set_id);
	if(bitCount > 0)	///< Vlc codec errors are detected from a zero or negative return value.
	{
		if( (bitsUsedSoFar + bitCount) <= allowedBits )
		{
			if(bsw)
				bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
		}//end if bitsUsedSoFar...
		else
			goto H264V2_WSLH_RUNOUTOFBITS_WRITE;
	}//end if bitCount...
	else
		goto H264V2_WSLH_VLCERROR_WRITE;
	bitsUsedSoFar += bitCount;

	///-------------------------- Frame Number ----------------------------------------------
	bitCount = _seqParam[_picParam[_slice._pic_parameter_set_id]._seq_parameter_set_id]._log2_max_frame_num_minus4 + 4;
	if( (bitsUsedSoFar + bitCount) <= allowedBits )
	{
		if(bsw)
			bsw->Write(bitCount, _slice._frame_num);
	}//end if bitsUsedSoFar...
	else
		goto H264V2_WSLH_RUNOUTOFBITS_WRITE;
	bitsUsedSoFar += bitCount;

	///-------------------------- Field Flags ----------------------------------------------
	if(!_seqParam[_picParam[_slice._pic_parameter_set_id]._seq_parameter_set_id]._frame_mbs_only_flag)
	{
		if( (bitsUsedSoFar + 2) <= allowedBits )	///< Both flags together.
		{
			if(bsw)
				bsw->Write(_slice._field_pic_flag);
			bitsUsedSoFar++;
			if(_slice._field_pic_flag)
			{
				if(bsw)
					bsw->Write(_slice._bottom_field_flag);
				bitsUsedSoFar++;
			}//end if _field_pic_flag...
		}//end if bitsUsedSoFar...
		else
			goto H264V2_WSLH_RUNOUTOFBITS_WRITE;
	}//end if !_frame_mbs_only_flag...

	///-------------------------- IDR Frame Count ------------------------------------------
	if(_nal._unit_type == NalHeaderH264::IDR_Slice)
	{
		bitCount = _pHeaderUnsignedVlcEnc->Encode(_slice._idr_pic_id);
		if(bitCount > 0)	///< Vlc codec errors are detected from a zero or negative return value.
		{
			if( (bitsUsedSoFar + bitCount) <= allowedBits )
			{
				if(bsw)
					bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
			}//end if bitsUsedSoFar...
			else
				goto H264V2_WSLH_RUNOUTOFBITS_WRITE;
		}//end if bitCount...
		else
			goto H264V2_WSLH_VLCERROR_WRITE;
		bitsUsedSoFar += bitCount;
	}//end if _unit_type...

	///-------------------------- Picture Order Count ---------------------------------------
	if(_seqParam[_picParam[_slice._pic_parameter_set_id]._seq_parameter_set_id]._pic_order_cnt_type == 0)
	{
		bitCount = _seqParam[_picParam[_slice._pic_parameter_set_id]._seq_parameter_set_id]._log2_max_pic_order_cnt_lsb_minus4 + 4;
		if( (bitsUsedSoFar + bitCount) <= allowedBits )
		{
			if(bsw)
				bsw->Write(bitCount, _slice._pic_order_cnt_lsb);
		}//end if bitsUsedSoFar...
		else
			goto H264V2_WSLH_RUNOUTOFBITS_WRITE;
		bitsUsedSoFar += bitCount;

		if(_picParam[_slice._pic_parameter_set_id]._pic_order_present_flag && !_slice._field_pic_flag)
		{
			bitCount = _pHeaderSignedVlcEnc->Encode(_slice._delta_pic_order_cnt_bottom);
			if(bitCount > 0)	///< Vlc codec errors are detected from a zero or negative return value.
			{
				if( (bitsUsedSoFar + bitCount) <= allowedBits )
				{
					if(bsw)
						bsw->Write(bitCount, _pHeaderSignedVlcEnc->GetCode());
				}//end if bitsUsedSoFar...
				else
					goto H264V2_WSLH_RUNOUTOFBITS_WRITE;
			}//end if bitCount...
			else
				goto H264V2_WSLH_VLCERROR_WRITE;
			bitsUsedSoFar += bitCount;
		}//end if _pic_order_present_flag...
	}//end if _pic_order_cnt_type...
	else if( (_seqParam[_picParam[_slice._pic_parameter_set_id]._seq_parameter_set_id]._pic_order_cnt_type == 1) &&
						!_seqParam[_picParam[_slice._pic_parameter_set_id]._seq_parameter_set_id]._delta_pic_order_always_zero_flag )
	{
		bitCount = _pHeaderSignedVlcEnc->Encode(_slice._delta_pic_order_cnt[0]);
		if(bitCount > 0)	///< Vlc codec errors are detected from a zero or negative return value.
		{
			if( (bitsUsedSoFar + bitCount) <= allowedBits )
			{
				if(bsw)
					bsw->Write(bitCount, _pHeaderSignedVlcEnc->GetCode());
			}//end if bitsUsedSoFar...
			else
				goto H264V2_WSLH_RUNOUTOFBITS_WRITE;
		}//end if bitCount...
		else
			goto H264V2_WSLH_VLCERROR_WRITE;
		bitsUsedSoFar += bitCount;

		if(_picParam[_slice._pic_parameter_set_id]._pic_order_present_flag && !_slice._field_pic_flag)
		{
				bitCount = _pHeaderSignedVlcEnc->Encode(_slice._delta_pic_order_cnt[1]);
				if(bitCount > 0)	///< Vlc codec errors are detected from a zero or negative return value.
				{
					if( (bitsUsedSoFar + bitCount) <= allowedBits )
					{
						if(bsw)
							bsw->Write(bitCount, _pHeaderSignedVlcEnc->GetCode());
					}//end if bitsUsedSoFar...
					else
						goto H264V2_WSLH_RUNOUTOFBITS_WRITE;
				}//end if bitCount...
				else
					goto H264V2_WSLH_VLCERROR_WRITE;
				bitsUsedSoFar += bitCount;
		}//end if _pic_order_present_flag...
	}//end else if _pic_order_cnt_type...

	if(_picParam[_slice._pic_parameter_set_id]._redundant_pic_cnt_present_flag)
	{
		bitCount = _pHeaderUnsignedVlcEnc->Encode(_slice._redundant_pic_cnt);
		if(bitCount > 0)	///< Vlc codec errors are detected from a zero or negative return value.
		{
			if( (bitsUsedSoFar + bitCount) <= allowedBits )
			{
				if(bsw)
					bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
			}//end if bitsUsedSoFar...
			else
				goto H264V2_WSLH_RUNOUTOFBITS_WRITE;
		}//end if bitCount...
		else
			goto H264V2_WSLH_VLCERROR_WRITE;
		bitsUsedSoFar += bitCount;
	}//end if _redundant_pic_cnt_present_flag...

	///-------------------------- Direct Spatial Prediction -----------------------------------
	if((_slice._type == SliceHeaderH264::B_Slice)||(_slice._type == SliceHeaderH264::B_Slice_All))
	{
		if( (bitsUsedSoFar + 1) <= allowedBits )
		{
			if(bsw)
				bsw->Write(_slice._direct_spatial_mv_pred_flag);
			bitsUsedSoFar++;
		}//end if bitsUsedSoFar...
		else
			goto H264V2_WSLH_RUNOUTOFBITS_WRITE;
	}//end if B_Slice...

	///-------------------------- Active Reference Indices -----------------------------------
	if( (_slice._type == SliceHeaderH264::P_Slice)||(_slice._type == SliceHeaderH264::SP_Slice)||(_slice._type == SliceHeaderH264::B_Slice)||
      (_slice._type == SliceHeaderH264::P_Slice_All)||(_slice._type == SliceHeaderH264::SP_Slice_All)||(_slice._type == SliceHeaderH264::B_Slice_All) )
	{
		if( (bitsUsedSoFar + 1) <= allowedBits )
		{
			if(bsw)
				bsw->Write(_slice._num_ref_idx_active_override_flag);
			bitsUsedSoFar++;
		}//end if bitsUsedSoFar...
		else
			goto H264V2_WSLH_RUNOUTOFBITS_WRITE;

		if(_slice._num_ref_idx_active_override_flag)
		{
			bitCount = _pHeaderUnsignedVlcEnc->Encode(_slice._num_ref_idx_l0_active_minus1);
			if(bitCount > 0)	///< Vlc codec errors are detected from a zero or negative return value.
			{
				if( (bitsUsedSoFar + bitCount) <= allowedBits )
				{
					if(bsw)
						bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
				}//end if bitsUsedSoFar...
				else
					goto H264V2_WSLH_RUNOUTOFBITS_WRITE;
			}//end if bitCount...
			else
				goto H264V2_WSLH_VLCERROR_WRITE;
			bitsUsedSoFar += bitCount;

			if((_slice._type == SliceHeaderH264::B_Slice)||(_slice._type == SliceHeaderH264::B_Slice))
			{
				bitCount = _pHeaderUnsignedVlcEnc->Encode(_slice._num_ref_idx_l1_active_minus1);
				if(bitCount > 0)	///< Vlc codec errors are detected from a zero or negative return value.
				{
					if( (bitsUsedSoFar + bitCount) <= allowedBits )
					{
						if(bsw)
							bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
					}//end if bitsUsedSoFar...
					else
						goto H264V2_WSLH_RUNOUTOFBITS_WRITE;
				}//end if bitCount...
				else
					goto H264V2_WSLH_VLCERROR_WRITE;
				bitsUsedSoFar += bitCount;
			}//end if B_Slice...
		}//end if _num_ref_idx_active_override_flag...
	}//end if P_Slice...

	///-------------------------- Reference Pic list reordering --------------------------------
	if((_slice._type != SliceHeaderH264::I_Slice)&&(_slice._type != SliceHeaderH264::SI_Slice) &&
     (_slice._type != SliceHeaderH264::I_Slice_All)&&(_slice._type != SliceHeaderH264::SI_Slice_All))
	{
		if( (bitsUsedSoFar + 1) <= allowedBits )
		{
			if(bsw)
				bsw->Write(_slice._ref_pic_list_reordering_flag_l0);
			bitsUsedSoFar++;
		}//end if bitsUsedSoFar...
		else
			goto H264V2_WSLH_RUNOUTOFBITS_WRITE;

		if(_slice._ref_pic_list_reordering_flag_l0)
		{
			do
			{
				bitCount = _pHeaderUnsignedVlcEnc->Encode(_slice._reordering_of_pic_nums_idc);
				if(bitCount > 0)	///< Vlc codec errors are detected from a zero or negative return value.
				{
					if( (bitsUsedSoFar + bitCount) <= allowedBits )
					{
						if(bsw)
							bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
					}//end if bitsUsedSoFar...
					else
						goto H264V2_WSLH_RUNOUTOFBITS_WRITE;
				}//end if bitCount...
				else
					goto H264V2_WSLH_VLCERROR_WRITE;
				bitsUsedSoFar += bitCount;

				if((_slice._reordering_of_pic_nums_idc == 0)||(_slice._reordering_of_pic_nums_idc == 1))
				{

					// TODO: Prepare list 0 reordering with _slice._abs_diff_pic_num_minus1.

					bitCount = _pHeaderUnsignedVlcEnc->Encode(_slice._abs_diff_pic_num_minus1);
					if(bitCount > 0)	///< Vlc codec errors are detected from a zero or negative return value.
					{
						if( (bitsUsedSoFar + bitCount) <= allowedBits )
						{
							if(bsw)
								bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
						}//end if bitsUsedSoFar...
						else
							goto H264V2_WSLH_RUNOUTOFBITS_WRITE;
					}//end if bitCount...
					else
						goto H264V2_WSLH_VLCERROR_WRITE;
					bitsUsedSoFar += bitCount;
				}//end if _reordering_of_pic_nums_idc...
				else if(_slice._reordering_of_pic_nums_idc == 2)
				{

					// TODO: Prepare list 0 reordering with _slice._long_term_pic_num.

					bitCount = _pHeaderUnsignedVlcEnc->Encode(_slice._long_term_pic_num);
					if(bitCount > 0)	///< Vlc codec errors are detected from a zero or negative return value.
					{
						if( (bitsUsedSoFar + bitCount) <= allowedBits )
						{
							if(bsw)
								bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
						}//end if bitsUsedSoFar...
						else
							goto H264V2_WSLH_RUNOUTOFBITS_WRITE;
					}//end if bitCount...
					else
						goto H264V2_WSLH_VLCERROR_WRITE;
					bitsUsedSoFar += bitCount;
				}//end else if _reordering_of_pic_nums_idc...

			}while(_slice._reordering_of_pic_nums_idc != 3);
		}//end if _ref_pic_list_reordering_flag_l0...

	}//end if !I_Slice...

	if((_slice._type == SliceHeaderH264::B_Slice)||(_slice._type == SliceHeaderH264::B_Slice_All))
	{
		/// B Slices are not supported for the baseline profile.
		// TODO: B-slice reordering syntax.
		goto H264V2_WSLH_MODEERROR_WRITE;
	}//end if B_Slice...

	///-------------------------- Weighted Prediction Table --------------------------------
	if( (_picParam[_slice._pic_parameter_set_id]._weighted_pred_flag && ( (_slice._type == SliceHeaderH264::P_Slice)||(_slice._type == SliceHeaderH264::P_Slice_All)||(_slice._type == SliceHeaderH264::SP_Slice)||(_slice._type == SliceHeaderH264::SP_Slice_All) )) ||
		  ((_picParam[_slice._pic_parameter_set_id]._weighted_bipred_idc == 1)&&( (_slice._type == SliceHeaderH264::B_Slice)||(_slice._type == SliceHeaderH264::B_Slice_All) )) )
	{
		// TODO: Prediction Weight Table syntax.
		goto H264V2_WSLH_MODEERROR_WRITE;
	}//end if ...

	///-------------------------- Decode Reference Pic Marking  --------------------------------
	if(_nal._ref_idc != 0)
	{
		if(_nal._unit_type == NalHeaderH264::IDR_Slice)
		{
			if( (bitsUsedSoFar + 2) <= allowedBits )
			{
				if(bsw)
				{
					bsw->Write(_slice._no_output_of_prior_pics_flag);
					bsw->Write(_slice._long_term_reference_flag);
				}//end if bsw...
				bitsUsedSoFar += 2;
			}//end if bitsUsedSoFar...
			else
				goto H264V2_WSLH_RUNOUTOFBITS_WRITE;
		}//end if IDR_Slice...
		else
		{
			if( (bitsUsedSoFar + 1) <= allowedBits )
			{
				if(bsw)
					bsw->Write(_slice._adaptive_ref_pic_marking_mode_flag);
				bitsUsedSoFar++;
			}//end if bitsUsedSoFar...
			else
				goto H264V2_WSLH_RUNOUTOFBITS_WRITE;

      if(_slice._adaptive_ref_pic_marking_mode_flag)
      {
			  // TODO: Implement Adaptive reference pic marking mode.
			  goto H264V2_WSLH_MODEERROR_WRITE;
      }//end if _adaptive_ref_pic_marking_mode_flag...

		}//end else...
	}//end if _ref_idc...

	///-------------------------- CABAC initialisation ----------------------------------
	if( _picParam[_slice._pic_parameter_set_id]._entropy_coding_mode_flag && 
		 (_slice._type != SliceHeaderH264::I_Slice) && (_slice._type != SliceHeaderH264::SI_Slice) &&
     (_slice._type != SliceHeaderH264::I_Slice_All) && (_slice._type != SliceHeaderH264::SI_Slice_All) )
	{
		bitCount = _pHeaderUnsignedVlcEnc->Encode(_slice._cabac_init_idc);
		if(bitCount > 0)	///< Vlc codec errors are detected from a zero or negative return value.
		{
			if( (bitsUsedSoFar + bitCount) <= allowedBits )
			{
				if(bsw)
					bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
			}//end if bitsUsedSoFar...
			else
				goto H264V2_WSLH_RUNOUTOFBITS_WRITE;
		}//end if bitCount...
		else
			goto H264V2_WSLH_VLCERROR_WRITE;
		bitsUsedSoFar += bitCount;
	}//end if _entropy_coding_mode_flag...

	///-------------------------- Delta Quant Param ---------------------------------------
	bitCount = _pHeaderSignedVlcEnc->Encode(_slice._qp_delta);
	if(bitCount > 0)	///< Vlc codec errors are detected from a zero or negative return value.
	{
		if( (bitsUsedSoFar + bitCount) <= allowedBits )
		{
			if(bsw)
				bsw->Write(bitCount, _pHeaderSignedVlcEnc->GetCode());
		}//end if bitsUsedSoFar...
		else
			goto H264V2_WSLH_RUNOUTOFBITS_WRITE;
	}//end if bitCount...
	else
		goto H264V2_WSLH_VLCERROR_WRITE;
	bitsUsedSoFar += bitCount;

	///-------------------------- Switching slices ---------------------------------------
	if( (_slice._type == SliceHeaderH264::SP_Slice)||(_slice._type == SliceHeaderH264::SI_Slice)||
      (_slice._type == SliceHeaderH264::SP_Slice_All)||(_slice._type == SliceHeaderH264::SI_Slice_All) )
	{
		if((_slice._type == SliceHeaderH264::SP_Slice)||(_slice._type == SliceHeaderH264::SP_Slice_All))
		{
			if( (bitsUsedSoFar + 1) <= allowedBits )
			{
				if(bsw)
					bsw->Write(_slice._sp_for_switch_flag);
				bitsUsedSoFar++;
			}//end if bitsUsedSoFar...
			else
				goto H264V2_WSLH_RUNOUTOFBITS_WRITE;
		}//end if SP_Slice...

		bitCount = _pHeaderSignedVlcEnc->Encode(_slice._qs_delta);
		if(bitCount > 0)	///< Vlc codec errors are detected from a zero or negative return value.
		{
			if( (bitsUsedSoFar + bitCount) <= allowedBits )
			{
				if(bsw)
					bsw->Write(bitCount, _pHeaderSignedVlcEnc->GetCode());
			}//end if bitsUsedSoFar...
			else
				goto H264V2_WSLH_RUNOUTOFBITS_WRITE;
		}//end if bitCount...
		else
			goto H264V2_WSLH_VLCERROR_WRITE;
		bitsUsedSoFar += bitCount;
	}//end if SP_Slice...

	///-------------------------- Deblocking Filter Control -----------------------------------
	if(_picParam[_slice._pic_parameter_set_id]._deblocking_filter_control_present_flag)
	{
		bitCount = _pHeaderUnsignedVlcEnc->Encode(_slice._disable_deblocking_filter_idc);
		if(bitCount > 0)	///< Vlc codec errors are detected from a zero or negative return value.
		{
			if( (bitsUsedSoFar + bitCount) <= allowedBits )
			{
				if(bsw)
					bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
			}//end if bitsUsedSoFar...
			else
				goto H264V2_WSLH_RUNOUTOFBITS_WRITE;
		}//end if bitCount...
		else
			goto H264V2_WSLH_VLCERROR_WRITE;
		bitsUsedSoFar += bitCount;

		if(_slice._disable_deblocking_filter_idc != 1)
		{
			bitCount = _pHeaderSignedVlcEnc->Encode(_slice._alpha_c0_offset_div2);
			if(bitCount > 0)	///< Vlc codec errors are detected from a zero or negative return value.
			{
				if( (bitsUsedSoFar + bitCount) <= allowedBits )
				{
					if(bsw)
						bsw->Write(bitCount, _pHeaderSignedVlcEnc->GetCode());
				}//end if bitsUsedSoFar...
				else
					goto H264V2_WSLH_RUNOUTOFBITS_WRITE;
			}//end if bitCount...
			else
				goto H264V2_WSLH_VLCERROR_WRITE;
			bitsUsedSoFar += bitCount;

			bitCount = _pHeaderSignedVlcEnc->Encode(_slice._beta_offset_div2);
			if(bitCount > 0)	///< Vlc codec errors are detected from a zero or negative return value.
			{
				if( (bitsUsedSoFar + bitCount) <= allowedBits )
				{
					if(bsw)
						bsw->Write(bitCount, _pHeaderSignedVlcEnc->GetCode());
				}//end if bitsUsedSoFar...
				else
					goto H264V2_WSLH_RUNOUTOFBITS_WRITE;
			}//end if bitCount...
			else
				goto H264V2_WSLH_VLCERROR_WRITE;
			bitsUsedSoFar += bitCount;
		}//end if _disable_deblocking_filter_idc...

	}//end if _deblocking_filter_control_present_flag...

	///-------------------------- Slice Groups ---------------------------------------------
	if( (_picParam[_slice._pic_parameter_set_id]._num_slice_groups_minus1 > 0)&&
		  (_picParam[_slice._pic_parameter_set_id]._slice_group_map_type >= 3) &&
			(_picParam[_slice._pic_parameter_set_id]._slice_group_map_type <= 5) )
	{
		// TODO: Implement slice group handling.
		goto H264V2_WSLH_MODEERROR_WRITE;

	}//end if _num_slice_groups_minus1...

	*bitsUsed = bitsUsedSoFar;
	return(0);

	H264V2_WSLH_RUNOUTOFBITS_WRITE:
		_errorStr = "[H264v2Codec::WriteSliceLayerHeader] Bits required exceeds max available for picture";
		*bitsUsed = bitsUsedSoFar;
		return(1);

	H264V2_WSLH_VLCERROR_WRITE:
		_errorStr = "[H264v2Codec::WriteSliceLayerHeader] Vlc encoder error";
		*bitsUsed = bitsUsedSoFar;
		return(2);

	H264V2_WSLH_MODEERROR_WRITE:
		_errorStr = "[H264v2Codec::WriteSliceLayerHeader] Slice type or mode not implemented";
		*bitsUsed = bitsUsedSoFar;
		return(2);

}//end WriteSliceLayerHeader.

/** Read the slice layer header from the bit stream.
This impementation reads slice header encoding. Each read checks if a 
bit underflow will occur after reading. Check for loss of vlc sync 
wherever possible. The checks are sufficiently frequent to warrant 
an early exit GOTO statement.
@param bsr						: Stream to read from.
@param remainingBits	: Upper limit to the readable bits.
@param bitsUsed				: Return the actual bits extracted.
@return								: Run out of bits = 1, more bits available = 0, error = 2.
*/
int H264v2Codec::ReadSliceLayerHeader(IBitStreamReader* bsr, int remainingBits, int* bitsUsed)
{
	int bitsUsedSoFar = 0;
	int numBits;

	if(bsr == NULL)
	{
		_errorStr = "[H264V2::ReadSliceLayerHeader] No bit stream reader";
		*bitsUsed = 0;
		return(2);
	}//end if !bsr...

	///-------------------------- Addr of first macroblock -----------------------------------
	_slice._first_mb_in_slice = _pHeaderUnsignedVlcDec->Decode(bsr);
	numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
	if(numBits == 0)	///< Return = 0 implies no valid vlc code.
		goto H264V2_RSLH_NOVLC_READ;
	bitsUsedSoFar += numBits;
	/// Vlc length unknown and so only checked after the fact but assumes non-destructive read.
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RSLH_RUNOUTOFBITS_READ;

	///-------------------------- Slice Type -------------------------------------------------
	_slice._type = _pHeaderUnsignedVlcDec->Decode(bsr);
	numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
	if(numBits == 0)	///< Return = 0 implies no valid vlc code.
		goto H264V2_RSLH_NOVLC_READ;
	bitsUsedSoFar += numBits;
	/// Vlc length unknown and so only checked after the fact but assumes non-destructive read.
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RSLH_RUNOUTOFBITS_READ;

	/// Limited implementation of slice types and modes for the Baseline profile.
	if((_slice._type != SliceHeaderH264::I_Slice) && (_slice._type != SliceHeaderH264::I_Slice_All) && 
     (_slice._type != SliceHeaderH264::P_Slice) && (_slice._type != SliceHeaderH264::P_Slice_All))
		goto H264V2_RSLH_NOMODE_READ;

	///-------------------------- Pic Param Set Id --------------------------------------------
	_slice._pic_parameter_set_id = _pHeaderUnsignedVlcDec->Decode(bsr);
	numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
	if(numBits == 0)	///< Return = 0 implies no valid vlc code.
		goto H264V2_RSLH_NOVLC_READ;
	bitsUsedSoFar += numBits;
	/// Vlc length unknown and so only checked after the fact but assumes non-destructive read.
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RSLH_RUNOUTOFBITS_READ;

	///-------------------------- Frame Number ----------------------------------------------
	numBits = _seqParam[_picParam[_slice._pic_parameter_set_id]._seq_parameter_set_id]._log2_max_frame_num_minus4 + 4;
	_slice._frame_num = bsr->Read(numBits);
	bitsUsedSoFar += numBits;
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RSLH_RUNOUTOFBITS_READ;

	///-------------------------- Field Flags ----------------------------------------------
	if(!_seqParam[_picParam[_slice._pic_parameter_set_id]._seq_parameter_set_id]._frame_mbs_only_flag)
	{
		_slice._field_pic_flag = bsr->Read();
		bitsUsedSoFar++;
		if(_slice._field_pic_flag)
		{
			_slice._bottom_field_flag = bsr->Read();
			bitsUsedSoFar++;
		}//end if _field_pic_flag...
		if( bitsUsedSoFar > remainingBits )
			goto H264V2_RSLH_RUNOUTOFBITS_READ;
	}//end if !_frame_mbs_only_flag...

	///-------------------------- IDR Frame Count ------------------------------------------
	if(_nal._unit_type == NalHeaderH264::IDR_Slice)
	{
		_slice._idr_pic_id = _pHeaderUnsignedVlcDec->Decode(bsr);
		numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
		if(numBits == 0)	///< Return = 0 implies no valid vlc code.
			goto H264V2_RSLH_NOVLC_READ;
		bitsUsedSoFar += numBits;
		if( bitsUsedSoFar > remainingBits )
			goto H264V2_RSLH_RUNOUTOFBITS_READ;
	}//end if _unit_type...

	///-------------------------- Picture Order Count ---------------------------------------
	if(_seqParam[_picParam[_slice._pic_parameter_set_id]._seq_parameter_set_id]._pic_order_cnt_type == 0)
	{
		numBits = _seqParam[_picParam[_slice._pic_parameter_set_id]._seq_parameter_set_id]._log2_max_pic_order_cnt_lsb_minus4 + 4;
		_slice._pic_order_cnt_lsb = bsr->Read(numBits);
		bitsUsedSoFar += numBits;
		if( bitsUsedSoFar > remainingBits )
			goto H264V2_RSLH_RUNOUTOFBITS_READ;

		if(_picParam[_slice._pic_parameter_set_id]._pic_order_present_flag && !_slice._field_pic_flag)
		{
			_slice._delta_pic_order_cnt_bottom = _pHeaderSignedVlcDec->Decode(bsr);
			numBits = _pHeaderSignedVlcDec->GetNumDecodedBits();
			if(numBits == 0)	///< Return = 0 implies no valid vlc code.
				goto H264V2_RSLH_NOVLC_READ;
			bitsUsedSoFar += numBits;
			if( bitsUsedSoFar > remainingBits )
				goto H264V2_RSLH_RUNOUTOFBITS_READ;
		}//end if _pic_order_present_flag...

	}//end if _pic_order_cnt_type...
	else if( (_seqParam[_picParam[_slice._pic_parameter_set_id]._seq_parameter_set_id]._pic_order_cnt_type == 1) &&
						!_seqParam[_picParam[_slice._pic_parameter_set_id]._seq_parameter_set_id]._delta_pic_order_always_zero_flag )
	{
		_slice._delta_pic_order_cnt[0] = _pHeaderSignedVlcDec->Decode(bsr);
		numBits = _pHeaderSignedVlcDec->GetNumDecodedBits();
		if(numBits == 0)	///< Return = 0 implies no valid vlc code.
			goto H264V2_RSLH_NOVLC_READ;
		bitsUsedSoFar += numBits;
		if( bitsUsedSoFar > remainingBits )
			goto H264V2_RSLH_RUNOUTOFBITS_READ;

		if(_picParam[_slice._pic_parameter_set_id]._pic_order_present_flag && !_slice._field_pic_flag)
		{
			_slice._delta_pic_order_cnt[1] = _pHeaderSignedVlcDec->Decode(bsr);
			numBits = _pHeaderSignedVlcDec->GetNumDecodedBits();
			if(numBits == 0)	///< Return = 0 implies no valid vlc code.
				goto H264V2_RSLH_NOVLC_READ;
			bitsUsedSoFar += numBits;
			if( bitsUsedSoFar > remainingBits )
				goto H264V2_RSLH_RUNOUTOFBITS_READ;
		}//end if _pic_order_present_flag...
	}//end else if _pic_order_cnt_type...

	if(_picParam[_slice._pic_parameter_set_id]._redundant_pic_cnt_present_flag)
	{
		_slice._redundant_pic_cnt = _pHeaderUnsignedVlcDec->Decode(bsr);
		numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
		if(numBits == 0)	///< Return = 0 implies no valid vlc code.
			goto H264V2_RSLH_NOVLC_READ;
		bitsUsedSoFar += numBits;
		if( bitsUsedSoFar > remainingBits )
			goto H264V2_RSLH_RUNOUTOFBITS_READ;
	}//end if _redundant_pic_cnt_present_flag...

	///-------------------------- Direct Spatial Prediction -----------------------------------
	if((_slice._type == SliceHeaderH264::B_Slice)||(_slice._type == SliceHeaderH264::B_Slice_All))
	{
		_slice._direct_spatial_mv_pred_flag = bsr->Read();
		bitsUsedSoFar++;
		if( bitsUsedSoFar > remainingBits )
			goto H264V2_RSLH_RUNOUTOFBITS_READ;
	}//end if B_Slice...

	///-------------------------- Active Reference Indices -----------------------------------
	if((_slice._type == SliceHeaderH264::P_Slice_All)||(_slice._type == SliceHeaderH264::P_Slice)||
     (_slice._type == SliceHeaderH264::SP_Slice)||(_slice._type == SliceHeaderH264::SP_Slice_All)||
     (_slice._type == SliceHeaderH264::B_Slice)||(_slice._type == SliceHeaderH264::B_Slice_All))
	{
		_slice._num_ref_idx_active_override_flag = bsr->Read();
		bitsUsedSoFar++;
		if( bitsUsedSoFar > remainingBits )
			goto H264V2_RSLH_RUNOUTOFBITS_READ;

		if(_slice._num_ref_idx_active_override_flag)
		{
			_slice._num_ref_idx_l0_active_minus1 = _pHeaderUnsignedVlcDec->Decode(bsr);
			numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
			if(numBits == 0)	///< Return = 0 implies no valid vlc code.
				goto H264V2_RSLH_NOVLC_READ;
			bitsUsedSoFar += numBits;
			if( bitsUsedSoFar > remainingBits )
				goto H264V2_RSLH_RUNOUTOFBITS_READ;

			if((_slice._type == SliceHeaderH264::B_Slice)||(_slice._type == SliceHeaderH264::B_Slice_All))
			{
				_slice._num_ref_idx_l1_active_minus1 = _pHeaderUnsignedVlcDec->Decode(bsr);
				numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
				if(numBits == 0)	///< Return = 0 implies no valid vlc code.
					goto H264V2_RSLH_NOVLC_READ;
				bitsUsedSoFar += numBits;
				if( bitsUsedSoFar > remainingBits )
					goto H264V2_RSLH_RUNOUTOFBITS_READ;
			}//end if B_Slice...
		}//end if _num_ref_idx_active_override_flag...
	}//end if P_Slice...

	///-------------------------- Reference Pic list reordering --------------------------------
	if((_slice._type != SliceHeaderH264::I_Slice)&&(_slice._type != SliceHeaderH264::SI_Slice) &&
     (_slice._type != SliceHeaderH264::I_Slice_All)&&(_slice._type != SliceHeaderH264::SI_Slice_All))
	{
		_slice._ref_pic_list_reordering_flag_l0 = bsr->Read();
		bitsUsedSoFar++;
		if( bitsUsedSoFar > remainingBits )
			goto H264V2_RSLH_RUNOUTOFBITS_READ;

		if(_slice._ref_pic_list_reordering_flag_l0)
		{
			do
			{
				_slice._reordering_of_pic_nums_idc = _pHeaderUnsignedVlcDec->Decode(bsr);
				numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
				if(numBits == 0)	///< Return = 0 implies no valid vlc code.
					goto H264V2_RSLH_NOVLC_READ;
				bitsUsedSoFar += numBits;
				if( bitsUsedSoFar > remainingBits )
					goto H264V2_RSLH_RUNOUTOFBITS_READ;

				if((_slice._reordering_of_pic_nums_idc == 0)||(_slice._reordering_of_pic_nums_idc == 1))
				{
					_slice._abs_diff_pic_num_minus1 = _pHeaderUnsignedVlcDec->Decode(bsr);
					numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
					if(numBits == 0)	///< Return = 0 implies no valid vlc code.
						goto H264V2_RSLH_NOVLC_READ;
					bitsUsedSoFar += numBits;
					if( bitsUsedSoFar > remainingBits )
						goto H264V2_RSLH_RUNOUTOFBITS_READ;

					// TODO: Handle list 0 reordering with _slice._abs_diff_pic_num_minus1.
					goto H264V2_RSLH_NOMODE_READ;

				}//end if _reordering_of_pic_nums_idc...
				else if(_slice._reordering_of_pic_nums_idc == 2)
				{
					_slice._long_term_pic_num = _pHeaderUnsignedVlcDec->Decode(bsr);
					numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
					if(numBits == 0)	///< Return = 0 implies no valid vlc code.
						goto H264V2_RSLH_NOVLC_READ;
					bitsUsedSoFar += numBits;
					if( bitsUsedSoFar > remainingBits )
						goto H264V2_RSLH_RUNOUTOFBITS_READ;

					// TODO: Handle list 0 reordering with _slice._long_term_pic_num.
					goto H264V2_RSLH_NOMODE_READ;

				}//end else if _reordering_of_pic_nums_idc...

			}while(_slice._reordering_of_pic_nums_idc != 3);
		}//end if _ref_pic_list_reordering_flag_l0...
	}//end if !I_Slice...

	if((_slice._type == SliceHeaderH264::B_Slice)||(_slice._type == SliceHeaderH264::B_Slice_All))
	{
		/// B Slices are not supported for the baseline profile.
		// TODO: B-slice reordering syntax.
		goto H264V2_RSLH_NOMODE_READ;
	}//end if B_Slice...

	///-------------------------- Weighted Prediction Table --------------------------------
	if( (_picParam[_slice._pic_parameter_set_id]._weighted_pred_flag && ( (_slice._type == SliceHeaderH264::P_Slice)||(_slice._type == SliceHeaderH264::P_Slice_All)||(_slice._type == SliceHeaderH264::SP_Slice)||(_slice._type == SliceHeaderH264::SP_Slice_All) )) ||
		  ((_picParam[_slice._pic_parameter_set_id]._weighted_bipred_idc == 1) && ( (_slice._type == SliceHeaderH264::B_Slice)||(_slice._type == SliceHeaderH264::B_Slice_All) )) )
	{
		// TODO: Prediction Weight Table syntax.
		goto H264V2_RSLH_NOMODE_READ;
	}//end if ...

	///-------------------------- Decode Reference Pic Marking  --------------------------------
	if(_nal._ref_idc != 0)
	{
		if(_nal._unit_type == NalHeaderH264::IDR_Slice)
		{
			_slice._no_output_of_prior_pics_flag = bsr->Read();
			_slice._long_term_reference_flag = bsr->Read();
			bitsUsedSoFar += 2;
			if( bitsUsedSoFar > remainingBits )
				goto H264V2_RSLH_RUNOUTOFBITS_READ;
		}//end if IDR_Slice...
		else
		{
			_slice._adaptive_ref_pic_marking_mode_flag = bsr->Read();
			bitsUsedSoFar++;
			if( bitsUsedSoFar > remainingBits )
				goto H264V2_RSLH_RUNOUTOFBITS_READ;

      if(_slice._adaptive_ref_pic_marking_mode_flag)
      {
			  // TODO: Implement Adaptive reference pic marking mode.
			  goto H264V2_RSLH_NOMODE_READ;
      }//end if _adaptive_ref_pic_marking_mode_flag...

		}//end else...
	}//end if _ref_idc...

	///-------------------------- CABAC initialisation  ---------------------------------------
	if( _picParam[_slice._pic_parameter_set_id]._entropy_coding_mode_flag && 
		 (_slice._type != SliceHeaderH264::I_Slice) && (_slice._type != SliceHeaderH264::SI_Slice) &&
     (_slice._type != SliceHeaderH264::I_Slice_All) && (_slice._type != SliceHeaderH264::SI_Slice_All) )
	{
		_slice._cabac_init_idc = _pHeaderUnsignedVlcDec->Decode(bsr);
		numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
		if(numBits == 0)	///< Return = 0 implies no valid vlc code.
			goto H264V2_RSLH_NOVLC_READ;
		bitsUsedSoFar += numBits;
		if( bitsUsedSoFar > remainingBits )
			goto H264V2_RSLH_RUNOUTOFBITS_READ;
	}//end if _entropy_coding_mode_flag...

	///-------------------------- Delta Quant Param ---------------------------------------
	_slice._qp_delta = _pHeaderSignedVlcDec->Decode(bsr);
	numBits = _pHeaderSignedVlcDec->GetNumDecodedBits();
	if(numBits == 0)	///< Return = 0 implies no valid vlc code.
		goto H264V2_RSLH_NOVLC_READ;
	bitsUsedSoFar += numBits;
	if( bitsUsedSoFar > remainingBits )
		goto H264V2_RSLH_RUNOUTOFBITS_READ;

	///-------------------------- Switching slices ---------------------------------------
	if( (_slice._type == SliceHeaderH264::SP_Slice)||(_slice._type == SliceHeaderH264::SP_Slice_All) || 
      (_slice._type == SliceHeaderH264::SI_Slice)||(_slice._type == SliceHeaderH264::SI_Slice_All) )
	{
		if((_slice._type == SliceHeaderH264::SP_Slice)||(_slice._type == SliceHeaderH264::SP_Slice_All))
		{
			_slice._sp_for_switch_flag = bsr->Read();
			bitsUsedSoFar++;
			if( bitsUsedSoFar > remainingBits )
				goto H264V2_RSLH_RUNOUTOFBITS_READ;
		}//end if SP_Slice...

		_slice._qs_delta = _pHeaderSignedVlcDec->Decode(bsr);
		numBits = _pHeaderSignedVlcDec->GetNumDecodedBits();
		if(numBits == 0)	///< Return = 0 implies no valid vlc code.
			goto H264V2_RSLH_NOVLC_READ;
		bitsUsedSoFar += numBits;
		if( bitsUsedSoFar > remainingBits )
			goto H264V2_RSLH_RUNOUTOFBITS_READ;
	}//end if SP_Slice...

	///-------------------------- Deblocking Filter Control -----------------------------------
	if(_picParam[_slice._pic_parameter_set_id]._deblocking_filter_control_present_flag)
	{
		_slice._disable_deblocking_filter_idc = _pHeaderSignedVlcDec->Decode(bsr);
		numBits = _pHeaderSignedVlcDec->GetNumDecodedBits();
		if(numBits == 0)	///< Return = 0 implies no valid vlc code.
			goto H264V2_RSLH_NOVLC_READ;
		bitsUsedSoFar += numBits;
		if( bitsUsedSoFar > remainingBits )
			goto H264V2_RSLH_RUNOUTOFBITS_READ;

		if(_slice._disable_deblocking_filter_idc != 1)
		{
			_slice._alpha_c0_offset_div2 = _pHeaderSignedVlcDec->Decode(bsr);
			numBits = _pHeaderSignedVlcDec->GetNumDecodedBits();
			if(numBits == 0)	///< Return = 0 implies no valid vlc code.
				goto H264V2_RSLH_NOVLC_READ;
			bitsUsedSoFar += numBits;
			if( bitsUsedSoFar > remainingBits )
				goto H264V2_RSLH_RUNOUTOFBITS_READ;

			_slice._beta_offset_div2 = _pHeaderSignedVlcDec->Decode(bsr);
			numBits = _pHeaderSignedVlcDec->GetNumDecodedBits();
			if(numBits == 0)	///< Return = 0 implies no valid vlc code.
				goto H264V2_RSLH_NOVLC_READ;
			bitsUsedSoFar += numBits;
			if( bitsUsedSoFar > remainingBits )
				goto H264V2_RSLH_RUNOUTOFBITS_READ;
		}//end if _disable_deblocking_filter_idc...
	}//end if _deblocking_filter_control_present_flag...

	///-------------------------- Slice Groups ---------------------------------------------
	if( (_picParam[_slice._pic_parameter_set_id]._num_slice_groups_minus1 > 0)&&
		  (_picParam[_slice._pic_parameter_set_id]._slice_group_map_type >= 3) &&
			(_picParam[_slice._pic_parameter_set_id]._slice_group_map_type <= 5) )
	{
		// TODO: Implement slice group handling.
		goto H264V2_RSLH_NOMODE_READ;

	}//end if _num_slice_groups_minus1...

	*bitsUsed = bitsUsedSoFar;
	return(0);

	H264V2_RSLH_RUNOUTOFBITS_READ:
		_errorStr = "H264V2:[ReadSliceLayerHeader] Insufficient bits to decode the picture";
		*bitsUsed = bitsUsedSoFar;
		return(1);

	H264V2_RSLH_NOVLC_READ:
		_errorStr = "H264V2:[ReadSliceLayerHeader] No valid vlc in bit stream";
		*bitsUsed = bitsUsedSoFar;
		return(2);

	H264V2_RSLH_NOMODE_READ:
		_errorStr = "H264V2:[ReadSliceLayerHeader] Slice type or mode not implemented";
		*bitsUsed = bitsUsedSoFar;
		return(2);

}//end ReadSliceLayerHeader.

/** Write trailing bits to a bit stream.
A stop 1 bit is written and then zero bits are written until a byte boundary. Each 
write checks if a bit overflow will occur before writing the stop bit. The zeros 
are not counted in the bitsUsed. If the input stream param is NULL then this method 
is used to count the bits only.
@param bsw					: Stream to write into.
@param allowedBits	: Upper limit to the writable bits.
@param bitsUsed			: Return the actual bits used.
@return							: Run out of bits = 1, more bits available = 0.
*/
int H264v2Codec::WriteTrailingBits(IBitStreamWriter* bsw, int allowedBits, int* bitsUsed)
{
	int bitsUsedSoFar = 0;

	/// Write the stop bit.
	if(allowedBits < 1)
	{
		_errorStr = "H264V2:[WriteTrailingBits] Run out of bits";
		*bitsUsed = 0;
		return(1);
	}//end if allowedBits...
	if(bsw)
		bsw->Write(1);
	bitsUsedSoFar++;

	/// Write zeros to the end of the byte. Do not count the trailing zeros.
	if(bsw)
	{
		int toByteBoundary = (bsw->GetStreamBitPos() % 8) + 1;
    if(toByteBoundary < 8)  ///< Do not allow the last byte to be 0x00
		  bsw->Write(toByteBoundary, 0);
		//bitsUsedSoFar += toByteBoundary;
	}//end if bsw...

	*bitsUsed = bitsUsedSoFar;
	return(0);
}// end WriteTrailingBits.

/** Read trailing bits from the bit stream.
This impementation reads the trailing bit encoding. Each read checks if a 
bit underflow will occur after reading. Check for loss of sync 
wherever possible. The checks are sufficiently frequent to warrant 
an early exit GOTO statement.
@param bsr						: Stream to read from.
@param remainingBits	: Upper limit to the readable bits.
@param bitsUsed				: Return the actual bits extracted.
@return								: Run out of bits = 1, more bits available = 0, error = 3.
*/
int H264v2Codec::ReadTrailingBits(IBitStreamReader* bsr, int remainingBits, int* bitsUsed)
{
	int bitsUsedSoFar = 0;
	int bit = 1;

	/// Read stop bit.
	if(remainingBits < 1)
	{
		_errorStr = "H264V2:[ReadTrailingBits] Run out of bits";
		*bitsUsed = 0;
		return(1);
	}//end if remainingBits...
	if(bsr)
		bit = bsr->Read();
	bitsUsedSoFar++;
	if(bit != 1)
	{
		_errorStr = "H264V2:[ReadTrailingBits] No stop bit detected";
		*bitsUsed = bitsUsedSoFar;
		return(3);
	}//end if !stop bit...

	/// Read zeros up to the end of the byte. Do not count the trailing zeros.
	if(bsr)
	{
		int toByteBoundary = (bsr->GetStreamBitPos() % 8) + 1;
    if(toByteBoundary < 8)  ///< Only read zeros within the same byte as the stop bit.
    {
		  bit = bsr->Read(toByteBoundary);
		  //bitsUsedSoFar += toByteBoundary;
		  if(bit != 0)
		  {
			  _errorStr = "H264V2:[ReadTrailingBits] Missing trailing zeros";
		    *bitsUsed = bitsUsedSoFar;
			  return(3);
		  }//end if !zero bits...
    }//end if toByteBoundary...
	}//end if bsr...

	*bitsUsed = bitsUsedSoFar;
	return(0);
}//end ReadTrailingBits.

/** Insert start code emulation prevention codes.
Scan the entire stream, excluding the 32 bit actual start code, and check 
for 24 bit 0x000000 - 0x000003 sequences. Replace them with 32 bit 
0x00000300 - 0x00000303 sequences. The extra byte (8 bits) for each 
emulation code is excluded from the number of allowed bits but is added 
to the total consumed bits. [NB: Care must be taken when checking for 
running out of bits to allow for these extra codes.] This method should 
only be called once the entire frame has been encoded.
@param bsw	        : Stream to write into.
@param startOffset  : Start evaluating the stream from a byte offset.
@return			        : Return the number of extra bits inserted.
*/
int H264v2Codec::InsertEmulationPrevention(IBitStreamWriter* bsw, int startOffset)
{
  if(bsw == NULL)
    return(0);

	int count = 0;

  unsigned char*  streamHead  = (unsigned char*)(bsw->GetStream());
  unsigned char*  stream = &(streamHead[startOffset]);

  /// It is assumed that the stream is fully encoded and the trailing bits have been added
  /// to ensure byte alignement of the stream. Therefore the current stream byte pos is the 
  /// number of encoded bytes in the stream.
  int  endPos  = bsw->GetStreamBytePos() - 1;
  if(endPos < 2) return(0);

  /// Find the occurrance of the start code emulation then shift down by copying backwards from the end. Note
  /// that 1st 4 bytes are the start code 0x00000001.
  for(int pos = 6; pos <= endPos; pos++)
  {
    if( (stream[pos] & 0xFC) == 0 ) ///< Check for 0, 1, 2 or 3.
    {
      if( (stream[pos-1] == 0)&&(stream[pos-2] == 0) ) ///< Check 2 preceeding bytes are zero.
      {
        for(int i = endPos; i >= pos; i--) ///< Shift down by 1 byte.
          stream[i+1] = stream[i];
        stream[pos] = 0x03; ///< Insert emulation prevention code.
        pos++;              ///< Continue from new shifted position.
        endPos++;           ///< Last byte is now shifted by 1 position.
        count++;
      }//end if stream...
    }//end if stream...
  }//end for pos...

	return(count * 8);
}// end InsertEmulationPrevention.

/** Remove start code emulation prevention codes.
Scan the entire stream and check for 24 bit 0x000003 sequence
and remove the 0x03 byte from the stream. This method should 
only be called once before decoding the entire frame.
@param bsr	: Stream to read from.
@return			: Return the number of extra bits removed.
*/
int H264v2Codec::RemoveEmulationPrevention(IBitStreamReader* bsr)
{
  if(bsr == NULL)
    return(0);

	int count = 0;

  unsigned char*  stream  = (unsigned char*)(bsr->GetStream());
  int             bits    = bsr->GetStreamBitSize();
  int             endPos  = (bits / 8) - 1;
  if( (bits % 8) != 0 )
    endPos++;

  /// Find the 1st occurrence of the emulation prevention code.
  int pos;
  for(pos = 2; pos <= endPos; pos++)
  {
    if(stream[pos] == 0x03) ///< Emulation prevention code = 0x03...
    {
      if( (stream[pos-1] == 0) && (stream[pos-2] == 0) )  ///< ...and is preceeded by 2 zero bytes.
      {
        /// Shift remaining bytes up by 1 byte.
        for(int i = pos; i < endPos; i++)
          stream[i] = stream[i+1];
        endPos--; ///< Last byte was shifted by 1.
        count++;
      }//end if stream...
    }//end if stream...
  }//end for pos...

	return(count * 8);
}// end RemoveEmulationPrevention.

/** Write the slice data layer to the global bit stream.
The encodings of all the macroblocks must be correctly defined before 
this method is called. The vlc encoding is performed first before
writing. This impementation writes every macroblock encoding in top-left 
to bottom-right order. Each write checks if a bit overflow will occur 
before writing. The check is sufficiently frequent to warrant an early 
exit GOTO statement. If the input stream param is NULL then this method is 
used to count the bits only.
@param bsw					: Stream to write into.
@param allowedBits	: Upper limit to the writable bits.
@param bitsUsed			: Return the actual bits used.
@return							: Run out of bits = 1, more bits available = 0, Vlc error = 2.
*/
int H264v2Codec::WriteSliceDataLayer(IBitStreamWriter* bsw, int allowedBits, int* bitsUsed)
{
	int mb, i;
	int	bitCount;
	int bitsUsedSoFar = 0;

 	/// All macroblocks are written in order.
	int len				= _mbLength;
	_mb_skip_run	= 0;

	/// ------------------------ Code the slice data -----------------------------------
	for(mb = 0; mb < len; mb++)
	{
		/// Short cut variables.
		MacroBlockH264* pMb = &(_pMb[mb]);

		if(!pMb->_skip)	///< Encoding must set the _skip flag even for Intra macroblocks.
		{
			if((_slice._type != SliceHeaderH264::I_Slice) && (_slice._type != SliceHeaderH264::SI_Slice) &&
         (_slice._type != SliceHeaderH264::I_Slice_All) && (_slice._type != SliceHeaderH264::SI_Slice_All))
			{
				/// ------------------------ Code the skip run -----------------------------------
				bitCount = _pHeaderUnsignedVlcEnc->Encode(_mb_skip_run);
				if(bitCount <= 0)	///< Vlc codec errors are detected from a zero or negative return value.
					goto H264V2_VLCERROR_WRITE;
				if( (bitsUsedSoFar + bitCount) > allowedBits )
					goto H264V2_RUNOUTOFBITS_WRITE;
				if(bsw)
					bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
				bitsUsedSoFar += bitCount;

				_mb_skip_run = 0;	///< ...and reset.
			}//end if !I_Slice...

			/// ------------------------ Code the macroblock data ------------------------------------
			/// Only non-skipped macroblocks call this method.
			int ret = WriteMacroBlockLayer(bsw, pMb, allowedBits - bitsUsedSoFar, &bitCount);
			if(ret) ///< An error has occurred.
			{
				/// _errorStr was set in the WriteMacroBlockLayer() method.
				*bitsUsed = bitsUsedSoFar + bitCount;
				return(ret);
			}//end if ret...
			bitsUsedSoFar += bitCount;
		}//end if !_skip...
		else
		{
			_mb_skip_run++;
			/// Ensure coeffs settings are synchronised for future use by neighbours.
			for(i = 0; i < MBH264_NUM_BLKS; i++)
				pMb->_blkParam[i].pBlk->SetNumCoeffs(0);
		}//end else...

	}//end for mb...

	/// If the final macroblocks of the slice were all skipped then a skip run must be coded onto the stream.
	if(_mb_skip_run)
	{
		if((_slice._type != SliceHeaderH264::I_Slice) && (_slice._type != SliceHeaderH264::SI_Slice) &&
       (_slice._type != SliceHeaderH264::I_Slice_All) && (_slice._type != SliceHeaderH264::SI_Slice_All))
		{
			bitCount = _pHeaderUnsignedVlcEnc->Encode(_mb_skip_run);
			if(bitCount <= 0)	///< Vlc codec errors are detected from a zero or negative return value.
				goto H264V2_VLCERROR_WRITE;
			if( (bitsUsedSoFar + bitCount) > allowedBits )
				goto H264V2_RUNOUTOFBITS_WRITE;
			if(bsw)
				bsw->Write(bitCount, _pHeaderUnsignedVlcEnc->GetCode());
			bitsUsedSoFar += bitCount;
		}//end if !I_Slice...
	}//end if _mb_skip_run...

	*bitsUsed = bitsUsedSoFar;
	return(0);

	H264V2_RUNOUTOFBITS_WRITE:
		_errorStr = "H264V2:[WriteSliceDataLayer] Bits required exceeds max available for picture";
		*bitsUsed = bitsUsedSoFar;
		return(1);

	H264V2_VLCERROR_WRITE:
		_errorStr = "H264V2:[WriteSliceDataLayer] Vlc encoder error";
		*bitsUsed = bitsUsedSoFar;
		return(2);

}//end WriteSliceDataLayer.

/** Read the slice data layer from the global bit stream.
This impementation reads every macroblock encoding in top-left to
bottom-right order. Each read checks if a bit underflow will occur 
after reading. Check for loss of vlc sync wherever possible. The 
checks are sufficiently frequent to warrant an early exit GOTO 
statement.
@param bsr						: Stream to read from.
@param remainingBits	: Upper limit to the readable bits.
@param bitsUsed				: Return the actual bits extracted.
@return								: Run out of bits = 1, more bits available = 0, error = 2.
*/
int H264v2Codec::ReadSliceDataLayer(IBitStreamReader* bsr, int remainingBits, int* bitsUsed)
{
	int mb, i;
	int bitsUsedSoFar = 0;
	int numBits;

	/// Whip through each macroblock. Extract the encoded macroblock
	/// from the bit stream and vlc decode the vectors and coeff's.
	int len				= _mbLength;
	_mb_skip_run	= 0;

	/// Get the first skip run from the stream for P slices.
	if((_slice._type != SliceHeaderH264::I_Slice) && (_slice._type != SliceHeaderH264::SI_Slice) &&
     (_slice._type != SliceHeaderH264::I_Slice_All) && (_slice._type != SliceHeaderH264::SI_Slice_All))
	{
		_mb_skip_run = _pHeaderUnsignedVlcDec->Decode(bsr);
		numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
		if(numBits == 0)	///< Return = 0 implies no valid vlc code.
			goto H264V2_NOVLC_READ;
		bitsUsedSoFar += numBits;
		/// Vlc length unknown and so only checked after the fact but assumes non-destructive read.
		if( bitsUsedSoFar > remainingBits )
			goto H264V2_RUNOUTOFBITS_READ;
	}//end if !I_Slice...

	for(mb = 0; mb < len; mb++)
	{
		/// Short cut variables.
		MacroBlockH264* pMb = &(_pMb[mb]);

		/// --------------- Decode the macroblock header from the stream ----------------------

		if(_mb_skip_run && (_slice._type != SliceHeaderH264::I_Slice) && (_slice._type != SliceHeaderH264::SI_Slice) && 
                       (_slice._type != SliceHeaderH264::I_Slice_All) && (_slice._type != SliceHeaderH264::SI_Slice_All))
		{
			pMb->_skip = 1;
			_mb_skip_run--;

			/// Skip implies a P_Skip macroblock type as Inter_16x16.
			pMb->_intraFlag											= 0;
			pMb->_mb_type												= MacroBlockH264::Inter_16x16;
			pMb->_mbPartPredMode								= MacroBlockH264::Inter_16x16;
			/// Skip implies a single 16x16 (0,0) motion vector difference.
			pMb->_mvdX[MacroBlockH264::_16x16]	= 0;
			pMb->_mvdY[MacroBlockH264::_16x16]	= 0;
			/// Set the motion vector to the predicted value.
      if(MacroBlockH264::SkippedZeroMotionPredCondition(pMb))
      {
        /// Force to zero.
        pMb->_mvX[MacroBlockH264::_16x16] = 0;
        pMb->_mvY[MacroBlockH264::_16x16] = 0;
      }//end if SkippedZeroMotionPredCondition...
      else
			  pMb->GetMbMotionMedianPred(pMb, &(pMb->_mvX[MacroBlockH264::_16x16]), &(pMb->_mvY[MacroBlockH264::_16x16]));
			pMb->_coded_blk_pattern							= 0;
			/// Ensure coeffs settings are synchronised for future use by neighbours.
			for(i = 0; i < MBH264_NUM_BLKS; i++)
				pMb->_blkParam[i].pBlk->SetNumCoeffs(0);
		}//end if _mb_skip_run...
		else
		{
			/// Macroblock not skipped and header must be extracted.
			pMb->_skip = 0;

			/// Macroblock type.
			_pMb[mb]._mb_type = _pMbTypeVlcDec->Decode(bsr);
			numBits = _pMbTypeVlcDec->GetNumDecodedBits();
			if(numBits == 0)	///< Return = 0 implies no valid vlc code.
				goto H264V2_NOVLC_READ;
			bitsUsedSoFar += numBits;
			/// Vlc length unknown and so only checked after the fact but assumes non-destructive read.
			if( bitsUsedSoFar > remainingBits )
				goto H264V2_RUNOUTOFBITS_READ;
			/// Unpack _intraFlag and _mbPartPredMode from the decoded _mb_type.
			_pMb[mb].UnpackMbType(&(_pMb[mb]), _slice._type);

			/// Intra requires lum and chr prediction modes, Inter requires reference 
			/// index lists and motion vector diff values.
			if(!_pMb[mb]._intraFlag)	///< Inter	(most common option)
			{
				int numOfVecs = 1;
				// TODO: Implement other partition modes.
				if(_pMb[mb]._mbPartPredMode == MacroBlockH264::Inter_16x16)
					numOfVecs = 1;
        else
          goto H264V2_NOMODE_READ;
				for(int vec = 0; vec < numOfVecs; vec++)
				{
					/// Get the motion vector differences for this macroblock.
					_pMb[mb]._mvdX[vec] = _pMbMotionVecDiffVlcDec->Decode(bsr);
					numBits = _pMbMotionVecDiffVlcDec->GetNumDecodedBits();
					if(numBits == 0)
						goto H264V2_NOVLC_READ;
					bitsUsedSoFar += numBits;

					_pMb[mb]._mvdY[vec] = _pMbMotionVecDiffVlcDec->Decode(bsr);
					numBits = _pMbMotionVecDiffVlcDec->GetNumDecodedBits();
					if(numBits == 0)
						goto H264V2_NOVLC_READ;
					bitsUsedSoFar += numBits;
					if( bitsUsedSoFar > remainingBits )
						goto H264V2_RUNOUTOFBITS_READ;

					/// Get the prediction vector from the neighbourhood.
					int predX, predY;
					_pMb[mb].GetMbMotionMedianPred(&(_pMb[mb]), &predX, &predY);
					_pMb[mb]._mvX[vec] = predX + _pMb[mb]._mvdX[vec];
					_pMb[mb]._mvY[vec] = predY + _pMb[mb]._mvdY[vec];
				}//end for vec...
			}//end if !_interFlag...
			else											/// Intra
			{
				// TODO: Implement Intra_8x8 and Intra_4x4 mode options.

				/// Get chr prediction mode.
				_pMb[mb]._intraChrPredMode = _pMbIChrPredModeVlcDec->Decode(bsr);
				numBits = _pMbIChrPredModeVlcDec->GetNumDecodedBits();
				if(numBits == 0)
					goto H264V2_NOVLC_READ;
				bitsUsedSoFar += numBits;
				if( bitsUsedSoFar > remainingBits )
					goto H264V2_RUNOUTOFBITS_READ;
			}//end else...

			/// If not Intra_16x16 mode then _coded_blk_pattern must be extracted.
			if( (_pMb[mb]._intraFlag && (_pMb[mb]._mbPartPredMode != MacroBlockH264::Intra_16x16)) || (!_pMb[mb]._intraFlag) )
			{
				/// Block coded pattern.
				int isInter = 1;
				if(_pMb[mb]._intraFlag) isInter = 0;
				numBits = _pBlkPattVlcDec->Decode2(bsr, &(_pMb[mb]._coded_blk_pattern), &isInter);
				if(numBits == 0)
					goto H264V2_NOVLC_READ;
				bitsUsedSoFar += numBits;
				if( bitsUsedSoFar > remainingBits )
					goto H264V2_RUNOUTOFBITS_READ;
			}//end if !Intra_16x16...

		}//end else not skipped...

		/// Disassemble the bit pattern to determine which blocks are to be decoded.
		_pMb[mb].GetCodedBlockPattern(&(_pMb[mb]));

		/// For Inter modes and non-Intra_16x16 modes, delta qp is read only if there is at least one block 
		/// that has coeffs. The all zero coeff condition still requires the pattern to be decoded such that 
		/// zeros are loaded into the blocks.
		_pMb[mb]._mb_qp_delta = 0;	///< Default value required.
		if( (_pMb[mb]._coded_blk_pattern > 0)||(_pMb[mb]._intraFlag && (_pMb[mb]._mbPartPredMode == MacroBlockH264::Intra_16x16)) )
		{
			/// Delta QP.
			_pMb[mb]._mb_qp_delta = _pDeltaQPVlcDec->Decode(bsr);
			numBits = _pDeltaQPVlcDec->GetNumDecodedBits();
			if(numBits == 0)
				goto H264V2_NOVLC_READ;
			bitsUsedSoFar += numBits;
			if( bitsUsedSoFar > remainingBits )
				goto H264V2_RUNOUTOFBITS_READ;
		}//end if _coded_blk_pattern...

		int prevMbIdx = _pMb[mb]._mbIndex - 1;
		if(prevMbIdx >= 0)	///< Previous macroblock is within the image boundaries.
		{
			if(_pMb[mb]._slice == _pMb[prevMbIdx]._slice)	/// Previous macroblock within same slice.
				_pMb[mb]._mbQP = _pMb[prevMbIdx]._mbQP + _pMb[mb]._mb_qp_delta;
			else
				_pMb[mb]._mbQP = _slice._qp + _pMb[mb]._mb_qp_delta;
		}//end if prevMbIdx...
		else
			_pMb[mb]._mbQP = _slice._qp + _pMb[mb]._mb_qp_delta;

		/// ------------------- Get the macroblock coeffs from the coded stream ---------------------
		int dcSkip	 = 0;
		int startBlk = 1;
		if( _pMb[mb]._intraFlag && (_pMb[mb]._mbPartPredMode == MacroBlockH264::Intra_16x16) )
		{
			startBlk = 0;	///< Change starting block to include block num = -1;
			dcSkip	 = 1;
		}//end if Intra_16x16...
		/// For Lum blocks that are Inter coded and not Intra_16x16 coded the DC coeff is not skipped.
		for(i = MBH264_LUM_0_0; i <= MBH264_LUM_3_3; i++)
			_pMb[mb]._blkParam[i].dcSkipFlag = dcSkip;

		for(i = startBlk; i < MBH264_NUM_BLKS; i++)
		{
			/// Simplify the block reference.
			BlockH264* pBlk = _pMb[mb]._blkParam[i].pBlk;

			if(pBlk->IsCoded())
			{
				/// Choose the appropriate dimension CAVLC codec.
				IContextAwareRunLevelCodec* pCAVLC = _pCAVLC2x2;
				if( (pBlk->GetHeight() == 4) && (pBlk->GetWidth() == 4) )
					pCAVLC = _pCAVLC4x4;

				/// Get num of neighbourhood coeffs as average of above and left block coeffs. Previous
				/// MB decodings in decoding order have already set the num of neighbourhood coeffs.
				int neighCoeffs = 0;
				if(_pMb[mb]._blkParam[i].neighbourIndicator)
				{
					if(_pMb[mb]._blkParam[i].neighbourIndicator > 0)
						neighCoeffs = BlockH264::GetNumNeighbourCoeffs(pBlk);
					else	///< Negative values for neighbourIndicator imply pass through.
						neighCoeffs = _pMb[mb]._blkParam[i].neighbourIndicator;
				}//end if neighbourIndicator...
				pCAVLC->SetParameter(pCAVLC->NUM_TOT_NEIGHBOR_COEFF_ID, neighCoeffs);	///< Prepare the vlc coder.
				pCAVLC->SetParameter(pCAVLC->DC_SKIP_FLAG_ID, _pMb[mb]._blkParam[i].dcSkipFlag);

				numBits = pBlk->RleDecode(pCAVLC, bsr);					///< Vlc decode from the stream.
				if(numBits <= 0)	///< Vlc codec errors are detected from a negative return value.
				{
					if(numBits == -2)
						goto H264V2_RUNOUTOFBITS_READ;
					goto H264V2_NOVLC_READ;
				}//end if numBits...
				bitsUsedSoFar += numBits;
				if( bitsUsedSoFar > remainingBits )
					goto H264V2_RUNOUTOFBITS_READ;
			}//end if IsCoded()...
			else
			{
				pBlk->SetNumCoeffs(0);	///< For future use.
				pBlk->Zero();
			}//end else...
		}//end for i...

		/// If end of skipped macroblocks then get the next skip run from the stream.
		if(!_mb_skip_run && !pMb->_skip && (_slice._type != SliceHeaderH264::I_Slice) && (_slice._type != SliceHeaderH264::SI_Slice) && 
                                       (_slice._type != SliceHeaderH264::I_Slice_All) && (_slice._type != SliceHeaderH264::SI_Slice_All) && (mb != (len-1)))
		{
			_mb_skip_run = _pHeaderUnsignedVlcDec->Decode(bsr);
			numBits = _pHeaderUnsignedVlcDec->GetNumDecodedBits();
			if(numBits == 0)	///< Return = 0 implies no valid vlc code.
				goto H264V2_NOVLC_READ;
			bitsUsedSoFar += numBits;
			/// Vlc length unknown and so only checked after the fact but assumes non-destructive read.
			if( bitsUsedSoFar > remainingBits )
				goto H264V2_RUNOUTOFBITS_READ;
		}//end if !_mb_skip_run...

	}//end for mb...

	*bitsUsed = bitsUsedSoFar;
	return(0);

	H264V2_RUNOUTOFBITS_READ:
		_errorStr = "H264V2:[ReadSliceDataLayer] Insufficient bits to decode the picture";
		*bitsUsed = bitsUsedSoFar;
		return(1);

	H264V2_NOVLC_READ:
		_errorStr = "H264V2:[ReadSliceDataLayer] No valid vlc in bit stream";
		*bitsUsed = bitsUsedSoFar;
		return(2);

	H264V2_NOMODE_READ:
		_errorStr = "H264V2:[ReadSliceDataLayer] Mode not supported";
		*bitsUsed = bitsUsedSoFar;
		return(3);
}//end ReadSliceDataLayer.

/** Write the macroblock layer to the global bit stream.
The encodings of all the macroblocks must be correctly defined before 
this method is called. The vlc encoding is performed first before
writing. This impementation writes one non-skipped macroblock encoding. 
Each write checks if a bit overflow will occur before writing. The check 
is sufficiently frequent to warrant an early exit GOTO statement. If the 
input stream param is NULL then this method is used to count the bits only.
@param bsw					: Stream to write into.
@param pMb					: Macroblock to encode.
@param allowedBits	: Upper limit to the writable bits.
@param bitsUsed			: Return the actual bits used.
@return							: Run out of bits = 1, more bits available = 0, Vlc error = 2.
*/
int H264v2Codec::WriteMacroBlockLayer(IBitStreamWriter* bsw, MacroBlockH264* pMb, int allowedBits, int* bitsUsed)
{
	int	bitCount, i;
	int bitsUsedSoFar = 0;

	/// Encode the vlc blocks with the context of the neighborhood number of coeffs. Map
	/// to the required H.264 coding order of each block when writing to the stream.

	/// ------------------------ Code the macroblock header -----------------------------------

	/// Macroblock type.
	bitCount = _pMbTypeVlcEnc->Encode(pMb->_mb_type);
	if(bitCount <= 0)	///< Vlc codec errors are detected from a zero or negative return value.
		goto H264V2_VLCERROR_WRITE_MB;
	if( (bitsUsedSoFar + bitCount) > allowedBits )
		goto H264V2_RUNOUTOFBITS_WRITE_MB;
	if(bsw)
		bsw->Write(bitCount, _pMbTypeVlcEnc->GetCode());
	bitsUsedSoFar += bitCount;

	/// Intra requires lum and chr prediction modes, Inter requires reference 
	/// index lists and motion vector diff values.
	if(!pMb->_intraFlag)	/// Inter	(most common option)
	{
		int numOfVecs = 1;
		if(pMb->_mbPartPredMode == MacroBlockH264::Inter_16x16)
			numOfVecs = 1;
		for(int vec = 0; vec < numOfVecs; vec++)
		{
			bitCount = _pMbMotionVecDiffVlcEnc->Encode(pMb->_mvdX[vec]);
			if(bitCount <= 0)
				goto H264V2_VLCERROR_WRITE_MB;
			if( (bitsUsedSoFar + bitCount) > allowedBits )
				goto H264V2_RUNOUTOFBITS_WRITE_MB;
			if(bsw)
				bsw->Write(bitCount, _pMbMotionVecDiffVlcEnc->GetCode());
			bitsUsedSoFar += bitCount;

			bitCount = _pMbMotionVecDiffVlcEnc->Encode(pMb->_mvdY[vec]);
			if(bitCount <= 0)
				goto H264V2_VLCERROR_WRITE_MB;
			if( (bitsUsedSoFar + bitCount) > allowedBits )
				goto H264V2_RUNOUTOFBITS_WRITE_MB;
			if(bsw)
				bsw->Write(bitCount, _pMbMotionVecDiffVlcEnc->GetCode());
			bitsUsedSoFar += bitCount;

		}//end for vec...
	}//end if !_intraFlag...
	else											/// Intra
	{
		// TODO: Implement Intra_8x8 and Intra_4x4 mode options.

		/// Write chr prediction mode.
		bitCount = _pMbIChrPredModeVlcEnc->Encode(pMb->_intraChrPredMode);
		if(bitCount <= 0)
			goto H264V2_VLCERROR_WRITE_MB;
		if( (bitsUsedSoFar + bitCount) > allowedBits )
			goto H264V2_RUNOUTOFBITS_WRITE_MB;
		if(bsw)
			bsw->Write(bitCount, _pMbIChrPredModeVlcEnc->GetCode());
		bitsUsedSoFar += bitCount;

	}//end else...

	/// If not Intra_16x16 mode then _coded_blk_pattern must be written.
	if( (pMb->_intraFlag && (pMb->_mbPartPredMode != MacroBlockH264::Intra_16x16)) || (!pMb->_intraFlag) )
	{
		/// Block coded pattern.
		int isInter = 1;
		if(pMb->_intraFlag) isInter = 0;
		bitCount = _pBlkPattVlcEnc->Encode2(pMb->_coded_blk_pattern, isInter);
		if(bitCount <= 0)
			goto H264V2_VLCERROR_WRITE_MB;
		if( (bitsUsedSoFar + bitCount) > allowedBits )
			goto H264V2_RUNOUTOFBITS_WRITE_MB;
		if(bsw)
			bsw->Write(bitCount, _pBlkPattVlcEnc->GetCode());
		bitsUsedSoFar += bitCount;

	}//end if !Intra_16x16...

	/// For Inter modes and non-Intra_16x16 modes, delta qp and the residual coeffs are written only if 
	/// there is at least one block that has coeffs. 
	if( (pMb->_coded_blk_pattern > 0)||(pMb->_intraFlag && (pMb->_mbPartPredMode == MacroBlockH264::Intra_16x16)) )
	{
		/// Delta QP.
		bitCount = _pDeltaQPVlcEnc->Encode(pMb->_mb_qp_delta);
		if(bitCount <= 0)
			goto H264V2_VLCERROR_WRITE_MB;
		if( (bitsUsedSoFar + bitCount) > allowedBits )
			goto H264V2_RUNOUTOFBITS_WRITE_MB;
		if(bsw)
			bsw->Write(bitCount, _pDeltaQPVlcEnc->GetCode());
		bitsUsedSoFar += bitCount;

	}//end if _coded_blk_pattern...

	/// ------------------ Code the macroblock data --------------------------------------------------------
	int dcSkip	 = 0;
	int startBlk = 1;
	if( (pMb->_intraFlag) && (pMb->_mbPartPredMode == MacroBlockH264::Intra_16x16) )
	{
		startBlk = 0;	///< Change starting block to include block num = -1;
		dcSkip	 = 1;
	}//end if Intra_16x16...
	/// For Lum blocks that are Inter coded or not Intra_16x16 coded, the DC coeff is not skipped.
	for(i = MBH264_LUM_0_0; i <= MBH264_LUM_3_3; i++)
		pMb->_blkParam[i].dcSkipFlag = dcSkip;

	for(i = startBlk; i < MBH264_NUM_BLKS; i++)
	{
		/// Simplify the block reference.
		BlockH264* pBlk = pMb->_blkParam[i].pBlk;

		if(pMb->_blkParam[i].pBlk->IsCoded())
		{
			/// Choose the appropriate dimension CAVLC codec. Only 4x4 or 2x2 are expected here.
			IContextAwareRunLevelCodec* pCAVLC = _pCAVLC2x2;
			if( ((pMb->_blkParam[i].pBlk)->GetHeight() == 4) && ((pMb->_blkParam[i].pBlk)->GetWidth() == 4) )
				pCAVLC = _pCAVLC4x4;

			/// Get num of neighbourhood coeffs as average of above and left block coeffs. Previous
			/// MB encodings in raster order have already set the num of coeffs.
			int neighCoeffs = 0;
			if(pMb->_blkParam[i].neighbourIndicator)
			{
				if(pMb->_blkParam[i].neighbourIndicator > 0)
					neighCoeffs = BlockH264::GetNumNeighbourCoeffs(pMb->_blkParam[i].pBlk);
				else	///< Negative values for neighbourIndicator imply pass through.
					neighCoeffs = pMb->_blkParam[i].neighbourIndicator;
			}//end if neighbourIndicator...
			pCAVLC->SetParameter(pCAVLC->NUM_TOT_NEIGHBOR_COEFF_ID, neighCoeffs);	///< Prepare the vlc coder.
			pCAVLC->SetParameter(pCAVLC->DC_SKIP_FLAG_ID, pMb->_blkParam[i].dcSkipFlag);

			///< Vlc encode and add to stream. The number of coeffs is set here for the blk.
			bitCount = (pMb->_blkParam[i].pBlk)->RleEncode(pCAVLC, bsw);					
			if(bitCount <= 0)
			{
				if(bitCount == -2)
					goto H264V2_RUNOUTOFBITS_WRITE_MB;
				goto H264V2_VLCERROR_WRITE_MB;
			}//end if bitCount...
			if( (bitsUsedSoFar + bitCount) > allowedBits )
				goto H264V2_RUNOUTOFBITS_WRITE_MB;
			bitsUsedSoFar += bitCount;
		}//end if IsCoded()...
		else
			pMb->_blkParam[i].pBlk->SetNumCoeffs(0);	///< For future use.
	}//end for i...

	*bitsUsed = bitsUsedSoFar;
	return(0);

	H264V2_RUNOUTOFBITS_WRITE_MB:
		_errorStr = "H264V2:[WriteMacroBlockLayer] Bits required exceeds max available for picture";
		*bitsUsed = bitsUsedSoFar;
		return(1);

	H264V2_VLCERROR_WRITE_MB:
		_errorStr = "H264V2:[WriteMacroBlockLayer] Vlc encoder error";
		*bitsUsed = bitsUsedSoFar;
		return(2);

}//end WriteMacroBlockLayer.

/** Count the bits required to encode the macroblock layer.
The encodings of all the macroblocks must be correctly defined before 
this method is called. This impementation counts the bits required for 
one non-skipped macroblock encoding. NB: No errors are checked.
@param pMb					: Macroblock to encode.
@return							: Total bit count.
*/
int H264v2Codec::MacroBlockLayerBitCounter(MacroBlockH264* pMb)
{
	int	i;
	int bitsUsedSoFar = 0;

	/// Encode the vlc blocks with the context of the neighborhood number of coeffs. Map
	/// to the required H.264 coding order of each block when writing to the stream.

	/// ------------------------ Code the macroblock header -----------------------------------

	/// Macroblock type.
	bitsUsedSoFar = _pMbTypeVlcEnc->Encode(pMb->_mb_type);

	/// Intra requires lum and chr prediction modes, Inter requires reference 
	/// index lists and motion vector diff values.
	if(!pMb->_intraFlag)	/// Inter	(most common option)
	{
		int numOfVecs = 1;
		if(pMb->_mbPartPredMode == MacroBlockH264::Inter_16x16)
			numOfVecs = 1;
		for(int vec = 0; vec < numOfVecs; vec++)
		{
			bitsUsedSoFar += _pMbMotionVecDiffVlcEnc->Encode(pMb->_mvdX[vec]);
			bitsUsedSoFar += _pMbMotionVecDiffVlcEnc->Encode(pMb->_mvdY[vec]);
		}//end for vec...
	}//end if !_intraFlag...
	else									/// Intra
	{
		// TODO: Implement Intra_8x8 and Intra_4x4 mode options.

		/// Write chr prediction mode.
		bitsUsedSoFar += _pMbIChrPredModeVlcEnc->Encode(pMb->_intraChrPredMode);
	}//end else...

	/// If not Intra_16x16 mode then _coded_blk_pattern must be written.
	if( (pMb->_intraFlag && (pMb->_mbPartPredMode != MacroBlockH264::Intra_16x16)) || (!pMb->_intraFlag) )
	{
		/// Block coded pattern.
		int isInter = 1;
		if(pMb->_intraFlag) isInter = 0;
		bitsUsedSoFar += _pBlkPattVlcEnc->Encode2(pMb->_coded_blk_pattern, isInter);
	}//end if !Intra_16x16...

	/// For Inter modes and non-Intra_16x16 modes, delta qp and the residual coeffs are written only if 
	/// there is at least one block that has coeffs. 
	if( (pMb->_coded_blk_pattern > 0)||(pMb->_intraFlag && (pMb->_mbPartPredMode == MacroBlockH264::Intra_16x16)) )
	{
		/// Delta QP.
		bitsUsedSoFar += _pDeltaQPVlcEnc->Encode(pMb->_mb_qp_delta);
	}//end if _coded_blk_pattern...

	/// ------------------ Code the macroblock data --------------------------------------------------------
	int dcSkip	 = 0;
	int startBlk = 1;
	if( (pMb->_intraFlag) && (pMb->_mbPartPredMode == MacroBlockH264::Intra_16x16) )
	{
		startBlk = 0;	///< Change starting block to include block num = -1;
		dcSkip	 = 1;
	}//end if Intra_16x16...
	/// For Lum blocks that are Inter coded or not Intra_16x16 coded, the DC coeff is not skipped.
	for(i = MBH264_LUM_0_0; i <= MBH264_LUM_3_3; i++)
		pMb->_blkParam[i].dcSkipFlag = dcSkip;

	for(i = startBlk; i < MBH264_NUM_BLKS; i++)
	{
		/// Simplify the block reference.
		BlockH264* pBlk = pMb->_blkParam[i].pBlk;

		if(pMb->_blkParam[i].pBlk->IsCoded())
		{
			/// Choose the appropriate dimension CAVLC codec. Only 4x4 or 2x2 are expected here.
			IContextAwareRunLevelCodec* pCAVLC = _pCAVLC2x2;
			if( ((pMb->_blkParam[i].pBlk)->GetHeight() == 4) && ((pMb->_blkParam[i].pBlk)->GetWidth() == 4) )
				pCAVLC = _pCAVLC4x4;

			/// Get num of neighbourhood coeffs as average of above and left block coeffs. Previous
			/// MB encodings in raster order have already set the num of coeffs.
			int neighCoeffs = 0;
			if(pMb->_blkParam[i].neighbourIndicator)
			{
				if(pMb->_blkParam[i].neighbourIndicator > 0)
					neighCoeffs = BlockH264::GetNumNeighbourCoeffs(pMb->_blkParam[i].pBlk);
				else	///< Negative values for neighbourIndicator imply pass through.
					neighCoeffs = pMb->_blkParam[i].neighbourIndicator;
			}//end if neighbourIndicator...
			pCAVLC->SetParameter(pCAVLC->NUM_TOT_NEIGHBOR_COEFF_ID, neighCoeffs);	///< Prepare the vlc coder.
			pCAVLC->SetParameter(pCAVLC->DC_SKIP_FLAG_ID, pMb->_blkParam[i].dcSkipFlag);

			///< Vlc encode and add to stream. The number of coeffs is set here for the blk.
			bitsUsedSoFar += (pMb->_blkParam[i].pBlk)->RleEncode(pCAVLC, NULL);					
		}//end if IsCoded()...
		else
			pMb->_blkParam[i].pBlk->SetNumCoeffs(0);	///< For future use.
	}//end for i...

	return(bitsUsedSoFar);
}//end MacroBlockLayerBitCounter.

/** Read the macroblock layer from the global bit stream.
This impementation reads every macroblock encoding in top-left to
bottom-right order. Each read checks if a bit underflow will occur 
after reading. Check for loss of vlc sync wherever possible. The 
checks are sufficiently frequent to warrant an early exit GOTO 
statement.
@param bsr						: Stream to read from.
@param remainingBits	: Upper limit to the readable bits.
@param bitsUsed				: Return the actual bits extracted.
@return								: Run out of bits = 1, more bits available = 0, error = 2.
*/
int H264v2Codec::ReadMacroBlockLayer(IBitStreamReader* bsr, int remainingBits, int* bitsUsed)
{
	int bitsUsedSoFar = 0;

	*bitsUsed = bitsUsedSoFar;
	return(0);
}//end ReadMacroBlockLayer.

/** Apply the in-loop edge filter.
Used in both the encoder and decoder to remove blocking artefacts on the 4x4 boundary
edges. It is applied macroblock by macroblock in raster scan order to the reference
images of both the Lum and Chr components.
@return	:	none.
*/
void H264v2Codec::ApplyLoopFilter(void)
{
	int mb,i,j;
	short** lumRef	= _RefLum->Get2DSrcPtr();	///< Image space to operate on.
	short** cbRef		= _RefCb->Get2DSrcPtr();
	short** crRef		= _RefCr->Get2DSrcPtr();

	for(mb = 0; mb < _mbLength; mb++)
	{
		MacroBlockH264* pMb			      = &(_pMb[mb]);
		MacroBlockH264* aboveMb       = _pMb[mb]._aboveMb;
		MacroBlockH264* leftMb	      = _pMb[mb]._leftMb;

		/// All macroblock boundaries that have intra neighbours use
		/// boundary strength = {3, 4}. Vertical filtering first.

		///---------------- Vertical Edges --------------------------------------
		if(leftMb != NULL)	///< Only look at macroblock boundary if there is a neighbour.
		{
			if(pMb->_intraFlag || leftMb->_intraFlag)	///< Left intra macroblock boundary (bS = 4).
			{
				for(i = 0; i < 16; i += 4)
					VerticalFilter(pMb, lumRef, 1, i, 0, 4, 4);
				for(i = 0; i < 8; i += 4)
				{
					VerticalFilter(pMb, cbRef, 0, i, 0, 4, 4);
					VerticalFilter(pMb, crRef, 0, i, 0, 4, 4);
				}//end for i...
			}//end if _intraFlag...
			else																	///< Left inter macroblock boundary.
			{
				// TODO: For this current implementation only one 16x16 vector is used
				// per macroblock and from the same single reference. Boundary 4x4 blocks 
        // are compared with the neighbouring macroblock motion vectors.

				int mvDiffersBy4 = 0;	///< Differ with neighbour by 4 quarter pel values.
				if( (H264V2_FAST_ABS32(pMb->_mvX[0] - leftMb->_mvX[0]) >= 4) || (H264V2_FAST_ABS32(pMb->_mvY[0] - leftMb->_mvY[0]) >= 4) )
					mvDiffersBy4 = 1;

				for(i = 0; i < 4; i++)
				{
					int bS = mvDiffersBy4;
					if(pMb->_lumBlk[i][0].GetNumCoeffs() || pMb->_lumBlk[i][0]._blkLeft->GetNumCoeffs() )	///< Coded coeffs in block with q or block with p.
						bS = 2;

					if(bS)
					{
						/// Apply the filter to this macroblock block boundary.
						VerticalFilter(pMb, lumRef, 1, i<<2, 0, 4, bS);	///< At (row = 4*i, col = 0) do iter = 4 rows.

						/// Apply to the aligned chr edge assuming 4:2:0 here only.
						VerticalFilter(pMb, cbRef, 0, i<<1, 0, 2, bS);	///< At (row = 2*i, col = 0) do iter = 2 rows.
						VerticalFilter(pMb, crRef, 0, i<<1, 0, 2, bS);	///< At (row = 2*i, col = 0) do iter = 2 rows.
					}//end if bS...
				}//end for i...

			}//end else...
		}//end if leftMb...

		if(pMb->_intraFlag)	///< Internal intra block edges.
		{
			for(j = 4; j < 16; j += 4)
				for(i = 0; i < 16; i += 4)	///< All rows first for each col.
					VerticalFilter(pMb, lumRef, 1, i, j, 4, 3);
			for(j = 4; j < 8; j += 4)
				for(i = 0; i < 8; i += 4)
				{
					VerticalFilter(pMb, cbRef, 0, i, j, 4, 3);
					VerticalFilter(pMb, crRef, 0, i, j, 4, 3);
				}//end for j & i...
		}//end if _intraFlag...
		else										///< Internal inter block edges.
		{
			// TODO: For this current implementation only one 16x16 motion vector is used
			// per macroblock and from the same single reference. Therefore all internal
			// blocks have the same motion vector i.e. difference = 0.

			for(j = 1; j < 4; j++)
				for(i = 0; i < 4; i++)
				{
					int bS = 0;
					if(pMb->_lumBlk[i][j].GetNumCoeffs() || pMb->_lumBlk[i][j]._blkLeft->GetNumCoeffs() )	///< Coded coeffs in block with q or block with p.
						bS = 2;

					if(bS)
					{
						/// Apply the filter to this block boundary.
						VerticalFilter(pMb, lumRef, 1, i<<2, j<<2, 4, bS);	///< At (row = 4*i, col = 4*j) do iter = 4 rows.

						/// Apply to the aligned chr edge assuming 4:2:0 here only.
						if(j == 2)
						{
							VerticalFilter(pMb, cbRef, 0, i<<1, j<<1, 2, bS);	///< At (row = 2*i, col = 2*j) do iter = 2 rows.
							VerticalFilter(pMb, crRef, 0, i<<1, j<<1, 2, bS);	///< At (row = 2*i, col = 2*j) do iter = 2 rows.
						}//end if j...
					}//end if bS...

				}//end for j & i...
		}//end else...

		///---------------- Horizontal Edges -------------------------------------
		if(aboveMb != NULL)	///< Only look at macroblock boundary if there is a neighbour.
		{
			if(pMb->_intraFlag || aboveMb->_intraFlag)	///< Above intra macroblock boundary. (bS = 4)
			{
				for(j = 0; j < 16; j += 4)
					HorizontalFilter(pMb, lumRef, 1, 0, j, 4, 4);
				for(j = 0; j < 8; j += 4)
				{
					HorizontalFilter(pMb, cbRef, 0, 0, j, 4, 4);
					HorizontalFilter(pMb, crRef, 0, 0, j, 4, 4);
				}//end for j...
			}//end if _intraFlag...
			else																	///< Above inter macroblock boundary.
			{
				// TODO: For this current implementation only one 16x16 motion vector is used
				// per macroblock and from the same single reference.

				int mvDiffersBy4 = 0;	///< Differ with neighbour by 4 quarter pel values.
				if( (H264V2_FAST_ABS32(pMb->_mvX[0] - aboveMb->_mvX[0]) >= 4) || (H264V2_FAST_ABS32(pMb->_mvY[0] - aboveMb->_mvY[0]) >= 4) )
					mvDiffersBy4 = 1;

				for(j = 0; j < 4; j++)
				{
					int bS = mvDiffersBy4;
					if(pMb->_lumBlk[0][j].GetNumCoeffs() || pMb->_lumBlk[0][j]._blkAbove->GetNumCoeffs() )	///< Coded coeffs in block with q or block with p.
						bS = 2;

					if(bS)
					{
						/// Apply the filter to this macroblock block boundary.
						HorizontalFilter(pMb, lumRef, 1, 0, j<<2, 4, bS);	///< At (row = 0, col = 4*j) do iter = 4 cols.

						/// Apply to the aligned chr edge assuming 4:2:0 here only.
						HorizontalFilter(pMb, cbRef, 0, 0, j<<1, 2, bS);	///< At (row = 0, col = 2*j) do iter = 2 cols.
						HorizontalFilter(pMb, crRef, 0, 0, j<<1, 2, bS);	///< At (row = 0, col = 2*j) do iter = 2 cols.
					}//end if bS...
				}//end for j...

			}//end else...
		}//end aboveMb...

		if(pMb->_intraFlag)	///< Internal intra block edges.
		{
			for(i = 4; i < 16; i += 4)
				for(j = 0; j < 16; j += 4)
					HorizontalFilter(pMb, lumRef, 1, i, j, 4, 3);
			for(i = 4; i < 8; i += 4)
				for(j = 0; j < 8; j += 4)
				{
					HorizontalFilter(pMb, cbRef, 0, i, j, 4, 3);
					HorizontalFilter(pMb, crRef, 0, i, j, 4, 3);
				}//end for i & j...
		}//end if _intraFlag...
		else										///< Internal inter block edges.
		{
			// TODO: For this current implementation only one 16x16 motion vector is used
			// per macroblock and from the same single reference. Therefore all internal
			// blocks have the same motion vector i.e. difference = 0.

			for(i = 1; i < 4; i++)
				for(j = 0; j < 4; j++)
				{
					int bS = 0;
					if(pMb->_lumBlk[i][j].GetNumCoeffs() || pMb->_lumBlk[i][j]._blkAbove->GetNumCoeffs() )	///< Coded coeffs in block with q or block with p.
						bS = 2;

					if(bS)
					{
						/// Apply the filter to this block boundary.
						HorizontalFilter(pMb, lumRef, 1, i<<2, j<<2, 4, bS);	///< At (row = 4*i, col = 4*j) do iter = 4 cols.

						/// Apply to the aligned chr edge assuming 4:2:0 here only.
						if(i == 2)
						{
							HorizontalFilter(pMb, cbRef, 0, i<<1, j<<1, 2, bS);	///< At (row = 2*i, col = 2*j) do iter = 2 cols.
							HorizontalFilter(pMb, crRef, 0, i<<1, j<<1, 2, bS);	///< At (row = 2*i, col = 2*j) do iter = 2 cols.
						}//end if i...
					}//end if bS...

				}//end for i & j...
		}//end else...

	}//end for mb...

}//end ApplyLoopFilter.

/** Apply the in-loop deblocking filter to vertical block edges.
The deblocking filter is applied only after the image has been fully
decoded. This method operates on one column with the given boundary
strength. The operation is defined in the ITU-T Recommendation 
H.264 (03/2005).
@param pMb							: Macroblock to operate on.
@param img							: Reference image to filter.
@param lumFlag					: Indicates the colour component of the ref image.
@param rowOff						: The row offset within the macroblock with the top-left corner as (0,0).
@param colOff						: The column offset.
@param iter							: Iterations down the col from (rowOff, colOff).
@param boundaryStrength	: Boundary strength to apply.
@return									: none
*/
void H264v2Codec::VerticalFilter(MacroBlockH264* pMb, short** img, int lumFlag, int rowOff, int colOff, int iter, int boundaryStrength)
{
	int i;
	int qPav, offX, offY;
	if(lumFlag)
	{
		qPav	= pMb->_mbQP;
		/// Modify to average qP with the neighbour if this is a mb edge.
		if( (pMb->_leftMb != NULL) && (colOff == 0) )
			qPav	= (qPav + pMb->_leftMb->_mbQP + 1) >> 1;
		offX	= pMb->_offLumX + colOff;
		offY	= pMb->_offLumY + rowOff;
	}//end if lumFlag...
	else
	{
		qPav	= MacroBlockH264::GetQPc(pMb->_mbQP);
		if( (pMb->_leftMb != NULL) && (colOff == 0) )
		  qPav	= (qPav + MacroBlockH264::GetQPc(pMb->_leftMb->_mbQP) + 1) >> 1;
    offX	= pMb->_offChrX + colOff;
		offY	= pMb->_offChrY + rowOff;
	}//end else...

	switch(boundaryStrength)
	{
		case 1:
		case 2:
		case 3:
		{
			int a = H264v2Codec::alpha[qPav];
			int b = H264v2Codec::beta[qPav];

			for(i = 0; i < iter; i++)	///< Iterate down the column.
			{
				int p1 = (int)(img[offY + i][offX - 2]);
				int p0 = (int)(img[offY + i][offX - 1]);
				int q0 = (int)(img[offY + i][offX]);
				int q1 = (int)(img[offY + i][offX + 1]);
				/// Test boundary differences to switch the filtering on/off.
				if( (H264V2_FAST_ABS32((p0-q0)) < a)&&(H264V2_FAST_ABS32((p1-p0)) < b)&&(H264V2_FAST_ABS32((q1-q0)) < b) )
				{
					int p2 = (int)(img[offY + i][offX - 3]);
					int q2 = (int)(img[offY + i][offX + 2]);
					int delta = (((q0 - p0) << 2) + (p1 - q1) + 4) >> 3;
					int tC0 = indexAbS[boundaryStrength-1][qPav];
					int tC;
					if(lumFlag)
						tC = tC0 + ((H264V2_FAST_ABS32((p2-p0)) < b)?1:0) + ((H264V2_FAST_ABS32((q2-q0)) < b)?1:0);
					else
						tC = tC0 + 1;
					if(delta < -tC)	delta = -tC;
					else if(delta > tC)	delta = tC;

					img[offY + i][offX - 1] = (short)(H264V2_CLIP255(p0 + delta));			///< p0.
					img[offY + i][offX]			= (short)(H264V2_CLIP255(q0 - delta));			///< q0.

					if( (H264V2_FAST_ABS32((p2-p0)) < b) && lumFlag )
					{
						delta = (p2 + ((p0 + q0 + 1) >> 1) - (p1 << 1)) >> 1;
						if(delta < -tC0)	delta = -tC0;
						else if(delta > tC0)	delta = tC0;

						img[offY + i][offX - 2] = (short)(p1 + delta);		                ///< p1.
					}//end if abs(p2)...

					if( (H264V2_FAST_ABS32((q2-q0)) < b) && lumFlag )
					{
						delta = (q2 + ((p0 + q0 + 1) >> 1) - (q1 << 1)) >> 1;
						if(delta < -tC0)	delta = -tC0;
						else if(delta > tC0)	delta = tC0;

						img[offY + i][offX + 1] = (short)(q1 + delta);		                ///< q1.
					}//end if abs(q2)...
				}//end if abs(p0)...
			}//end for i...
			break;
		}//end case 1,2 & 3...
		case 4:	///< This bS = 4 only occurs for intra boundaries and therefore must be colOff = 0;.
		{
			int a = H264v2Codec::alpha[qPav];
			int b = H264v2Codec::beta[qPav];

			for(i = 0; i < iter; i++)	///< Iterate down the column.
			{
				int p1 = (int)(img[offY + i][offX - 2]);
				int p0 = (int)(img[offY + i][offX - 1]);
				int q0 = (int)(img[offY + i][offX]);
				int q1 = (int)(img[offY + i][offX + 1]);
				/// Test boundary differences to switch the filtering on/off.
				if( (H264V2_FAST_ABS32((p0-q0)) < a)&&(H264V2_FAST_ABS32((p1-p0)) < b)&&(H264V2_FAST_ABS32((q1-q0)) < b) )
				{
					int p2 = (int)(img[offY + i][offX - 3]);
					int q2 = (int)(img[offY + i][offX + 2]);
					if( (H264V2_FAST_ABS32((p2-p0)) < b)&&(H264V2_FAST_ABS32((p0-q0)) < ((a >> 2) + 2))&& lumFlag )
					{
						img[offY + i][offX - 1] = (short)((p2 + (2*p1) + (2*p0) + (2*q0) + q1 + 4) >> 3);														///< p0.
						img[offY + i][offX - 2] = (short)((p2 + p1 + p0 + q0 + 2) >> 2);																						///< p1.
						img[offY + i][offX - 3] = (short)(((2*(int)(img[offY + i][offX - 4])) + (3*p2) + p1 + p0 + q0 + 4) >> 3);		///< p2.
					}//end if abs(p2)...
					else
					{
						img[offY + i][offX - 1] = (short)(((2*p1) + p0 + q1 + 2) >> 2);																							///< p0.
					}//end else...

					if( (H264V2_FAST_ABS32((q2-q0)) < b)&&(H264V2_FAST_ABS32((p0-q0)) < ((a >> 2) + 2))&& lumFlag )
					{
						img[offY + i][offX]			= (short)((p1 + (2*p0) + (2*q0) + (2*q1) + q2 + 4) >> 3);														///< q0.
						img[offY + i][offX + 1] = (short)((p0 + q0 + q1 + q2 + 2) >> 2);																						///< q1.
						img[offY + i][offX + 2] = (short)(((2*(int)(img[offY + i][offX + 3])) + (3*q2) + q1 + q0 + p0 + 4) >> 3);		///< q2.
					}//end if abs(p2)...
					else
					{
						img[offY + i][offX] = (short)(((2*q1) + q0 + p1 + 2) >> 2);																									///< q0.
					}//end else...
				}//end if abs(p0)...
			}//end for i...

			break;
		}//end case 4...
	}//end switch boundaryStrength...

}//end VerticalFilter.

/** Apply the in-loop deblocking filter to horizontal block edges.
The deblocking filter is applied only after the image has been fully
decoded. This method operates on one row with the given boundary
strength. The operation is defined in the ITU-T Recommendation 
H.264 (03/2005).
@param pMb							: Macroblock to operate on.
@param img							: Reference image to filter.
@param lumFlag					: Indicates the colour component of the ref image.
@param rowOff						: The row offset within the macroblock with the top-left corner as (0,0).
@param colOff						: The column offset.
@param iter							: Iterations along the row from (rowOff, colOff).
@param boundaryStrength	: Boundary strength to apply.
@return									: none
*/
void H264v2Codec::HorizontalFilter(MacroBlockH264* pMb, short** img, int lumFlag, int rowOff, int colOff, int iter, int boundaryStrength)
{
	int i;
	int qPav, offX, offY;
	if(lumFlag)
	{
		qPav	= pMb->_mbQP;
		/// Modify to average qP with the neighbour if this is a mb edge.
		if( (pMb->_aboveMb != NULL) && (rowOff == 0) )
			qPav	= (qPav + pMb->_aboveMb->_mbQP + 1) >> 1;
		offX	= pMb->_offLumX + colOff;
		offY	= pMb->_offLumY + rowOff;
	}//end if lumFlag...
	else
	{
		qPav	= MacroBlockH264::GetQPc(pMb->_mbQP);
		if( (pMb->_aboveMb != NULL) && (rowOff == 0) )
			qPav	= (qPav + MacroBlockH264::GetQPc(pMb->_aboveMb->_mbQP) + 1) >> 1;
		offX	= pMb->_offChrX + colOff;
		offY	= pMb->_offChrY + rowOff;
	}//end else...

	switch(boundaryStrength)
	{
		case 1:
		case 2:
		case 3:
		{
			int a = H264v2Codec::alpha[qPav];
			int b = H264v2Codec::beta[qPav];

			for(i = 0; i < iter; i++)	///< Iterate along the row.
			{
				int p1 = (int)(img[offY - 2][offX + i]);
				int p0 = (int)(img[offY - 1][offX + i]);
				int q0 = (int)(img[offY][offX + i]);
				int q1 = (int)(img[offY + 1][offX + i]);
				/// Test boundary differences to switch the filtering on/off.
				if( (H264V2_FAST_ABS32((p0-q0)) < a)&&(H264V2_FAST_ABS32((p1-p0)) < b)&&(H264V2_FAST_ABS32((q1-q0)) < b) )
				{
					int p2 = (int)(img[offY - 3][offX + i]);
					int q2 = (int)(img[offY + 2][offX + i]);
					int delta = (((q0 - p0) << 2) + (p1 - q1) + 4) >> 3;
					int tC0 = indexAbS[boundaryStrength-1][qPav];
					int tC;
					if(lumFlag)
						tC = tC0 + ((H264V2_FAST_ABS32((p2-p0)) < b)?1:0) + ((H264V2_FAST_ABS32((q2-q0)) < b)?1:0);
					else
						tC = tC0 + 1;
					if(delta < -tC)	delta = -tC;
					else if(delta > tC)	delta = tC;

					img[offY - 1][offX + i] = (short)(H264V2_CLIP255(p0 + delta));			///< p0.
					img[offY][offX + i]			= (short)(H264V2_CLIP255(q0 - delta));			///< q0.

					if( (H264V2_FAST_ABS32((p2-p0)) < b) && lumFlag )
					{
						delta = (p2 + ((p0 + q0 + 1) >> 1) - (p1 << 1)) >> 1;
						if(delta < -tC0)	delta = -tC0;
						else if(delta > tC0)	delta = tC0;

						img[offY - 2][offX + i] = (short)(p1 + delta);			///< p1.
					}//end if abs(p2)...

					if( (H264V2_FAST_ABS32((q2-q0)) < b) && lumFlag )
					{
						delta = (q2 + ((p0 + q0 + 1) >> 1) - (q1 << 1)) >> 1;
						if(delta < -tC0)	delta = -tC0;
						else if(delta > tC0)	delta = tC0;

						img[offY + 1][offX + i] = (short)(q1 + delta);			///< q1.
					}//end if abs(q2)...
				}//end if abs(p0)...
			}//end for i...
			break;
		}//end case 1,2 & 3...
		case 4:	///< This bS = 4 only occurs for intra boundaries and therefore must be rowOff = 0;.
		{
			int a = H264v2Codec::alpha[qPav];
			int b = H264v2Codec::beta[qPav];

			for(i = 0; i < iter; i++)	///< Iterate along the row.
			{
				int p1 = (int)(img[offY - 2][offX + i]);
				int p0 = (int)(img[offY - 1][offX + i]);
				int q0 = (int)(img[offY][offX + i]);
				int q1 = (int)(img[offY + 1][offX + i]);
				/// Test boundary differences to switch the filtering on/off.
				if( (H264V2_FAST_ABS32((p0-q0)) < a)&&(H264V2_FAST_ABS32((p1-p0)) < b)&&(H264V2_FAST_ABS32((q1-q0)) < b) )
				{
					int p2 = (int)(img[offY - 3][offX + i]);
					int q2 = (int)(img[offY + 2][offX + i]);
					if( (H264V2_FAST_ABS32((p2-p0)) < b)&&(H264V2_FAST_ABS32((p0-q0)) < ((a >> 2) + 2))&& lumFlag )
					{
						img[offY - 1][offX + i] = (short)((p2 + (2*p1) + (2*p0) + (2*q0) + q1 + 4) >> 3);														///< p0.
						img[offY - 2][offX + i] = (short)((p2 + p1 + p0 + q0 + 2) >> 2);																						///< p1.
						img[offY - 3][offX + i] = (short)(((2*(int)(img[offY - 4][offX + i])) + (3*p2) + p1 + p0 + q0 + 4) >> 3);		///< p2.
					}//end if abs(p2)...
					else
					{
						img[offY - 1][offX + i] = (short)(((2*p1) + p0 + q1 + 2) >> 2);																							///< p0.
					}//end else...

					if( (H264V2_FAST_ABS32((q2-q0)) < b)&&(H264V2_FAST_ABS32((p0-q0)) < ((a >> 2) + 2))&& lumFlag )
					{
						img[offY][offX + i]			= (short)((p1 + (2*p0) + (2*q0) + (2*q1) + q2 + 4) >> 3);														///< q0.
						img[offY + 1][offX + i] = (short)((p0 + q0 + q1 + q2 + 2) >> 2);																						///< q1.
						img[offY + 2][offX + i] = (short)(((2*(int)(img[offY + 3][offX + i])) + (3*q2) + q1 + q0 + p0 + 4) >> 3);		///< q2.
					}//end if abs(p2)...
					else
					{
						img[offY][offX + i] = (short)(((2*q1) + q0 + p1 + 2) >> 2);																									///< q0.
					}//end else...
				}//end if abs(p0)...
			}//end for i...

			break;
		}//end case 4...
	}//end switch boundaryStrength...

}//end HorizontalFilter.

/** Transform and Quantise an Intra_16x16 macroblock
This method provides a speed improvement for macroblock processing and code refactoring.
@param pMb	: Macroblock to transform.
@return			: none
*/
void H264v2Codec::TransAndQuantIntra16x16MBlk(MacroBlockH264* pMb)
{
	int i;
	int mbLumQP	= pMb->_mbQP;
	int mbChrQP = MacroBlockH264::GetQPc(pMb->_mbQP);

	/// The transform (mode = TransformOnly) and quantisation (mode = QuantOnly) are separated 
	/// for the 4x4 AC blocks in this method and therefore the quant parameter does not need to 
	/// be set. But the DC blocks are in the TransformAndQuant mode and require the setting.
	_pF4x4TLum->SetParameter(IForwardTransform::QUANT_ID, mbLumQP);
	_pF4x4TChr->SetParameter(IForwardTransform::QUANT_ID, mbChrQP);
	_pF4x4TChr->SetMode(IForwardTransform::TransformOnly);	
	_pFDC4x4T->SetParameter(IForwardTransform::QUANT_ID, mbLumQP);
	_pFDC2x2T->SetParameter(IForwardTransform::QUANT_ID, mbChrQP);

	/// Do the forward 4x4 transform on the non-DC 4x4 blocks without scaling or quantisation. Pull
	/// out the DC terms from each block and populate the DC blocks and then scale and quant the
	/// non-DC 4x4 blocks. Note that this is done in raster scan order and not coding order to
	/// align each position in the DC block with the spatial location of the 4x4 block in the image.
	BlockH264*	pCbBlk		= &(pMb->_cbBlk[0][0]);	///< Assume these are linear arrays that wrap in raster scan order.
	BlockH264*	pCrBlk		= &(pMb->_crBlk[0][0]);
	short*			pDcCbBlk	= pMb->_cbDcBlk.GetBlk();
	short*			pDcCrBlk	= pMb->_crDcBlk.GetBlk();
	for(i = 0; i < 4; i++)
	{
		/// Unroll the inner loop.

		short*			pDcLumBlk	= &((pMb->_lumDcBlk.GetBlk())[4*i]);	///< Short cuts.
		BlockH264*	pLumBlk		= &(pMb->_lumBlk[i][0]);

		/// Transform without scaling or quant.
		_pF4x4TLum->SetMode(IForwardTransform::TransformOnly);	

		pLumBlk->ForwardTransform(_pF4x4TLum);	///< [i][0]
		*pDcLumBlk++ = pLumBlk->GetDC(); ///< Set the Lum DC block from the DC term of this block and scale it without rounding.
		(pLumBlk++)->SetDC(0); ///< Required to ensure the feedback loop operates properly.

		pLumBlk->ForwardTransform(_pF4x4TLum);	///< [i][1]
		*pDcLumBlk++ = pLumBlk->GetDC();
		(pLumBlk++)->SetDC(0);

		pLumBlk->ForwardTransform(_pF4x4TLum);	///< [i][2]
		*pDcLumBlk++ = pLumBlk->GetDC();
		(pLumBlk++)->SetDC(0);

		pLumBlk->ForwardTransform(_pF4x4TLum);	///< [i][3]
		*pDcLumBlk = pLumBlk->GetDC();
		pLumBlk->SetDC(0);

		_pF4x4TChr->SetMode(IForwardTransform::TransformOnly);

		pCbBlk->ForwardTransform(_pF4x4TChr);	///< [i]
		*pDcCbBlk++ = pCbBlk->GetDC();
		pCbBlk->SetDC(0);

		pCrBlk->ForwardTransform(_pF4x4TChr);	///< [i]
		*pDcCrBlk++ = pCrBlk->GetDC();
		pCrBlk->SetDC(0);

		/// Now scale and quant.
		_pF4x4TLum->SetMode(IForwardTransform::QuantOnly);	
		(pLumBlk--)->Quantise(_pF4x4TLum);	///< [i][3]
		(pLumBlk--)->Quantise(_pF4x4TLum);	///< [i][2]
		(pLumBlk--)->Quantise(_pF4x4TLum);	///< [i][1]
		pLumBlk->Quantise(_pF4x4TLum);			///< [i][0]
		_pF4x4TChr->SetMode(IForwardTransform::QuantOnly);	
		(pCbBlk++)->Quantise(_pF4x4TChr);
		(pCrBlk++)->Quantise(_pF4x4TChr);

	}//end for i...

	/// Transform and quant the DC blocks.
	pMb->_lumDcBlk.ForwardTransform(_pFDC4x4T);
	pMb->_cbDcBlk.ForwardTransform(_pFDC2x2T);
	pMb->_crDcBlk.ForwardTransform(_pFDC2x2T);

}//end TransAndQuantIntra16x16MBlk.

/** Transform and Quantise an Intra_16x16 mode block.
Code refactoring method to transform the block and store the DC coeff before quantisation. The
transform must have the quantisation parameter preset before calling this method.
@param pTQ					: Transform to use.
@param pBlk					: Blk to process.
@param pDcBlkCoeff	: reference to store the DC coeff.
@return							: none.
*/
void H264v2Codec::TransAndQuantIntra16x16ModeBlk(IForwardTransform* pTQ, BlockH264* pBlk, short* pDcBlkCoeff)
{
	pTQ->SetMode(IForwardTransform::TransformOnly);	
	pBlk->ForwardTransform(pTQ);
	*pDcBlkCoeff = pBlk->GetDC();	///< Set the DC block from the DC term of this block.
	pBlk->SetDC(0);								///< Required to ensure the feedback loop operates properly.
	pTQ->SetMode(IForwardTransform::QuantOnly);	
	pBlk->Quantise(pTQ);
}//end TransAndQuantIntra16x16ModeBlk.

/** Inverse Transform and Quantise an Intra_16x16 macroblock
This method provides a speed improvement for macroblock processing and code refactoring.
@param pMb				: Macroblock to transform.
@param tmpBlkFlag	: Indicate temp blocks to be used.
@return						: none
*/
void H264v2Codec::InverseTransAndQuantIntra16x16MBlk(MacroBlockH264* pMb, int tmpBlkFlag)
{
	int					i;
	BlockH264*	pCbBlk;
	BlockH264*	pCrBlk;
	short*			pDcCbBlk;
	short*			pDcCrBlk;
	int					mbLumQP	= pMb->_mbQP;
	int					mbChrQP = MacroBlockH264::GetQPc(pMb->_mbQP);

	/// The inverse transform (mode = TransformOnly) and quantisation (mode = QuantOnly) are separated 
	/// for the 4x4 AC blocks in this method and therefore the quant parameter does not need to 
	/// be set. But the DC blocks are in the TransformAndQuant mode and require the setting.
	_pIDC4x4T->SetParameter(IInverseTransform::QUANT_ID, mbLumQP);
	_pIDC2x2T->SetParameter(IInverseTransform::QUANT_ID, mbChrQP);

	if(tmpBlkFlag)
	{
		/// Copy all blks to temp blks.
		MacroBlockH264::CopyBlksToTmpBlks(pMb, 0, MBH264_NUM_BLKS - 1);

		/// Inverse transform & inverse quantise DC blocks.
		pMb->_lumDcBlkTmp.InverseTransform(_pIDC4x4T);
		pMb->_cbDcBlkTmp.InverseTransform(_pIDC2x2T);
		pMb->_crDcBlkTmp.InverseTransform(_pIDC2x2T);

		/// Short cut pointers for Chr components.
		pCbBlk		= &(pMb->_cbBlkTmp[0][0]);	///< Assume these are linear arrays that wrap in raster scan order.
		pCrBlk		= &(pMb->_crBlkTmp[0][0]);
		pDcCbBlk	= pMb->_cbDcBlkTmp.GetBlk();	///< Actual array of coeffs.
		pDcCrBlk	= pMb->_crDcBlkTmp.GetBlk();
	}//end if tmpBlkFlag...
	else
	{
		/// Inverse transform & inverse quantise DC blocks.
		pMb->_lumDcBlk.InverseTransform(_pIDC4x4T);
		pMb->_cbDcBlk.InverseTransform(_pIDC2x2T);
		pMb->_crDcBlk.InverseTransform(_pIDC2x2T);

		/// Short cut pointers for Chr components.
		pCbBlk		= &(pMb->_cbBlk[0][0]);	///< Assume these are linear arrays that wrap in raster scan order.
		pCrBlk		= &(pMb->_crBlk[0][0]);
		pDcCbBlk	= pMb->_cbDcBlk.GetBlk();	///< Actual array of coeffs.
		pDcCrBlk	= pMb->_crDcBlk.GetBlk();
	}//end else...

	/// Inverse quant for the main Lum, Cb and Cr blocks, insert the DC terms from the 
	/// DC blocks and then complete the inverse transform process. Note that this is 
	/// done in raster scan order and not coding order to align each position in the 
	/// DC block with the spatial location of the 4x4 block in the image.

	for(i = 0; i < 4; i++)	///< 16 Lum blocks + 4 Cb Chr and 4 Cr Chr blocks.
	{
		short*			pDcLumBlk;
		BlockH264*	pLumBlk;

		/// Unroll the inner loop.

		if(tmpBlkFlag)
		{
			pDcLumBlk	= &((pMb->_lumDcBlkTmp.GetBlk())[4*i + 3]);	///< Short cuts.
			pLumBlk		= &(pMb->_lumBlkTmp[i][0]);
		}//end if tmpBlkFlag...
		else
		{
			pDcLumBlk	= &((pMb->_lumDcBlk.GetBlk())[4*i + 3]);	///< Short cuts.
			pLumBlk		= &(pMb->_lumBlk[i][0]);
		}//end else...

		/// Inverse scale and quant.
		_pI4x4TLum->SetParameter(IInverseTransform::QUANT_ID, mbLumQP);
		_pI4x4TLum->SetMode(IInverseTransform::QuantOnly);	
		(pLumBlk++)->InverseQuantise(_pI4x4TLum);	///< [i][0]
		(pLumBlk++)->InverseQuantise(_pI4x4TLum);	///< [i][1]
		(pLumBlk++)->InverseQuantise(_pI4x4TLum);	///< [i][2]
		pLumBlk->InverseQuantise(_pI4x4TLum);			///< [i][3]
		_pI4x4TChr->SetParameter(IInverseTransform::QUANT_ID, mbChrQP);
		_pI4x4TChr->SetMode(IInverseTransform::QuantOnly);	
		pCbBlk->InverseQuantise(_pI4x4TChr);
		pCrBlk->InverseQuantise(_pI4x4TChr);

		/// Inverse transform without scaling or quant.
		_pI4x4TLum->SetMode(IInverseTransform::TransformOnly);	
		_pI4x4TChr->SetMode(IInverseTransform::TransformOnly);	

		pLumBlk->SetDC(*pDcLumBlk--);						///< Load the DC term from the DC block.
		(pLumBlk--)->InverseTransform(_pI4x4TLum);	///< [i][3]

		pLumBlk->SetDC(*pDcLumBlk--);
		(pLumBlk--)->InverseTransform(_pI4x4TLum);	///< [i][2]

		pLumBlk->SetDC(*pDcLumBlk--);
		(pLumBlk--)->InverseTransform(_pI4x4TLum);	///< [i][1]

		pLumBlk->SetDC(*pDcLumBlk);
		pLumBlk->InverseTransform(_pI4x4TLum);			///< [i][0]

		pCbBlk->SetDC(*pDcCbBlk++);
		(pCbBlk++)->InverseTransform(_pI4x4TChr);	///< [i]

		pCrBlk->SetDC(*pDcCrBlk++);
		(pCrBlk++)->InverseTransform(_pI4x4TChr);	///< [i]
	}//end for i...

}//end InverseTransAndQuantIntra16x16MBlk.

/** Inverse Quantise and Transform an Intra_16x16 mode block.
Code refactoring method to inverse quantise the block and set the DC coeff before the inverse
transform. The inverse transform must have the quantisation parameter preset before calling 
this method.
@param pTQ					: Inverse Transform to use.
@param pBlk					: Blk to process.
@param pDcBlkCoeff	: reference to get the DC coeff.
@return							: none.
*/
void H264v2Codec::InvTransAndQuantIntra16x16ModeBlk(IInverseTransform* pTQ, BlockH264* pBlk, short* pDcBlkCoeff)
{
	pTQ->SetMode(IInverseTransform::QuantOnly);	
	pBlk->InverseQuantise(pTQ);
	pBlk->SetDC(*pDcBlkCoeff);	///< Set the DC coeff for this block from the DC block.
	pTQ->SetMode(IInverseTransform::TransformOnly);	
	pBlk->InverseTransform(pTQ);
}//end InvTransAndQuantIntra16x16ModeBlk.

/** Transform and Quantise an Inter_16x16 macroblock
This method provides a speed improvement for macroblock processing and code refactoring.
@param pMb	: Macroblock to transform.
@return			: none
*/
void H264v2Codec::TransAndQuantInter16x16MBlk(MacroBlockH264* pMb)
{
	int i;
	int mbLumQP	= pMb->_mbQP;
	int mbChrQP = MacroBlockH264::GetQPc(pMb->_mbQP);

	/// The transform (mode = TransformOnly) and quantisation (mode = QuantOnly) are separated 
	/// for the chr 4x4 AC blocks in this method. The lum and DC blocks are in the 
  /// TransformAndQuant mode and require the setting.
	_pF4x4TLum->SetMode(IForwardTransform::TransformAndQuant);	
	_pF4x4TLum->SetParameter(IForwardTransform::QUANT_ID, mbLumQP);
	_pF4x4TChr->SetParameter(IForwardTransform::QUANT_ID, mbChrQP);
	_pFDC2x2T->SetParameter(IForwardTransform::QUANT_ID, mbChrQP);

	/// Do the forward 4x4 transform on the non-DC 4x4 blocks without scaling or quantisation. Pull
	/// out the DC terms from each chr block and populate the DC blocks and then scale and quant the
	/// non-DC 4x4 blocks. Note that this is done in raster scan order and not coding order to
	/// align each position in the DC block with the spatial location of the 4x4 block in the image.
	BlockH264*	pCbBlk		= &(pMb->_cbBlk[0][0]);	///< Assume these are linear arrays that wrap in raster scan order.
	BlockH264*	pCrBlk		= &(pMb->_crBlk[0][0]);
	short*			pDcCbBlk	= pMb->_cbDcBlk.GetBlk();
	short*			pDcCrBlk	= pMb->_crDcBlk.GetBlk();
	for(i = 0; i < 4; i++)
	{
		/// Unroll the inner loop.

		BlockH264*	pLumBlk		= &(pMb->_lumBlk[i][0]);	///< Short cuts.

		/// Transform Lum with scaling and quant.
		(pLumBlk++)->ForwardTransform(_pF4x4TLum);	///< [i][0]
		(pLumBlk++)->ForwardTransform(_pF4x4TLum);	///< [i][1]
		(pLumBlk++)->ForwardTransform(_pF4x4TLum);	///< [i][2]
		pLumBlk->ForwardTransform(_pF4x4TLum);			///< [i][3]

    /// For chr, split the transform and quant processes. 
		_pF4x4TChr->SetMode(IForwardTransform::TransformOnly);	

		pCbBlk->ForwardTransform(_pF4x4TChr);	///< [i]
		*pDcCbBlk++ = pCbBlk->GetDC();
		pCbBlk->SetDC(0);

		pCrBlk->ForwardTransform(_pF4x4TChr);	///< [i]
		*pDcCrBlk++ = pCrBlk->GetDC();
		pCrBlk->SetDC(0);

		/// Now scale and quant chr.
		_pF4x4TChr->SetMode(IForwardTransform::QuantOnly);	
		(pCbBlk++)->Quantise(_pF4x4TChr);
		(pCrBlk++)->Quantise(_pF4x4TChr);

	}//end for i...

	/// Transform and quant the DC blocks.
	pMb->_cbDcBlk.ForwardTransform(_pFDC2x2T);
	pMb->_crDcBlk.ForwardTransform(_pFDC2x2T);

}//end TransAndQuantInter16x16MBlk.

/** Inverse Transform and Quantise an Inter_16x16 macroblock
This method provides a speed improvement for macroblock processing and code refactoring.
@param pMb				: Macroblock to inverse transform.
@param tmpBlkFlag	: Indicate temp blocks to be used.
@return						: none
*/
void H264v2Codec::InverseTransAndQuantInter16x16MBlk(MacroBlockH264* pMb, int tmpBlkFlag)
{
	int					i;
	BlockH264*	pLumBlk;
	BlockH264*	pCbBlk;
	BlockH264*	pCrBlk;
	short*			pDcCbBlk;
	short*			pDcCrBlk;
	int					mbLumQP	= pMb->_mbQP;
	int					mbChrQP = MacroBlockH264::GetQPc(pMb->_mbQP);

	/// The inverse transform (mode = TransformOnly) and quantisation (mode = QuantOnly) are separated 
	/// for the 4x4 AC blocks in this method and therefore the quant parameter does not need to 
	/// be set. But the DC blocks are in the TransformAndQuant mode and require the setting.
	_pI4x4TLum->SetMode(IInverseTransform::TransformAndQuant);	
	_pI4x4TLum->SetParameter(IInverseTransform::QUANT_ID, mbLumQP);
	_pI4x4TChr->SetParameter(IInverseTransform::QUANT_ID, mbChrQP);
	_pIDC2x2T->SetParameter(IInverseTransform::QUANT_ID, mbChrQP);

	if(tmpBlkFlag)
	{
		/// Copy all blks, excluding Lum DC blk, to temp blks.
		MacroBlockH264::CopyBlksToTmpBlksCoeffOnly(pMb, 1, MBH264_NUM_BLKS - 1);

		/// Inverse transform & inverse quantise DC blocks.
		pMb->_cbDcBlkTmp.InverseTransform(_pIDC2x2T);
		pMb->_crDcBlkTmp.InverseTransform(_pIDC2x2T);

		/// Short cut pointers for Lum & Chr components.
    pLumBlk		= &(pMb->_lumBlkTmp[0][0]);
		pCbBlk		= &(pMb->_cbBlkTmp[0][0]);	///< Assume these are linear arrays that wrap in raster scan order.
		pCrBlk		= &(pMb->_crBlkTmp[0][0]);
		pDcCbBlk	= pMb->_cbDcBlkTmp.GetBlk();	///< Actual array of coeffs.
		pDcCrBlk	= pMb->_crDcBlkTmp.GetBlk();
	}//end if tmpBlkFlag...
	else
	{
		/// Inverse transform & inverse quantise DC blocks.
		pMb->_cbDcBlk.InverseTransform(_pIDC2x2T);
		pMb->_crDcBlk.InverseTransform(_pIDC2x2T);

		/// Short cut pointers for Chr components.
    pLumBlk		= &(pMb->_lumBlk[0][0]);
		pCbBlk		= &(pMb->_cbBlk[0][0]);	///< Assume these are linear arrays that wrap in raster scan order.
		pCrBlk		= &(pMb->_crBlk[0][0]);
		pDcCbBlk	= pMb->_cbDcBlk.GetBlk();	///< Actual array of coeffs.
		pDcCrBlk	= pMb->_crDcBlk.GetBlk();
	}//end else...

	/// Inverse quant for the main Lum, Cb and Cr blocks, insert the DC terms from the 
	/// DC blocks and then complete the inverse transform process. Note that this is 
	/// done in raster scan order and not coding order to align each position in the 
	/// DC block with the spatial location of the 4x4 block in the image.

	for(i = 0; i < 4; i++)	///< 16 Lum blocks + 4 Cb Chr and 4 Cr Chr blocks.
	{
		/// Unroll the inner loop.

		/// Inverse scale, quant and transform Lum.
		(pLumBlk++)->InverseTransform(_pI4x4TLum);	///< [i][0]
		(pLumBlk++)->InverseTransform(_pI4x4TLum);	///< [i][1]
		(pLumBlk++)->InverseTransform(_pI4x4TLum);	///< [i][2]
		(pLumBlk++)->InverseTransform(_pI4x4TLum);	///< [i][3]

    /// Inverse scale and quant first for Chr.
		_pI4x4TChr->SetMode(IInverseTransform::QuantOnly);	
		pCbBlk->InverseQuantise(_pI4x4TChr);
		pCrBlk->InverseQuantise(_pI4x4TChr);

		/// Inverse transform after scaling and quant.
		_pI4x4TChr->SetMode(IInverseTransform::TransformOnly);	
		pCbBlk->SetDC(*pDcCbBlk++);
		(pCbBlk++)->InverseTransform(_pI4x4TChr);	///< [i]

		pCrBlk->SetDC(*pDcCrBlk++);
		(pCrBlk++)->InverseTransform(_pI4x4TChr);	///< [i]
	}//end for i...

}//end InverseTransAndQuantInter16x16MBlk.

/** Get Intra Lum prediction.
Select the best prediction mode based on the min distortion with the input img. Predict 
from the ref lum img and write it to the pred parameter. Used by the encoder. Return the 
intra_16x16 prediction mode. 
@param pMb				: Macroblock to predict.
@param in					: Lum input image.
@param ref				: Reference lum img to predict from.
@param pred				: 16x16 overlay to write result to.
@return						: Prediction mode.
*/
int H264v2Codec::GetIntra16x16LumPredAndMode(MacroBlockH264* pMb, OverlayMem2Dv2* in, OverlayMem2Dv2* ref, OverlayMem2Dv2* pred)
{
	int i,j;
	int a,b,c;	///< Plane mode intermediates.
	int predDC;	///< Other mode intermediates.
	int modeDist[4] = { 0, 0, 0, 0 }; ///< Accumulator for the distortion tests.

	int mode = MacroBlockH264::Intra_16x16_DC;

	short**	in2D		= in->Get2DSrcPtr();
	int			iOffX		= in->GetOriginX();
	int			iOffY		= in->GetOriginY();
	short**	ref2D		= ref->Get2DSrcPtr();
	int			rOffX		= ref->GetOriginX();
	int			rOffY		= ref->GetOriginY();
	short**	pred2D	= pred->Get2DSrcPtr();
	int			pOffX		= pred->GetOriginX();
	int			pOffY		= pred->GetOriginY();

	/// Every mode has intermediate values that the predictions are based on and
	/// these must be prepared first. The values are further dependent on the 
	/// availability of the neighbourhood.
	bool all						= (pMb->_aboveMb != NULL)&&(pMb->_aboveLeftMb != NULL)&&(pMb->_leftMb != NULL);
	bool aboveOnly			= (pMb->_aboveMb != NULL)&&(pMb->_leftMb == NULL);	///< Don't care about above left.
	bool leftOnly				= (pMb->_aboveMb == NULL)&&(pMb->_leftMb != NULL);	///< Don't care about above left.
	bool aboveAndLeft		= (pMb->_aboveMb != NULL)&&(pMb->_leftMb != NULL);	///< Don't care about above left.

	/// The selection process uses a patial sum of the distortion method where a grid of sampling points
	/// is tested with new sets of points at each iteration. If there is no change in the winning mode
	/// after the iteration then that mode is selected.
	int pos = 0;	///< Position in the test sample point array. (Accumulates after each iteration.)
	int noChange = 0;
	int iter;

	if(all)	///< Every mode is a candidate.
	{
		///------------------- DC mode preparation -----------------------------------------
		predDC = 0;
		for(i = 0; i < 16; i++)
			predDC += ( (int)ref2D[rOffY - 1][rOffX + i] + (int)ref2D[rOffY + i][rOffX - 1]);
		predDC = (predDC + 16) >> 5;

		///------------------- Plane mode preparation --------------------------------------
		int		H = 0;
		int		V = 0;
		for(i = 0; i < 8; i++)
		{
			H += (i + 1)*((int)ref2D[rOffY - 1][rOffX + 8 + i] - (int)ref2D[rOffY - 1][rOffX + 6 - i]);
			V += (i + 1)*((int)ref2D[rOffY + 8 + i][rOffX - 1] - (int)ref2D[rOffY + 6 - i][rOffX - 1]);
		}//end for i...
		b = (5*H + 32) >> 6;
		c = (5*V + 32) >> 6;
		a = ((int)ref2D[rOffY + 15][rOffX - 1] + (int)ref2D[rOffY - 1][rOffX + 15]) << 4;

		///------------------- Test for min ------------------------------------------------
		/// Calculate the distortion at each test point and accumulate for each mode.
		for(iter = 0; (iter < 8) && (!noChange); iter++)
		{
			for(; pos < test16Limit[iter]; pos++)
			{
				int y = test16Y[pos];
				int x = test16X[pos];

				/// Vertical mode.
				int diff0 = (int)in2D[iOffY + y][iOffX + x] - (int)ref2D[rOffY - 1][rOffX + x];
				modeDist[MacroBlockH264::Intra_16x16_Vert] += (diff0*diff0);
				/// Horiz mode.
				int diff1 = (int)in2D[iOffY + y][iOffX + x] - (int)ref2D[rOffY + y][rOffX - 1];
				modeDist[MacroBlockH264::Intra_16x16_Horiz] += (diff1*diff1);
				/// DC mode.
				diff0 = (int)in2D[iOffY + y][iOffX + x] - predDC;
				modeDist[MacroBlockH264::Intra_16x16_DC] += (diff0*diff0);
				/// Plane mode.
				diff1 = (int)in2D[iOffY + y][iOffX + x] - H264V2_CLIP255( (a + b*(x-7) + c*(y-7) + 16) >> 5 );
				modeDist[MacroBlockH264::Intra_16x16_Plane] += (diff1*diff1);
			}//end for pos...

			/// The index of the lowest distortion value is the selected mode.
			int prevMode = mode;
			mode = MacroBlockH264::Intra_16x16_Vert;	///< MacroBlockH264::Intra_16x16_Vert = 0.
			for(i = MacroBlockH264::Intra_16x16_Horiz; i < 4 ; i++)	///< Total of 4 modes.
			{
				if(modeDist[i] < modeDist[mode])
					mode = i;
			}//end for i...

			if(iter > 0)	///< Do not test the 1st iteration.
			{
				if(mode == prevMode)
					noChange = 1;
			}//end if iter...

		}//end for iter...

	}//end if all...
	else if(aboveAndLeft)
	{
		///------------------- DC mode preparation -----------------------------------------
		predDC = 0;
		for(i = 0; i < 16; i++)
			predDC += ( (int)ref2D[rOffY - 1][rOffX + i] + (int)ref2D[rOffY + i][rOffX - 1]);
		predDC = (predDC + 16) >> 5;

		///------------------- Test for min ------------------------------------------------
		for(iter = 0; (iter < 8) && (!noChange); iter++)
		{
			for(; pos < test16Limit[iter]; pos++)
			{
				int y = test16Y[pos];
				int x = test16X[pos];

				/// Vertical mode.
				int diff0 = (int)in2D[iOffY + y][iOffX + x] - (int)ref2D[rOffY - 1][rOffX + x];
				modeDist[MacroBlockH264::Intra_16x16_Vert] += (diff0*diff0);
				/// Horiz mode.
				int diff1 = (int)in2D[iOffY + y][iOffX + x] - (int)ref2D[rOffY + y][rOffX - 1];
				modeDist[MacroBlockH264::Intra_16x16_Horiz] += (diff1*diff1);
				/// DC mode.
				int diff2 = (int)in2D[iOffY + y][iOffX + x] - predDC;
				modeDist[MacroBlockH264::Intra_16x16_DC] += (diff2*diff2);
			}//end for pos...

			/// The index of the lowest distortion value is the selected mode.
			int prevMode = mode;
			mode = MacroBlockH264::Intra_16x16_Vert;	///< MacroBlockH264::Intra_16x16_Vert = 0.
			for(i = MacroBlockH264::Intra_16x16_Horiz; i < 3 ; i++)	///< Total of 3 modes. Plane mode excluded.
			{
				if(modeDist[i] < modeDist[mode])
					mode = i;
			}//end for i...

			if(iter > 0)	///< Do not test the 1st iteration.
			{
				if(mode == prevMode)
					noChange = 1;
			}//end if iter...

		}//end for iter...

	}//end else if aboveAndLeft...
	else if(leftOnly)
	{
		///------------------- DC mode preparation -----------------------------------------
		predDC = 0;
		for(i = 0; i < 16; i++)
			predDC += (int)ref2D[rOffY + i][rOffX - 1];
		predDC = (predDC + 8) >> 4;

		///------------------- Test for min ------------------------------------------------
		for(iter = 0; (iter < 8) && (!noChange); iter++)
		{
			for(; pos < test16Limit[iter]; pos++)
			{
				int y = test16Y[pos];
				int x = test16X[pos];

				/// Horiz mode.
				int diff0 = (int)in2D[iOffY + y][iOffX + x] - (int)ref2D[rOffY + y][rOffX - 1];
				modeDist[MacroBlockH264::Intra_16x16_Horiz] += (diff0*diff0);
				/// DC mode.
				int diff1 = (int)in2D[iOffY + y][iOffX + x] - predDC;
				modeDist[MacroBlockH264::Intra_16x16_DC] += (diff1*diff1);
			}//end for pos...

			/// The index of the lowest distortion value is the selected mode.
			int prevMode = mode;
			mode = MacroBlockH264::Intra_16x16_Horiz;	///< MacroBlockH264::Intra_16x16_Horiz = 0.
			if(modeDist[MacroBlockH264::Intra_16x16_DC] < modeDist[MacroBlockH264::Intra_16x16_Horiz])
				mode = MacroBlockH264::Intra_16x16_DC;

			if(iter > 0)	///< Do not test the 1st iteration.
			{
				if(mode == prevMode)
					noChange = 1;
			}//end if iter...

		}//end for iter...

	}//end else if leftOnly...
	else if(aboveOnly)
	{
		///------------------- DC mode preparation -----------------------------------------
		predDC = 0;
		for(i = 0; i < 16; i++)
			predDC += (int)ref2D[rOffY - 1][rOffX + i];
		predDC = (predDC + 8) >> 4;

		///------------------- Test for min ------------------------------------------------
		for(iter = 0; (iter < 8) && (!noChange); iter++)
		{
			for(; pos < test16Limit[iter]; pos++)
			{
				int y = test16Y[pos];
				int x = test16X[pos];

				/// Vertical mode.
				int diff0 = (int)in2D[iOffY + y][iOffX + x] - (int)ref2D[rOffY - 1][rOffX + x];
				modeDist[MacroBlockH264::Intra_16x16_Vert] += (diff0*diff0);
				/// DC mode.
				int diff1 = (int)in2D[iOffY + y][iOffX + x] - predDC;
				modeDist[MacroBlockH264::Intra_16x16_DC] += (diff1*diff1);
			}//end for pos...

			/// The index of the lowest distortion value is the selected mode.
			int prevMode = mode;
			mode = MacroBlockH264::Intra_16x16_Vert;	///< MacroBlockH264::Intra_16x16_Vert = 0.
			if(modeDist[MacroBlockH264::Intra_16x16_DC] < modeDist[MacroBlockH264::Intra_16x16_Vert])
				mode = MacroBlockH264::Intra_16x16_DC;

			if(iter > 0)	///< Do not test the 1st iteration.
			{
				if(mode == prevMode)
					noChange = 1;
			}//end if iter...

		}//end for iter...

	}//end else if aboveOnly...
	else
	{
		/// Use the DC default mode when there are no valid neighbours and use half the 
		/// max Lum value (128) as the prediction. This is typically the top left corner
		/// of the slice.
		pred->Fill(128);
		return(mode);	///< Early exit.
	}//end else...

	/// Fill the prediction block with the lowest distortion mode prediction.
	switch(mode)
	{
		case MacroBlockH264::Intra_16x16_Vert:
			{
				for(i = 0; i < 16; i++)
					for(j = 0; j < 16; j++)
						pred2D[pOffY + i][pOffX + j] = ref2D[rOffY - 1][rOffX + j];
			}//end Vert block...
			break;
		case MacroBlockH264::Intra_16x16_Horiz:
			{
				for(i = 0; i < 16; i++)
					for(j = 0; j < 16; j++)
						pred2D[pOffY + i][pOffX + j] = ref2D[rOffY + i][rOffX - 1];
			}//end Horiz block...
			break;
		case MacroBlockH264::Intra_16x16_DC:
			pred->Fill(predDC);
			break;
		case MacroBlockH264::Intra_16x16_Plane:
			{
				for(i = 0; i < 16; i++)
					for(j = 0; j < 16; j++)
						pred2D[pOffY + i][pOffX + j] = H264V2_CLIP255( (a + b*(j-7) + c*(i-7) + 16) >> 5 );
			}//end Horiz block...
			break;
	}//end switch mode...

	return(mode);
}//end GetIntra16x16LumPredAndMode.

/** Get Intra Lum prediction.
Apply the prediction mode specified to predict from the ref lum img and write it to the pred 
parameter. Used by the decoder.  
@param pMb				: Macroblock to predict.
@param ref				: Reference lum img to predict from.
@param pred				: 16x16 overlay to write result to.
@param predMode		: Mode to apply.
@return						: 1 = success, 0 = failure.
*/
int H264v2Codec::GetIntra16x16LumPred(MacroBlockH264* pMb, OverlayMem2Dv2* ref, OverlayMem2Dv2* pred, int predMode)
{
	int i,j;

	short**	ref2D		= ref->Get2DSrcPtr();
	int			rOffX		= ref->GetOriginX();
	int			rOffY		= ref->GetOriginY();
	short**	pred2D	= pred->Get2DSrcPtr();
	int			pOffX		= pred->GetOriginX();
	int			pOffY		= pred->GetOriginY();

	/// Fill the prediction block with the appropriate mode prediction.
	switch(predMode)
	{
		case MacroBlockH264::Intra_16x16_Vert:
			{
				if(pMb->_aboveMb != NULL)	///< Vert prediction only cares about the macroblock above.
				{
					for(i = 0; i < 16; i++)
						for(j = 0; j < 16; j++)
							pred2D[pOffY + i][pOffX + j] = ref2D[rOffY - 1][rOffX + j];
				}//end if aboveMb...
				else
				{
					/// Prediction fails.
					pred->Fill(128);	///< Safty net to fill with half max value.
					return(0);
				}//end else...
			}//end Vert block...
			break;
		case MacroBlockH264::Intra_16x16_Horiz:
			{
				if(pMb->_leftMb != NULL)	///< Horiz prediction only cares about the macroblock to the left.
				{
					for(i = 0; i < 16; i++)
						for(j = 0; j < 16; j++)
							pred2D[pOffY + i][pOffX + j] = ref2D[rOffY + i][rOffX - 1];
				}//end if leftMb...
				else
				{
					/// Prediction fails.
					pred->Fill(128);	///< Safty net to fill with half max value.
					return(0);
				}//end else...
			}//end Horiz block...
			break;
		case MacroBlockH264::Intra_16x16_DC:
			{
				bool aboveAndLeft		= (pMb->_aboveMb != NULL)&&(pMb->_leftMb != NULL);
				bool aboveOnly			= (pMb->_aboveMb != NULL)&&(pMb->_leftMb == NULL);
				bool leftOnly				= (pMb->_aboveMb == NULL)&&(pMb->_leftMb != NULL);

				int predDC = 0;
				if(aboveAndLeft)
				{
					for(i = 0; i < 16; i++)
						predDC += ( (int)ref2D[rOffY - 1][rOffX + i] + (int)ref2D[rOffY + i][rOffX - 1]);
					predDC = (predDC + 16) >> 5;
				}//end if aboveAndLeft...
				else if(leftOnly)
				{
					for(i = 0; i < 16; i++)
						predDC += (int)ref2D[rOffY + i][rOffX - 1];
					predDC = (predDC + 8) >> 4;
				}//end else if leftOnly...
				else if(aboveOnly)
				{
					for(i = 0; i < 16; i++)
						predDC += (int)ref2D[rOffY - 1][rOffX + i];
					predDC = (predDC + 8) >> 4;
				}//end else if aboveOnly...
				else
				{
					predDC = 128;	///< Half the max value.
				}//end else...

				pred->Fill(predDC);
			}//end DC block...
			break;
		case MacroBlockH264::Intra_16x16_Plane:
			{
				bool all = (pMb->_aboveMb != NULL)&&(pMb->_aboveLeftMb != NULL)&&(pMb->_leftMb != NULL);

				if(all)	///< All the neighbouring macroblocks must be available for prediction.
				{
					int		H = 0;
					int		V = 0;
					for(i = 0; i < 8; i++)
					{
						H += (i + 1)*((int)ref2D[rOffY - 1][rOffX + 8 + i] - (int)ref2D[rOffY - 1][rOffX + 6 - i]);
						V += (i + 1)*((int)ref2D[rOffY + 8 + i][rOffX - 1] - (int)ref2D[rOffY + 6 - i][rOffX - 1]);
					}//end for i...
					int b = (5*H + 32) >> 6;
					int c = (5*V + 32) >> 6;
					int a = ((int)ref2D[rOffY + 15][rOffX - 1] + (int)ref2D[rOffY - 1][rOffX + 15]) << 4;

					for(i = 0; i < 16; i++)
						for(j = 0; j < 16; j++)
							pred2D[pOffY + i][pOffX + j] = H264V2_CLIP255( (a + b*(j-7) + c*(i-7) + 16) >> 5 );
				}//end if all...
				else
				{
					/// Prediction fails.
					pred->Fill(128);	///< Safty net to fill with half max value.
					return(0);
				}//end else...
			}//end Horiz block...
			break;
	}//end switch predMode...

	return(1);
}//end GetIntra16x16LumPred.

/** Get Intra Lum DC prediction.
Predict from the input Lum img and write it to the pred parameter. If 
there are no valid neighbours to make the prediction then use half the 
max Lum value (128) as the prediction.
@param pMb				: Macroblock to transform.
@param lum				: Lum img to predict from.
@param pred				: 16x16 overlay to write result to.
@return						: none
*/
void H264v2Codec::GetIntra16x16LumDCPred(MacroBlockH264* pMb, OverlayMem2Dv2* lum, OverlayMem2Dv2* pred)
{
	int offX, offY, i;
	int dirCnt		= 0;
	int predValue	= 0;

	short** img = lum->Get2DSrcPtr();

	if(pMb->_aboveMb != NULL)
	{
		dirCnt++;
		/// The last row of the macroblock above.
		offX = pMb->_aboveMb->_offLumX;
		offY = pMb->_aboveMb->_offLumY + 15;
		for(i = 0; i < 16; i++)
			predValue += img[offY][offX + i];
	}//end if _aboveMb...

	if(pMb->_leftMb != NULL)
	{
		dirCnt++;
		/// The last col of the macroblock to the left.
		offX = pMb->_leftMb->_offLumX + 15;
		offY = pMb->_leftMb->_offLumY;
		for(i = 0; i < 16; i++)
			predValue += img[offY + i][offX];
	}//end if _leftMb...

	/// Set the accumulated DC prediction value.
	if(dirCnt == 2)
		predValue = (predValue + 16) >> 5;
	else if(dirCnt == 1)
		predValue = (predValue + 8) >> 4;
	else
		predValue = 128;	///< Half max Lum value.

	pred->Fill(predValue);

}//end GetIntra16x16LumDCPred.

/** Get Intra Lum Plane prediction.
Predict from the input Lum img and write it to the pred parameter. If 
there are no valid neighbours to make the prediction then use half the 
max Lum value (128) as the prediction. The H.264 std does not specify 
this last case for Plane pred.
@param pMb				: Macroblock to transform.
@param lum				: Lum img to predict from.
@param pred				: 16x16 overlay to write result to.
@return						: 1 = pred was made, 0 = no macroblocks available for pred.
*/
int H264v2Codec::GetIntra16x16LumPlanePred(MacroBlockH264* pMb, OverlayMem2Dv2* lum, OverlayMem2Dv2* pred)
{
	int i,j;

	if( (pMb->_aboveMb != NULL)&&(pMb->_aboveLeftMb != NULL)&&(pMb->_leftMb != NULL) )
	{
		short**	img2D		= lum->Get2DSrcPtr();
		int			iOffX		= lum->GetOriginX();	///< Top left of img2D pMb location.
		int			iOffY		= lum->GetOriginY();
		short**	pred2D	= pred->Get2DSrcPtr();
		int			pOffX		= pred->GetOriginX();
		int			pOffY		= pred->GetOriginY();

		int		H = 0;
		int		V = 0;
		for(i = 0; i < 8; i++)
		{
			H += (i + 1)*((int)img2D[iOffY - 1][iOffX + 8 + i] - (int)img2D[iOffY - 1][iOffX + 6 - i]);
			V += (i + 1)*((int)img2D[iOffY + 8 + i][iOffX - 1] - (int)img2D[iOffY + 6 - i][iOffX - 1]);
		}//end for i...
		int b = (5*H + 32) >> 6;
		int c = (5*V + 32) >> 6;
		int a = ((int)img2D[iOffY + 15][iOffX - 1] + (int)img2D[iOffY - 1][iOffX + 15]) << 4;

    for(i = 0; i < 16; i++)
			for(j = 0; j < 16; j++)
			{
				short x = (short)((a + b*(j-7) + c*(i-7) + 16) >> 5);
				pred2D[pOffY+i][pOffX+j] = (short)(H264V2_CLIP255(x));
			}//end for i & j...

		return(1);	///< Successful prediction.
	}//end if _aboveMb...

	/// Safety precaution not defined in H.264 standard. If the macroblock above, 
	/// above left and left are not available for prediction then this mode should 
	/// never be used.
	pred->Fill(128);
	return(0);	///< Prediction macroblocks do not exist.
}//end GetIntra16x16LumPlanePred.

/** Get Intra Vertical prediction.
Predict from the input img and write it to the pred parameter. The dimensions
of the pred block define the size of the operation. If there are no valid 
neighbours to make the prediction then use half the max value (128) as 
the prediction. The H.264 std does not specify this last case for vert pred.
@param pMb				: Macroblock to transform.
@param img				: Img to predict from.
@param pred				: Overlay to write prediction result to.
@param lumFlag		: Is the prediction for lum?
@return						: 1 = pred was made, 0 = no macroblocks available for pred.
*/
int H264v2Codec::GetIntraVertPred(MacroBlockH264* pMb, OverlayMem2Dv2* img, OverlayMem2Dv2* pred, int lumFlag)
{
	int i;
	MacroBlockH264* amb	= pMb->_aboveMb;
	
	if(amb != NULL)
	{
		short**	img2D		= img->Get2DSrcPtr();
		short**	pred2D	= pred->Get2DSrcPtr();
		int	width		= pred->GetWidth();
		int	height	= pred->GetHeight();
		int offX		= pred->GetOriginX();
		int offY		= pred->GetOriginY();
		int byteLen	= width * sizeof(short);
		/// The last row of the macroblock above.
		int iOffX, iOffY;
		if(lumFlag)
		{
			iOffX = amb->_offLumX;
			iOffY = amb->_offLumY + (img->GetHeight() - 1);
		}//end if lumFlag...
		else
		{
			iOffX = amb->_offChrX;
			iOffY = amb->_offChrY + (img->GetHeight() - 1);
		}//end else...
		short* ps = &(img2D[iOffY][iOffX]);
		for(i = 0; i < height; i++)	///< Rows.
			memcpy((void *)(&(pred2D[offY + i][offX])), (const void *)ps, byteLen);
		return(1);
	}//end if amb...

	/// Safety precaution not defined in H.264 standard. If the macroblock above
	/// is not available for prediction then this mode should never be used.
	pred->Fill(128);
	return(0);	///< Macroblock above does not exist.
}//end GetIntraVertPred.

/** Get Intra Horizontal prediction.
Predict from the input img and write it to the pred parameter. The dimensions
of the pred block define the size of the operation. If there are no valid 
neighbours to make the prediction then use half the max value (128) as 
the prediction. The H.264 std does not specify this last case for vert pred.
@param pMb				: Macroblock to transform.
@param img				: Img to predict from.
@param pred				: Overlay to write prediction result to.
@param lumFlag		: Is the prodiction for lum?
@return						: 1 = pred was made, 0 = no macroblocks available for pred.
*/
int H264v2Codec::GetIntraHorizPred(MacroBlockH264* pMb, OverlayMem2Dv2* img, OverlayMem2Dv2* pred, int lumFlag)
{
	int i,j;
	MacroBlockH264* lmb	= pMb->_leftMb;
	
	if(lmb != NULL)
	{
		short**	img2D		= img->Get2DSrcPtr();
		short**	pred2D	= pred->Get2DSrcPtr();
		int	width		= pred->GetWidth();
		int	height	= pred->GetHeight();
		int offX		= pred->GetOriginX();
		int offY		= pred->GetOriginY();
		int byteLen	= width * sizeof(short);
		/// The last col of the macroblock to the left.
		int iOffX, iOffY;
		if(lumFlag)
		{
			iOffX = lmb->_offLumX + (img->GetWidth() - 1);
			iOffY = lmb->_offLumY ;
		}//end if lumFlag...
		else
		{
			iOffX = lmb->_offChrX + (img->GetWidth() - 1);
			iOffY = lmb->_offChrY;
		}//end else...
		for(i = 0; i < height; i++)	///< Rows.
		{
			short* pd = &(pred2D[offY + i][offX]);
			short  x  = img2D[iOffY + i][iOffX];
			for(j = 0; j < width; j++)
				*pd++ = x;
		}//end for i...
		return(1);
	}//end if lmb...

	/// Safety precaution not defined in H.264 standard. If the left macroblock
	/// is not available for prediction then this mode should never be used.
	pred->Fill(128);
	return(0);	///< Left macroblock does not exist.
}//end GetIntraHorizPred.

/** Get Intra Chr prediction.
Select the best prediction mode based on the min distortion with the input img. Predict 
from the ref chr img and write it to the pred parameter. Used by the encoder. Return the 
intra_8x8 prediction mode. 
@param pMb				: Macroblock to predict.
@param cb					: Cb Chr input image.
@param cr					: Cr Chr input image.
@param refCb			: Reference chr Cb img to predict from.
@param refCr			: Reference chr Cr img to predict from.
@param predCb			: 8x8 Cb overlay to write result to.
@param predCr			: 8x8 Cr overlay to write result to.
@return						: Prediction mode.
*/
int H264v2Codec::GetIntra8x8ChrPredAndMode(MacroBlockH264* pMb, OverlayMem2Dv2* cb,			OverlayMem2Dv2* cr, 
																																OverlayMem2Dv2* refCb,	OverlayMem2Dv2* refCr, 
																																OverlayMem2Dv2* predCb, OverlayMem2Dv2* predCr)
{
	int i,j;
	int aCb,bCb,cCb,aCr,bCr,cCr;	///< Plane mode intermediates.
	int predCbDC[4] = { 0, 0, 0, 0};	///< DC mode intermediates.
	int predCrDC[4] = { 0, 0, 0, 0};
	int modeDist[4] = { 0, 0, 0, 0 }; ///< Accumulator for the distortion tests.

	int mode = MacroBlockH264::Intra_Chr_DC;

	short**	inCb2D		= cb->Get2DSrcPtr();
	short**	inCr2D		= cr->Get2DSrcPtr();
	int			iOffX			= cb->GetOriginX();	///< Assume Cb and Cr have the same offsets.
	int			iOffY			= cb->GetOriginY();
	short**	refCb2D		= refCb->Get2DSrcPtr();
	short**	refCr2D		= refCr->Get2DSrcPtr();
	int			rOffX			= refCb->GetOriginX();
	int			rOffY			= refCb->GetOriginY();
	short**	predCb2D	= predCb->Get2DSrcPtr();
	short**	predCr2D	= predCr->Get2DSrcPtr();
	int			pOffX			= predCb->GetOriginX();
	int			pOffY			= predCr->GetOriginY();

	/// Every mode has intermediate values that the predictions are based on and
	/// these must be prepared first. The values are further dependent on the 
	/// availability of the neighbourhood.
	bool all						= (pMb->_aboveMb != NULL)&&(pMb->_aboveLeftMb != NULL)&&(pMb->_leftMb != NULL);
	bool aboveOnly			= (pMb->_aboveMb != NULL)&&(pMb->_leftMb == NULL);	///< Don't care about above left.
	bool leftOnly				= (pMb->_aboveMb == NULL)&&(pMb->_leftMb != NULL);	///< Don't care about above left.
	bool aboveAndLeft		= (pMb->_aboveMb != NULL)&&(pMb->_leftMb != NULL);	///< Don't care about above left.

	/// The selection process uses a patial sum of the distortion method where a grid of sampling points
	/// is tested with new sets of points at each iteration. If there is no change in the winning mode
	/// after the iteration then that mode is selected.
	int pos = 0;	///< Position in the test sample point array. (Accumulates after each iteration.)
	int noChange = 0;
	int iter;

	if(all)	///< Every mode is a candidate.
	{
		///------------------- DC mode preparation -----------------------------------------
		for(i = 0; i < 4; i++)	///< Accumulate sums.
		{
			predCbDC[0] += ( (int)refCb2D[rOffY - 1][rOffX + i] + (int)refCb2D[rOffY + i][rOffX - 1]);
			predCbDC[1] +=   (int)refCb2D[rOffY - 1][rOffX + 4 + i];
			predCbDC[2] +=   (int)refCb2D[rOffY + 4 + i][rOffX - 1];
			predCbDC[3] += ( (int)refCb2D[rOffY - 1][rOffX + 4 + i] + (int)refCb2D[rOffY + 4 + i][rOffX - 1]);

			predCrDC[0] += ( (int)refCr2D[rOffY - 1][rOffX + i] + (int)refCr2D[rOffY + i][rOffX - 1]);
			predCrDC[1] +=   (int)refCr2D[rOffY - 1][rOffX + 4 + i];
			predCrDC[2] +=   (int)refCr2D[rOffY + 4 + i][rOffX - 1];
			predCrDC[3] += ( (int)refCr2D[rOffY - 1][rOffX + 4 + i] + (int)refCr2D[rOffY + 4 + i][rOffX - 1]);
		}//end for i...

    /// Calculate average values.
		predCbDC[0] = (predCbDC[0] + 4) >> 3;
		predCbDC[1] = (predCbDC[1] + 2) >> 2;
		predCbDC[2] = (predCbDC[2] + 2) >> 2;
		predCbDC[3] = (predCbDC[3] + 4) >> 3;

		predCrDC[0] = (predCrDC[0] + 4) >> 3;
		predCrDC[1] = (predCrDC[1] + 2) >> 2;
		predCrDC[2] = (predCrDC[2] + 2) >> 2;
		predCrDC[3] = (predCrDC[3] + 4) >> 3;

		///------------------- Plane mode preparation --------------------------------------
		int		HCb = 0;
		int		VCb = 0;
		int		HCr = 0;
		int		VCr = 0;
		for(i = 0; i < 4; i++)
		{
			HCb += (i + 1)*((int)refCb2D[rOffY - 1][rOffX + 4 + i] - (int)refCb2D[rOffY - 1][rOffX + 2 - i]);
			VCb += (i + 1)*((int)refCb2D[rOffY + 4 + i][rOffX - 1] - (int)refCb2D[rOffY + 2 - i][rOffX - 1]);
			HCr += (i + 1)*((int)refCr2D[rOffY - 1][rOffX + 4 + i] - (int)refCr2D[rOffY - 1][rOffX + 2 - i]);
			VCr += (i + 1)*((int)refCr2D[rOffY + 4 + i][rOffX - 1] - (int)refCr2D[rOffY + 2 - i][rOffX - 1]);
		}//end for i...
		bCb = (34*HCb + 32) >> 6;
		cCb = (34*VCb + 32) >> 6;
		aCb = ((int)refCb2D[rOffY + 7][rOffX - 1] + (int)refCb2D[rOffY - 1][rOffX + 7]) << 4;
		bCr = (34*HCr + 32) >> 6;
		cCr = (34*VCr + 32) >> 6;
		aCr = ((int)refCr2D[rOffY + 7][rOffX - 1] + (int)refCr2D[rOffY - 1][rOffX + 7]) << 4;

		///------------------- Test for min ------------------------------------------------
		/// Calculate the distortion at each test point and accumulate for each mode.
		for(iter = 0; (iter < 4) && (!noChange); iter++)
		{
			for(; pos < test8Limit[iter]; pos++)
			{
				int y = test8Y[pos];
				int x = test8X[pos];
				/// DC mode.
				int quadrant = 0;
				if( (y < 4)&&(x > 3) )
					quadrant = 1;
				else if(y > 3)
				{
					if(x < 4)
						quadrant = 2;
					else
						quadrant = 3;
				}//end else if y...
				int diff0 = (int)inCb2D[iOffY + y][iOffX + x] - predCbDC[quadrant];
				int diff1 = (int)inCr2D[iOffY + y][iOffX + x] - predCrDC[quadrant];
				modeDist[MacroBlockH264::Intra_Chr_DC] += ((diff0*diff0) + (diff1*diff1));
				/// Horiz mode.
				int diff2 = (int)inCb2D[iOffY + y][iOffX + x] - (int)refCb2D[rOffY + y][rOffX - 1];
				int diff3 = (int)inCr2D[iOffY + y][iOffX + x] - (int)refCr2D[rOffY + y][rOffX - 1];
				modeDist[MacroBlockH264::Intra_Chr_Horiz] += ((diff2*diff2) + (diff3*diff3));
				/// Vertical mode.
				diff0 = (int)inCb2D[iOffY + y][iOffX + x] - (int)refCb2D[rOffY - 1][rOffX + x];
				diff1 = (int)inCr2D[iOffY + y][iOffX + x] - (int)refCr2D[rOffY - 1][rOffX + x];
				modeDist[MacroBlockH264::Intra_Chr_Vert] += ((diff0*diff0) + (diff1*diff1));
				/// Plane mode.
				diff2 = (int)inCb2D[iOffY + y][iOffX + x] - H264V2_CLIP255( (aCb + bCb*(x-3) + cCb*(y-3) + 16) >> 5 );
				diff3 = (int)inCr2D[iOffY + y][iOffX + x] - H264V2_CLIP255( (aCr + bCr*(x-3) + cCr*(y-3) + 16) >> 5 );
				modeDist[MacroBlockH264::Intra_Chr_Plane] += ((diff2*diff2) + (diff3*diff3));
			}//end for pos...

			/// The index of the lowest distortion value is the selected mode.
			int prevMode = mode;
			mode = MacroBlockH264::Intra_Chr_DC;	///< MacroBlockH264::Intra_Chr_DC = 0.
			for(i = MacroBlockH264::Intra_Chr_Horiz; i < 4 ; i++)	///< Total of 4 modes.
			{
				if(modeDist[i] < modeDist[mode])
					mode = i;
			}//end for i...

			if(iter > 0)	///< Do not test the 1st iteration.
			{
				if(mode == prevMode)
					noChange = 1;
			}//end if iter...

		}//end for iter...

	}//end if all...
	else if(aboveAndLeft)
	{
		///------------------- DC mode preparation -----------------------------------------
		for(i = 0; i < 4; i++)	///< Accumulate sums.
		{
			predCbDC[0] += ( (int)refCb2D[rOffY - 1][rOffX + i] + (int)refCb2D[rOffY + i][rOffX - 1]);
			predCbDC[1] +=   (int)refCb2D[rOffY - 1][rOffX + 4 + i];
			predCbDC[2] +=   (int)refCb2D[rOffY + 4 + i][rOffX - 1];
			predCbDC[3] += ( (int)refCb2D[rOffY - 1][rOffX + 4 + i] + (int)refCb2D[rOffY + 4 + i][rOffX - 1]);

			predCrDC[0] += ( (int)refCr2D[rOffY - 1][rOffX + i] + (int)refCr2D[rOffY + i][rOffX - 1]);
			predCrDC[1] +=   (int)refCr2D[rOffY - 1][rOffX + 4 + i];
			predCrDC[2] +=   (int)refCr2D[rOffY + 4 + i][rOffX - 1];
			predCrDC[3] += ( (int)refCr2D[rOffY - 1][rOffX + 4 + i] + (int)refCr2D[rOffY + 4 + i][rOffX - 1]);
		}//end for i...

    /// Calculate average values.
		predCbDC[0] = (predCbDC[0] + 4) >> 3;
		predCbDC[1] = (predCbDC[1] + 2) >> 2;
		predCbDC[2] = (predCbDC[2] + 2) >> 2;
		predCbDC[3] = (predCbDC[3] + 4) >> 3;

		predCrDC[0] = (predCrDC[0] + 4) >> 3;
		predCrDC[1] = (predCrDC[1] + 2) >> 2;
		predCrDC[2] = (predCrDC[2] + 2) >> 2;
		predCrDC[3] = (predCrDC[3] + 4) >> 3;

		///------------------- Test for min ------------------------------------------------
		/// Calculate the distortion at each test point and accumulate for each mode.
		for(iter = 0; (iter < 4) && (!noChange); iter++)
		{
			for(; pos < test8Limit[iter]; pos++)
			{
				int y = test8Y[pos];
				int x = test8X[pos];
				/// DC mode.
				int quadrant = 0;
				if( (y < 4)&&(x > 3) )
					quadrant = 1;
				else if(y > 3)
				{
					if(x < 4)
						quadrant = 2;
					else
						quadrant = 3;
				}//end else if y...
				int diff0 = (int)inCb2D[iOffY + y][iOffX + x] - predCbDC[quadrant];
				int diff1 = (int)inCr2D[iOffY + y][iOffX + x] - predCrDC[quadrant];
				modeDist[MacroBlockH264::Intra_Chr_DC] += ((diff0*diff0) + (diff1*diff1));
				/// Horiz mode.
				int diff2 = (int)inCb2D[iOffY + y][iOffX + x] - (int)refCb2D[rOffY + y][rOffX - 1];
				int diff3 = (int)inCr2D[iOffY + y][iOffX + x] - (int)refCr2D[rOffY + y][rOffX - 1];
				modeDist[MacroBlockH264::Intra_Chr_Horiz] += ((diff2*diff2) + (diff3*diff3));
				/// Vertical mode.
				diff0 = (int)inCb2D[iOffY + y][iOffX + x] - (int)refCb2D[rOffY - 1][rOffX + x];
				diff1 = (int)inCr2D[iOffY + y][iOffX + x] - (int)refCr2D[rOffY - 1][rOffX + x];
				modeDist[MacroBlockH264::Intra_Chr_Vert] += ((diff0*diff0) + (diff1*diff1));
			}//end for pos...

			/// The index of the lowest distortion value is the selected mode.
			int prevMode = mode;
			mode = MacroBlockH264::Intra_Chr_DC;	///< MacroBlockH264::Intra_Chr_DC = 0.
			for(i = MacroBlockH264::Intra_Chr_Horiz; i < 3 ; i++)	///< Plane mode excluded.
			{
				if(modeDist[i] < modeDist[mode])
					mode = i;
			}//end for i...

			if(iter > 0)	///< Do not test the 1st iteration.
			{
				if(mode == prevMode)
					noChange = 1;
			}//end if iter...

		}//end for iter...

	}//end else if aboveAndLeft...
	else if(leftOnly)
	{
		///------------------- DC mode preparation -----------------------------------------
		for(i = 0; i < 4; i++)	///< Accumulate sums.
		{
			predCbDC[0] += (int)refCb2D[rOffY + i][rOffX - 1];
			predCbDC[2] += (int)refCb2D[rOffY + 4 + i][rOffX - 1];

			predCrDC[0] += (int)refCr2D[rOffY + i][rOffX - 1];
			predCrDC[2] += (int)refCr2D[rOffY + 4 + i][rOffX - 1];
		}//end for i...
		/// Calculate average values.
		predCbDC[0] = (predCbDC[0] + 2) >> 2;
		predCbDC[1] = predCbDC[0];
		predCbDC[2] = (predCbDC[2] + 2) >> 2;
		predCbDC[3] = predCbDC[2];

		predCrDC[0] = (predCrDC[0] + 2) >> 2;
		predCrDC[1] = predCrDC[0];
		predCrDC[2] = (predCrDC[2] + 2) >> 2;
		predCrDC[3] = predCrDC[2];

		///------------------- Test for min ------------------------------------------------
		for(iter = 0; (iter < 4) && (!noChange); iter++)
		{
			for(; pos < test8Limit[iter]; pos++)
			{
				int y = test8Y[pos];
				int x = test8X[pos];
				/// DC mode.
				int partition = 0;
				if(y > 3)
					partition = 2;
				int diff0 = (int)inCb2D[iOffY + y][iOffX + x] - predCbDC[partition];
				int diff1 = (int)inCr2D[iOffY + y][iOffX + x] - predCrDC[partition];
				modeDist[MacroBlockH264::Intra_Chr_DC] += ((diff0*diff0) + (diff1*diff1));
				/// Horiz mode.
				int diff2 = (int)inCb2D[iOffY + y][iOffX + x] - (int)refCb2D[rOffY + y][rOffX - 1];
				int diff3 = (int)inCr2D[iOffY + y][iOffX + x] - (int)refCr2D[rOffY + y][rOffX - 1];
				modeDist[MacroBlockH264::Intra_Chr_Horiz] += ((diff2*diff2) + (diff3*diff3));
			}//end for pos...

			/// The index of the lowest distortion value is the selected mode.
			int prevMode = mode;
			mode = MacroBlockH264::Intra_Chr_DC;	///< MacroBlockH264::Intra_Chr_DC = 0.
			if(modeDist[MacroBlockH264::Intra_Chr_Horiz] < modeDist[MacroBlockH264::Intra_Chr_DC])
				mode = MacroBlockH264::Intra_Chr_Horiz;

			if(iter > 0)	///< Do not test the 1st iteration.
			{
				if(mode == prevMode)
					noChange = 1;
			}//end if iter...

		}//end for iter...

	}//end else if leftOnly...
	else if(aboveOnly)
	{
		///------------------- DC mode preparation -----------------------------------------
		for(i = 0; i < 4; i++)	///< Accumulate sums.
		{
			predCbDC[0] += (int)refCb2D[rOffY - 1][rOffX + i];
			predCbDC[1] += (int)refCb2D[rOffY - 1][rOffX + 4 + i];

			predCrDC[0] += (int)refCr2D[rOffY - 1][rOffX + i];
			predCrDC[1] += (int)refCr2D[rOffY - 1][rOffX + 4 + i];
		}//end for i...
		/// Calculate average values.
		predCbDC[0] = (predCbDC[0] + 2) >> 2;
		predCbDC[2] = predCbDC[0];
		predCbDC[1] = (predCbDC[1] + 2) >> 2;
		predCbDC[3] = predCbDC[1];

		predCrDC[0] = (predCrDC[0] + 2) >> 2;
		predCrDC[2] = predCrDC[0];
		predCrDC[1] = (predCrDC[1] + 2) >> 2;
		predCrDC[3] = predCrDC[1];

		///------------------- Test for min ------------------------------------------------
		for(iter = 0; (iter < 4) && (!noChange); iter++)
		{
			for(; pos < test8Limit[iter]; pos++)
			{
				int y = test8Y[pos];
				int x = test8X[pos];
				/// DC mode.
				int partition = 0;
				if(x > 3)
					partition = 1;
				int diff0 = (int)inCb2D[iOffY + y][iOffX + x] - predCbDC[partition];
				int diff1 = (int)inCr2D[iOffY + y][iOffX + x] - predCrDC[partition];
				modeDist[MacroBlockH264::Intra_Chr_DC] += ((diff0*diff0) + (diff1*diff1));
				/// Vertical mode.
				int diff2 = (int)inCb2D[iOffY + y][iOffX + x] - (int)refCb2D[rOffY - 1][rOffX + x];
				int diff3 = (int)inCr2D[iOffY + y][iOffX + x] - (int)refCr2D[rOffY - 1][rOffX + x];
				modeDist[MacroBlockH264::Intra_Chr_Vert] += ((diff2*diff2) + (diff3*diff3));
			}//end for pos...

			/// The index of the lowest distortion value is the selected mode.
			int prevMode = mode;
			mode = MacroBlockH264::Intra_Chr_DC;	///< MacroBlockH264::Intra_Chr_DC = 0.
			if(modeDist[MacroBlockH264::Intra_Chr_Vert] < modeDist[MacroBlockH264::Intra_Chr_DC])
				mode = MacroBlockH264::Intra_Chr_Vert;

			if(iter > 0)	///< Do not test the 1st iteration.
			{
				if(mode == prevMode)
					noChange = 1;
			}//end if iter...

		}//end for iter...

	}//end else if aboveOnly...
	else
	{
		/// Use the DC default mode when there are no valid neighbours and use half the 
		/// max Chr value (128) as the prediction. This is typically the top left corner
		/// of the slice.
		predCb->Fill(128);
		predCr->Fill(128);
		return(mode);	///< Early exit.
	}//end else...

	/// Fill the prediction block with the lowest distortion mode prediction.
	switch(mode)
	{
		case MacroBlockH264::Intra_Chr_DC:
			{
				for(i = 0; i < 4; i++)
					for(j = 0; j < 4; j++)
					{
						predCb2D[pOffY + i][pOffX + j]					= predCbDC[0];
						predCb2D[pOffY + i][pOffX + 4 + j]			= predCbDC[1];
						predCb2D[pOffY + 4 + i][pOffX + j]			= predCbDC[2];
						predCb2D[pOffY + 4 + i][pOffX + 4 + j]	= predCbDC[3];

						predCr2D[pOffY + i][pOffX + j]					= predCrDC[0];
						predCr2D[pOffY + i][pOffX + 4 + j]			= predCrDC[1];
						predCr2D[pOffY + 4 + i][pOffX + j]			= predCrDC[2];
						predCr2D[pOffY + 4 + i][pOffX + 4 + j]	= predCrDC[3];
					}//end for i & j...
			}//end DC block...
			break;
		case MacroBlockH264::Intra_Chr_Horiz:
			{
				for(i = 0; i < 8; i++)
					for(j = 0; j < 8; j++)
					{
						predCb2D[pOffY + i][pOffX + j] = refCb2D[rOffY + i][rOffX - 1];
						predCr2D[pOffY + i][pOffX + j] = refCr2D[rOffY + i][rOffX - 1];
					}//end i & j...
			}//end Horiz block...
			break;
		case MacroBlockH264::Intra_Chr_Vert:
			{
				for(i = 0; i < 8; i++)
					for(j = 0; j < 8; j++)
					{
						predCb2D[pOffY + i][pOffX + j] = refCb2D[rOffY - 1][rOffX + j];
						predCr2D[pOffY + i][pOffX + j] = refCr2D[rOffY - 1][rOffX + j];
					}//end for i & j...
			}//end Vert block...
			break;
		case MacroBlockH264::Intra_Chr_Plane:
			{
				for(i = 0; i < 8; i++)
					for(j = 0; j < 8; j++)
					{
						predCb2D[pOffY + i][pOffX + j] = H264V2_CLIP255( (aCb + bCb*(j-3) + cCb*(i-3) + 16) >> 5 );
						predCr2D[pOffY + i][pOffX + j] = H264V2_CLIP255( (aCr + bCr*(j-3) + cCr*(i-3) + 16) >> 5 );
					}//end for i & j...
			}//end Plane block...
			break;
	}//end switch mode...

	return(mode);
}//end GetIntra8x8ChrPredAndMode.

/** Get Intra Chr DC prediction.
Predict from the parameter Chr img and write it to the pred parameter. If 
there are no valid neighbours to make the prediction then use the 
mid chr value (128) as the prediction. The H.264 std does not specify 
this last case for DC pred.
@param pMb				: Macroblock to transform.
@param chr				: Chr img to predict from.
@param pred				: 8x8 overlay to write result to.
@return						: none
*/
void H264v2Codec::GetIntra8x8ChrDCPred(MacroBlockH264* pMb, OverlayMem2Dv2* chr, OverlayMem2Dv2* pred)
{
	int offX, offY, i;
	int predValue[4];
	int dirCnt		= 0;
	int sum[4]		= { 0, 0, 0, 0};

	short** img = chr->Get2DSrcPtr();

	if(pMb->_aboveMb != NULL)
	{
		dirCnt++;
		/// The last row of the macroblock above.
		offX = pMb->_aboveMb->_offChrX;
		offY = pMb->_aboveMb->_offChrY + 7;
		for(i = 0; i < 4; i++)
		{
			sum[0] += img[offY][offX + i];
			sum[1] += img[offY][offX + 4 + i];
		}//end for i...
	}//end if _aboveMb...

	if(pMb->_leftMb != NULL)
	{
		dirCnt++;
		/// The last col of the macroblock to the left.
		offX = pMb->_leftMb->_offChrX + 7;
		offY = pMb->_leftMb->_offChrY;
		for(i = 0; i < 4; i++)
		{
			sum[2] += img[offY + i][offX];
			sum[3] += img[offY + 4 + i][offX];
		}//end for i...
	}//end if _leftMb...

	/// Set the accumulated DC prediction values.
	if(dirCnt == 2)
	{
		predValue[0] = (sum[0] + sum[2] + 4) >> 3;
		predValue[1] = (sum[1] + 2) >> 2;
		predValue[2] = (sum[3] + 2) >> 2;
		predValue[3] = (sum[1] + sum[3] + 4) >> 3;
	}//end if dirCnt...
	else if(dirCnt == 1)
	{
		if(pMb->_leftMb == NULL)
		{
			predValue[0] = (sum[0] + 2) >> 2;
			predValue[1] = (sum[1] + 2) >> 2;
			predValue[2] = (sum[0] + 2) >> 2;
			predValue[3] = (sum[1] + 2) >> 2;
		}//end if !leftMb...
		else
		{
			predValue[0] = (sum[2] + 2) >> 2;
			predValue[1] = (sum[2] + 2) >> 2;
			predValue[2] = (sum[3] + 2) >> 2;
			predValue[3] = (sum[3] + 2) >> 2;
		}//end else...
	}//end else if dirCnt...
	else
	{
		/// Mid Chr value.
		predValue[0] = 128;
		predValue[1] = 128;
		predValue[2] = 128;
		predValue[3] = 128;
	}//end else...

	int xBase = pred->GetOriginX();
	int yBase = pred->GetOriginY();
	pred->SetOverlayDim(4,4);
	for(int y = 0, i = 0; y < 2; y++)
		for(int x = 0; x < 2; x++, i++)
		{
			pred->SetOrigin(xBase + x*4, yBase + y*4);
			pred->Fill(predValue[i]);
		}//end for y & x...

	/// Put the block back into shape.
	pred->SetOverlayDim(8,8);
	pred->SetOrigin(xBase, yBase);

}//end GetIntra8x8ChrDCPred.

/** Get Intra Chr Plane prediction.
Predict from the parameter Chr img and write it to the pred parameter. This
implementation if for YCbCr 4:2:0 chroma_format_idc = 1 only. If there are 
no valid neighbours to make the prediction then use the mid chr value (128) 
as the prediction. The H.264 std does not specify this last case for DC pred.
@param pMb				: Macroblock to predict.
@param chr				: Chr img to predict from.
@param pred				: 8x8 overlay to write result to.
@return						: 1 = pred was made, 0 = no macroblocks available for pred.
*/
int H264v2Codec::GetIntra8x8ChrPlanePred(MacroBlockH264* pMb, OverlayMem2Dv2* chr, OverlayMem2Dv2* pred)
{
	int i,j;

	if( (pMb->_aboveMb != NULL)&&(pMb->_aboveLeftMb != NULL)&&(pMb->_leftMb != NULL) )
	{
		short**	img2D		= chr->Get2DSrcPtr();
		int			iOffX		= chr->GetOriginX();	///< Top left of img2D pMb location.
		int			iOffY		= chr->GetOriginY();
		short**	pred2D	= pred->Get2DSrcPtr();
		int			pOffX		= pred->GetOriginX();
		int			pOffY		= pred->GetOriginY();

		int		H = 0;
		int		V = 0;
		for(i = 0; i < 4; i++)
		{
			H += (i + 1)*((int)img2D[iOffY - 1][iOffX + 4 + i] - (int)img2D[iOffY - 1][iOffX + 2 - i]);
			V += (i + 1)*((int)img2D[iOffY + 4 + i][iOffX - 1] - (int)img2D[iOffY + 2 - i][iOffX - 1]);
		}//end for i...
		int b = (34*H + 32) >> 6;
		int c = (34*V + 32) >> 6;
		int a = ((int)img2D[iOffY + 7][iOffX - 1] + (int)img2D[iOffY - 1][iOffX + 7]) << 4;

    for(i = 0; i < 8; i++)
			for(j = 0; j < 8; j++)
			{
				short x = (short)((a + b*(j-3) + c*(i-3) + 16) >> 5);
				pred2D[pOffY+i][pOffX+j] = (short)(H264V2_CLIP255(x));
			}//end for i & j...

		return(1);	///< Successful prediction.
	}//end if _aboveMb...

	/// Safety precaution not defined in H.264 standard. If the macroblock above, 
	/// above left and left are not available for prediction then this mode should 
	/// never be used.
	pred->Fill(128);
	return(0);	///< Prediction macroblocks do not exist.
}//end GetIntra8x8ChrPlanePred.

/*
-----------------------------------------------------------------------
  Private nested classes.                                
-----------------------------------------------------------------------
*/
/** Encode the image data for Intra IDR pictures.
Process all slices and their macroblocks in order. The reference is always 
written into for this implementation regardless of the state of the writeRef
parameter. It is required for for later macroblock prediction. The macroblock 
objs are prepared for coding onto the bit stream. The inclusion of the allowed 
bits param provides for scope to use bit allocation procedures. Ver. 1 limits 
the encoding to a single slice with no partitioning.
@param allowedBits	: Total remaining bits to target.
@param bitsUsed			: Num of bits used for this encoding (return value).
@param writeRef			: Ignored.
@return							: 1 = success, 0 = error.
*/
int H264v2Codec::IntraImgPlaneEncoderImplStdVer1::Encode(int allowedBits, int* bitsUsed, int writeRef)
{
	int mb;
	int len	= _codec->_mbLength;

	/// Set up the input and ref image mem overlays.
	_codec->_Lum->SetOverlayDim(4,4);
	_codec->_Cb->SetOverlayDim(4,4);
	_codec->_Cr->SetOverlayDim(4,4);
	_codec->_RefLum->SetOverlayDim(4,4);
	_codec->_RefCb->SetOverlayDim(4,4);
	_codec->_RefCr->SetOverlayDim(4,4);
	_codec->_16x16->SetOverlayDim(16, 16);
	_codec->_16x16->SetOrigin(0, 0);
	_codec->_8x8_0->SetOverlayDim(8, 8);
	_codec->_8x8_0->SetOrigin(0, 0);
	_codec->_8x8_1->SetOverlayDim(8, 8);
	_codec->_8x8_1->SetOrigin(0, 0);

	/// All integer transforms are Intra in this method.
	_codec->_pF4x4TLum->SetMode(IForwardTransform::TransformOnly);
	_codec->_pF4x4TLum->SetParameter(IForwardTransform::INTRA_FLAG_ID, 1);
	_codec->_pF4x4TChr->SetMode(IForwardTransform::TransformOnly);
	_codec->_pF4x4TChr->SetParameter(IForwardTransform::INTRA_FLAG_ID, 1);
	/// By default the DC transforms were set in the TransformOnly mode in the Open() method.
	_codec->_pFDC4x4T->SetParameter(IForwardTransform::INTRA_FLAG_ID, 1);
	_codec->_pFDC2x2T->SetParameter(IForwardTransform::INTRA_FLAG_ID, 1);

	/// Whip through each macroblock in the slice and encode. The stream writing of
	/// the macroblock is seperate to allow further decision making later.
	for(mb = 0; mb < len; mb++)
	{
		_codec->_pMb[mb]._mbQP = _codec->_slice._qp;
		_codec->ProcessIntraMbImplStd(&(_codec->_pMb[mb]), 0);
	}//end for mb...

	*bitsUsed = 0;
	return(1);
}//end IntraImgPlaneEncoderImplStdVer1::Encode.

/** Decode of the Intra macroblocks to the reference img.
The macroblock obj encodings must be fully defined before calling
this method.
@return	: 1 = success, 0 = error.
*/
int H264v2Codec::IntraImgPlaneDecoderImplStdVer1::Decode(void)
{
	int mb;
	int len	= _codec->_mbLength;

	/// Set up the input and ref image mem overlays.
	_codec->_RefLum->SetOverlayDim(4,4);
	_codec->_RefCb->SetOverlayDim(4,4);
	_codec->_RefCr->SetOverlayDim(4,4);
	_codec->_16x16->SetOverlayDim(16, 16);
	_codec->_8x8_0->SetOverlayDim(8, 8);
	_codec->_8x8_1->SetOverlayDim(8, 8);

	/// Whip through each macroblock. Decode the extracted encodings. All modes
	/// and parameters have been extracted by the ReadMacroBlockLayer() method.
	for(mb = 0; mb < len; mb++)
	{
		/// Simplify the referencing to the current macroblock.
		MacroBlockH264* pMb = &(_codec->_pMb[mb]);
		int lOffX = pMb->_offLumX;
		int lOffY = pMb->_offLumY;
		int cOffX = pMb->_offChrX;
		int cOffY = pMb->_offChrY;

		/// --------------------- Inverse Transform & Quantisation --------------------------
		if(pMb->_mbPartPredMode == MacroBlockH264::Intra_16x16)
			_codec->InverseTransAndQuantIntra16x16MBlk(pMb, 0);

		/// --------------------- Image Prediction and Storing -------------------------------------
		/// From the prediction mode settings, make the appropriate prediction macroblock and then
		/// add the inverse transformed and quantised values to it. Fill the image (difference) colour 
		/// components from all the non-DC 4x4 blks (i.e. Not blks = -1, 17, 18) of the macroblock blocks.

		/// Store blocks into ref img.
		MacroBlockH264::StoreBlks(pMb, _codec->_RefLum, lOffX, lOffY, _codec->_RefCb, _codec->_RefCr, cOffX, cOffY, 0);

		/// Predict the output from the previously decoded neighbour ref macroblocks.
		/// Lum.
		_codec->_RefLum->SetOverlayDim(16, 16);
		_codec->_RefLum->SetOrigin(lOffX, lOffY); ///< Align the Ref Lum img block with this macroblock.

		switch(pMb->_intra16x16PredMode)
		{
			case MacroBlockH264::Intra_16x16_Vert:
				_codec->GetIntraVertPred(pMb, _codec->_RefLum, _codec->_16x16, 1);
				break;
			case MacroBlockH264::Intra_16x16_Horiz:
				_codec->GetIntraHorizPred(pMb, _codec->_RefLum, _codec->_16x16, 1);
				break;
			case MacroBlockH264::Intra_16x16_DC:
				_codec->GetIntra16x16LumDCPred(pMb, _codec->_RefLum, _codec->_16x16);
				break;
			case MacroBlockH264::Intra_16x16_Plane:
				_codec->GetIntra16x16LumPlanePred(pMb, _codec->_RefLum, _codec->_16x16);
				break;
		}//end switch _intra16x16PredMode...
		
		_codec->_RefCb->SetOverlayDim(8, 8);
		_codec->_RefCr->SetOverlayDim(8, 8);
		_codec->_RefCb->SetOrigin(cOffX, cOffY);
		_codec->_RefCr->SetOrigin(cOffX, cOffY);

		switch(pMb->_intraChrPredMode)
		{
			case MacroBlockH264::Intra_Chr_DC:
				_codec->GetIntra8x8ChrDCPred(pMb, _codec->_RefCb, _codec->_8x8_0);
				_codec->GetIntra8x8ChrDCPred(pMb, _codec->_RefCr, _codec->_8x8_1);
				break;
			case MacroBlockH264::Intra_Chr_Horiz:
				_codec->GetIntraHorizPred(pMb, _codec->_RefCb, _codec->_8x8_0, 0);
				_codec->GetIntraHorizPred(pMb, _codec->_RefCr, _codec->_8x8_1, 0);
				break;
			case MacroBlockH264::Intra_Chr_Vert:
				_codec->GetIntraVertPred(pMb, _codec->_RefCb, _codec->_8x8_0, 0);
				_codec->GetIntraVertPred(pMb, _codec->_RefCr, _codec->_8x8_1, 0);
				break;
			case MacroBlockH264::Intra_Chr_Plane:
				_codec->GetIntra8x8ChrPlanePred(pMb, _codec->_RefCb, _codec->_8x8_0);
				_codec->GetIntra8x8ChrPlanePred(pMb, _codec->_RefCr, _codec->_8x8_1);
				break;
		}//end switch _intraChrPredMode...

		/// --------------------- Add the prediction -----------------------------------------------
		/// Lum.
		_codec->_RefLum->SetOverlayDim(16, 16);
		_codec->_RefLum->SetOrigin(lOffX, lOffY);           ///< Align the Ref Lum img block with this macroblock.
		_codec->_RefLum->AddWithClip255(*(_codec->_16x16));	///< Add pred to ref Lum and leave result in ref img.
		/// Cb.
		_codec->_RefCb->SetOverlayDim(8, 8);
		_codec->_RefCb->SetOrigin(cOffX, cOffY);
		_codec->_RefCb->AddWithClip255(*(_codec->_8x8_0));
		/// Cr.
		_codec->_RefCr->SetOverlayDim(8, 8);
		_codec->_RefCr->SetOrigin(cOffX, cOffY);
		_codec->_RefCr->AddWithClip255(*(_codec->_8x8_1));

	}//end for mb...

	return(1);
}//end IntraImgPlaneDecoderImplStdVer1::Decode.

/** Encode the image data for Intra pictures with adaptive PQ (minmax).
The encoder seeks to find the optimal quant value for each macroblock using a minmax algorithm with
the constraint of a max number of bits specified in the method parameter list by allowedBits.
@param allowedBits	: Total remaining bits to target.
@param bitsUsed			: Num of bits used for this encoding.
@param writeRef			: 1/0 = do/do not update ref.
@return							: 1 = success, 0 = error.
*/
int H264v2Codec::IntraImgPlaneEncoderImplMinMax::Encode(int allowedBits, int* bitsUsed, int writeRef)
{
	int len	= _codec->_mbLength;
	int mb;
	/// Mark the macroblock from where min bit rate macroblocks must be encoded if the slice is to be truncated.
	int minMBIndex = len;
	/// Set a bound on the max quality by choosing a min qp value that the MinMax search is limited to.
	int qEnd = 4;

	/// Set up the input and ref image mem overlays.
	_codec->_Lum->SetOverlayDim(4,4);
	_codec->_Cb->SetOverlayDim(4,4);
	_codec->_Cr->SetOverlayDim(4,4);
	_codec->_RefLum->SetOverlayDim(4,4);
	_codec->_RefCb->SetOverlayDim(4,4);
	_codec->_RefCr->SetOverlayDim(4,4);
	_codec->_16x16->SetOverlayDim(16, 16);
	_codec->_16x16->SetOrigin(0, 0);
	_codec->_8x8_0->SetOverlayDim(8, 8);
	_codec->_8x8_0->SetOrigin(0, 0);
	_codec->_8x8_1->SetOverlayDim(8, 8);
	_codec->_8x8_1->SetOrigin(0, 0);

	/// All integer transforms are Intra in this method.
	_codec->_pF4x4TLum->SetMode(IForwardTransform::TransformOnly);
	_codec->_pF4x4TLum->SetParameter(IForwardTransform::INTRA_FLAG_ID, 1);
	_codec->_pF4x4TChr->SetMode(IForwardTransform::TransformOnly);
	_codec->_pF4x4TChr->SetParameter(IForwardTransform::INTRA_FLAG_ID, 1);
	/// By default the DC transforms were set in the TransformOnly mode in the Open() method.
	_codec->_pFDC4x4T->SetParameter(IForwardTransform::INTRA_FLAG_ID, 1);
	_codec->_pFDC2x2T->SetParameter(IForwardTransform::INTRA_FLAG_ID, 1);

	/// Initialisation step: 
	///		lower(D,R) : QP = H264V2_MAX_QP.
	///		upper(D,R) : QP = 1.
	/// Note that the D values represent the worst (max) distortion of only one macroblock in the entire
	/// frame but the R values represent the accumulated (total) rate of all macroblocks.
	int	Dl					= 0;
	int	Du					= 0;
	int	Rl					= 0;
	int	Ru					= 0;
	int	R						= 0;
	int	D						= 0;
	int	Dmax				= 0;
	int mbDmax			= 0;
	int	invalidated = 0;
	int	iterations	= 0;

	/// Test QP = H264V2_MAX_QP for all macroblocks and determine if this min possible rate is, at the very 
	/// least, less than the target rate. Damage control is required if not, where minimal encodings
	/// are attempted for the macroblocks. Otherwise, proceed with the initialisation.
	for(mb = 0; mb < len; mb++)
	{
		MacroBlockH264* pMb = &(_codec->_pMb[mb]);

		/// The 1st macroblock has a delta offset from _slice._qp to get it to _mbQP = H264V2_MAX_QP.
		pMb->_mbQP    = H264V2_MAX_QP;
		/// Include all macroblocks at the start.
		pMb->_include = 1;	
		_pQl[mb]			= H264V2_MAX_QP;
		_pQ[mb]				= H264V2_MAX_QP;	///< Active quantiser vector is always the lower (max distortion) side.

		/// Accumulate the rate and find the largest (max) distortion of all macroblocks.
		Rl += _codec->ProcessIntraMbImplStd(pMb, 1);
		if(pMb->_distortion[H264V2_MAX_QP] > Dl)
		{
			Dl			= pMb->_distortion[H264V2_MAX_QP];
			mbDmax	= mb;	///< Mark the macroblock with the peak distortion.
		}//end if _distortion...

	}//end for mb...

	/// Test that there is room to adapt else do damage control.
	int oldBitCount = Rl;
	if(Rl <= allowedBits)
	{
		/// Create an epsilon at 0.4% of the available bits with a min limit of 16 bits.
		/// Adjust the available bits to be epsilon below allowedBits.
		int closeEnough = allowedBits/250;	///< 0.4%
		if(closeEnough < 16)
			closeEnough = 16;
		int closeEnoughDist = 8;	///< Sqr error per macroblock.

#ifdef H264V2_FAST_MINMAX
    /// Model based faster algorithm to improve on the bi-section method.

		/// Modify the target rate to a close offset on the low rate side.
		int bitTarget = allowedBits - closeEnough;

		/// The worst case at QP = H264V2_MAX_QP uses less bits than allowed so there is some
		/// room to manouver and we can proceed. The upper point is initially not calculated
		/// directly but will hold a valid value during the algorithm convergence after the
		/// first iteration.
		/// The upper point is initiallised to some wild guess.
		Du = 1;									///< Min (im)possible.
		Ru = allowedBits << 8;	///< Some very large number.

		/// Initialisation is complete and the adapting bi-section/power model algorithm starts 
		/// here. The stopping criteria are 1) a rate less than, but close to, the allowed bits
		/// or 2) small change in distortion when reducing the interval.
		int goingDown = 1;
		int done = 0;
		while(!done)
		{
			int prevDmax = Dmax;

			/// Make a prediction assuming a power law model.
			int model = 0;
			Dmax = _codec->FitDistPowerModel(Rl, Dl, Ru, Du, bitTarget);
			/// Only use this prediction if it is bounded by (Rl,Dl) and (Ru,Du).
			if( (Dmax < Du)||(Dmax > Dl) )	///< Out of bound.
			{
				Dmax = _codec->FitDistLinearModel(Rl, Dl, Ru, Du, bitTarget);
				model = 1;
			}//end if linear...

			/// Encourage the descent direction by adding a decreasing factor to the
			/// predicted Dmax such that convergence to Dmax is from the (Rl,Dl) side.
			Dmax += abs(Dl - Dmax)/4;

			if( (Dmax < Du)||(Dmax > Dl)||(Dmax == prevDmax) )	///< Still out of bound.
			{
				Dmax = ((Du + Dl) + 1) >> 1;	///< Set the midpoint max distortion.
				model = 2;
			}//end if bisection...

			/// At each macroblock reduce the quant value until the distortion is lower
			/// than Dmax. pQ[] must always hold the lower rate (smaller valued) quant vector 
			/// as the previous best choice.
			R = 0;
			int firstMbChange = len;
			if(invalidated)	///< Frame encoded coeffs must be invalidated if _pQ[] does not represent the actual QPs used.
				firstMbChange = 0;
			for(mb = 0; mb < len; mb++)
			{
				MacroBlockH264* pMb = &(_codec->_pMb[mb]);

				/// Record the found QP where the macroblock dist is just below Dmax and accumulate the rate for this macroblock.
				_pQ[mb] = _codec->GetMbQPBelowDmaxVer2(*pMb, _pQ[mb], Dmax, &firstMbChange, qEnd, TRUE);
				R += pMb->_rate[_pQ[mb]];

				/// An accurate early exit strategy is not possible because the model prediction require two valid (Dmax,R) points 
				/// for the whole frame. 

			}//end for mb...

			/// Test the stopping criteria.
			int rBndDiff = abs(Ru - Rl);			///< Optimal solution is rate bounded in a small enough range.
			int dDiff = abs(prevDmax - Dmax);	///< Has not changed by much from the previous iteration (converged).
			int rDiff = abs(bitTarget - R);		///< Close enough to the allowed bits.
			if( (rBndDiff < (4*closeEnough)) ||
					( (rDiff < closeEnough)||(/*(R <= allowedBits)&&*/(dDiff < closeEnoughDist))) ||
          (iterations > 20) )
		  {
				/// The new point is the preferable point to choose but if the rate is not below allowedBits
				/// then the (Rl,Dl) point is selected. Use the invalidated signal to indicate if the mbs 
        /// must be re-processed.
				if(R > allowedBits)
				{
					/// Load the lower vector as it is always below the target rate.
					memcpy((void *)_pQ, (const void *)_pQl, len * sizeof(int));
					R = Rl;
          invalidated = 1;
				}//end if R...
        else
          invalidated = 0;

				/// Optimal solution is found.
				done = 1;
			}//end if rDiff...
			else
			{
				if(allowedBits > R)	///< Inner predicted point is now lower point.
				{
					Dl	= Dmax;
					Rl	= R;
					/// pQ[] is the lower vector. 
					memcpy((void *)_pQl, (const void *)_pQ, len * sizeof(int));
					invalidated = 0;
				}//end if allowedBits...
				else							///< Inner predicted point is now upper point.
				{
					Du	= Dmax;
					Ru	= R;
					/// pQ[] must be reset to lower vector to maintain a descent convergence.
					memcpy((void *)_pQ, (const void *)_pQl, len * sizeof(int));
					invalidated = 1;	///< pQ[] now does not represented the state of the last encoded frame.
				}//end else...

			}//end else...

			iterations++;
		}//end while !done...

#else	///< !H264V2_FAST_MINMAX

		/// The worst case at QP = H264V2_MAX_QP uses less bits than allowed so there is some
		/// room to manouver and we can proceed. The upper point is initially not calculated
		/// directly but after the 1st iteration it will hold a valid value.
		Du = 0;	///< Min (im)possible.

		/// Initialisation is complete and the bi-section algorithm starts here. The 
		/// stopping criteria are 1) a rate less than, but close to, the allowed bits
		/// or 2) small change in distortion when halving.
		int goingDown	= 1;
		int done			= 0;
		while(!done)
		{
			int prevDmax = Dmax;
			Dmax = ((Du + Dl) + 1) >> 1;	///< Set the midpoint max distortion.

			/// At each macroblock reduce the QP value until the distortion is lower
			/// than Dmax. Set mb QP to first macroblock quant. pQ[] must always hold
			/// the lower quant vector as the previous best choice.
			R = 0;
			D = 0;
			int firstMbChange = len;
			if(invalidated)	///< Frame encoded coeffs must be invalidated if _pQ[] does not represent the actual QPs used.
				firstMbChange = 0;
			for(mb = 0; mb < len; mb++)
			{
				MacroBlockH264* pMb = &(_codec->_pMb[mb]);

				/// Record the found QP where the macroblock dist is just below Dmax and accumulate the rate for this macroblock.
				_pQ[mb] = _codec->GetMbQPBelowDmax(*pMb, _pQ[mb], Dmax, goingDown, &firstMbChange, qEnd, TRUE);
				R += pMb->_rate[_pQ[mb]];
				if(pMb->_distortion[_pQ[mb]] > D)
					D = pMb->_distortion[_pQ[mb]];

				/// Early exit test. The pQ[] is incomplete and must not be used.
				if(R > allowedBits)
				{
					/// Rate must be effectively infinite to ensure it does not meet the close enough
					/// stopping criterion below. We multiply by 2 to make it arbitrarily large enough.
					R = allowedBits << 1;	
					break;
				}//end if R...

			}//end for mb...

			/// Test the stopping criteria.
			invalidated = 0;	///< Default case.
			int dDiff = abs(prevDmax - Dmax);
			int rDiff = allowedBits - R;
			if( ((rDiff >= 0)&&(rDiff < closeEnough))||((rDiff >= 0)&&((dDiff < closeEnoughDist))) )
			{
        invalidated = 1;
				/// Optimal solution is found.
				done = 1;
			}//end if rDiff...
			else
			{
				if(allowedBits > R)	///< Midpoint is now lower point.
				{
					Dl = Dmax;
					/// pQ[] is already lower vector.
					memcpy((void *)_pQl, (const void *)_pQ, len * sizeof(int));
				}//end if allowedBits...
				else								///< Midpoint is now upper point.
				{
					Du = Dmax;
					/// pQ[] must be reset to lower vector. If an early exit occurred then
					/// it is loaded with a valid vector.
					memcpy((void *)_pQ, (const void *)_pQl, len * sizeof(int));
					invalidated = 1;	///< pQ[] now does not represented the state of the last encoded frame.
				}//end else...
			}//end else...

			//////////////////////////////////////////////////////////
			iterations++;
			H264V2_IT++;
		}//end while !done...

#endif	///< End H264V2_FAST_MINMAX.

		/// Set the macroblock QP values to their found states.
    if(invalidated)
    {
		  for(mb = 0; mb < len; mb++)
		  {
			  _codec->_pMb[mb]._mbQP = _pQ[mb];
		  }//end for mb...
    }//end if invalidated...

	}//end if Rl...
	else /// If max QP was used and still exceeds allowed bits then truncate with QP = H264V2_MAX_QP as damage control.
	{
    /// The absolute min bits is deterministic because all mbs after the 1st one (delta QP from slice 
    /// QP is different for 1st mb) use the same number of bits for every mb. If this is not
		/// achievable then we cannot proceed anyway, so exit with a truncated bits error message.
		int bitsFor1stMinMacroblock = _codec->ProcessIntraMbImplStdMin(&(_codec->_pMb[0]));
    int bitsPerMinMacroblock    = _codec->ProcessIntraMbImplStdMin(&(_codec->_pMb[1]));

		int minRequiredBits = bitsFor1stMinMacroblock + (bitsPerMinMacroblock * (len-1));
		if(minRequiredBits <= allowedBits)
		{
      /// Rl is valid for QP = 51.
      int avgBitsPerQP51Macroblocks = (Rl + (len/2))/len;

			R = 0;
      int selectedQP = H264V2_MAX_QP; ///< Start with max QP.
			for(mb = 0; mb < len; mb++)
			{
				MacroBlockH264* pMb = &(_codec->_pMb[mb]);

				///< Select QP based on avg remaining bits per mb with preference to max QP.
				selectedQP = H264V2_MAX_QP;
        if(mb != 0) ///< Always use max QP for the 1st mb.
        {
          int avgBitsPerMacroblockRemaining = ((allowedBits - R) + ((len - mb)/2))/(len - mb);
          if(avgBitsPerMacroblockRemaining < avgBitsPerQP51Macroblocks)
            selectedQP = 52;
        }//end if mb...

				pMb->_mbQP = selectedQP;
				int mbBits = _codec->ProcessIntraMbImplStd(pMb, 1);

				/// There has to be enough bits left over to encode the remaining macroblocks
				/// as min null Intra macroblocks.
				int lclAllowedBits = (allowedBits - R) - ((len - 1 - mb) * bitsPerMinMacroblock);
				if(mbBits > lclAllowedBits)
				{
					/// Mark the macroblock whereafter only min null intra macroblocks will fit.
					minMBIndex = mb;
          R += (len - mb) * bitsPerMinMacroblock;
					break;
				}//end if mbBits...

				R += mbBits;
			}//end for mb...

      /// Re-instate the encoded value of the mbs that were processed.
      invalidated = 1;
      for(mb = 0; mb < minMBIndex; mb++)
        _codec->_pMb[mb]._mbQP = _codec->_pMb[mb]._mbEncQP;
		}//end if minRequiredBits...
    else
    {
      /// Even with forced DC prediction and zero'ed coeffs, the pic cannot be encoded.
			_codec->_errorStr = "[H264Codec::IntraImgPlaneEncoderImplMinMax::Encode] Bits exceeded in trucation";
	    *bitsUsed = 0;
	    return(0);
    }//end else...

	}//end else...

	/// Apply the selected quant with the full feedback loop to all macroblocks.
  if(invalidated)
  {
//  	int bitCount = 0;
	  for(mb = 0; mb < len; mb++)
	  {
		  MacroBlockH264* pMb = &(_codec->_pMb[mb]);

		  /// Encode the macroblocks until min null macroblocks.
		  if(mb < minMBIndex)
			  _codec->ProcessIntraMbImplStd(pMb, 0);
//  			bitCount += _codec->ProcessIntraMbImplStd(pMb, 1);
		  else
			  _codec->ProcessIntraMbImplStdMin(pMb);
//  			bitCount += _codec->ProcessIntraMbImplStdMin(pMb);

	  }//end for mb...

  }//end if invalidated...

  *bitsUsed = 0;
	return(1);
}//end IntraImgPlaneEncoderImplMinMax::Encode.

/** Get distortion from an r vs d power model.
@return	:	Distortion predicted
*/
int H264v2Codec::FitDistPowerModel(int rl, int dl, int ru, int du, int r)
{
	int d = dl;
	if( (dl != du)&&(rl != ru) )
	{
		/// Curve fit the points (rl,dl) and (ru,du) to a power function.
		double beta = -(log((double)rl/(double)ru))/(log((double)dl/(double)du));
		double alpha = (double)rl/(pow((double)dl, -beta));
		/// Use the power function to predict where r will be. r = alpha.d^(-beta)
		d = (int)exp(-log((double)r/alpha)/beta);
	}//end if dl...

	return(d);
}//end FitDistPowerModel.

/** Get distortion from an r vs d linear model.
@return	:	Distortion predicted
*/
int H264v2Codec::FitDistLinearModel(int rl, int dl, int ru, int du, int r)
{
	int d = dl;
	if(rl != ru)
	{
		/// Curve fit the points (rl,dl) and (ru,du) to a linear function.
		d = (int)( (double)((r-rl)*(dl-du))/(double)(rl-ru) ) + dl;
	}//end if dl...

	return(d);
}//end FitDistLinearModel.

/** Get the next quant QP value that has a distortion less than Dmax.
@param mb						: Macroblock to operate on.
@param atQP					: Current QP for this macroblk.
@param Dmax					: Max value to get below.
@param decQP				: Decrement=1/increment=0 QP from atQ for this macroblock.
@param changeMb			: Macroblock index from where the last change was made.
@param lowestQP			: Lowest allowed quant.
@param intra				: Is macroblock Intra.
@return							: QP value that has dist < Dmax for this macroblock.
*/
int H264v2Codec::GetMbQPBelowDmax(MacroBlockH264 &mb, int atQP, int Dmax, int decQP, int* changeMb, int lowestQP, bool intra)
{
	/// 1st iteration required to ensure sync with macroblock dependencies if
	/// it was invalidated or changed since last encoding.
	int i = atQP;
	int	di = mb._distortion[i];
	int mbIndex = mb._mbIndex;
	int lclChangeMb = *changeMb;

	/// Macroblocks are dependent on (predicted from) previous macroblocks in the slice. If
	/// this mb is located after the previously changed mb then its dependencies have been
	/// altered and it must be re-encoded.
	if(mbIndex >= lclChangeMb)
	{
		mb._mbQP = i;
		if(!intra)
			;
		else
			ProcessIntraMbImplStd(&mb, 1);

		di = mb._distortion[i];
	}//end if mbIndex...

	/// If an increasing QP is requested but di > Dmax then a decrease
	/// in mb QP is actually what is required.
	if( !decQP && (di > Dmax) )
		decQP = 1;

	if(decQP)	///< Decreasing QP until di < Dmax.
	{
		/// Check if it is necessary to look further.
		if( (i > lowestQP)&&(di > Dmax) )
		{
			if(mbIndex < lclChangeMb)
				lclChangeMb = mbIndex;

			/// Iterate QP down from i. If i drops to lowestQ without di <= Dmax
			/// then leave as it is the best that can be done.
			while( (i > lowestQP)&&(di > Dmax) )
			{
				i -= MbStepSize[i];
				if(i < lowestQP) 
					i = lowestQP;

				mb._mbQP = i;
				if(!intra)
					;
				else
					ProcessIntraMbImplStd(&mb, 1);
				di = mb._distortion[i];
			}//end while i...

		}//end if i...
	}//end if decQP...
	else		///< Increasing QP until di > Dmax and then decrease back to below Dmax.
	{
		/// Check if it is necessary to look further.
		if(i < H264V2_MAX_QP)
		{
			if(mbIndex < lclChangeMb)
				lclChangeMb = mbIndex;

			/// Iterate QP up from i until di > Dmax or i = H264V2_MAX_QP (51).
//			int previ = i;
			while( (i < H264V2_MAX_QP)&&(di <= Dmax) )
			{
//				previ = i;
				i += MbStepSize[i];
				if(i > H264V2_MAX_QP) 
					i = H264V2_MAX_QP;

				mb._mbQP = i;
				if(!intra)
					;
				else
					ProcessIntraMbImplStd(&mb, 1);
				di = mb._distortion[i];
			}//end while i...

			/// Compensate only if a solution was found otherwise QP = H264V2_MAX_QP is the
			/// best that can be done. It is expected to step back only once but if the step
			/// up process above was not implemented because di > Dmax then iterating down
			/// several times may be required.
			if(di > Dmax)
			{
				/// Iterate down from i. If i drops to lowestQ without di <= Dmax
				/// then leave as it is the best that can be done.
				while( (i > lowestQP)&&(di > Dmax) )
				{
					i -= MbStepSize[i];
					if(i < lowestQP) 
						i = lowestQP;

					mb._mbQP = i;
					if(!intra)
						;
					else
						ProcessIntraMbImplStd(&mb, 1);
					di = mb._distortion[i];
				}//end while i...

			}//end if di...

		}//end if i...

	}//end else...

	/// Return QP value where dist just less than Dmax for this macroblock.
	*changeMb = lclChangeMb;
	return(i);
}//end GetMbQPBelowDmax.

/** Get the next quant value that has a distortion less than Dmax with decrement only.
@param mb						: Macroblock to operate on.
@param atQ					: Current q for this macroblk.
@param Dmax					: Max value to get below.
@param changeMb			: Macroblock index from where the last change was made.
@param lowestQ			: Lowest allowed quant.
@param intra				: Is macroblock Intra.
@return							: QP value that has dist < Dmax for this macroblock.
*/
int H264v2Codec::GetMbQPBelowDmaxVer2(MacroBlockH264 &mb, int atQ, int Dmax, int* changeMb, int lowestQ, bool intra)
{
	int i						= atQ;
	int	di					= mb._distortion[i];
	int mbIndex			= mb._mbIndex;
	int lclChangeMb = *changeMb;

	/// Macroblocks are dependent on (predicted from) previous macroblocks in the slice. If
	/// this mb is located after the previously changed mb then its dependencies have been
	/// altered and it must be re-encoded.
	if(mbIndex >= lclChangeMb)
	{
		mb._mbQP = i;
		if(!intra)
    {
      /// For P mbs, if the previous encoding QP was the same then only the rate of the mb
      /// will change. This rate change is due to a change in the number of coeffs of neighbouring
      /// mbs and their changed QP values.
       if(mb._mbEncQP == i)
      {
        int rate = 0;
        mb._mb_qp_delta = GetDeltaQP(&mb);        ///< Process new delta QP.
        if(!mb._skip)        
          rate = MacroBlockLayerBitCounter(&mb);  ///< Process mb for new neighbour coeffs values.
        mb._rate[mb._mbEncQP] = rate;             ///< Update this new rate.
      }//end if _mbEncQP...
      else
      {
			  ProcessInterMbImplStd(&mb, 0, 1);
       }//end else...

    }//end if !intra...
		else
    {
			ProcessIntraMbImplStd(&mb, 1);
    }//end else...

		di = mb._distortion[i];
	}//end if mbIndex...

  /// The delta QP for a mb is constrained to {-26...25}. Therefore if
  /// the current QP value results in a dQP > 25 or < -26 then it must 
  /// be reprocessed before continuing regardless of the Dmax value.
  int prevQP = GetPrevMbQP(&mb);
  int minQP = prevQP - 26;
  if(minQP < lowestQ) minQP = lowestQ;
  int maxQP = prevQP + 25;
  if(maxQP > H264V2_MAX_QP) maxQP = H264V2_MAX_QP;
  if( (i < minQP)||(i > maxQP) )
  {
    if(i < minQP) i = minQP;
    else i = maxQP;
		mb._mbQP = i;
    if(!intra)
			ProcessInterMbImplStd(&mb, 0, 1);
    else
			ProcessIntraMbImplStd(&mb, 1);

		di = mb._distortion[i];
  }//end if i...

	/// Check if it is necessary to look further.
	if( (i > minQP)&&(di > Dmax) )
	{
		if(mbIndex < lclChangeMb)
			lclChangeMb = mbIndex;

		/// Iterate QP down from i. If i drops to minQP without di <= Dmax
		/// then leave, as it is the best that can be done.
		while( (i > minQP)&&(di > Dmax) )
		{
		  i -= MbStepSize[i];
			if(i < minQP) i = minQP;

			mb._mbQP = i;
      if(!intra)
			  ProcessInterMbImplStd(&mb, 0, 1);
      else
			  ProcessIntraMbImplStd(&mb, 1);

			di = mb._distortion[i];

		}//end while i...
	}//end if i...

	/// Return QP value where dist just less than Dmax for this macroblock.
	*changeMb = lclChangeMb;
	return(i);
}//end GetMacroblkQuantBelowDmaxVer2.

/** Get the next quant QP value that has a distortion less than Dmax with approximations.
@param mb						: Macroblock to operate on.
@param atQP					: Current QP for this macroblk.
@param Dmax					: Max value to get below.
@param epsilon			: The Dmax approximation only has to be within epsilon of Dmax.
@param decQP				: Decrement=1/increment=0 QP from atQ for this macroblock.
@param changeMb			: Macroblock index from where the last change was made.
@param lowestQP			: Lowest allowed quant.
@param intra				: Is macroblock Intra.
@return							: QP value that has dist < Dmax for this macroblock.
*/
static int H264V2_IT_NOCHANGE = 0;
int H264v2Codec::GetMbQPBelowDmaxApprox(MacroBlockH264 &mb, int atQP, int Dmax, int epsilon, int decQP, int* changeMb, int lowestQP, bool intra)
{
	int i						= atQP;
	int	di					= mb._distortion[i];
	int mbIndex			= mb._mbIndex;
	int lclChangeMb = *changeMb;

	/// Macroblocks are dependent on (predicted from) previous macroblocks in the slice. If
	/// this mb is located after the previously changed mb then its dependencies have been
	/// altered and it must be re-encoded. However, as it will be changed anyway, either up
	/// or down, a short cut approximation is to assume that the previous encoding at the
	/// current QP is approximately correct and the descent/ascent proceeds with these values.
	/// But the mb must be processed at least once before exiting to be valid.
	int atLeastOnce = 0;

	/// If an increasing QP is requested but di > Dmax then a decrease
	/// in mb QP is actually what is required.
	if( !decQP && (di > Dmax) )
		decQP = 1;

	if(decQP)	///< Decreasing QP until di < Dmax.
	{
		/// Check if it is necessary to look further.
		if( (i > lowestQP)&&(di > Dmax) )
		{
			if(mbIndex < lclChangeMb)
				lclChangeMb = mbIndex;

			atLeastOnce = 1;	///< Signal that the mb is encoded.

			/// Iterate QP down from i. If i drops to lowestQ without di <= Dmax
			/// then leave as it is the best that can be done.
			while( (i > lowestQP)&&(di > Dmax) )
			{
				i -= MbStepSize[i];
				if(i < lowestQP) 
					i = lowestQP;

				mb._mbQP = i;
				if(!intra)
					;
				else
					ProcessIntraMbImplStd(&mb, 1);
				di = mb._distortion[i];
			}//end while i...

		}//end if i...
	}//end if decQP...
	else		///< Increasing QP until di > Dmax and then decrease back to below Dmax.
	{
		/// Check if it is necessary to look further.
		if( (i < H264V2_MAX_QP)&&((Dmax - di) > epsilon) )
		{
			if(mbIndex < lclChangeMb)
				lclChangeMb = mbIndex;

			atLeastOnce = 1;	///< Signal that the mb is encoded.

			/// Iterate QP up from i until di > Dmax or i = H264V2_MAX_QP (51).
			while( (i < H264V2_MAX_QP)&&(di <= Dmax) )
			{
				i += MbStepSize[i];
				if(i > H264V2_MAX_QP) 
					i = H264V2_MAX_QP;

				mb._mbQP = i;
				if(!intra)
					;
				else
					ProcessIntraMbImplStd(&mb, 1);
				di = mb._distortion[i];
			}//end while i...

			/// Compensate only if a solution was found otherwise QP = H264V2_MAX_QP is the
			/// best that can be done. It is expected to step back only once but if the step
			/// up process above was not implemented because di > Dmax then iterating down
			/// several times may be required.
			if(di > Dmax)
			{
				/// Iterate down from i. If i drops to lowestQ without di <= Dmax
				/// then leave as it is the best that can be done.
				while( (i > lowestQP)&&(di > Dmax) )
				{
					i -= MbStepSize[i];
					if(i < lowestQP) 
						i = lowestQP;

					mb._mbQP = i;
					if(!intra)
						;
					else
						ProcessIntraMbImplStd(&mb, 1);
					di = mb._distortion[i];
				}//end while i...

			}//end if di...

		}//end if i...

	}//end else...

	/// Re-sync the mb if it was not already processed.
	if((mbIndex >= lclChangeMb)&&(!atLeastOnce))
	{
		mb._mbQP = i;
		if(!intra)
			;
		else
			ProcessIntraMbImplStd(&mb, 1);
	}//end if mbIndex...

	/// Return QP value where dist just less than Dmax for this macroblock.
	*changeMb = lclChangeMb;
	return(i);
}//end GetMbQPBelowDmaxApprox.

/** The forward and inverse loop of a Std Intra macroblock.
Code factoring usage. Includes reading the input image, IT, quantisation,
vlc encoding and determining coding patterns, type, etc. Return the 
bit cost.
@param pMb		: Macroblock to operate on.
@param withDR	: With distortion-rate calculations.
@return				: Bit cost.
*/
int H264v2Codec::ProcessIntraMbImplStd(MacroBlockH264* pMb, int withDR)
{
	/// NB: _mbQP must be correctly defined before this method is called.
  pMb->_mbEncQP = pMb->_mbQP; ///< This method may alter the mb QP therefore keep an orignal copy.

  /// The mb QP is only defined for {0..51} but an extra value of QP = 52 is used here for min bit encodings.
  /// QP = 52: Full pred, DC coeffs coded, AC coeffs zero'ed, _mbQP set to 51.
  int acCoeffsCoded = 1;
  if(pMb->_mbEncQP > 51)
  {
    acCoeffsCoded = 0;
    pMb->_mbQP = H264V2_MAX_QP;  ///< Reset to max QP for all cases.
  }//end if _mbEncQP...

	int distortion = 0;

	int lOffX = pMb->_offLumX;
	int lOffY = pMb->_offLumY;
	int cOffX = pMb->_offChrX;
	int cOffY = pMb->_offChrY;

	/// Base settings for all intra macroblocks.
	pMb->_skip			= 0;
	pMb->_intraFlag = 1;

	///------------------- Image prediction and loading ----------------------------------------
	/// Currently only Intra 16x16 mode implemented. 
	pMb->_mbPartPredMode= MacroBlockH264::Intra_16x16;

	/// Predict the input from the previously decoded neighbour ref macroblocks then subtract the
	/// prediction from the input.

	/// Lum...
	_RefLum->SetOverlayDim(16, 16);
	_RefLum->SetOrigin(lOffX, lOffY); ///< Align the Ref Lum img block with this macroblock.
	_Lum->SetOverlayDim(16, 16);
	_Lum->SetOrigin(lOffX, lOffY);		///< Align the Lum img block with this macroblock.
	/// Select the best mode and get the prediction. Pred stored in _16x16.
  pMb->_intra16x16PredMode = GetIntra16x16LumPredAndMode(pMb, _Lum, _RefLum, _16x16);

  _Lum->Read(*(_RefLum));						///< Read from input Lum into ref Lum.
  _RefLum->Sub16x16(*(_16x16));			///< Subtract pred from ref Lum and leave result in ref img.

	/// ... and Chr components.
	_RefCb->SetOverlayDim(8, 8);
	_RefCr->SetOverlayDim(8, 8);
	_RefCb->SetOrigin(cOffX, cOffY);
	_RefCr->SetOrigin(cOffX, cOffY);
	_Cb->SetOverlayDim(8, 8);
	_Cb->SetOrigin(cOffX, cOffY);
	_Cr->SetOverlayDim(8, 8);
	_Cr->SetOrigin(cOffX, cOffY);
  pMb->_intraChrPredMode = GetIntra8x8ChrPredAndMode(pMb,	_Cb, _Cr,	_RefCb,	_RefCr, _8x8_0, _8x8_1);

	_Cb->Read(*(_RefCb));
	_RefCb->Sub8x8(*(_8x8_0));
	_Cr->Read(*(_RefCr));
	_RefCr->Sub8x8(*(_8x8_1));

	/// Fill all the non-DC 4x4 blks (Not blks = -1, 17, 18) of the macroblock blocks with 
	/// the differnce Lum and Chr after prediction.
	MacroBlockH264::LoadBlks(pMb, _RefLum, lOffX, lOffY, _RefCb, _RefCr, cOffX, cOffY);

	/// ------------------ Transform & Quantisation --------------------------------------------
	if(pMb->_mbPartPredMode == MacroBlockH264::Intra_16x16)
		TransAndQuantIntra16x16MBlk(pMb);
  if(!acCoeffsCoded)
  {
    /// Zero AC coeff 4x4 blks in the mb and clear the num of coeffs member.
	  for(int blk = MBH264_LUM_0_0; blk <= MBH264_LUM_3_3; blk++)
	  {
		  BlockH264* pBlk = pMb->_blkParam[blk].pBlk;
      (pBlk->GetBlkOverlay())->Clear();
      pBlk->SetNumCoeffs(0);
	  }//end for blk...
  }//end if !acCoeffsCoded...

	/// ------------------ Calc distortion from the feedback loop --------------------------------
	/// Implement the feedback loop into the ref image. The block coeffs are still required for 
	/// the context-aware vlc coding and therefore the temporary working blocks are used for this
	/// feedback loop.
	/// --------------------- Inverse Transform & Quantisation -------------------------------
	if(pMb->_mbPartPredMode == MacroBlockH264::Intra_16x16)
		InverseTransAndQuantIntra16x16MBlk(pMb, 1);

	/// --------------------- Image Storing into Ref -----------------------------------------
	/// Fill the ref (difference) lum and chr from all the non-DC 4x4 
	/// blks (i.e. Not blks = -1, 17, 18) of the macroblock temp blocks. 
	MacroBlockH264::StoreBlks(pMb, _RefLum, lOffX, lOffY, _RefCb, _RefCr, cOffX, cOffY, 1);

	/// --------------------- Add the prediction ---------------------------------------------
	/// Lum.
	_RefLum->SetOverlayDim(16, 16);
	_RefLum->SetOrigin(lOffX, lOffY);			    ///< Align the Ref Lum img block with this macroblock.
	_RefLum->Add16x16WithClip255(*(_16x16));	///< Add pred to ref Lum and leave result in ref img.
	/// Cb.
	_RefCb->SetOverlayDim(8, 8);
	_RefCb->SetOrigin(cOffX, cOffY);
	_RefCb->Add8x8WithClip255(*(_8x8_0));
	/// Cr.
	_RefCr->SetOverlayDim(8, 8);
	_RefCr->SetOrigin(cOffX, cOffY);
	_RefCr->Add8x8WithClip255(*(_8x8_1));

	if(withDR)
	{
		/// --------------------- Calc distortion with Input -------------------------------------
		distortion += _RefLum->Tsd16x16(*_Lum);	///< Total square distortion between Ref and input Lum.
		distortion += _RefCb->Tsd8x8(*_Cb);
		distortion += _RefCr->Tsd8x8(*_Cr);
	}//end if withDR...

	/// ------------------ Set patterns and type -----------------------------------------------
	/// Determine the coded Lum and Chr patterns. The _codedBlkPatternLum, _codedBlkPatternChr 
	/// and _coded_blk_pattern members are set.
	MacroBlockH264::SetCodedBlockPattern(pMb);

	/// Determine the delta quantisation parameter
  pMb->_mb_qp_delta = GetDeltaQP(pMb);

	/// Determine the macroblock type. From the prediction modes and patterns set the 
	/// _mb_type member.
	MacroBlockH264::SetType(pMb, _slice._type);

	/// ------------------ Calc the Rate and store results -----------------------------------------
	/// Store distortion-rate pair for this quant value.
	if(withDR)
	{
		pMb->_rate[pMb->_mbEncQP]				= MacroBlockLayerBitCounter(pMb);
		pMb->_distortion[pMb->_mbEncQP]	= distortion;
	}//end if withDR...

	return(pMb->_rate[pMb->_mbEncQP]);
}//end ProcessIntraMbImplStd.

/** The absolute min bit encoding of a Std Intra macroblock.
Set and measure the encoding for a min bit implementation of the std Intra
macroblock. This only includes DC prediction and all residual DC and AC
coeffs are zero'ed. QP is set to max. Return the bit cost.
@param pMb	: Macroblock to operate on.
@return		  : Bit cost.
*/
int H264v2Codec::ProcessIntraMbImplStdMin(MacroBlockH264* pMb)
{
	int rate  = 0;
	int lOffX = pMb->_offLumX;
	int lOffY = pMb->_offLumY;
	int cOffX = pMb->_offChrX;
	int cOffY = pMb->_offChrY;

	/// Base settings for all intra macroblocks.
	pMb->_mbQP			= H264V2_MAX_QP;
	pMb->_skip			= 0;
	pMb->_intraFlag = 1;

	///------------------- Image prediction and loading ----------------------------------------
	/// Currently only Intra 16x16 mode implemented. 
	pMb->_mbPartPredMode= MacroBlockH264::Intra_16x16;

	/// Predict the input from the previously decoded neighbour ref macroblocks then subtract the
	/// prediction from the input.

	/// Lum...
	_RefLum->SetOverlayDim(16, 16);
	_RefLum->SetOrigin(lOffX, lOffY); ///< Align the Ref Lum img block with this macroblock.
	/// Force the selection to DC Lum prediction for all macroblocks.
	pMb->_intra16x16PredMode = MacroBlockH264::Intra_16x16_DC;
	GetIntra16x16LumDCPred(pMb, _RefLum, _16x16);

  _RefLum->Write16x16(*(_16x16)); ///< Write the prediction into the ref.

	/// ... and Chr components.
	_RefCb->SetOverlayDim(8, 8);
	_RefCr->SetOverlayDim(8, 8);
	_RefCb->SetOrigin(cOffX, cOffY);
	_RefCr->SetOrigin(cOffX, cOffY);

	pMb->_intraChrPredMode = MacroBlockH264::Intra_Chr_DC;
	GetIntra8x8ChrDCPred(pMb, _RefCb, _8x8_0);
	GetIntra8x8ChrDCPred(pMb, _RefCr, _8x8_1);

  _RefCb->Write8x8(*(_8x8_0));
  _RefCr->Write8x8(*(_8x8_1));

  /// Clear all mb coeffs for encoding. Implies all difference DC and AC coeffs are zero.
	for(int blk = 0; blk < MBH264_NUM_BLKS; blk++)
	{
		BlockH264* pBlk = pMb->_blkParam[blk].pBlk;
    (pBlk->GetBlkOverlay())->Clear();
    pBlk->SetNumCoeffs(0);
	}//end for blk...

	/// ------------------ Set patterns and type -----------------------------------------------
	/// Determine the coded Lum and Chr patterns. The _codedBlkPatternLum, _codedBlkPatternChr 
	/// and _coded_blk_pattern members are set.
	MacroBlockH264::SetCodedBlockPattern(pMb);

	/// Determine the delta quantisation parameter. For Intra slices
	/// no previous macroblocks are skipped.
	int prevMbIdx = pMb->_mbIndex - 1;
	if(prevMbIdx >= 0)	///< Previous macroblock is within the image boundaries.
	{
		if(pMb->_slice == _pMb[prevMbIdx]._slice)	///< Previous macroblock within same slice.
			pMb->_mb_qp_delta = pMb->_mbQP - _pMb[prevMbIdx]._mbQP;
		else
			pMb->_mb_qp_delta = pMb->_mbQP - _slice._qp;
	}//end if prevMbIdx...
	else
		pMb->_mb_qp_delta = pMb->_mbQP - _slice._qp;

	/// Determine the macroblock type. From the prediction modes and patterns set the 
	/// _mb_type member.
	MacroBlockH264::SetType(pMb, _slice._type);

	/// ------------------ Calc the Rate --------------------------------------------------------
	rate = MacroBlockLayerBitCounter(pMb);

	return(rate);
}//end ProcessIntraMbImplStdMin.

/** Encode the image data for Inter pictures.
Process all macroblocks in order and only write the result to the ref image 
space if the writeRef code is set. Bit 1 of writeRef refers to motion compensation
and bit 0 to adding the difference to the ref. The motion estimation process with 
its associated data structures is assumed to have been completed before this 
method is called. The macroblock obj is prepared for coding onto the bit stream.
Note: For iteratively calling this method;
	Call 1:			writeRef = 2 (compensation to ref but no adding).
	Call 2 - X: writeRef = 0 (ref is motion compensated already from call 1).
	Call final:	writeRef = 1 (allow final add to ref).
@param allowedBits	: Total remaining bits to target.
@param bitsUsed			: Num of bits used for this encoding.
@param writeRef			: Bit code 1x/0x = do/do not comp. x1/x0 do/do not add ref.
@return							: 1 = success, 0 = error.
*/
int H264v2Codec::InterImgPlaneEncoderImplStdVer1::Encode(int allowedBits, int* bitsUsed, int writeRef)
{
	/// Whip through each macroblock and encode. The stream writing of
	/// the macroblock is seperate to allow further rate-distortion decision 
	/// making later on in the encoding process.
	int compRef	= writeRef & 2;
	int addRef	= writeRef & 1;
	int len			= _codec->_mbLength;

	/// Slice without partitioning and therefore only one set of slice parameters.

	/// Set up the input and ref image mem overlays.
	_codec->_Lum->SetOverlayDim(4,4);
	_codec->_Cb->SetOverlayDim(4,4);
	_codec->_Cr->SetOverlayDim(4,4);
	_codec->_RefLum->SetOverlayDim(4,4);
	_codec->_RefCb->SetOverlayDim(4,4);
	_codec->_RefCr->SetOverlayDim(4,4);
	_codec->_16x16->SetOverlayDim(16, 16);
	_codec->_8x8_0->SetOverlayDim(8, 8);
	_codec->_8x8_1->SetOverlayDim(8, 8);

	/// All integer transforms are Inter in this method.
	_codec->_pF4x4TLum->SetMode(IForwardTransform::TransformOnly);
	_codec->_pF4x4TLum->SetParameter(IForwardTransform::INTRA_FLAG_ID, 0);
	_codec->_pF4x4TChr->SetMode(IForwardTransform::TransformOnly);
	_codec->_pF4x4TChr->SetParameter(IForwardTransform::INTRA_FLAG_ID, 0);
	/// By default the DC transforms were set in the TransformOnly mode in the Open() method.

	/// Motion estimation has been previously performed outside of this method and therefore
	/// only motion compensation is required here. Prepare for motion compensation on a per 
	/// macoblock basis. The vectors themselves are held in _pMotionEstimationResult.
	if(compRef)
		_codec->_pMotionCompensator->PrepareForSingleVectorMode();

	/// Get the motion vector list to work with. Assume SIMPLE2D type list as only a
	/// single 16x16 motion vector is considered per macroblock in this implementation
	/// i.e. Inter_16x16 mode only
	int	listLen = _codec->_pMotionEstimationResult->GetLength();
	if(listLen != len)
	{
		*bitsUsed = 0;
		return(0);	///< Error: Motion vector list must match.
	}//end if listLen...

	/// Rip through each macroblock as a linear array and process the
	/// motion vector and each block within the macroblock.
	for(int mb = 0; mb < len; mb++)
	{
		/// Simplify the referencing to the current macroblock.
		MacroBlockH264* pMb = &(_codec->_pMb[mb]);

		///------------------- Motion compensation ------------------------------------------------
		pMb->_mbPartPredMode = MacroBlockH264::Inter_16x16;	///< Fixed at 16x16 for now.

		/// Get the 16x16 motion vector from the motion estimation result list and apply
		/// it to the macroblock. The reference image will then hold the compensated macroblock
		/// to be used as the prediction for calcualting the residual.
		int mvx = _codec->_pMotionEstimationResult->GetSimpleElement(mb, 0);
		int mvy = _codec->_pMotionEstimationResult->GetSimpleElement(mb, 1);
		if(compRef)
			_codec->_pMotionCompensator->Compensate(pMb->_offLumX, pMb->_offLumY, mvx, mvy);

		/// Store the vector for this macroblock.
		pMb->_mvX[MacroBlockH264::_16x16]	= mvx;
		pMb->_mvY[MacroBlockH264::_16x16]	= mvy;

		/// Get the predicted motion vector for this macroblock as the median of the neighbourhood 
		/// vectors and subtract it from the macroblock vector.
		int predX, predY;
		MacroBlockH264::GetMbMotionMedianPred(pMb, &predX, &predY);
		pMb->_mvdX[MacroBlockH264::_16x16] = mvx - predX;
		pMb->_mvdY[MacroBlockH264::_16x16] = mvy - predY;

		///------------------- Macroblock processing ----------------------------------------------
		pMb->_mbQP = _codec->_slice._qp;
    _codec->ProcessInterMbImplStd(pMb, addRef, 0);

	}//end for mb...

	*bitsUsed = 0;
	return(1);
}//end InterImgPlaneEncoderImplStdVer1::Encode.

/// Does NOT include motion prediction and compensation. Assumes these are already set before called.
/// @return : Bits consumed or 0 for skipped mb.
int H264v2Codec::ProcessInterMbImplStd(MacroBlockH264* pMb, int addRef, int withDR)
{
	/// NB: _mbQP must be correctly defined before this method is called.
  pMb->_mbEncQP  = pMb->_mbQP;  ///< mbQP may be altered in this method therefore store the requested QP.
	int distortion = 0;
  int rate       = 0;

	int lOffX = pMb->_offLumX;
	int lOffY = pMb->_offLumY;
	int cOffX = pMb->_offChrX;
	int cOffY = pMb->_offChrY;

	/// Base settings for all inter macroblocks.
	pMb->_skip			= 0;
	pMb->_intraFlag = 0;

	/// Subtract the ref (compensated) macroblock from the input macroblock and place it in the temp image blocks.

	/// Lum...
	_RefLum->SetOverlayDim(16, 16);
	_RefLum->SetOrigin(lOffX, lOffY);   ///< Align the Ref Lum img block with this macroblock.
	_Lum->SetOverlayDim(16, 16);
	_Lum->SetOrigin(lOffX, lOffY);		  ///< Align the Lum img block with this macroblock.
	_16x16->SetOverlayDim(16, 16);
	_16x16->SetOrigin(0, 0);

	_Lum->Read(*(_16x16));				      ///< Read from input Lum into temp.
	_16x16->Sub16x16(*(_RefLum));	      ///< Subtract ref Lum and leave result in temp.

	/// ... and Chr components.
	_RefCb->SetOverlayDim(8, 8);
	_RefCr->SetOverlayDim(8, 8);
	_RefCb->SetOrigin(cOffX, cOffY);
	_RefCr->SetOrigin(cOffX, cOffY);
	_Cb->SetOverlayDim(8, 8);
	_Cb->SetOrigin(cOffX, cOffY);
	_Cr->SetOverlayDim(8, 8);
	_Cr->SetOrigin(cOffX, cOffY);
	_8x8_0->SetOverlayDim(8, 8);
	_8x8_0->SetOrigin(0, 0);
	_8x8_1->SetOverlayDim(8, 8);
	_8x8_1->SetOrigin(0, 0);

	_Cb->Read(*(_8x8_0));
	_8x8_0->Sub8x8(*(_RefCb));
	_Cr->Read(*(_8x8_1));
	_8x8_1->Sub8x8(*(_RefCr));

	/// Fill all the non-DC 4x4 blks (Not blks = -1, 17, 18) of the macroblock blocks with 
	/// the residual image colour components (after motion compensation/prediction).
	MacroBlockH264::LoadBlks(pMb, _16x16, 0, 0, _8x8_0, _8x8_1, 0, 0);

	/// ------------------ Transform & Quantisation --------------------------------------------
	if(pMb->_mbPartPredMode == MacroBlockH264::Inter_16x16)
		TransAndQuantInter16x16MBlk(pMb);

	/// ------------------ Set patterns and type -----------------------------------------------
	/// Determine the coded Lum and Chr patterns. The _codedBlkPatternLum, _codedBlkPatternChr 
	/// and _coded_blk_pattern members are set.
	MacroBlockH264::SetCodedBlockPattern(pMb);

	/// Determine the delta quantisation parameter. Note that it is not coded onto the bit stream
	/// if all 4x4 blocks have no coeffs (not coded). The quant param for this macroblock is then
	/// assumed to be the same as the previous non-skipped macroblock within the same slice. This
	/// works because multiplying any quant param by zero is still zero. NB: This alters the _mbQP
  /// value therefore the original QP used for the encoding is stored in _mbEncQP.
  pMb->_mb_qp_delta = GetDeltaQP(pMb);

	/// Determine the macroblock type. From the prediction modes and patterns set the 
	/// _mb_type member.
	MacroBlockH264::SetType(pMb, _slice._type);

	/// Only Inter_16x16 mode is currently implemented so only 1 motion vector is 
	/// checked for the macroblock skip mode.
	if(pMb->_coded_blk_pattern == 0)
  {
    /// First set of conditions for skip are dependent on a zero 16x16 single motion vector.
    if(MacroBlockH264::SkippedZeroMotionPredCondition(pMb))
    {
      if( (pMb->_mvX[MacroBlockH264::_16x16] == 0) && (pMb->_mvY[MacroBlockH264::_16x16] == 0) )
		    pMb->_skip = 1;
    }//end if SkippedZeroMotionPredCondition...
    /// Second condition is dependent on the previously calculated median difference motion vector.
    else
    {
      if( (pMb->_mvdX[MacroBlockH264::_16x16] == 0)&&(pMb->_mvdY[MacroBlockH264::_16x16] == 0) ) 
		    pMb->_skip = 1;
    }//end else...
  }//end _coded_blk_pattern...

	/// ------------------ Calc distortion from feedback loop ----------------------------------
	/// Implement the feedback loop into the ref image. The block coeffs are still required for 
	/// the context-aware vlc coding and therefore the temporary working blocks within the macroblock 
	/// class are used for this feedback loop.
	if(pMb->_coded_blk_pattern)
	{
		/// --------------------- Inverse Transform & Quantisation -------------------------------
		if(pMb->_mbPartPredMode == MacroBlockH264::Inter_16x16)
			InverseTransAndQuantInter16x16MBlk(pMb, 1);

		/// --------------------- Image Storing into Ref -----------------------------------------
		/// Fill the temp image (difference) colour components from all the non-DC 4x4 
		/// blks (i.e. Not blks = -1, 17, 18) of the macroblock temp blocks. 
		MacroBlockH264::StoreBlks(pMb, _16x16, 0, 0, _8x8_0, _8x8_1, 0, 0, 1);

		/// --------------------- Add the prediction ---------------------------------------------
		/// Lum.
		_RefLum->SetOverlayDim(16, 16);
		_RefLum->SetOrigin(lOffX, lOffY);			///< Align the Ref Lum img block with this macroblock.
		_16x16->SetOverlayDim(16, 16);
		_16x16->SetOrigin(0, 0);
    _16x16->Add16x16WithClip255(*(_RefLum));            ///< Add ref Lum and leave result in temp img.
    if(addRef)
      _RefLum->Write16x16(*(_16x16));

		/// Cb.
		_RefCb->SetOverlayDim(8, 8);
		_RefCb->SetOrigin(cOffX, cOffY);
		_8x8_0->SetOverlayDim(8, 8);
		_8x8_0->SetOrigin(0, 0);
    _8x8_0->Add8x8WithClip255(*(_RefCb));
    if(addRef)
		  _RefCb->Write8x8(*(_8x8_0));
		/// Cr.
		_RefCr->SetOverlayDim(8, 8);
		_RefCr->SetOrigin(cOffX, cOffY);
		_8x8_1->SetOverlayDim(8, 8);
		_8x8_1->SetOrigin(0, 0);
    _8x8_1->Add8x8WithClip255(*(_RefCr));
    if(addRef)
		  _RefCr->Write8x8(*(_8x8_1));

	}//end if _coded_blk_pattern...

	/// ------------------ Calc the Distortion & Rate and store results ----------------------------------
	/// Store distortion-rate pair for this quant value.
	if(withDR)
	{
		/// Calc distortion with Input. If there was a coded blk pattern then compare with temp img else
    /// with the unaltered ref img.
    OverlayMem2Dv2* pLI   = _RefLum;
    OverlayMem2Dv2* pCbI  = _RefCb;
    OverlayMem2Dv2* pCrI  = _RefCr;
    if(pMb->_coded_blk_pattern)
    {
      pLI   = _16x16;
      pCbI  = _8x8_0;
      pCrI  = _8x8_1;
    }//end else if _coded_blk_pattern...

		distortion += pLI->Tsd16x16(*_Lum);	///< Total square distortion between Ref and input Lum.
		distortion += pCbI->Tsd8x8(*_Cb);
		distortion += pCrI->Tsd8x8(*_Cr);
		pMb->_distortion[pMb->_mbEncQP]	= distortion;

    if(!pMb->_skip)
		  rate = MacroBlockLayerBitCounter(pMb);
    else
    {
			/// Ensure coeffs settings are synchronised for future use by neighbours.
			for(int i = 1; i < MBH264_NUM_BLKS; i++)
				pMb->_blkParam[i].pBlk->SetNumCoeffs(0);
    }//end else...

    pMb->_rate[pMb->_mbEncQP] = rate;
	}//end if withDR...

	return(rate);
}//end ProcessInterMbImplStd.

/// Does NOT include motion prediction and compensation. Assumes these mb members are already set before called.
int H264v2Codec::ProcessInterMbImplStdMin(MacroBlockH264* pMb)
{
  /// Set the difference to the compensated reference to zero and therefore no operation needs to be
  /// performed on the ref. Zero and clear all blocks.

	/// NB: _mbQP must be correctly defined before this method is called.
  int rate = 0;

	/// Base settings for all inter macroblocks.
	pMb->_intraFlag = 0;
	pMb->_skip = 0;

	/// Clear the delta quantisation parameter and set the _mbQP to the same as the previous non-skipped mb. The
  /// _mbQP does not have any impact because all coeffs are set to zero.
	pMb->_mb_qp_delta = 0;
	/// Find the previous non-skipped macroblock.
	int prevMbIdx = pMb->_mbIndex - 1;
	/// Re-align the quantisation param for future delta qp calculations.
	if(prevMbIdx >= 0)
	{
		if(pMb->_slice == _pMb[prevMbIdx]._slice)	/// Previous macroblock within same slice.
			pMb->_mbQP = _pMb[prevMbIdx]._mbQP;
		else
			pMb->_mbQP = _slice._qp;
	}//end if prevMbIdx...
	else
		pMb->_mbQP = _slice._qp;

  /// Force all values to zero before encoding.
	_16x16->SetOverlayDim(16, 16);
	_16x16->SetOrigin(0, 0);
  _16x16->Clear();
	_8x8_0->SetOverlayDim(8, 8);
	_8x8_0->SetOrigin(0, 0);
  _8x8_0->Clear();
	_8x8_1->SetOverlayDim(8, 8);
	_8x8_1->SetOrigin(0, 0);
  _8x8_1->Clear();

	/// Fill all the non-DC 4x4 blks (Not blks = -1, 17, 18) of the macroblock blocks with 
	/// the residual image colour components (after motion compensation/prediction).
	MacroBlockH264::LoadBlks(pMb, _16x16, 0, 0, _8x8_0, _8x8_1, 0, 0);

	/// ------------------ Transform & Quantisation --------------------------------------------
	if(pMb->_mbPartPredMode == MacroBlockH264::Inter_16x16)
		TransAndQuantInter16x16MBlk(pMb);

	/// ------------------ Set patterns and type -----------------------------------------------
	/// Determine the coded Lum and Chr patterns. The _codedBlkPatternLum, _codedBlkPatternChr 
	/// and _coded_blk_pattern members are set.
	MacroBlockH264::SetCodedBlockPattern(pMb);

	/// Determine the macroblock type. From the prediction modes and patterns set the 
	/// _mb_type member.
	MacroBlockH264::SetType(pMb, _slice._type);

	/// Only Inter_16x16 mode is currently implemented so only 1 motion vector is 
	/// checked for the macroblock skip mode. If motion is zero then mb is skipped.
	if(pMb->_coded_blk_pattern == 0)
  {
    /// First set of conditions for skip are dependent on a zero 16x16 single motion vector.
    if(MacroBlockH264::SkippedZeroMotionPredCondition(pMb))
    {
      if( (pMb->_mvX[MacroBlockH264::_16x16] == 0) && (pMb->_mvY[MacroBlockH264::_16x16] == 0) )
		    pMb->_skip = 1;
    }//end if SkippedZeroMotionPredCondition...
    /// Second condition is dependent on the previously calculated median difference motion vector.
    else
    {
      if( (pMb->_mvdX[MacroBlockH264::_16x16] == 0)&&(pMb->_mvdY[MacroBlockH264::_16x16] == 0) ) 
		    pMb->_skip = 1;
    }//end else...
  }//end _coded_blk_pattern...

  if(!pMb->_skip)
		rate = MacroBlockLayerBitCounter(pMb);
  else
  {
		/// Ensure coeffs settings are synchronised for future use by neighbours.
		for(int i = 1; i < MBH264_NUM_BLKS; i++)
			pMb->_blkParam[i].pBlk->SetNumCoeffs(0);
  }//end else...

  pMb->_rate[0] = rate;

	return(rate);
}//end ProcessInterMbImplStdMin.

/** Calc delta QP based on macroblock QP.
The coded block pattern, _intraFlag and _mbQP values must be correctly set before calling this 
method. The _skip mode for P mbs alters the _mbQP value.
@param pMb    : Macroblock to operate on.
@return       : Delta QP for these mb parameters.
*/
int H264v2Codec::GetDeltaQP(MacroBlockH264* pMb)
{
  int deltaQP = 0;

	/// Find the previous non-skipped macroblock. For Intra slices no previous macroblocks are skipped.
	int prevMbIdx = pMb->_mbIndex - 1;

	if(pMb->_coded_blk_pattern || pMb->_intraFlag)
	{
		if(prevMbIdx >= 0)	///< Previous macroblock is within the image boundaries.
		{
			if(pMb->_slice == _pMb[prevMbIdx]._slice)	/// Previous macroblock within same slice.
				deltaQP = pMb->_mbQP - _pMb[prevMbIdx]._mbQP;
			else
				deltaQP = pMb->_mbQP - _slice._qp;
		}//end if prevMbIdx...
		else
			deltaQP = pMb->_mbQP - _slice._qp;
	}//end if _coded_blk_pattern...
	else
	{
		/// Re-align the quantisation param for future delta qp calculations.
		if(prevMbIdx >= 0)
		{
			if(pMb->_slice == _pMb[prevMbIdx]._slice)	/// Previous macroblock within same slice.
				pMb->_mbQP = _pMb[prevMbIdx]._mbQP;
			else
				pMb->_mbQP = _slice._qp;
		}//end if prevMbIdx...
		else
			pMb->_mbQP = _slice._qp;
	}//end else...

  return(deltaQP);
}//end GetDeltaQP.

/** Get the previous macroblock QP value.
Picture and slice boundaries are considered in returning the
neighbouring mb QP value.
@param pMb    : Macroblock to operate on.
@return       : Previous Mb QP.
*/
int H264v2Codec::GetPrevMbQP(MacroBlockH264* pMb)
{
	int prevMbIdx = pMb->_mbIndex - 1;

  ///< Previous macroblock is within the image boundaries and within same slice.
	if( (prevMbIdx >= 0)&&(pMb->_slice == _pMb[prevMbIdx]._slice) )
    return(_pMb[prevMbIdx]._mbQP);
  else
    return(_slice._qp);
}//end GetPrevMbQP.


/** Get the next macroblock QP value.
Picture and slice boundaries are considered in returning the
neighbouring mb QP value.
@param pMb    : Macroblock to operate on.
@return       : Next Mb QP.
*/
int H264v2Codec::GetNextMbQP(MacroBlockH264* pMb)
{
	int nextMbIdx = pMb->_mbIndex + 1;

  ///< Previous macroblock is within the image boundaries and within same slice.
	if( (nextMbIdx < _mbLength)&&(pMb->_slice == _pMb[nextMbIdx]._slice) )
    return(_pMb[nextMbIdx]._mbQP);
  else
    return(pMb->_mbIndex);
}//end GetPrevMbQP.

/** Encode the image data for Inter pictures with QP adaptation at macroblock level.
Process all macroblocks in order and only write the result to the ref image 
space if the writeRef code is set. Bit 1 of writeRef refers to motion compensation
and bit 0 to adding the difference to the ref. The motion estimation process with 
its associated data structures is assumed to have been completed before this 
method is called. The macroblock obj is prepared for coding onto the bit stream.
Note: For iteratively calling this method;
	Call 1:			writeRef = 2 (compensation to ref but no adding).
	Call 2 + X: writeRef = 0 (ref is motion compensated already from call 1).
	Call final:	writeRef = 1 (allow final add to ref).
The macroblock wide _mbPQ is adapted to ensure that the resulting stream will use
no more than allowedBits. This algorithm uses a bisection technique to minimise the
max distortion by adapting the macroblock quant value until the nearest rate to the
total allowable bits is achieved. 
@param allowedBits	: Total remaining bits to target.
@param bitsUsed			: Num of bits used for this encoding.
@param writeRef			: Bit code 1x/0x = do/do not comp. x1/x0 do/do not add ref.
@return							: 1 = success, 0 = error.
*/
int H264v2Codec::InterImgPlaneEncoderImplMinMax::Encode(int allowedBits, int* bitsUsed, int writeRef)
{
	int mb, bitCount;
	int compRef			= writeRef & 2;
	int addRef			= writeRef & 1;
	int len					= _codec->_mbLength;
	int minMBIndex	= len; ///< Mark the macroblock from where min units must be encoded.

	/// Set a bound on the max quality by choosing a min qp value that the MinMax search is limited to.
	int qEnd = 4;

	/// Slice without partitioning and therefore only one set of slice parameters.

	/// Set up the input and ref image mem overlays.
	_codec->_Lum->SetOverlayDim(4,4);
	_codec->_Cb->SetOverlayDim(4,4);
	_codec->_Cr->SetOverlayDim(4,4);
	_codec->_RefLum->SetOverlayDim(4,4);
	_codec->_RefCb->SetOverlayDim(4,4);
	_codec->_RefCr->SetOverlayDim(4,4);
	_codec->_16x16->SetOverlayDim(16, 16);
	_codec->_8x8_0->SetOverlayDim(8, 8);
	_codec->_8x8_1->SetOverlayDim(8, 8);

	/// All integer transforms are Inter in this method.
	_codec->_pF4x4TLum->SetMode(IForwardTransform::TransformOnly);
	_codec->_pF4x4TLum->SetParameter(IForwardTransform::INTRA_FLAG_ID, 0);
	_codec->_pF4x4TChr->SetMode(IForwardTransform::TransformOnly);
	_codec->_pF4x4TChr->SetParameter(IForwardTransform::INTRA_FLAG_ID, 0);
	/// By default the DC transforms were set in the TransformOnly mode in the Open() method.

	///	---------------------- Motion Vector encoding ---------------------------------------
	
	/// In this implementation QP adaptation is performed after full motion compensation.
	/// Get the motion vector list to work with. Assume SIMPLE2D type list.
	int	listLen = _codec->_pMotionEstimationResult->GetLength();
	if(listLen != len)
	{
		*bitsUsed = 0;
		return(0);	///< Error: Motion vector list must match.
	}//end if listLen...

	/// Prepare the compensation on a per macroblock basis.
	if(compRef)
		_codec->_pMotionCompensator->PrepareForSingleVectorMode();

	/// Count the bits used for the motion vectors by iterating through the
	/// macroblocks and encoding the differential motion vector diff (_mvdX,_mvdY). 
  /// Check that there are sufficient bits available for each vector. If not then 
  /// truncate the estimation result and mark all remaining macroblocks as skipped. 
  /// Store the bit cost of the mb with only motion vectors coded in _rate[0].
	int bitCost							= 0;
  int mbSkipRun           = 0;
  /// Start with all mbs skipped to the end by counting bits for mbSkipRun = len;
	int minPictureBitsToEnd = _codec->_pHeaderUnsignedVlcEnc->Encode(len);
	int runOutOfBits				= 0;
	for(mb = 0; (mb < len)&&(!runOutOfBits); mb++)
	{
    MacroBlockH264* pMb = &(_codec->_pMb[mb]);  ///< Simplify mb addressing.

    /// Set the mb with the the motion vector, do the prediction to get the MVD and apply the motion compensation.
    pMb->_mbPartPredMode = MacroBlockH264::Inter_16x16; ///< Currently only 16x16 mode supported.
    /// Extract the 16x16 vector from the motion estimation result list.
		int mvx = _codec->_pMotionEstimationResult->GetSimpleElement(mb, 0);
		int mvy = _codec->_pMotionEstimationResult->GetSimpleElement(mb, 1);
    /// Store the motion vector in the mb.
    pMb->_mvX[MacroBlockH264::_16x16] = mvx;
    pMb->_mvY[MacroBlockH264::_16x16] = mvy;
    /// Get the predicted motion vector for this mb as the median of the neighbourhood vectors and subtract from the mb vector.
    int predX, predY;
    MacroBlockH264::GetMbMotionMedianPred(pMb, &predX, &predY);
    pMb->_mvdX[MacroBlockH264::_16x16] = mvx - predX;
    pMb->_mvdY[MacroBlockH264::_16x16] = mvy - predY;

		/// Motion compensate the macroblock.
		if(compRef)
			_codec->_pMotionCompensator->Compensate(pMb->_offLumX, pMb->_offLumY, mvx, mvy);

		int lclAllowedBits	= (allowedBits - bitCost) - minPictureBitsToEnd;	///< So far before encoding this MVD pair.

    /// Process the compensated mb as motion vector only with zero'ed residual to measure its rate cost.
		int lclBitCost = _codec->ProcessInterMbImplStdMin(pMb);

    /// For skipped mbs, the previous skipped bit count must be included in the minPictureBitsToEnd.
    if(!pMb->_skip)
    {
      /// Sum the run and coding bit contributions.
      lclBitCost += _codec->_pHeaderUnsignedVlcEnc->Encode(mbSkipRun);

		  /// Are there enough bits left to encode this run + MVD pair?
		  if(lclBitCost < lclAllowedBits)
			  bitCost	+= lclBitCost;  ///< Update the accumulator.
		  else	///< Truncation condition.
		  {
			  runOutOfBits = 1;
			  break;
		  }//end else...

      mbSkipRun = 0; ///< Reset.

    }//end if !_skip...
    else  ///< Skipped.
      mbSkipRun++;

		/// Update remaining bits for next mb where all remaining mbs are skipped.
    int mbSkippedToEnd = mbSkipRun + ((len-1) - mb); 
    if(mbSkippedToEnd)
		  minPictureBitsToEnd = _codec->_pHeaderUnsignedVlcEnc->Encode(mbSkippedToEnd);
    else
      minPictureBitsToEnd = 0;
	}//end for mb...
  /// Include the last run to the end.
  if(!runOutOfBits)
  {
    bitCost += minPictureBitsToEnd;
    if(bitCost >= allowedBits)  ///< ...and check.
      runOutOfBits = 1;
  }//end if !runOutOfBits...
	int minMvBitsToEnd = bitCost;	///< Hold the total motion vector with zero residual bit cost.

	/// Clean up all remaining macroblocks as skipped and exit. In this implementation
	/// when there are not enough bits to encode all the motion vectors the estimated list
	/// is truncated with [0,0] difference vectors or [0,0] motion with special conditions 
  /// and re-compensated.
	if(runOutOfBits)
	{
    /// Back up to the last non-skipped mb that was the correct count for bitCost. This is the point
    /// at which there were enough bits available to skip to the end.
    mb -= mbSkipRun;
    if(mb < 0) mb = 0;  ///< Not sure this will ever occurr.
    mbSkipRun = 0;

    /// Re-process all mbs as min encodings to the end (i.e. all skipped).
    for( ; mb < len; mb++)
    {
      MacroBlockH264* pMb = &(_codec->_pMb[mb]);  ///< Simplify mb addressing.

      /// Set the mb with the the motion vector, do the prediction to get the MVD and apply the motion compensation.
      pMb->_mbPartPredMode = MacroBlockH264::Inter_16x16; ///< Currently only 16x16 mode supported.

      /// Get the predicted motion vector for this mb as the median of the neighbourhood vectors and set 
      /// the mb vector to this value or the special condition values to force a skipped mb.
      int predX,predY;
      MacroBlockH264::GetMbMotionMedianPred(pMb, &predX, &predY);
      if( !MacroBlockH264::SkippedZeroMotionPredCondition(pMb) )
      {
        pMb->_mvX[MacroBlockH264::_16x16] = predX;
        pMb->_mvY[MacroBlockH264::_16x16] = predY;
      }//end if !SkippedZeroMotionPredCondition...
      else
      {
        pMb->_mvX[MacroBlockH264::_16x16] = 0;
        pMb->_mvY[MacroBlockH264::_16x16] = 0;
      }//end else...
			_codec->_pMotionEstimationResult->SetSimpleElement(mb, 0, pMb->_mvX[MacroBlockH264::_16x16]);
			_codec->_pMotionEstimationResult->SetSimpleElement(mb, 1, pMb->_mvY[MacroBlockH264::_16x16]);
      /// ...and therefore motion differnce is set.
      pMb->_mvdX[MacroBlockH264::_16x16] = pMb->_mvX[MacroBlockH264::_16x16] - predX;
      pMb->_mvdY[MacroBlockH264::_16x16] = pMb->_mvY[MacroBlockH264::_16x16] - predY;

		  /// Re-compensate the last valid compensated macroblock and all the remaining macroblocks to ensure forward dependency.
		  if(compRef)
      {
        _codec->_pMotionCompensator->Invalidate();
			  _codec->_pMotionCompensator->Compensate(pMb->_offLumX, pMb->_offLumY, pMb->_mvX[MacroBlockH264::_16x16], pMb->_mvY[MacroBlockH264::_16x16]);
      }//end if compRef...

      /// Process the compensated mb as skipped to measure its rate cost.
		  _codec->ProcessInterMbImplStdMin(pMb);
      mbSkipRun++;

    }//end for mb...

    /// Add last skip run.
    if(mbSkipRun)
      bitCost += _codec->_pHeaderUnsignedVlcEnc->Encode(mbSkipRun);

		/// ...and then exit.
    int ret = 1;
    if(bitCost > allowedBits)
    {
			_codec->_errorStr = "[H264Codec::InterImgPlaneEncoderImplMinMax::Encode] Bits exceeded in trucation";
      ret = 0;
    }//end if bitCost...

		*bitsUsed = 0;
		return(ret); ///< Something is coded so it is considered successful.
	}//end if runOutOfBits...

	/*
	-------------------------------------------------------------------------------------------
	Residual macroblock Q search.
	-------------------------------------------------------------------------------------------
	*/
 /// Initialisation step: 
	///		lower(D,R) : QP = H264V2_MAX_QP.
	/// Note that the D values represent the worst distortion of only one macroblock in the entire
	/// frame but the R values represent the accumulated rate of all macroblocks.
	int	Dl		= 0;
	int	Du		= 0;
	int	Rl		= 0;
	int	Ru		= 0;
	int	R			= 0;
	int	D			= 0;
	int	Dmax	= 0;
	int iterations	= 0;
	int	invalidated = 0;
	int mbDmax			= 0;

	/// Test QP = H264V2_MAX_QP for all macroblocks and determine if this min possible rate is, at the very 
	/// least, less than the target rate. Damage control is required if not, where minimal encodings
	/// are attempted for the macroblocks. Otherwise, proceed with the initialisation.
  mbSkipRun = 0;
	for(mb = 0; mb < len; mb++)
	{
    MacroBlockH264* pMb = &(_codec->_pMb[mb]);  ///< Simplify mb addressing.

    /// Note that the 1st mb will have a delta offset from _slice._qp to get it to _mbQP = H264V2_MAX_QP.
		pMb->_mbQP	  = H264V2_MAX_QP;
		/// Include all macroblocks at the start.
		pMb->_include = 1;	
		_pQl[mb]      = H264V2_MAX_QP;
		_pQ[mb]	      = H264V2_MAX_QP;	///< Active quantiser vector is always the lower (max distortion) side.

		/// Accumulate the rate and find the largest distortion for all mbs. Call returns 0 for skipped mbs. The ref
    /// holding the compensated motion is not updated with the encoding in this traversal of the mbs.
		Rl += _codec->ProcessInterMbImplStd(pMb, 0, 1);
    if(!pMb->_skip)
    {
      /// Sum of skip run and coded mb bits accumulated.
      Rl += _codec->_pHeaderUnsignedVlcEnc->Encode(mbSkipRun);
      mbSkipRun = 0;
    }//end if !_skip...
    else
      mbSkipRun++;

		if(pMb->_distortion[H264V2_MAX_QP] > Dl)
		{
			Dl = pMb->_distortion[H264V2_MAX_QP];
			mbDmax = mb;	///< Mark the mb index with the peak distortion.
		}//end if _distortion...

		/// Early exit to speed up search if damage control is required.
		if(Rl > allowedBits)
			break;
	}//end for mb...
  /// Add last skip run.
  if(mbSkipRun)
    Rl += _codec->_pHeaderUnsignedVlcEnc->Encode(mbSkipRun);

	/// Test that there is room to adapt from quant param = H264V2_MAX_QP, else do damage control.
	if(Rl <= allowedBits)
	{
		/// Create an epsilon at 0.4% of the available bits with a min limit of 16 bits.
		/// Adjust the available bits to be epsilon below allowedBits.
		int closeEnough = allowedBits/250;	///< 0.4%
		if(closeEnough < 16)
			closeEnough = 16;
		int closeEnoughDist = 8;	///< Sqr error per macroblock.

    /// Model based faster algorithm to improve on the bi-section method.

		/// Modify the target rate to a close offset on the low rate side.
		int bitTarget = allowedBits - closeEnough;

		/// The worst case at QP = H264V2_MAX_QP uses less bits than allowed so there is some
		/// room to manouver and we can proceed. The upper point is initially not calculated
		/// directly but will hold a valid value during the algorithm convergence after the
		/// first iteration.
		/// The upper point is initiallised to some wild guess.
		Du = 1;									///< Min (im)possible.
		Ru = allowedBits << 8;	///< Some very large number.

		/// Initialisation is complete and the adapting bi-section/power model algorithm starts 
		/// here. The stopping criteria are 1) a rate less than, but close to, the allowed bits
		/// or 2) small change in distortion when reducing the interval.
		int done = 0;
		while(!done)
		{
			int prevDmax = Dmax;

			/// Make a prediction assuming a power law model.
			Dmax = _codec->FitDistPowerModel(Rl, Dl, Ru, Du, bitTarget);
			/// Only use this prediction if it is bounded by (Rl,Dl) and (Ru,Du).
			if( (Dmax < Du)||(Dmax > Dl) )	///< Out of bound.
			{
				Dmax = _codec->FitDistLinearModel(Rl, Dl, Ru, Du, bitTarget);
			}//end if linear...

			/// Encourage the descent direction by adding a decreasing factor to the
			/// predicted Dmax such that convergence to Dmax is from the (Rl,Dl) side.
		  Dmax += abs(Dl - Dmax)/4;

			if( (Dmax < Du)||(Dmax > Dl)||(Dmax == prevDmax) )	///< Still out of bound.
			{
				Dmax = ((Du + Dl) + 1) >> 1;	///< Set the midpoint max distortion.
			}//end if bisection...

			/// At each macroblock reduce the quant value until the distortion is lower
			/// than Dmax. pQ[] must always hold the lower rate (smaller valued) quant vector 
			/// as the previous best choice.
			R = 0;
			int firstMbChange = len;
			if(invalidated)	///< Frame encoded coeffs must be invalidated if _pQ[] does not represent the actual QPs used.
				firstMbChange = 0;
      mbSkipRun = 0;
			for(mb = 0; mb < len; mb++)
			{
				MacroBlockH264* pMb = &(_codec->_pMb[mb]);

				/// Record the found QP where the macroblock dist is just below Dmax and accumulate the rate for this macroblock.
				_pQ[mb] = _codec->GetMbQPBelowDmaxVer2(*pMb, _pQ[mb], Dmax, &firstMbChange, qEnd, FALSE);
				R += pMb->_rate[_pQ[mb]]; ///< Rate = 0 for skipped mbs.
        if(!pMb->_skip)
        {
          /// Sum of skip run and coded mb bits accumulated.
          R += _codec->_pHeaderUnsignedVlcEnc->Encode(mbSkipRun);
          mbSkipRun = 0;
        }//end if !_skip...
        else
          mbSkipRun++;

				/// An accurate early exit strategy is not possible because the model prediction require two valid (Dmax,R) points 
				/// for the whole frame. 

			}//end for mb...
      /// Add last skip run.
      if(mbSkipRun)
        R += _codec->_pHeaderUnsignedVlcEnc->Encode(mbSkipRun);

			/// Test the stopping criteria.
			int rBndDiff = abs(Ru - Rl);			///< Optimal solution is rate bounded in a small enough range.
			int dDiff = abs(prevDmax - Dmax);	///< Has not changed by much from the previous iteration (converged).
			int rDiff = abs(bitTarget - R);		///< Close enough to the allowed bits.
			if( (rBndDiff < (4*closeEnough)) ||
					( (rDiff < closeEnough)||(/*(R <= allowedBits)&&*/(dDiff < closeEnoughDist))) ||
          (iterations > 18) )
		  {
				/// The new point is the preferable point to choose but if the rate is not below allowedBits
				/// then the (Rl,Dl) point is selected. Use the invalidated signal to indicate if the mbs 
        /// must be re-processed.
				if(R > allowedBits)
				{
					/// Load the lower vector as it is always below the target rate.
					memcpy((void *)_pQ, (const void *)_pQl, len * sizeof(int));
					R = Rl;
          invalidated = 1;
				}//end if R...
        else
          invalidated = 0;

				/// Optimal solution is found.
				done = 1;
			}//end if rDiff...
			else
			{
				if(allowedBits > R)	///< Inner predicted point is now lower point.
				{
					Dl	= Dmax;
					Rl	= R;
					/// pQ[] is the lower vector. 
					memcpy((void *)_pQl, (const void *)_pQ, len * sizeof(int));
					invalidated = 0;
				}//end if allowedBits...
				else							///< Inner predicted point is now upper point.
				{
					Du	= Dmax;
					Ru	= R;
					/// pQ[] must be reset to lower vector to maintain a descent convergence.
					memcpy((void *)_pQ, (const void *)_pQl, len * sizeof(int));
					invalidated = 1;	///< pQ[] now does not represented the state of the last encoded frame.
				}//end else...

			}//end else...

			iterations++;
		}//end while !done...

		/// Set the macroblock QP values to their found states.
    if(invalidated)
    {
		  for(mb = 0; mb < len; mb++)
		  {
			  _codec->_pMb[mb]._mbQP    = _pQ[mb];
			  _codec->_pMb[mb]._mbEncQP = _pQ[mb];
		  }//end for mb...
    }//end if invalidated...

	}//end if Rl...
	else	///< Damage control.
	{
		/// If QP = H264V2_MAX_QP and still does not fit then a truncation process is nessecary. The truncation
		/// is somewhere between every mb encoded with motion with zero'ed coeffs and some mbs with max QP. The 
    /// _rate[0] member of the mbs holds the min encoding and is zero for skipped. Proceed by encoding each
    /// skip run and max QP pair per mb and then count the bits to the end.
		R         = 0;	///< Reset.
    mbSkipRun = 0;
		for(mb = 0; mb < len; mb++)
		{
      MacroBlockH264* pMb = &(_codec->_pMb[mb]);  ///< Simplify mb addressing.

      /// Do mb encoding including motion and max QP.
		  pMb->_mbQP = H264V2_MAX_QP;
		  R += _codec->ProcessInterMbImplStd(pMb, 0, 1);  ///< Returns 0 if skipped.
      if(!pMb->_skip)
      {
        /// Sum of skip run and coded mb bits accumulated.
        R += _codec->_pHeaderUnsignedVlcEnc->Encode(mbSkipRun);
        mbSkipRun = 0;

        /// Add up min encodings from the next mb to the end.
        int bitsToEnd = 0;
        int run       = 0;
        int mbi       = mb + 1;
        while(mbi < len)
        {
          MacroBlockH264* pMbi = &(_codec->_pMb[mbi]);
          int r = pMbi->_rate[0];
          if(r) ///< Not skipped.
          {
            bitsToEnd += (r + _codec->_pHeaderUnsignedVlcEnc->Encode(run));
            run = 0;
          }//end if r...
          else
            run++;
          mbi++;
        }//end while mbi...
        if(run)
           bitsToEnd += _codec->_pHeaderUnsignedVlcEnc->Encode(run);

        /// With this mb max QP encoded and min encoding for all remaining mbs to the end,
        /// if it does not fit then mark it as the min encoding point and break.
        if((R + bitsToEnd) >= allowedBits)
        {
				  minMBIndex = mb;
				  break;
        }//end if R...
      }//end if !_skip...
      else
        mbSkipRun++;

		}//end for mb...

	}//end else...

	/*
	-------------------------------------------------------------------------------------------
	Residual macroblock final encoding.
	-------------------------------------------------------------------------------------------
	*/
	/// Rip through each macroblock as a linear array and process each block within the macroblock.
	/// The _mbEncPQ values have been set in the above encoding.
//  mbSkipRun = 0;
	bitCount = 0;
	for(int mb = 0; mb < len; mb++)
	{
    MacroBlockH264* pMb = &(_codec->_pMb[mb]);

    /// Encode the mbs and add to Ref img until min null mb position.
    if(mb < minMBIndex)
    {
      pMb->_mbQP = pMb->_mbEncQP; ///< Restore the QP that the mb was coded with.
      bitCount += _codec->ProcessInterMbImplStd(pMb, 1, 0);
//      bitCount += _codec->ProcessInterMbImplStd(pMb, 1, 1);
    }//end if mb...
    else
      bitCount += _codec->ProcessInterMbImplStdMin(pMb);
//    if(!pMb->_skip)
//    {
//      bitCount += _codec->_pHeaderUnsignedVlcEnc->Encode(mbSkipRun);
//      mbSkipRun = 0;
//    }//end if !_skip...
//    else
//      mbSkipRun++;

	}//end for mb...
  /// Add last skip run.
//  if(mbSkipRun)
//    bitCount += _codec->_pHeaderUnsignedVlcEnc->Encode(mbSkipRun);

	*bitsUsed = 0;
	return(1);
}//end InterImgPlaneEncoderImplMinMax::Encode.

/** Decode the Inter macroblocks to the reference img.
The macroblock obj encodings must be fully defined before calling
this method.
@return	: 1 = success, 0 = error.
*/
int H264v2Codec::InterImgPlaneDecoderImplStdVer1::Decode(void)
{
	int mb;
	int len	= _codec->_mbLength;

	/// Set up the input and ref image mem overlays.
	_codec->_RefLum->SetOverlayDim(4,4);
	_codec->_RefCb->SetOverlayDim(4,4);
	_codec->_RefCr->SetOverlayDim(4,4);
	_codec->_16x16->SetOverlayDim(16, 16);
	_codec->_8x8_0->SetOverlayDim(8, 8);
	_codec->_8x8_1->SetOverlayDim(8, 8);

	/// Prepare for motion compensation on a per macoblock basis.
	_codec->_pMotionCompensator->PrepareForSingleVectorMode();

	/// Whip through each macroblock. Decode the extracted encodings. All modes
	/// and parameters have been extracted by the ReadMacroBlockLayer() method.
	for(mb = 0; mb < len; mb++)
	{
		/// Simplify the referencing to the current macroblock.
		MacroBlockH264* pMb = &(_codec->_pMb[mb]);
		int lOffX = pMb->_offLumX;
		int lOffY = pMb->_offLumY;
		int cOffX = pMb->_offChrX;
		int cOffY = pMb->_offChrY;

		///------------------- Motion compensation -----------------------------------------------------------
		if(pMb->_mbPartPredMode != MacroBlockH264::Inter_16x16)	///< Fixed at 16x16 mode for now.
		{
			_codec->_errorStr = "[H264V2::InterImgPlaneDecoderImplStdVer1::Decode] Only supports Inter_16x16 mode";
			return(0);
		}//end if !Inter_16x16...

		/// Compensate the vector and hope the encoder estimator ensured that
		/// all pels fall inside the image space. The motion vector was decoded
		/// from the vector differences in the ReadMacroBlockLayer() method.
		_codec->_pMotionCompensator->Compensate(lOffX, lOffY, pMb->_mvX[MacroBlockH264::_16x16], pMb->_mvY[MacroBlockH264::_16x16]);

		if(pMb->_coded_blk_pattern)
		{
			/// --------------------- Inverse Transform & Quantisation -------------------------------
			if(pMb->_mbPartPredMode == MacroBlockH264::Inter_16x16)
				_codec->InverseTransAndQuantInter16x16MBlk(pMb, 0);

			/// --------------------- Image Storing into Ref -----------------------------------------
			/// Fill the image (difference) colour components from all the non-DC 4x4 
			/// blks (i.e. Not blks = -1, 17, 18) of the macroblock temp blocks. 
			MacroBlockH264::StoreBlks(pMb, _codec->_16x16, 0, 0, _codec->_8x8_0, _codec->_8x8_1, 0, 0, 0);

			/// --------------------- Add the prediction ---------------------------------------------
			/// Lum.
			_codec->_RefLum->SetOverlayDim(16, 16);
			_codec->_RefLum->SetOrigin(lOffX, lOffY); ///< Align the Ref Lum img block with this macroblock.
			_codec->_16x16->SetOverlayDim(16, 16);
			_codec->_16x16->SetOrigin(0, 0);
			_codec->_RefLum->AddWithClip255(*(_codec->_16x16));	///< Add to ref Lum and leave result in ref img.
			/// Cb.
			_codec->_RefCb->SetOverlayDim(8, 8);
			_codec->_RefCb->SetOrigin(cOffX, cOffY);
			_codec->_8x8_0->SetOverlayDim(8, 8);
			_codec->_8x8_0->SetOrigin(0, 0);
			_codec->_RefCb->AddWithClip255(*(_codec->_8x8_0));
			/// Cr.
			_codec->_RefCr->SetOverlayDim(8, 8);
			_codec->_RefCr->SetOrigin(cOffX, cOffY);
			_codec->_8x8_1->SetOverlayDim(8, 8);
			_codec->_8x8_1->SetOrigin(0, 0);
			_codec->_RefCr->AddWithClip255(*(_codec->_8x8_1));

		}//end if _coded_blk_pattern...

	}//end for mb...

	return(1);
}//end InterImgPlaneDecoderImplStdVer1::Decode.

/*
-------------------------------------------------------------------------------------------
	Private helper methods for prediction tools.
-------------------------------------------------------------------------------------------
*/

/** Calc the median of 3 numbers.
@param x	:	1st num.
@param y	:	2nd num.
@param z	:	3rd num.
@return		: The median of x, y and z.
*/
int H264v2Codec::Median(int x, int y, int z)
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

void H264v2Codec::DumpBlock(OverlayMem2Dv2* pBlk, char* filename, const char* title)
{
  int i,j; 

  MeasurementTable* pT = new MeasurementTable();
	pT->SetTitle(title);

  int cols = pBlk->GetWidth();
  int rows = pBlk->GetHeight();

	pT->Create(cols, rows);
  for(j = 0; j < cols; j++)
  {
    pT->SetHeading(j, "");
    pT->SetDataType(j, MeasurementTable::INT);
  }//end for j...

	for(i = 0; i < rows; i++)
		for(j = 0; j < cols; j++)
			pT->WriteItem(j, i, pBlk->Read(j, i));

  pT->Save(filename, ",");

  delete pT;
}//end DumpBlock.

