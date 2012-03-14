/** @file

MODULE				: H264v2Codec

TAG						: H264V2C

FILE NAME			: H264v2Codec.h

DESCRIPTION		: A 2nd generation of video codecs based on the H.264 standard
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
#ifndef _H264V2CODEC_H
#define _H264V2CODEC_H

#pragma once

#include "ICodecv2.h"
#include "ICodecInnerAccess.h"

#include "RGBtoYUV420Converter.h"
#include "YUV420toRGBConverter.h"

#include "IBitStreamWriter.h"
#include "IBitStreamReader.h"

#include "OverlayMem2Dv2.h"
#include "IForwardTransform.h"
#include "IInverseTransform.h"
#include "IRunLengthCodec.h"

#include "VectorStructList.h"
#include "IMotionEstimator.h"
#include "IMotionCompensator.h"

#include "IVlcEncoder.h"
#include "IVlcDecoder.h"

#include "NalHeaderH264.h"
#include "SliceHeaderH264.h"
#include "SeqparamSetH264.h"
#include "PicparamSetH264.h"

#include "MacroBlockH264.h" 

/*
===========================================================================
  Codec specific definitions.
===========================================================================
*/

/// Defined Modes of Operation for bit allocation stratergies - "mode of operation".
#define	H264V2_OPEN										0	/// (Default) Slice QP defines the bit usage without control.
#define H264V2_MB_QP_MINMAX_ADAPTIVE	1	/// Macroblock QP is adapted to meet the frame bit target.

/// Seq and Pic param max encoded length.
#define	H264V2_ENC_PARAM_LEN          32

/// Use non-reversible CCIR-601 colour conversions.
//#define _CCIR601

/*
===========================================================================
  Class definition.
===========================================================================
*/
class H264v2Codec : public ICodecv2, public ICodecInnerAccess
{
/// construction
public:
  H264v2Codec();
	virtual ~H264v2Codec();

/// ICodecv2 Interface Implementation
public:

	int		GetParameter(const char* type, int* length, void* value);
	void	GetParameterName(int ordinal, const char** name, int* length);
	int		SetParameter(const char* type, const char* value);

	char* GetErrorStr(void) { return((char *)_errorStr); }
	int		Ready(void) { return(_codecIsOpen); }
	int		GetCompressedBitLength(void) { return((int)_bitStreamSize); }
	int		GetCompressedByteLength(void) 
		{ int x = (int)_bitStreamSize/8; if(_bitStreamSize & 0x7) x++; return(x); }
	void* GetReference(int refNum) { return( (void*)_pRLum ); }

	void	Restart(void);
	int		Open(void);
	int		Close(void);
	int		Code(void* pSrc, void* pCmp, int codeParameter);
	int		Decode(void* pCmp, int bitLength, void* pDst);

/// ICodecInnerAccess Interface Implementation
public:
	void* GetMember(const char* type, int* length);
	void	GetMemberName(int ordinal, const char** name, int* length);
	int		SetMember(const char* type, void* pValue);

/// Codec parameters.
private:
	/// Unique Codec identification.
	int		_idCode;																				///"codecid"
	/// The codec parameters are seperated into 2 categories. Those that are static throughout the
	/// life span of an Open()/Close() session and those that are dynamic and can be changed per
	/// picture Code()/Decode(). 

	/// -------------- Static Parameters ------------------------------------------------------ 
	/// Set before Open() and cleared at Close().

  /// Description of the source/destination image.
  int		_width;																					///< "width"
  int		_height;																				///< "height"
	int		_inColour;																			///< "incolour"
	int		_outColour;																			///< "outcolour"
  bool  _flip;                                          ///< "stores whether the internal color converters should flip the image during conversion


	/// Modes of operation (bit allocation strategies).
	int		_modeOfOperation;																///< "mode of operation"

  /// Parameter set handling.
	int		_currSeqParam;																	///< "seq param set"
	int		_currPicParam;																	///< "pic param set"
	int		_genParamSetOnOpen;															///< "generate param set on open"
  int   _prependParamSetsToIPic;                        ///< "prepend param sets to i-pictures"

	int		_startCodeEmulationPrevention;									///< "start code emulation prevention"


