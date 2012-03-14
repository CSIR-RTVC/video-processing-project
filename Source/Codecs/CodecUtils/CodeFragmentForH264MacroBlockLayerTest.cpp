/** @file

MODULE				: CodeFragmentForH264MacroBlockLayerTest

TAG						: CFH264MBLT

FILE NAME			: CodeFragmentForH264MacroBlockLayerTest.cpp

DESCRIPTION		: A code fragment of the unit tests for the H.264 macroblock layer 
								defined in ITU-T Rec. H.264 (03/2005). This implementation will 
								not compile directly as all #include header files are excluded.

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

/** Test macroblock layer encode and decode implementations.
@return	: 1 = success, 0 = failed..
*/
#define H264V2C_MbType			0
#define H264V2C_QP					1
#define H264V2C_IChrPedMode	2
#define H264V2C_MVx					3
#define H264V2C_MVy					4

#define H264V2C_BYTESIZE 4096

static int H264V2C_ParamSet[3][3][5];

int H264v2Codec::TestMacroBlockLayer(void)
{
	int i,j,y,x,test,ret;
	int result = 1;
	int TESTS = 1024;
	int qStep, qErr;
	int sliceQP, sliceType;

	int qfactor[6] = { 10, 11, 13, 14, 16, 18 };

	/// Scaling is required for the DC coeffs to match the 4x4 
	/// inverse IT saling of the AC coeffs.
	int dc4x4Scale[16] =
		{ 4, 4, 4, 4, 
			4, 4, 4, 4, 
			4, 4, 4, 4,
			4, 4, 4, 4 
		};
	int dc2x2Scale[4] =
		{ 4, 4, 
			4, 4 
		};

	/// Create Integer Transformers and their inverses to convert the test values.
	IForwardTransform* pF4x4T = new FastForward4x4ITImpl1();
	pF4x4T->SetMode(IForwardTransform::TransformOnly);

	IForwardTransform* pFDC4x4T = new FastForwardDC4x4ITImpl1();
	pFDC4x4T->SetMode(IForwardTransform::TransformAndQuant);

	IForwardTransform* pFDC2x2T = new FastForwardDC2x2ITImpl1();
	pFDC2x2T->SetMode(IForwardTransform::TransformAndQuant);

	/// Create an inverse IT to use and to compare with.
	IInverseTransform* pI4x4T = new FastInverse4x4ITImpl1();
	pI4x4T->SetMode(IInverseTransform::TransformOnly);

	IInverseTransform* pIDC4x4T = new FastInverseDC4x4ITImpl1();
	pIDC4x4T->SetMode(IInverseTransform::TransformAndQuant);
	pIDC4x4T->SetScale((void *)dc4x4Scale);

	IInverseTransform* pIDC2x2T = new FastInverseDC2x2ITImpl1();
	pIDC2x2T->SetMode(IInverseTransform::TransformAndQuant);
	pIDC2x2T->SetScale((void *)dc2x2Scale);

	/// Use a bit stream reader/writer to code to and from.
	int bitSize = H264V2C_BYTESIZE * 8;
	unsigned char stream[H264V2C_BYTESIZE];
	IBitStreamWriter* pBsw = new BitStreamWriterMSB();
	pBsw->SetStream((void *)stream, bitSize);
	IBitStreamReader* pBsr = new BitStreamReaderMSB();
	pBsr->SetStream((void *)stream, bitSize);

	/// Create the vlc encoders and decoders for use with CAVLC.
	IVlcEncoder*	pPrefixVlcEnc					= new PrefixH264VlcEncoderImpl1();
	IVlcDecoder*	pPrefixVlcDec					= new PrefixH264VlcDecoderImpl1();
	IVlcEncoder*	pCoeffTokenVlcEnc			= new CoeffTokenH264VlcEncoder();
	IVlcDecoder*	pCoeffTokenVlcDec			= new CoeffTokenH264VlcDecoder();
	IVlcEncoder*	pTotalZeros4x4VlcEnc	= new TotalZeros4x4H264VlcEncoder();
	IVlcDecoder*	pTotalZeros4x4VlcDec	= new TotalZeros4x4H264VlcDecoder();
	IVlcEncoder*	pTotalZeros2x2VlcEnc	= new TotalZeros2x2H264VlcEncoder();
	IVlcDecoder*	pTotalZeros2x2VlcDec	= new TotalZeros2x2H264VlcDecoder();
	IVlcEncoder*	pRunBeforeVlcEnc			= new RunBeforeH264VlcEncoder();
	IVlcDecoder*	pRunBeforeVlcDec			= new RunBeforeH264VlcDecoder();

	/// Create a CAVLC for each type.
	IContextAwareRunLevelCodec* pCAVLC4x4 = new CAVLCH264Impl();
	IContextAwareRunLevelCodec* pCAVLC2x2 = new CAVLCH264Impl();
	/// Attach the vlc encoders and decoders excluding the total zeros vlc.
	pCAVLC4x4->SetMode(CAVLCH264Impl::Mode4x4);
	((CAVLCH264Impl *)pCAVLC4x4)->SetTokenCoeffVlcEncoder(pCoeffTokenVlcEnc);
	((CAVLCH264Impl *)pCAVLC4x4)->SetTokenCoeffVlcDecoder(pCoeffTokenVlcDec);
	((CAVLCH264Impl *)pCAVLC4x4)->SetPrefixVlcEncoder(pPrefixVlcEnc);
	((CAVLCH264Impl *)pCAVLC4x4)->SetPrefixVlcDecoder(pPrefixVlcDec);
	((CAVLCH264Impl *)pCAVLC4x4)->SetRunBeforeVlcEncoder(pRunBeforeVlcEnc);
	((CAVLCH264Impl *)pCAVLC4x4)->SetRunBeforeVlcDecoder(pRunBeforeVlcDec);
	((CAVLCH264Impl *)pCAVLC4x4)->SetTotalZerosVlcEncoder(pTotalZeros4x4VlcEnc);
	((CAVLCH264Impl *)pCAVLC4x4)->SetTotalZerosVlcDecoder(pTotalZeros4x4VlcDec);

	pCAVLC2x2->SetMode(CAVLCH264Impl::Mode2x2);
	((CAVLCH264Impl *)pCAVLC2x2)->SetTokenCoeffVlcEncoder(pCoeffTokenVlcEnc);
	((CAVLCH264Impl *)pCAVLC2x2)->SetTokenCoeffVlcDecoder(pCoeffTokenVlcDec);
	((CAVLCH264Impl *)pCAVLC2x2)->SetPrefixVlcEncoder(pPrefixVlcEnc);
	((CAVLCH264Impl *)pCAVLC2x2)->SetPrefixVlcDecoder(pPrefixVlcDec);
	((CAVLCH264Impl *)pCAVLC2x2)->SetRunBeforeVlcEncoder(pRunBeforeVlcEnc);
	((CAVLCH264Impl *)pCAVLC2x2)->SetRunBeforeVlcDecoder(pRunBeforeVlcDec);
	((CAVLCH264Impl *)pCAVLC2x2)->SetTotalZerosVlcEncoder(pTotalZeros2x2VlcEnc);
	((CAVLCH264Impl *)pCAVLC2x2)->SetTotalZerosVlcDecoder(pTotalZeros2x2VlcDec);

	/// Vlc encoder and decoder for the coded block pattern.
	IVlcEncoder*	pBlkPattVlcEnc	= new CodedBlkPatternH264VlcEncoder();
	IVlcDecoder*	pBlkPattVlcDec	= new CodedBlkPatternH264VlcDecoder();

	/// Vlc encoder and decoder for the delta QP.
	IVlcEncoder*	pDeltaQPVlcEnc	= new ExpGolombSignedVlcEncoder();
	IVlcDecoder*	pDeltaQPVlcDec	= new ExpGolombSignedVlcDecoder();

	/// Vlc encoder and decoder for the macroblock type.
	IVlcEncoder*	pMbTypeVlcEnc	= new ExpGolombUnsignedVlcEncoder();
	IVlcDecoder*	pMbTypeVlcDec	= new ExpGolombUnsignedVlcDecoder();

	/// Vlc encoder and decoder for intra chr pred mode. ExpGolomb codecs
	// are stateless therefore they can be reused.
	IVlcEncoder*	pMbIChrPredModeVlcEnc	= pMbTypeVlcEnc;
	IVlcDecoder*	pMbIChrPredModeVlcDec	= pMbTypeVlcDec;

	/// Vlc encoder and decoder for motion vector differences. ExpGolomb codecs
	// are stateless therefore they can be reused.
	IVlcEncoder*	pMbMotionVecDiffVlcEnc	= pDeltaQPVlcEnc;
	IVlcDecoder*	pMbMotionVecDiffVlcDec	= pDeltaQPVlcDec;

	/// Simulate an image of 3x3 macroblocks, Lum 48x48, Chr 2x24x24.
	short* pImg = new short[(48*48) + 2*(24*24)];	///< Hold the original to compare with.
	short* pLum = new short[(48*48) + 2*(24*24)];	///< Contiguous mem for Lum, Cb and Cr.
	short* pCb	= &(pLum[48*48]);									///< end of Lum.
	short* pCr  = &(pLum[(48*48)+(24*24)]);				///< end of Cb.
	/// Create macroblock size overlays for each image colour component.
	OverlayMem2Dv2* Lum = new OverlayMem2Dv2(pLum, 48, 48, 16, 16);
	OverlayMem2Dv2* Cb	= new OverlayMem2Dv2(pCb, 24, 24, 8, 8);
	OverlayMem2Dv2* Cr	= new OverlayMem2Dv2(pCr, 24, 24, 8, 8);
	/// Set a 2-D array of macroblocks.
	MacroBlockH264*		pMb	= new MacroBlockH264[3*3];
	MacroBlockH264**	Mb	= new MacroBlockH264*[3];	/// Macroblock row pointers.
	/// Set the row pointers.
	for(i = 0; i < 3; i++)
		Mb[i] = &(pMb[i*3]);

	/// Initialise all internal and neighbourhood members. Parameters = Mb[3][3] macroblocks
	///	where macroblock indecies 0..8 are in slice 0.
	MacroBlockH264::Initialise(3, 3, 0, 8, 0, Mb);

	srand(23995);
	for(test = 0; test < TESTS; test++)
	{
		/// Slice type: 0 = I_Slice, 1 = P_Slice.
		sliceType	= (int)( (((double)rand()/(double)RAND_MAX) * 1.0) + 0.5 );

		/// Random sliceQP value for all macroblocks mbQP = { 0...51}.
		//	sliceQP = (int)( (((double)rand()/(double)RAND_MAX) * 51.0) + 0.5 );
		sliceQP = test % 52;
		qStep		= (qfactor[sliceQP % 6] << (sliceQP / 6))/16;
		qErr		= qStep + (((sliceQP * 2)+2)/3);
		if(qErr < 1) qErr = 1;

		/// Simulate an image with YCbCr values at a mid point and then random
		/// deviations from it with a limited range for each test.
		/// ********** Construct a typical test sample image. Midpoint between -256..255.
		short midPoint = (short)( (((double)rand()/(double)RAND_MAX) * 511.0) - 256.0 + 0.5 );
		/// Deviation limit between 0..128.
		short devLimit = (short)( (((double)rand()/(double)RAND_MAX) * 128.0) + 0.5 );
		for(i = 0; i < (48*48); i++)	///< Lum and Chr.
		{
			int x = (short)( (((double)rand()/(double)RAND_MAX) * (double)devLimit) - ((double)devLimit/2.0) + 0.5 );
			x = x + midPoint;
			if(x > 255) x = 255;
			else if(x < -255) x = -255;
			pLum[i] = x;
			if(i < (24*24))	///< Chr sample.
			{
				/// Range is halved for Chr.
				x = (short)( (((double)rand()/(double)RAND_MAX) * ((double)devLimit/2.0)) - ((double)devLimit/4.0) + 0.5 );
				x = x + midPoint;
				if(x > 255) x = 255;
				else if(x < -255) x = -255;
				pCb[i] = x;
				x = (short)( (((double)rand()/(double)RAND_MAX) * ((double)devLimit/2.0)) - ((double)devLimit/4.0) + 0.5 );
				x = x + midPoint;
				if(x > 255) x = 255;
				else if(x < -255) x = -255;
				pCr[i] = x;
			}//end if Chr...
		}//end for i...
		/// Make an image copy.
		memcpy((void *)pImg, (const void *)pLum, sizeof(short) * ((48*48) + 2*(24*24)));

		/// ************** Encode ***********************************
		Lum->SetOverlayDim(4,4);
		Cb->SetOverlayDim(4,4);
		Cr->SetOverlayDim(4,4);

		pBsw->Reset();
		int totalEncBits = 0;

		for(y = 0; y < 3; y++)
			for(x = 0; x < 3; x++)
			{
				/// Set macroblock parameters to code to.
				if(sliceType == 0)	///< I-Slice has only intra macroblocks.
					Mb[y][x]._intraFlag = 1;
				else	///< Simulate with a random selection of macroblock coding type.
					Mb[y][x]._intraFlag	= (int)( (((double)rand()/(double)RAND_MAX) * 1.0) + 0.5 );

				/// Simulate a change in mbQP that can only be in the range {-26...+25} from the
				/// previous mbQP value. Set mbQP either side of SliceQP for each macroblock.
				int deltaQP	= (int)( (((double)rand()/(double)RAND_MAX) * 25.0) + 0.5 );
				Mb[y][x]._mbQP	= sliceQP - deltaQP;
				if(Mb[y][x]._mbQP < 0)
					Mb[y][x]._mbQP = 0;
				if(Mb[y][x]._mbQP > 51)
					Mb[y][x]._mbQP = 51;

				/// Respond to these settings for this macroblock.
				pF4x4T->SetParameter(IForwardTransform::QUANT_ID, Mb[y][x]._mbQP);
				pF4x4T->SetParameter(IForwardTransform::INTRA_FLAG_ID, Mb[y][x]._intraFlag);
				pFDC4x4T->SetParameter(IForwardTransform::QUANT_ID, Mb[y][x]._mbQP);
				pFDC4x4T->SetParameter(IForwardTransform::INTRA_FLAG_ID, 1);
				pFDC2x2T->SetParameter(IForwardTransform::QUANT_ID, MacroBlockH264::GetQPc(Mb[y][x]._mbQP));
				pFDC2x2T->SetParameter(IForwardTransform::INTRA_FLAG_ID, 1);

				/// --------------------- Prediction --------------------------------------------
				if(Mb[y][x]._intraFlag)
				{
					/// Simulate a random selection of the Intra prediction mode.
					Mb[y][x]._mbPartPredMode			= MacroBlockH264::Intra_16x16;	///< Currently only 16x16 implemented.
					Mb[y][x]._intra16x16PredMode	= (int)( (((double)rand()/(double)RAND_MAX) * 3.0) + 0.5 );
					Mb[y][x]._intraChrPredMode		= (int)( (((double)rand()/(double)RAND_MAX) * 3.0) + 0.5 );
				}//end if _intraFlag...
				else	//if inter
				{
					//Mb[y][x]._mbPartPredMode			= (int)( (((double)rand()/(double)RAND_MAX) * 4.0) + 0.5 );
					Mb[y][x]._mbPartPredMode = MacroBlockH264::Inter_16x16;	///< Fixed at 16x16 for now.
					/// The max range of motion vectors for the baseline profile in quarter pel units
					/// for a 352x288 picture size is x = [-8192, 8191], y = [-512, 511].
					Mb[y][x]._mvX[MacroBlockH264::_16x16]	= (int)( (((double)rand()/(double)RAND_MAX) * 16384.0) + 0.5 ) - 8192;
					Mb[y][x]._mvY[MacroBlockH264::_16x16]	= (int)( (((double)rand()/(double)RAND_MAX) * 1024.0) + 0.5 ) - 512;

					/// Get the predicted motion vector for this macroblock as the median of the neighbourhood vectors.
					int predX, predY;
					Mb[y][x].GetMbMotionPred(&(Mb[y][x]), &predX, &predY);
					Mb[y][x]._mvdX[MacroBlockH264::_16x16] = Mb[y][x]._mvX[MacroBlockH264::_16x16] - predX;
					Mb[y][x]._mvdY[MacroBlockH264::_16x16] = Mb[y][x]._mvY[MacroBlockH264::_16x16] - predY;
				}//end else...

				/// --------------------- Image Loading -----------------------------------------
				/// Fill all the non-DC 4x4 blks (Not blks = -1, 17, 18) of the macroblock blocks with 
				/// the image (difference) colour components after prediction.
				for(i = MBH264_LUM_0_0; i < MBH264_NUM_BLKS; i++)
				{
					BlockH264* pBlk = Mb[y][x]._blkParam[i].pBlk;

					if(!(pBlk->IsDc()) )	///< Ignore DC blocks.
					{
						/// Default to assuming a Lum block.
						OverlayMem2Dv2* img			= Lum;
						int							offX		= Mb[y][x]._offLumX + pBlk->_offX;
						int							offY		= Mb[y][x]._offLumY + pBlk->_offY;
						int							colour	= pBlk->GetColour();
						if(colour != BlockH264::LUM)	///< ...else is chr block.
						{
							offX = Mb[y][x]._offChrX + pBlk->_offX;
							offY = Mb[y][x]._offChrY + pBlk->_offY;
							if(colour == BlockH264::CB)
								img = Cb;
							else	///< BlockH264::CR
								img = Cr;
						}//end if colour...

						/// Align the macroblock block with the image space.
						img->SetOrigin(offX, offY);
						/// Read from the image into the block.
						img->Read(*(pBlk->GetBlkOverlay()));

					}//end if !Dc blk...
				}//end for i...

				/// --------------------- Transform & Quantisation ----------------------------------
				/// Do the forward 4x4 transform on the non-DC 4x4 blocks without scaling or quantisation. Pull
				/// out the DC terms from each block and populate the DC blocks and then scale and quant the
				/// non-DC 4x4 blocks. Note that this is done in raster scan order and not coding order to
				/// align each position in the DC block with the spatial location of the 4x4 block in the image.

				for(i = 0; i < 4; i++)
					for(j = 0; j < 4; j++)
					{
						/// Transform without scaling and quant.
						Mb[y][x]._lumBlk[i][j].ForwardTransform(pF4x4T);
						/// Set the Lum DC block from the DC term of this block.
						if( Mb[y][x]._mbPartPredMode == MacroBlockH264::Intra_16x16 )
						{
							(Mb[y][x]._lumDcBlk.GetBlk())[4*i + j] = Mb[y][x]._lumBlk[i][j].GetDC();
							Mb[y][x]._lumBlk[i][j].SetDC(0);
						}//end if Intra_16x16...
						/// Now scale and quant.
						Mb[y][x]._lumBlk[i][j].Quantise(pF4x4T, Mb[y][x]._mbQP);

						if( (i < 2)&&(j < 2))	///< Chr components.
						{
							Mb[y][x]._cbBlk[i][j].ForwardTransform(pF4x4T);
							(Mb[y][x]._cbDcBlk.GetBlk())[2*i + j] = Mb[y][x]._cbBlk[i][j].GetDC();
							Mb[y][x]._cbBlk[i][j].SetDC(0);
							Mb[y][x]._cbBlk[i][j].Quantise(pF4x4T, MacroBlockH264::GetQPc(Mb[y][x]._mbQP));
							Mb[y][x]._crBlk[i][j].ForwardTransform(pF4x4T);
							(Mb[y][x]._crDcBlk.GetBlk())[2*i + j] = Mb[y][x]._crBlk[i][j].GetDC();
							Mb[y][x]._crBlk[i][j].SetDC(0);
							Mb[y][x]._crBlk[i][j].Quantise(pF4x4T, MacroBlockH264::GetQPc(Mb[y][x]._mbQP));
						}//end if Chr...
					}//end for i & j...

				/// Transform and quant the DC blocks.
				if( Mb[y][x]._mbPartPredMode == MacroBlockH264::Intra_16x16 )
					Mb[y][x]._lumDcBlk.ForwardTransform(pFDC4x4T);

				Mb[y][x]._cbDcBlk.ForwardTransform(pFDC2x2T);
				Mb[y][x]._crDcBlk.ForwardTransform(pFDC2x2T);

				/// Determine the coded Lum and Chr patterns. The _codedBlkPatternLum, _codedBlkPatternChr 
				/// and _coded_blk_pattern members are set.
				Mb[y][x].SetCodedBlockPattern(&(Mb[y][x]));

				/// Determine the delta quantisation parameter.
				// TODO: Cater for previous macroblocks that are skipped.
				int prevMbIdx = Mb[y][x]._mbIndex - 1;
				if(prevMbIdx >= 0)	///< Previous macroblock is within the image boundaries.
				{
					if(Mb[y][x]._slice == pMb[prevMbIdx]._slice)	/// Previous macroblock within same slice.
						Mb[y][x]._mb_qp_delta = Mb[y][x]._mbQP - pMb[prevMbIdx]._mbQP;
					else
						Mb[y][x]._mb_qp_delta = Mb[y][x]._mbQP - sliceQP;
				}//end if prevMbIdx...
				else
					Mb[y][x]._mb_qp_delta = Mb[y][x]._mbQP - sliceQP;

				/// Determine the macroblock type. From the prediction modes and patterns set the 
				/// _mb_type member.
				Mb[y][x].SetType(&(Mb[y][x]), sliceType);

				/// Store macroblock members for later comparison.
				H264V2C_ParamSet[y][x][H264V2C_MbType]			= Mb[y][x]._mb_type;
				H264V2C_ParamSet[y][x][H264V2C_QP]					= Mb[y][x]._mbQP;
				H264V2C_ParamSet[y][x][H264V2C_IChrPedMode]	= Mb[y][x]._intraChrPredMode;
				H264V2C_ParamSet[y][x][H264V2C_MVx]					= Mb[y][x]._mvX[MacroBlockH264::_16x16];
				H264V2C_ParamSet[y][x][H264V2C_MVy]					= Mb[y][x]._mvY[MacroBlockH264::_16x16];

				/// --------------------- VLC Encoding -----------------------------------------
				/// Encode the vlc blocks with the context of the neighborhood number of coeffs. Map
				/// to the required H.264 coding order of each block when writing to the stream.

				/// Code the macroblock header.

				/// Macroblock type.
				ret = pMbTypeVlcEnc->Encode(Mb[y][x]._mb_type);
				pBsw->Write(ret, pMbTypeVlcEnc->GetCode());
				totalEncBits += ret;

				/// Intra requires lum and chr prediction modes, Inter requires reference 
				/// index lists and motion vector diff values.
				if(!Mb[y][x]._intraFlag)	/// Inter	(most common option)
				{
					int numOfVecs = 1;
					if(Mb[y][x]._mbPartPredMode == MacroBlockH264::Inter_16x16)
						numOfVecs = 1;
					for(int vec = 0; vec < numOfVecs; vec++)
					{
						ret = pMbMotionVecDiffVlcEnc->Encode(Mb[y][x]._mvdX[vec]);
						pBsw->Write(ret, pMbMotionVecDiffVlcEnc->GetCode());
						totalEncBits += ret;

						ret = pMbMotionVecDiffVlcEnc->Encode(Mb[y][x]._mvdY[vec]);
						pBsw->Write(ret, pMbMotionVecDiffVlcEnc->GetCode());
						totalEncBits += ret;
					}//end for vec...
				}//end if !_interFlag...
				else											/// Intra
				{
					// TODO: Implement Intra_8x8 and Intra_4x4 mode options.

					/// Write chr prediction mode.
					ret = pMbIChrPredModeVlcEnc->Encode(Mb[y][x]._intraChrPredMode);
					pBsw->Write(ret, pMbIChrPredModeVlcEnc->GetCode());
					totalEncBits += ret;
				}//end else...

				/// If not Intra_16x16 mode then _coded_blk_pattern must be written.
				if( Mb[y][x]._mbPartPredMode != MacroBlockH264::Intra_16x16 )
				{
					/// Block coded pattern.
					int isInter = 1;
					if(Mb[y][x]._intraFlag) isInter = 0;
					ret = pBlkPattVlcEnc->Encode2(Mb[y][x]._coded_blk_pattern, isInter);
					pBsw->Write(ret, pBlkPattVlcEnc->GetCode());
					totalEncBits += ret;
				}//end if !Intra_16x16...

				/// Delta QP.
				ret = pDeltaQPVlcEnc->Encode(Mb[y][x]._mb_qp_delta);
				pBsw->Write(ret, pDeltaQPVlcEnc->GetCode());
				totalEncBits += ret;

				int dcSkip	 = 0;
				int startBlk = 1;
				if( Mb[y][x]._mbPartPredMode == MacroBlockH264::Intra_16x16 )
				{
					startBlk = 0;	///< Change starting block to include block num = -1;
					dcSkip	 = 1;
				}//end if Intra_16x16...
				/// For Lum blocks that are Inter coded or not Intra_16x16 coded, the DC coeff is not skipped.
				for(i = MBH264_LUM_0_0; i <= MBH264_LUM_3_3; i++)
					Mb[y][x]._blkParam[i].dcSkipFlag = dcSkip;

				for(i = startBlk; i < MBH264_NUM_BLKS; i++)
				{
					if(Mb[y][x]._blkParam[i].pBlk->IsCoded())
					{
						/// Choose the appropriate dimension CAVLC codec. Only 4x4 or 2x2 are expected here.
						IContextAwareRunLevelCodec* pCAVLC = pCAVLC2x2;
						if( ((Mb[y][x]._blkParam[i].pBlk)->GetHeight() == 4) && ((Mb[y][x]._blkParam[i].pBlk)->GetWidth() == 4) )
							pCAVLC = pCAVLC4x4;

						/// Get num of neighbourhood coeffs as average of above and left block coeffs. Previous
						/// MB encodings in raster order have already set the num of coeffs.
						int neighCoeffs = 0;
						if(Mb[y][x]._blkParam[i].neighbourIndicator)
						{
							if(Mb[y][x]._blkParam[i].neighbourIndicator > 0)
								neighCoeffs = BlockH264::GetNumNeighbourCoeffs(Mb[y][x]._blkParam[i].pBlk);
							else	///< Negative values for neighbourIndicator imply pass through.
								neighCoeffs = Mb[y][x]._blkParam[i].neighbourIndicator;
						}//end if neighbourIndicator...
						pCAVLC->SetParameter(pCAVLC->NUM_TOT_NEIGHBOR_COEFF_ID, neighCoeffs);	///< Prepare the vlc coder.
						pCAVLC->SetParameter(pCAVLC->DC_SKIP_FLAG_ID, Mb[y][x]._blkParam[i].dcSkipFlag);

						ret = (Mb[y][x]._blkParam[i].pBlk)->RleEncode(pCAVLC, pBsw);					///< Vlc encode and add to stream.
						if(ret <= 0)	///< Vlc codec errors are detected from a negative return value.
							result = 0;
						else totalEncBits += ret;
					}//end if IsCoded()...
					else
						Mb[y][x]._blkParam[i].pBlk->SetNumCoeffs(0);	///< For future use.
				}//end for i...

			}//end for y & x...

		/// *********** Clear all **********************************************
		memset((void *)pLum, 0, sizeof(short) * ((48*48) + 2*(24*24)));

		for(y = 0; y < 3; y++)
			for(x = 0; x < 3; x++)
			{
				for(i = 0; i < 4; i++)
					for(j = 0; j < 4; j++)
					{
						Mb[y][x]._lumBlk[i][j].Zero();
						if((i < 2)&&(j < 2))
						{
							Mb[y][x]._cbBlk[i][j].Zero();
							Mb[y][x]._crBlk[i][j].Zero();
						}//end if i...
					}//end for i & j...
				Mb[y][x]._lumDcBlk.Zero();
				Mb[y][x]._cbDcBlk.Zero();
				Mb[y][x]._crDcBlk.Zero();
				Mb[y][x]._coded_blk_pattern						= 0;
				Mb[y][x]._mb_qp_delta									= 0;
				Mb[y][x]._mb_type											= 0;
				Mb[y][x]._intra16x16PredMode					= 0;
				Mb[y][x]._intraChrPredMode						= 0;
				Mb[y][x]._mvX[MacroBlockH264::_16x16] = 0;
				Mb[y][x]._mvY[MacroBlockH264::_16x16] = 0;
				Mb[y][x]._mvdX[MacroBlockH264::_16x16] = 0;
				Mb[y][x]._mvdY[MacroBlockH264::_16x16] = 0;
			}//end for y & x...

		/// *********** Decode **************************************************

		/// Decode the vlc blocks with the context of the neighborhood number of coeffs.
		pBsr->Reset();
		int totalDecBits = 0;

		for(y = 0; y < 3; y++)
			for(x = 0; x < 3; x++)
			{
				/// --------------------- VLC Decoding -----------------------------------------
				/// Decode the macroblock header from the stream.

				/// Macroblock type.
				Mb[y][x]._mb_type = pMbTypeVlcDec->Decode(pBsr);
				ret = pMbTypeVlcDec->GetNumDecodedBits();
				totalDecBits += ret;
				/// Unpack _intraFlag and _mbPartPredMode from _mb_type.
				Mb[y][x].UnpackMbType(&(Mb[y][x]), sliceType);

				/// Intra requires lum and chr prediction modes, Inter requires reference 
				/// index lists and motion vector diff values.
				if(!Mb[y][x]._intraFlag)	/// Inter	(most common option)
				{
					int numOfVecs = 1;
					if(Mb[y][x]._mbPartPredMode == MacroBlockH264::Inter_16x16)
						numOfVecs = 1;
					for(int vec = 0; vec < numOfVecs; vec++)
					{
						/// Get the motion vector differences for this macroblock.
						Mb[y][x]._mvdX[vec] = pMbMotionVecDiffVlcDec->Decode(pBsr);
						totalDecBits += pMbMotionVecDiffVlcDec->GetNumDecodedBits();
						Mb[y][x]._mvdY[vec] = pMbMotionVecDiffVlcDec->Decode(pBsr);
						totalDecBits += pMbMotionVecDiffVlcDec->GetNumDecodedBits();
						/// Get the prediction vector from the neighbourhood.
						int predX, predY;
						Mb[y][x].GetMbMotionPred(&(Mb[y][x]), &predX, &predY);
						Mb[y][x]._mvX[vec] = predX + Mb[y][x]._mvdX[vec];
						Mb[y][x]._mvY[vec] = predY + Mb[y][x]._mvdY[vec];
					}//end for vec...
				}//end if !_interFlag...
				else											/// Intra
				{
					// TODO: Implement Intra_8x8 and Intra_4x4 mode options.

					/// Get chr prediction mode.
					Mb[y][x]._intraChrPredMode = pMbIChrPredModeVlcDec->Decode(pBsr);
					ret = pMbIChrPredModeVlcDec->GetNumDecodedBits();
					totalDecBits += ret;
				}//end else...

				/// If not Intra_16x16 mode then _coded_blk_pattern must be extracted.
				if( Mb[y][x]._mbPartPredMode != MacroBlockH264::Intra_16x16 )
				{
					/// Block coded pattern.
					int isInter = 1;
					if(Mb[y][x]._intraFlag) isInter = 0;
					ret = pBlkPattVlcDec->Decode2(pBsr, &(Mb[y][x]._coded_blk_pattern), &isInter);
					totalDecBits += ret;
				}//end if !Intra_16x16...

				/// Disassemble the bit pattern to determine which blocks are to be decoded.
				Mb[y][x].GetCodedBlockPattern(&(Mb[y][x]));

				/// Delta QP.
				Mb[y][x]._mb_qp_delta = pDeltaQPVlcDec->Decode(pBsr);
				ret = pDeltaQPVlcDec->GetNumDecodedBits();
				totalDecBits += ret;
				int prevMbIdx = Mb[y][x]._mbIndex - 1;
				if(prevMbIdx >= 0)	///< Previous macroblock is within the image boundaries.
				{
					if(Mb[y][x]._slice == pMb[prevMbIdx]._slice)	/// Previous macroblock within same slice.
						Mb[y][x]._mbQP = pMb[prevMbIdx]._mbQP + Mb[y][x]._mb_qp_delta;
					else
						Mb[y][x]._mbQP = sliceQP + Mb[y][x]._mb_qp_delta;
				}//end if prevMbIdx...
				else
					Mb[y][x]._mbQP = sliceQP + Mb[y][x]._mb_qp_delta;

				/// Respond to these header settings for this macroblock.
				pI4x4T->SetParameter(IInverseTransform::QUANT_ID, Mb[y][x]._mbQP);
				pIDC4x4T->SetParameter(IInverseTransform::QUANT_ID, Mb[y][x]._mbQP);
				pIDC2x2T->SetParameter(IInverseTransform::QUANT_ID, MacroBlockH264::GetQPc(Mb[y][x]._mbQP));

				/// Get the macroblock coeffs from the coded stream.
				int dcSkip	 = 0;
				int startBlk = 1;
				if( Mb[y][x]._mbPartPredMode == MacroBlockH264::Intra_16x16 )
				{
					startBlk = 0;	///< Change starting block to include block num = -1;
					dcSkip	 = 1;
				}//end if Intra_16x16...
				/// For Lum blocks that are Inter coded and not Intra_16x16 coded the DC coeff is not skipped.
				for(i = MBH264_LUM_0_0; i <= MBH264_LUM_3_3; i++)
					Mb[y][x]._blkParam[i].dcSkipFlag = dcSkip;

				for(i = startBlk; i < MBH264_NUM_BLKS; i++)
				{
					if(Mb[y][x]._blkParam[i].pBlk->IsCoded())
					{
						/// Choose the appropriate dimension CAVLC codec.
						IContextAwareRunLevelCodec* pCAVLC = pCAVLC2x2;
						if( ((Mb[y][x]._blkParam[i].pBlk)->GetHeight() == 4) && ((Mb[y][x]._blkParam[i].pBlk)->GetWidth() == 4) )
							pCAVLC = pCAVLC4x4;

						/// Get num of neighbourhood coeffs as average of above and left block coeffs. Previous
						/// MB decodings in decoding order have already set the num of neighbourhood coeffs.
						int neighCoeffs = 0;
						if(Mb[y][x]._blkParam[i].neighbourIndicator)
						{
							if(Mb[y][x]._blkParam[i].neighbourIndicator > 0)
								neighCoeffs = BlockH264::GetNumNeighbourCoeffs(Mb[y][x]._blkParam[i].pBlk);
							else	///< Negative values for neighbourIndicator imply pass through.
								neighCoeffs = Mb[y][x]._blkParam[i].neighbourIndicator;
						}//end if neighbourIndicator...
						pCAVLC->SetParameter(pCAVLC->NUM_TOT_NEIGHBOR_COEFF_ID, neighCoeffs);	///< Prepare the vlc coder.
						pCAVLC->SetParameter(pCAVLC->DC_SKIP_FLAG_ID, Mb[y][x]._blkParam[i].dcSkipFlag);

						ret = (Mb[y][x]._blkParam[i].pBlk)->RleDecode(pCAVLC, pBsr);					///< Vlc decode from the stream.
						if(ret <= 0)	///< Vlc codec errors are detected from a negative return value.
							result = 0;
						else totalDecBits += ret;
					}//end if IsCoded()...
					else
					{
						Mb[y][x]._blkParam[i].pBlk->SetNumCoeffs(0);	///< For future use.
						Mb[y][x]._blkParam[i].pBlk->Zero();
					}//end else...
				}//end for i...

				/// --------------------- Inverse Transform & Quantisation --------------------------
				/// Inverse transform & inverse quantise Lum DC components.
				if( Mb[y][x]._mbPartPredMode == MacroBlockH264::Intra_16x16 )
					Mb[y][x]._lumDcBlk.InverseTransform(pIDC4x4T);
				/// Inverse transform & inverse quantise Chr DC components.
				Mb[y][x]._cbDcBlk.InverseTransform(pIDC2x2T);
				Mb[y][x]._crDcBlk.InverseTransform(pIDC2x2T);

				/// Inverse quant for the main Lum, Cb and Cr blocks, insert the DC terms from the 
				/// DC blocks and then complete the inverse transform process.
				for(i = 0; i < 4; i++)	///< 16 Lum blocks + 4 Cb Chr and 4 Cr Chr blocks.
					for(j = 0; j < 4; j++)
					{
						/// Lum.
						Mb[y][x]._lumBlk[i][j].InverseQuantise(pI4x4T, Mb[y][x]._mbQP);
						/// Put the Lum DC components in position with a 4x scale factor. Not sure why
						/// this scale factor is required in the InverseQuantise() method yet.
						if( Mb[y][x]._mbPartPredMode == MacroBlockH264::Intra_16x16 )
							Mb[y][x]._lumBlk[i][j].SetDC((Mb[y][x]._lumDcBlk.GetBlk())[4*i + j]);
						/// Inverse transform.
						Mb[y][x]._lumBlk[i][j].InverseTransform(pI4x4T);

						if((i < 2)&&(j < 2))	///< Chr.
						{
							Mb[y][x]._cbBlk[i][j].InverseQuantise(pI4x4T, MacroBlockH264::GetQPc(Mb[y][x]._mbQP));
							Mb[y][x]._cbBlk[i][j].SetDC((Mb[y][x]._cbDcBlk.GetBlk())[2*i + j]);
							Mb[y][x]._cbBlk[i][j].InverseTransform(pI4x4T);
							Mb[y][x]._crBlk[i][j].InverseQuantise(pI4x4T, MacroBlockH264::GetQPc(Mb[y][x]._mbQP));
							Mb[y][x]._crBlk[i][j].SetDC((Mb[y][x]._crDcBlk.GetBlk())[2*i + j]);
							Mb[y][x]._crBlk[i][j].InverseTransform(pI4x4T);
						}//end if i...
					}//end for i & j...

				/// --------------------- Image Storing -----------------------------------------
				/// Fill the image (difference) colour components from all the non-DC 4x4 
				/// blks (i.e. Not blks = -1, 17, 18) of the macroblock blocks. 
				for(i = MBH264_LUM_0_0; i < MBH264_NUM_BLKS; i++)
				{
					BlockH264* pBlk = Mb[y][x]._blkParam[i].pBlk;

					if(!(pBlk->IsDc()) )	///< Ignore DC blocks.
					{
						/// Default to assuming a Lum block.
						OverlayMem2Dv2* img			= Lum;
						int							offX		= Mb[y][x]._offLumX + pBlk->_offX;
						int							offY		= Mb[y][x]._offLumY + pBlk->_offY;
						int							colour	= pBlk->GetColour();
						if(colour != BlockH264::LUM)	///< ...else is chr block.
						{
							offX = Mb[y][x]._offChrX + pBlk->_offX;
							offY = Mb[y][x]._offChrY + pBlk->_offY;
							if(colour == BlockH264::CB)
								img = Cb;
							else	///< BlockH264::CR
								img = Cr;
						}//end if colour...

						/// Align the macroblock block with the image space.
						img->SetOrigin(offX, offY);
						/// Write the block into the image space.
						img->Write(*(pBlk->GetBlkOverlay()));

					}//end if !Dc blk...
				}//end for i...

			}//end for y & x...

		/// ************ Evaluate **************************************************
		for(i = 0; i < ((48*48) + 2*(24*24)); i++)
		{
			if( abs(pLum[i] - pImg[i]) > qErr )	///< Allow rounding and quant error.
				result = 0;
		}//end for i...
		for(y = 0; y < 3; y++)
			for(x = 0; x < 3; x++)
			{
				if( (Mb[y][x]._mb_type != H264V2C_ParamSet[y][x][H264V2C_MbType]) ||
					  (Mb[y][x]._mbQP != H264V2C_ParamSet[y][x][H264V2C_QP]) ||
						((Mb[y][x]._intraFlag) && (Mb[y][x]._intraChrPredMode != H264V2C_ParamSet[y][x][H264V2C_IChrPedMode])) || 
						((!Mb[y][x]._intraFlag) && (Mb[y][x]._mvX[MacroBlockH264::_16x16] != H264V2C_ParamSet[y][x][H264V2C_MVx])
																		&& (Mb[y][x]._mvY[MacroBlockH264::_16x16] != H264V2C_ParamSet[y][x][H264V2C_MVy])) ) 
				result = 0;
			}//end for y & x...

	}//end for test...

	/// Release all mem variables.
	delete[] Mb;
	delete[] pMb;
	delete Cr;			///< Delete macroblock overlays.
	delete Cb;
	delete Lum;
	delete[] pLum;
	delete[] pImg;

	/// Chuck out everything in reverse order of instantiation.
	delete pMbTypeVlcDec;
	delete pMbTypeVlcEnc;
	delete pDeltaQPVlcDec;
	delete pDeltaQPVlcEnc;
	delete pBlkPattVlcDec;
	delete pBlkPattVlcEnc;
	delete pCAVLC2x2;
	delete pCAVLC4x4;
	delete pPrefixVlcEnc;
	delete pPrefixVlcDec;
	delete pCoeffTokenVlcEnc;
	delete pCoeffTokenVlcDec;
	delete pTotalZeros4x4VlcEnc;
	delete pTotalZeros4x4VlcDec;
	delete pTotalZeros2x2VlcEnc;
	delete pTotalZeros2x2VlcDec;
	delete pRunBeforeVlcEnc;
	delete pRunBeforeVlcDec;
	delete pBsw;
	delete pBsr;
	delete pIDC2x2T;
	delete pIDC4x4T;
	delete pI4x4T;
	delete pFDC2x2T;
	delete pFDC4x4T;
	delete pF4x4T;

	return(result);
}//end TestMacroBlockLayer.