	/// -------------- Dynamic Parameters ------------------------------------------------------ 
	/// Set before Code()/Decode(). Remain in effect until modified.

	/// Picture properties.
  int		_pictureCodingType;															///"picture coding type"	(Will be interpreted into/from NAL type)
  int		_lastPicCodingType;															///"last pic coding type"
	int		_pQuant;																				///"quality"

	/// I-Picture modifiers.
	int		_autoIPicture;																	///"autoipicture"
	int		_iPictureMultiplier;														///"ipicturemultiplier"
	int		_iPictureFraction;															///"ipicturefraction"

/// Attributes
private:
  /// Intermediate input/output YCbCr image mem members.
  int							_lumWidth;		/// pix width.
  int							_lumHeight;		/// pix height.
  short*					_pLum;				/// Space to compress from and decompress to.
	int							_chrWidth;
	int							_chrHeight;
	short*					_pChrU;
	short*					_pChrV;

	/// 2-D overlays for YCbCr picture mem.
	OverlayMem2Dv2*	_Lum;
	OverlayMem2Dv2*	_Cb;
	OverlayMem2Dv2*	_Cr;

  /// Reference image mem members.
  short*					_pRLum;
  short*					_pRChrU;
  short*					_pRChrV;

	/// 2-D overlays for YCbCr picture reference mem.
	OverlayMem2Dv2*	_RefLum;
	OverlayMem2Dv2*	_RefCb;
	OverlayMem2Dv2*	_RefCr;

	/// Temp 16x16 and 8x8 mem blocks for use during macroblock prediction.
	short*					_p16x16;
	OverlayMem2Dv2*	_16x16;
	short*					_p8x8_0;
	OverlayMem2Dv2*	_8x8_0;
	short*					_p8x8_1;
	OverlayMem2Dv2*	_8x8_1;

private:
  void				ResetMembers(void);
	int					CodeNonPicNALTypes(void* pCmp, int frameBitLimit);

	int					SetSeqParamSet(int index);
	int					WriteSeqParamSet(IBitStreamWriter* bsw, int allowedBits, int* bitsUsed, int index);
	int					ReadSeqParamSet(IBitStreamReader* bsr, int remainingBits, int* bitsUsed, int* idx, int* changedFlag);
	int					SetPicParamSet(int index, int seqRef);
	int					WritePicParamSet(IBitStreamWriter* bsw, int allowedBits, int* bitsUsed, int index);
	int					ReadPicParamSet(IBitStreamReader* bsr, int remainingBits, int* bitsUsed, int* idx, int* changedFlag);
	int					GetCodecParams(int picParamSet);

	int					WriteNALHeader(IBitStreamWriter* bsw, int allowedBits, int* bitsUsed);
	int					ReadNALHeader(IBitStreamReader* bsr, int remainingBits, int* bitsUsed);
	int					WriteSliceLayerHeader(IBitStreamWriter* bsw, int allowedBits, int* bitsUsed);
	int					ReadSliceLayerHeader(IBitStreamReader* bsr, int remainingBits, int* bitsUsed);

	int					WriteTrailingBits(IBitStreamWriter* bsw, int allowedBits, int* bitsUsed);
	int					ReadTrailingBits(IBitStreamReader* bsr, int remainingBits, int* bitsUsed);

  int         InsertEmulationPrevention(IBitStreamWriter* bsw, int startOffset);
  int         RemoveEmulationPrevention(IBitStreamReader* bsr);

	int					WriteSliceDataLayer(IBitStreamWriter* bsw, int allowedBits, int* bitsUsed);
	int					ReadSliceDataLayer(IBitStreamReader* bsr, int remainingBits, int* bitsUsed);

	int					WriteMacroBlockLayer(IBitStreamWriter* bsw, MacroBlockH264* pMb, int allowedBits, int* bitsUsed);
	int					MacroBlockLayerBitCounter(MacroBlockH264* pMb);
	int					ReadMacroBlockLayer(IBitStreamReader* bsr, int remainingBits, int* bitsUsed);

	void				ApplyLoopFilter(void);
	void				VerticalFilter(MacroBlockH264* pMb, short** img, int lumFlag, int rowOff, int colOff, int iter, int boundaryStrength);
	void				HorizontalFilter(MacroBlockH264* pMb, short** img, int lumFlag, int rowOff, int colOff, int iter, int boundaryStrength);

	void				TransAndQuantIntra16x16MBlk(MacroBlockH264* pMb);
	void				TransAndQuantIntra16x16ModeBlk(IForwardTransform* pTQ, BlockH264* pBlk, short* pDcBlkCoeff);
	void				InverseTransAndQuantIntra16x16MBlk(MacroBlockH264* pMb, int tmpBlkFlag);
	void				InvTransAndQuantIntra16x16ModeBlk(IInverseTransform* pTQ, BlockH264* pBlk, short* pDcBlkCoeff);

	void				TransAndQuantInter16x16MBlk(MacroBlockH264* pMb);
	void				InverseTransAndQuantInter16x16MBlk(MacroBlockH264* pMb, int tmpBlkFlag);

	int					GetIntra16x16LumPredAndMode(MacroBlockH264* pMb, OverlayMem2Dv2* in, OverlayMem2Dv2* ref, OverlayMem2Dv2* pred);
	int					GetIntra16x16LumPred(MacroBlockH264* pMb, OverlayMem2Dv2* ref, OverlayMem2Dv2* pred, int predMode);
	void				GetIntra16x16LumDCPred(MacroBlockH264* pMb, OverlayMem2Dv2* lum, OverlayMem2Dv2* pred);
	int					GetIntra16x16LumPlanePred(MacroBlockH264* pMb, OverlayMem2Dv2* lum, OverlayMem2Dv2* pred);
	int					GetIntraVertPred(MacroBlockH264* pMb, OverlayMem2Dv2* img, OverlayMem2Dv2* pred, int lumFlag);
	int					GetIntraHorizPred(MacroBlockH264* pMb, OverlayMem2Dv2* img, OverlayMem2Dv2* pred, int lumFlag);
	void				GetIntra8x8ChrDCPred(MacroBlockH264* pMb, OverlayMem2Dv2* chr, OverlayMem2Dv2* pred);
	int					GetIntra8x8ChrPlanePred(MacroBlockH264* pMb, OverlayMem2Dv2* chr, OverlayMem2Dv2* pred);
	int					GetIntra8x8ChrPredAndMode(MacroBlockH264* pMb,	OverlayMem2Dv2* cb,			OverlayMem2Dv2* cr,			 
																																	OverlayMem2Dv2* refCb,	OverlayMem2Dv2* refCr, 
																																	OverlayMem2Dv2* predCb, OverlayMem2Dv2* predCr);

	int					Median(int x, int y, int z);
  static void DumpBlock(OverlayMem2Dv2* pBlk, char* filename, const char* title);

/// The implementations of img plane encoders and decoders is done through a common
/// interface and as private nested classes.
private:
	/// Common interface for img plane encoders and decoders as nested classes.
	class IImagePlaneEncoder
	{
		public:
			virtual ~IImagePlaneEncoder(void) { }
			virtual int Encode(int allowedBits, int* bitsUsed, int writeRef) = 0;
			virtual int Create(int length) { return(1); }
	};//end class IImagePlaneEncoder.
	friend class IImagePlaneEncoder;

	class IImagePlaneDecoder
	{
		public:
			virtual ~IImagePlaneDecoder(void) { }
			virtual int Decode(void) = 0;
	};//end class IImagePlaneDecoder.
	friend class IImagePlaneDecoder;

	/// Implementations of img plane encoders and decoders as nested classes of codec.

	/// Image Plane Encoders.

	class IntraImgPlaneEncoderImplStdVer1 : public IImagePlaneEncoder	/// I-frame IDR baseline encoder.
	{
		public:
			IntraImgPlaneEncoderImplStdVer1(H264v2Codec* codec) { _codec = codec; }
			virtual ~IntraImgPlaneEncoderImplStdVer1(void) { }
			int Encode(int allowedBits, int* bitsUsed, int writeRef);
		private:
			H264v2Codec* _codec;
	};//end class IntraImgPlaneEncoderImplStdVer1.
	friend class IntraImgPlaneEncoderImplStdVer1;

	class IntraImgPlaneEncoderImplMinMax : public IImagePlaneEncoder	///< I-frame macroblock level Quant adaptive encoder.
	{
		/// MinMax algorithm with rate constraint. 
		public:
			IntraImgPlaneEncoderImplMinMax(H264v2Codec* codec) { _codec = codec; _pQ = NULL; _pQl = NULL; }
			virtual ~IntraImgPlaneEncoderImplMinMax(void) { if(_pQ != NULL) delete[] _pQ; _pQ = NULL; }
			int Encode(int allowedBits, int* bitsUsed, int writeRef);
			int Create(int length)
				{ if(_pQ != NULL) delete[] _pQ; _pQ = NULL;
					_pQ = new int[2 * length]; if(_pQ == NULL) return(0);
					_pQl = _pQ + length; return(1);
				}
		private:
			H264v2Codec* _codec;
			int*	_pQ;
			int*	_pQl;
	};//end class IntraImgPlaneEncoderImplMinMax.
	friend class IntraImgPlaneEncoderImplMinMax;

	class InterImgPlaneEncoderImplStdVer1 : public IImagePlaneEncoder	/// P-frame baseline encoder.
	{
		public:
			InterImgPlaneEncoderImplStdVer1(H264v2Codec* codec) { _codec = codec; }
			virtual ~InterImgPlaneEncoderImplStdVer1(void) { }
			int Encode(int allowedBits, int* bitsUsed, int writeRef);
		private:
			H264v2Codec* _codec;
	};//end class InterImgPlaneEncoderImplStdVer1.
	friend class InterImgPlaneEncoderImplStdVer1;

	class InterImgPlaneEncoderImplMinMax : public IImagePlaneEncoder	///< P-frame macroblock PQ adaptive encoder.
	{
		/// MinMax algorithm with rate constraint. Uses bisection method to find optimal selection.
		public:
			InterImgPlaneEncoderImplMinMax(H264v2Codec* codec) { _codec = codec; _pQ = NULL; _pQl = NULL; }
			virtual ~InterImgPlaneEncoderImplMinMax(void) { if(_pQ != NULL) delete[] _pQ; _pQ = NULL; }
			int Encode(int allowedBits, int* bitsUsed, int writeRef);
			int Create(int length) 
				{ if(_pQ != NULL) delete[] _pQ; _pQ = NULL;
					_pQ = new int[2 * length]; if(_pQ == NULL) return(0);
					_pQl = _pQ + length; return(1);
				}
		private:
			H264v2Codec* _codec;
			int*	_pQ;
			int*	_pQl;
	};//end class InterImgPlaneEncoderImplMinMax.
	friend class InterImgPlaneEncoderImplMinMax;

	/// Image Plane Decoders.

	class IntraImgPlaneDecoderImplStdVer1 : public IImagePlaneDecoder	/// I-frame IDR baseline decoder.
	{
		public:				
			IntraImgPlaneDecoderImplStdVer1(H264v2Codec* codec) { _codec = codec; }
			virtual ~IntraImgPlaneDecoderImplStdVer1(void) { }
			int Decode(void);
		private:
			H264v2Codec* _codec;
	};//end class IntraImgPlaneDecoderImplStdVer1.
	friend class IntraImgPlaneDecoderImplStdVer1;

	class InterImgPlaneDecoderImplStdVer1 : public IImagePlaneDecoder	/// P-frame baseline decoder.
	{
		public:
			InterImgPlaneDecoderImplStdVer1(H264v2Codec* codec) { _codec = codec; }
			virtual ~InterImgPlaneDecoderImplStdVer1(void) { }
			int Decode(void);
		private:
			H264v2Codec* _codec;
	};//end class InterImgPlaneDecoderImplStdVer1.
	friend class InterImgPlaneDecoderImplStdVer1;

	/// Code refactoring methods.
	int FitDistPowerModel(int rl, int dl, int ru, int du, int r);
	int FitDistLinearModel(int rl, int dl, int ru, int du, int r);

//	int ForwardLoopIntraImplStd(MacroBlockH264 &mb, int withDR);																		
	int ProcessIntraMbImplStd(MacroBlockH264* pMb, int withDR);																		
	int ProcessIntraMbImplStdMin(MacroBlockH264* pMb);																		
	int ProcessInterMbImplStd(MacroBlockH264* pMb, int addRef, int withDR);																		
	int ProcessInterMbImplStdMin(MacroBlockH264* pMb);																		
	int GetDeltaQP(MacroBlockH264* pMb);																	
	int GetMbQPBelowDmax(MacroBlockH264 &mb, int atQP, int Dmax, int decQP, int* changeMb, int lowestQP, bool intra);
	int GetMbQPBelowDmaxVer2(MacroBlockH264 &mb, int atQ, int Dmax, int* changeMb, int lowestQ, bool intra);
	int GetMbQPBelowDmaxApprox(MacroBlockH264 &mb, int atQP, int Dmax, int epsilon, int decQP, int* changeMb, int lowestQP, bool intra);

  /// In-line methods. 

  /** Get the previous macroblock QP value.
  Picture and slice boundaries are considered in returning the
  neighbouring mb QP value.
  @param pMb    : Macroblock to operate on.
  @return       : Previous Mb QP.
  */
  inline int GetPrevMbQP(MacroBlockH264* pMb);

  /** Get the next macroblock QP value.
  Picture and slice boundaries are considered in returning the
  neighbouring mb QP value.
  @param pMb    : Macroblock to operate on.
  @return       : Next Mb QP.
  */
  inline int GetNextMbQP(MacroBlockH264* pMb);

/// Codec Status.
private:
	char*		_errorStr;
	int			_codecIsOpen;
	int			_bitStreamSize;

/// Constants.
private:
	/// The id must be unique for every codec implementation.
	static const unsigned short H264V2_ID = 1003;

	/// List of all parameter names accessed via GetParameterName().
	static const int		PARAMETER_LEN;		///< "parameters".
	static const char*	PARAMETER_LIST[];	///< ...of PARAMETER_LEN.

	/// List of all member names accessed via GetMemberName().
	static const int		MEMBER_LEN;			///< "members".
	static const char*	MEMBER_LIST[];	///< ...of MEMBER_LEN.

	/// Scaling matrix for DC coeffs. Unknown reason.
	static const int dc4x4Scale[];
	static const int dc2x2Scale[];

	/// Test sampling point coordinates for Intra_16x16 and Intra_8x8 prediction mode selection.
	static const int test16X[];
	static const int test16Y[];
	static const int test16Limit[];
	static const int test8X[];
	static const int test8Y[];
	static const int test8Limit[];

	/// Loop filter constants.
	static const int alpha[52]; 
	static const int beta[52];
	static const int indexAbS[3][52];

	/// Fast search on the macroblock distortion vs. QP curve 
	/// changes the step size.
	static const int MbStepSize[];

/// Operational members.
private:
	/// Compressed data stream access members.
	IBitStreamWriter*			_pBitStreamWriter;
	IBitStreamReader*			_pBitStreamReader;

	/// An input colour converter.
	RGBtoYUV420Converter*	_pInColourConverter;
	YUV420toRGBConverter* _pOutColourConverter;

	/// 4x4 and 2x2 IT DC and AC transform filters.
	IForwardTransform* _pF4x4TLum;
	IForwardTransform* _pF4x4TChr;
	IForwardTransform* _pFDC4x4T;
	IForwardTransform* _pFDC2x2T;
	IInverseTransform* _pI4x4TLum;
	IInverseTransform* _pI4x4TChr;
	IInverseTransform* _pIDC4x4T;
	IInverseTransform* _pIDC2x2T;

	/// The motion estimator distortion is tested to	determine if an
	/// I-frame would be more appropriate for the frame. But for abs
	/// diff algorithms a different linear factor is required compared
	/// to squared error algorithms.
	int										_motionFactor;						///< Sqr Err = 4, Abs diff = 2.
	/// Auto I-frame detection test variable to hold the previous picture distortion.
	long									_prevMotionDistortion;
	///	Auto I-frame flags to indicate which macroblocks must be excluded
	/// from the decision during the motion estimation process.
	bool*									_autoIFrameIncluded;

	IMotionEstimator*			_pMotionEstimator;				///< Selected estimator dependent on mode.
	VectorStructList*			_pMotionEstimationResult;	///< Motion vector list generated by estimator.
	IMotionCompensator*		_pMotionCompensator;			///< Selected compensator dependent on mode.
	VectorStructList*			_pMotionVectors;					///< Motion vector list input to compensators.

	/// Vlc encoders and decoders for use with CAVLC.
	IVlcEncoder*	_pPrefixVlcEnc;
	IVlcDecoder*	_pPrefixVlcDec;
	IVlcEncoder*	_pCoeffTokenVlcEnc;
	IVlcDecoder*	_pCoeffTokenVlcDec;
	IVlcEncoder*	_pTotalZeros4x4VlcEnc;
	IVlcDecoder*	_pTotalZeros4x4VlcDec;
	IVlcEncoder*	_pTotalZeros2x2VlcEnc;
	IVlcDecoder*	_pTotalZeros2x2VlcDec;
	IVlcEncoder*	_pRunBeforeVlcEnc;
	IVlcDecoder*	_pRunBeforeVlcDec;
	/// ... and the remaining vlc encoders and decoders.
	IVlcEncoder*	_pBlkPattVlcEnc;
	IVlcDecoder*	_pBlkPattVlcDec;
	IVlcEncoder*	_pDeltaQPVlcEnc;
	IVlcDecoder*	_pDeltaQPVlcDec;
	IVlcEncoder*	_pMbTypeVlcEnc;
	IVlcDecoder*	_pMbTypeVlcDec;
	IVlcEncoder*	_pMbIChrPredModeVlcEnc;
	IVlcDecoder*	_pMbIChrPredModeVlcDec;
	IVlcEncoder*	_pMbMotionVecDiffVlcEnc;
	IVlcDecoder*	_pMbMotionVecDiffVlcDec;
	/// Context-Aware vlc codecs for each block size.
	IContextAwareRunLevelCodec* _pCAVLC4x4;
	IContextAwareRunLevelCodec* _pCAVLC2x2;
	/// General header vlc encoders and decoders.
	IVlcEncoder*	_pHeaderUnsignedVlcEnc;
	IVlcDecoder*	_pHeaderUnsignedVlcDec;
	IVlcEncoder*	_pHeaderSignedVlcEnc;
	IVlcDecoder*	_pHeaderSignedVlcDec;

	/// Macroblocks for the image.
	int										_mbLength;	///< No. of 16 x 16 macroblocks for this image size.
	MacroBlockH264*				_pMb;				///< Base macroblock linear reference.
	MacroBlockH264**			_Mb;				///< Macroblock 2-D reference.

	/// Internal picture properties. (For now)

	/// NAL unit definition.
	NalHeaderH264		_nal;
	/// Parameter sets with their current selections that reference the parameter arrays. 
	SeqParamSetH264	_seqParam[32];
	PicParamSetH264	_picParam[255];
  int             _encSeqParamByteLen;
  unsigned char   _pEncSeqParam[H264V2_ENC_PARAM_LEN];    ///< Cached current encoded seq param set. (32 bytes)
  int             _encPicParamByteLen;
  unsigned char   _pEncPicParam[H264V2_ENC_PARAM_LEN];    ///< Cached current encoded pic param set. (32 bytes)

	/// H.264 has inter prediction mechanisms that permit multiple refrence pictures/frames and
	/// control parameters to maintain a list of these references. However, in this implementation
	/// the GOP are IPPPPPP..... and all P-frames use only the previous frame for reference. No
	/// reference lists (List0) are maintained and frame number counting is consecutively incremented
	/// from the previous IDR-frame.
	int						_frameNum;				///< Used for SliceHeader._frame_num
	int						_maxFrameNum;			///< Used for and derived from SeqParamSet._log2_max_frame_num_minus4
	int						_idrFrameNum;			///< Used for SliceHeader._idr_pic_id

	/// This implementation has only 1 slice for the entire frame without
	/// partitioning and therefore the slice parameters are simple.
	SliceHeaderH264	_slice;
	int							_mb_skip_run;		///< For P-Slices a skip run preceeds each macroblock.

	/// Image plane encoders/decoders. 
	IImagePlaneEncoder*		_pIntraImgPlaneEncoder;
	IImagePlaneEncoder*		_pInterImgPlaneEncoder;
	IImagePlaneDecoder*		_pIntraImgPlaneDecoder;
	IImagePlaneDecoder*		_pInterImgPlaneDecoder;

};//H264v2Codec class.


#endif	//end _H264V2CODEC_H
