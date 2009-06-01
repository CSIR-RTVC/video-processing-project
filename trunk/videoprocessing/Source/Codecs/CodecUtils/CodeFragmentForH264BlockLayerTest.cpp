/** @file

MODULE				: CodeFragmentForH264BlockLayerTest

TAG						: CFH264BLT

FILE NAME			: CodeFragmentForH264BlockLayerTest.cpp

DESCRIPTION		: A code fragment of the unit tests for the CAVLC codec and the
								H.264 block layer defined in ITU-T Rec. H.264 (03/2005). This
								will not compile as all #include header files are excluded.

COPYRIGHT			: (c)CSIR 2007-2010 all rights resevered

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

/// There are two test functions defined in this fragment.
/// TestCAVLC() and TestBlockLayer().

/** Test CAVLC.
@return	: 1 = success, 0 = failed..
*/
#define H264V2C_BYTESIZE 4096
int H264v2Codec::TestCAVLC(void)
{
	int i,j,k;
	int TESTS = 4096;
	int result = 1;
	short a[16] = { 0, 3,-1, 0, 
									0,-1, 1, 0,
									1, 0, 0, 0,
									0, 0, 0, 0};
	short b[16];

	/// Use a bit stream reader/writer.
	int bitSize = H264V2C_BYTESIZE * 8;
	unsigned char stream[H264V2C_BYTESIZE];
	IBitStreamWriter* pBsw = new BitStreamWriterMSB();
	pBsw->SetStream((void *)stream, bitSize);
	IBitStreamReader* pBsr = new BitStreamReaderMSB();
	pBsr->SetStream((void *)stream, bitSize);

	/// Test the associated Vlc encoders & decoders used in CAVLC.

	/// 1. Prefix encoding & decoding.
	IVlcEncoder*	pPrefixVlcEnc = new PrefixH264VlcEncoderImpl1();
	IVlcDecoder*	pPrefixVlcDec = new PrefixH264VlcDecoderImpl1();
	for( i = 0; i < 32; i++)	///< Encode.
	{
		int numCodedBits = pPrefixVlcEnc->Encode(i);
		pBsw->Write(numCodedBits, pPrefixVlcEnc->GetCode());
	}//end for i...
	for( i = 0; i < 32; i++)	///< Decode.
	{
		int code = pPrefixVlcDec->Decode(pBsr);
		if(code != i)
			result = 0;
	}//end for i...

	/// 2. Coeff Token encoding & decoding.
	IVlcEncoder*	pCoeffTokenVlcEnc = new CoeffTokenH264VlcEncoder();
	IVlcDecoder*	pCoeffTokenVlcDec = new CoeffTokenH264VlcDecoder();
	int nC, tO, tC;
	pBsw->Reset();
	for(nC = 0; nC <= 9; nC++)	///< Encode.
	{
		for(tO = 0; tO <= 3; tO++)
		{
			for(tC = tO; tC <= 16; tC++)
			{
				int numCodedBits = pCoeffTokenVlcEnc->Encode3(tC, tO, nC);
				pBsw->Write(numCodedBits, pCoeffTokenVlcEnc->GetCode());
			}//end for tC...
		}//end for tO...
	}//end for nC...
	nC = -1;
	for(tO = 0; tO <= 3; tO++)
	{
		for(tC = tO; tC <= 4; tC++)
		{
			int numCodedBits = pCoeffTokenVlcEnc->Encode3(tC, tO, nC);
			pBsw->Write(numCodedBits, pCoeffTokenVlcEnc->GetCode());
		}//end for tC...
	}//end for tO...
	nC = -2;
	for(tO = 0; tO <= 3; tO++)
	{
		for(tC = tO; tC <= 8; tC++)
		{
			int numCodedBits = pCoeffTokenVlcEnc->Encode3(tC, tO, nC);
			pBsw->Write(numCodedBits, pCoeffTokenVlcEnc->GetCode());
		}//end for tC...
	}//end for tO...

	pBsr->Reset();
	for(nC = 0; nC <= 9; nC++)	///< Decode.
	{
		for(tO = 0; tO <= 3; tO++)
		{
			for(tC = tO; tC <= 16; tC++)
			{
				int trailingOnes	= 0;
				int totalCoeffs		= 0;
				int numCodedBits = pCoeffTokenVlcDec->Decode3(pBsr, &totalCoeffs, &trailingOnes, &nC);
				if( (tC != totalCoeffs)||(tO != trailingOnes) )
					result = 0;
			}//end for tC...
		}//end for tO...
	}//end for nC...
	nC = -1;
	for(tO = 0; tO <= 3; tO++)
	{
		for(tC = tO; tC <= 4; tC++)
		{
			int trailingOnes	= 0;
			int totalCoeffs		= 0;
			int numCodedBits = pCoeffTokenVlcDec->Decode3(pBsr, &totalCoeffs, &trailingOnes, &nC);
			if( (tC != totalCoeffs)||(tO != trailingOnes) )
				result = 0;
		}//end for tC...
	}//end for tO...
	nC = -2;
	for(tO = 0; tO <= 3; tO++)
	{
		for(tC = tO; tC <= 8; tC++)
		{
			int trailingOnes	= 0;
			int totalCoeffs		= 0;
			int numCodedBits = pCoeffTokenVlcDec->Decode3(pBsr, &totalCoeffs, &trailingOnes, &nC);
			if( (tC != totalCoeffs)||(tO != trailingOnes) )
				result = 0;
		}//end for tC...
	}//end for tO...

	/// 3. Total Zeros encoding & decoding.
	IVlcEncoder*	pTotalZeros4x4VlcEnc = new TotalZeros4x4H264VlcEncoder();
	IVlcDecoder*	pTotalZeros4x4VlcDec = new TotalZeros4x4H264VlcDecoder();
	int tZ;
	pBsw->Reset();
	for(tC = 1; tC <= 15; tC++)	///< Encode.
	{
		for(tZ = 0; tZ <= (16-tC); tZ++)
		{
			int numCodedBits = pTotalZeros4x4VlcEnc->Encode2(tZ, tC);
			pBsw->Write(numCodedBits, pTotalZeros4x4VlcEnc->GetCode());
		}//end for tZ...
	}//end for tC...

	pBsr->Reset();
	for(tC = 1; tC <= 15; tC++)	///< Decode.
	{
		for(tZ = 0; tZ <= (16-tC); tZ++)
		{
			int totalZeros = 0;
			int numCodedBits = pTotalZeros4x4VlcDec->Decode2(pBsr, &totalZeros, &tC);
			if(totalZeros != tZ)
				result = 0;
		}//end for tZ...
	}//end for tC...

	IVlcEncoder*	pTotalZeros2x2VlcEnc = new TotalZeros2x2H264VlcEncoder();
	IVlcDecoder*	pTotalZeros2x2VlcDec = new TotalZeros2x2H264VlcDecoder();
	pBsw->Reset();
	for(tC = 1; tC <= 3; tC++)	///< Encode.
	{
		for(tZ = 0; tZ <= (4-tC); tZ++)
		{
			int numCodedBits = pTotalZeros2x2VlcEnc->Encode2(tZ, tC);
			pBsw->Write(numCodedBits, pTotalZeros2x2VlcEnc->GetCode());
		}//end for tZ...
	}//end for tC...

	pBsr->Reset();
	for(tC = 1; tC <= 3; tC++)	///< Decode.
	{
		for(tZ = 0; tZ <= (4-tC); tZ++)
		{
			int totalZeros = 0;
			int numCodedBits = pTotalZeros2x2VlcDec->Decode2(pBsr, &totalZeros, &tC);
			if(totalZeros != tZ)
				result = 0;
		}//end for tZ...
	}//end for tC...

	IVlcEncoder*	pTotalZeros2x4VlcEnc = new TotalZeros2x4H264VlcEncoder();
	IVlcDecoder*	pTotalZeros2x4VlcDec = new TotalZeros2x4H264VlcDecoder();
	pBsw->Reset();
	for(tC = 1; tC <= 7; tC++)	///< Encode.
	{
		for(tZ = 0; tZ <= (8-tC); tZ++)
		{
			int numCodedBits = pTotalZeros2x4VlcEnc->Encode2(tZ, tC);
			pBsw->Write(numCodedBits, pTotalZeros2x4VlcEnc->GetCode());
		}//end for tZ...
	}//end for tC...

	pBsr->Reset();
	for(tC = 1; tC <= 7; tC++)	///< Decode.
	{
		for(tZ = 0; tZ <= (8-tC); tZ++)
		{
			int totalZeros = 0;
			int numCodedBits = pTotalZeros2x4VlcDec->Decode2(pBsr, &totalZeros, &tC);
			if(totalZeros != tZ)
				result = 0;
		}//end for tZ...
	}//end for tC...

	/// 4. Run-before encoding & decoding.
	IVlcEncoder*	pRunBeforeVlcEnc = new RunBeforeH264VlcEncoder();
	IVlcDecoder*	pRunBeforeVlcDec = new RunBeforeH264VlcDecoder();
	int rB, zL;
	pBsw->Reset();
	for(zL = 1; zL <= 14; zL++)	///< Encode.
	{
		for(rB = 0; rB <= zL; rB++)
		{
			int numCodedBits = pRunBeforeVlcEnc->Encode2(rB, zL);
			pBsw->Write(numCodedBits, pRunBeforeVlcEnc->GetCode());
		}//end for rB...
	}//end for zL...

	pBsr->Reset();
	for(zL = 1; zL <= 14; zL++)	///< Decode.
	{
		for(rB = 0; rB <= zL; rB++)
		{
			int runBefore = 0;
			int numCodedBits = pRunBeforeVlcDec->Decode2(pBsr, &runBefore, &zL);
			if(runBefore != rB)
				result = 0;
		}//end for rB...
	}//end for zL...


	/// Test the CAVLC encoding & decoding.

	/// All vlc encoders and decoders are tested. Now load them into the 
	/// CAVLC for usage in this test proceedure.
	IContextAwareRunLevelCodec* pCAVLC = new CAVLCH264Impl();
	/// Design unit test during development.
	result = CAVLCH264Impl::TestLevelPrefixSuffix();

	/// Testing for 4x4 and 2x2 modes.
	int mode;
	/// Vlc settings for all modes.
	((CAVLCH264Impl *)pCAVLC)->SetTokenCoeffVlcEncoder(pCoeffTokenVlcEnc);
	((CAVLCH264Impl *)pCAVLC)->SetTokenCoeffVlcDecoder(pCoeffTokenVlcDec);
	((CAVLCH264Impl *)pCAVLC)->SetPrefixVlcEncoder(pPrefixVlcEnc);
	((CAVLCH264Impl *)pCAVLC)->SetPrefixVlcDecoder(pPrefixVlcDec);
	((CAVLCH264Impl *)pCAVLC)->SetRunBeforeVlcEncoder(pRunBeforeVlcEnc);
	((CAVLCH264Impl *)pCAVLC)->SetRunBeforeVlcDecoder(pRunBeforeVlcDec);

	int totNumCoeffs = 16;	///< Default value.
	for(mode = CAVLCH264Impl::Mode2x2; mode <= CAVLCH264Impl::Mode4x4; mode++)
	{
		if(mode == CAVLCH264Impl::Mode2x2)
		{
			totNumCoeffs = 4;
			pCAVLC->SetMode(CAVLCH264Impl::Mode2x2);
			((CAVLCH264Impl *)pCAVLC)->SetTotalZerosVlcEncoder(pTotalZeros2x2VlcEnc);
			((CAVLCH264Impl *)pCAVLC)->SetTotalZerosVlcDecoder(pTotalZeros2x2VlcDec);
		}//end if mode...
		else if(mode == CAVLCH264Impl::Mode4x4)
		{
			totNumCoeffs = 16;
			pCAVLC->SetMode(CAVLCH264Impl::Mode4x4);
			((CAVLCH264Impl *)pCAVLC)->SetTotalZerosVlcEncoder(pTotalZeros4x4VlcEnc);
			((CAVLCH264Impl *)pCAVLC)->SetTotalZerosVlcDecoder(pTotalZeros4x4VlcDec);
		}//end else if mode...

		for(k = 0; k < TESTS; k++)
		{
			/// Load the test samples.
			int existingZeros = 0;
			for(i = 0; i < totNumCoeffs; i++)
			{
				b[i] = 0;	///< Clear the decoded to block.
				a[i] = (short)( (((double)rand()/(double)RAND_MAX) * 1023.0) - 256.0 + 0.5 );
				if(i != 0)
					a[i] = (short)( ((double)a[i])/((double)(2*i)) );	///< Flatten function towards high freq.
				if(a[i] == 0)
					existingZeros++;
			}//end for i...
			/// Number of zeros in the test block 0...16 or 0...4.
			int numOfZeros = k % (totNumCoeffs + 1);
			/// Random unique positions for these zeros.
			if(numOfZeros > existingZeros)
			{
				for(i = 0; i < (numOfZeros - existingZeros); i++)
				{
					int attempts	= 1;
					int zeroPos		= (int)( (((double)rand()/(double)RAND_MAX) * ((double)(totNumCoeffs-1))) + 0.5 );
					while((a[zeroPos] == 0)&&(attempts < 1024))
					{
						zeroPos	= (int)( (((double)rand()/(double)RAND_MAX) * ((double)(totNumCoeffs-1))) + 0.5 );
						attempts++;
					}//end while a[]...
					a[zeroPos] = 0;
				}//end for i...
			}//end if numOfZeros...
			/// Number of trailing ones in the test block 0...3.
			int numOfOnes = k % 4;
			/// Replace non-zero coeffs from the end with ones.
			for(i = (totNumCoeffs-1); (i >= 0)&&(numOfOnes > 0); i--)
			{
				if(a[i] != 0)
				{
					if(a[i] < 0)
						a[i] = -1;
					else
						a[i] = 1;
					numOfOnes--;
				}//end if a[]...
			}//end for i...

			/// Now do the actual test.
			for(int dcSkip = 0; dcSkip <= 1; dcSkip++)
			{
				for(nC = 0; nC <= totNumCoeffs; nC++)	///< Step through all neighbourhood coeff totals.
				{
					pCAVLC->SetParameter(CAVLCH264Impl::NUM_TOT_NEIGHBOR_COEFF_ID, nC);
					pCAVLC->SetParameter(CAVLCH264Impl::DC_SKIP_FLAG_ID, 0);

					/// Encoding.
					pBsw->Reset();
					int numEncodedBits = pCAVLC->Encode(a, pBsw);

					/// Decoding.
					pBsr->Reset();
					int numDecodedBits = pCAVLC->Decode(pBsr, b);

					for(j = pCAVLC->GetParameter(CAVLCH264Impl::DC_SKIP_FLAG_ID); j < totNumCoeffs; j++)
					{
						if(a[j] != b[j])
							result = 0;
					}//end for j...
				}//end for nC...
			}//end for dcSkip...
		}//end for k...

	}//end for mode...

	delete pCAVLC;

	delete pRunBeforeVlcDec;
	delete pRunBeforeVlcEnc;
	delete pTotalZeros2x4VlcDec;
	delete pTotalZeros2x4VlcEnc;
	delete pTotalZeros2x2VlcDec;
	delete pTotalZeros2x2VlcEnc;
	delete pTotalZeros4x4VlcDec;
	delete pTotalZeros4x4VlcEnc;
	delete pCoeffTokenVlcDec;
	delete pCoeffTokenVlcEnc;
	delete pPrefixVlcDec;
	delete pPrefixVlcEnc;

	delete pBsr;
	delete pBsw;

	return(result);
}//end TestCAVLC.

/** Test block layer encode and decode implementations.
@return	: 1 = success, 0 = failed..
*/
int H264v2Codec::TestBlockLayer(void)
{
	int i,j,test,ret;
	int result = 1;
	int TESTS = 1024;

	int q			= 1;	///< Step size.
	int	qErr	= q + (q/2) + 1;

	/// Block coding order as defined by H.264 standard.
	int macroBlkCodingOrderY[4][4] =
		{ { 0, 0, 1, 1 }, 
			{ 0, 0, 1, 1 }, 
			{ 2, 2, 3, 3 }, 
			{ 2, 2, 3, 3 } 
		};
	int macroBlkCodingOrderX[4][4] =
		{ { 0, 1, 0, 1 }, 
			{ 2, 3, 2, 3 }, 
			{ 0, 1, 0, 1 }, 
			{ 2, 3, 2, 3 } 
		};

	/// Simulate a macroblock structure with one 16x16 luminance and two 
	/// 8x8 chrominance components.
	short Yo[256], Uo[64], Vo[64];
	short Y[256], U[64], V[64];
	/// Create 4x4 mem overlays on the macroblock.
	OverlayMem2Dv2* pY = new OverlayMem2Dv2(Y, 16, 16, 4, 4);
	OverlayMem2Dv2* pU = new OverlayMem2Dv2(U, 8, 8, 4, 4);
	OverlayMem2Dv2* pV = new OverlayMem2Dv2(V, 8, 8, 4, 4);
	/// ...and now for the blocks. Included are one lum 4x4 DC block and
	/// two chr 2x2 DC blocks. Overlays are created on instantiation.
	BlockH264 Yblk[4][4]; ///< Default block dimensions are 4x4.
	BlockH264 Ublk[2][2];
	BlockH264 Vblk[2][2];

	for(i = 0; i < 4; i++)	///< Initialise all location aware members.
		for(j = 0; j < 4; j++)
		{
			/// Lum.
			Yblk[i][j]._offX	= j*4;
			Yblk[i][j]._offY	= i*4;
			if(i == 0)	///< Falls into MB above and in row 3.
				Yblk[i][j]._blkAbove = NULL;	///< In this case, there are no MBs above, so set to invalid.	
			else
				Yblk[i][j]._blkAbove = &(Yblk[i-1][j]); ///< In current MB.
			if(j == 0)	///< Falls into MB to the left and in col 3.
				Yblk[i][j]._blkLeft = NULL;	///< In this case, there are no MBs to the left, so set to invalid.
			else
				Yblk[i][j]._blkLeft = &(Yblk[i][j-1]); ///< In current MB.
			/// Chr.
			if((i < 2)&&(j < 2))
			{
				Ublk[i][j]._offX	= j*4;
				Ublk[i][j]._offY	= i*4;
				Vblk[i][j]._offX	= j*4;
				Vblk[i][j]._offY	= i*4;
				if(i == 0)	///< Falls into MB above and in row 3.
				{
					Ublk[i][j]._blkAbove = NULL;	///< In this case, there are no MBs above, so set to invalid.
					Vblk[i][j]._blkAbove = NULL;
				}//end if i...
				else
				{
					Ublk[i][j]._blkAbove = &(Ublk[i-1][j]); ///< In current MB.
					Vblk[i][j]._blkAbove = &(Vblk[i-1][j]); ///< In current MB.
				}//end else...
				if(j == 0)	///< Falls into MB to the left and in col 3.
				{
					Ublk[i][j]._blkLeft = NULL;	///< In this case, there are no MBs to the left, so set to invalid.
					Vblk[i][j]._blkLeft = NULL;
				}//end if j...
				else
				{
					Ublk[i][j]._blkLeft = &(Ublk[i][j-1]); ///< In current MB.
					Vblk[i][j]._blkLeft = &(Vblk[i][j-1]); ///< In current MB.
				}//end else...
			}//end if i...
		}//end for i & j...
	/// Ignore neighbourhood of DC blocks (for this test).
	BlockH264 YDCblk;
	BlockH264 UDCblk(2,2);
	BlockH264 VDCblk(2,2);

	/// Create Integer Transformers and their inverses to convert the test values.
	IForwardTransform* pF4x4T = new FastForward4x4ITImpl1();
	pF4x4T->SetParameter(IForwardTransform::QUANT_ID, q);
	pF4x4T->SetParameter(IForwardTransform::INTRA_FLAG_ID, 1);
	pF4x4T->SetMode(IForwardTransform::TransformOnly);

	IForwardTransform* pFDC4x4T = new FastForwardDC4x4ITImpl1();
	pFDC4x4T->SetParameter(IForwardTransform::QUANT_ID, q);
	pFDC4x4T->SetParameter(IForwardTransform::INTRA_FLAG_ID, 1);
	pFDC4x4T->SetMode(IForwardTransform::TransformAndQuant);

	IForwardTransform* pFDC2x2T = new FastForwardDC2x2ITImpl1();
	pFDC2x2T->SetParameter(IForwardTransform::QUANT_ID, q);
	pFDC2x2T->SetParameter(IForwardTransform::INTRA_FLAG_ID, 1);
	pFDC2x2T->SetMode(IForwardTransform::TransformAndQuant);

	/// Create an inverse IT to use and to compare with.
	IInverseTransform* pI4x4T = new FastInverse4x4ITImpl1();
	pI4x4T->SetParameter(IInverseTransform::QUANT_ID, q);
	pI4x4T->SetMode(IInverseTransform::TransformOnly);

	IInverseTransform* pIDC4x4T = new FastInverseDC4x4ITImpl1();
	pIDC4x4T->SetParameter(IInverseTransform::QUANT_ID, q);
	pIDC4x4T->SetMode(IInverseTransform::TransformAndQuant);

	IInverseTransform* pIDC2x2T = new FastInverseDC2x2ITImpl1();
	pIDC2x2T->SetParameter(IInverseTransform::QUANT_ID, q);
	pIDC2x2T->SetMode(IInverseTransform::TransformAndQuant);

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

	/// Simulate a macroblock of YUV values at a mid point and then random
	/// deviations from it with a limited range for each test.
	srand(23995);
	for(test = 0; test < TESTS; test++)
	{
		/// **** Construct a typical test sample macroblock. Midpoint between -256..255.
		short midPoint = (short)( (((double)rand()/(double)RAND_MAX) * 511.0) - 256.0 + 0.5 );
		/// Deviation limit between 0..128.
		short devLimit = (short)( (((double)rand()/(double)RAND_MAX) * 128.0) + 0.5 );
		for(i = 0; i < 256; i++)	///< Lum and Chr.
		{
			int x = (short)( (((double)rand()/(double)RAND_MAX) * (double)devLimit) - ((double)devLimit/2.0) + 0.5 );
			x = x + midPoint;
			if(x > 255) x = 255;
			else if(x < -255) x = -255;
			Yo[i] = x;
			Y[i]	= Yo[i];
			if(i < 64)	///< Chr sample.
			{
				/// Range is halved for Chr.
				x = (short)( (((double)rand()/(double)RAND_MAX) * ((double)devLimit/2.0)) - ((double)devLimit/4.0) + 0.5 );
				x = x + midPoint;
				if(x > 255) x = 255;
				else if(x < -255) x = -255;
				Uo[i] = x;
				U[i]	= Uo[i];
				x = (short)( (((double)rand()/(double)RAND_MAX) * ((double)devLimit/2.0)) - ((double)devLimit/4.0) + 0.5 );
				x = x + midPoint;
				if(x > 255) x = 255;
				else if(x < -255) x = -255;
				Vo[i] = x;
				V[i]	= Vo[i];
			}//end if Chr...
		}//end for i...

		/// ************ Encode **************

		/// Load each block from the macroblock and apply the forward transform.
		for(i = 0; i < 4; i++)	///< 16 Lum blocks + 4 U Chr and 4 V Chr blocks.
			for(j = 0; j < 4; j++)
			{
				/// Lum.
				pY->SetOrigin(Yblk[i][j]._offX, Yblk[i][j]._offY);	///< Set overlay origin for macroblock.
				/// Copy from macroblock to block.
				pY->Read(*(Yblk[i][j].GetBlkOverlay()));
				/// Transform without scaling and quant.
				Yblk[i][j].ForwardTransform(pF4x4T);

				/// Chr.
				if((i < 2)&&(j < 2))
				{
					pU->SetOrigin(Ublk[i][j]._offX, Ublk[i][j]._offY);
					/// Copy from macroblock to block.
					pU->Read(*(Ublk[i][j].GetBlkOverlay()));
					/// Transform.
					Ublk[i][j].ForwardTransform(pF4x4T);

					pV->SetOrigin(Vblk[i][j]._offX, Vblk[i][j]._offY);
					/// Copy from macroblock to block.
					pV->Read(*(Vblk[i][j].GetBlkOverlay()));
					/// Transform.
					Vblk[i][j].ForwardTransform(pF4x4T);
				}//end if i...

			}//end for i & j...

		/// Pull out the Lum DC components. This is done in the 16x16 Intra mode only.
		short* pYDC = YDCblk.GetBlk();
		for(i = 0; i < 4; i++)
			for(j = 0; j < 4; j++)
			{
				pYDC[4*i + j] = Yblk[i][j].GetDC();
				Yblk[i][j].SetDC(0);	///< Not really necessary.
			}//end for i & j...
		/// Transform & quantise Lum DC components.
		YDCblk.ForwardTransform(pFDC4x4T);

		/// Pull out the Chr DC components. This is done in all modes.
		short* pUDC = UDCblk.GetBlk();
		short* pVDC = VDCblk.GetBlk();
		for(i = 0; i < 2; i++)
			for(j = 0; j < 2; j++)
			{
				pUDC[2*i + j] = Ublk[i][j].GetDC();
				Ublk[i][j].SetDC(0);
				pVDC[2*i + j] = Vblk[i][j].GetDC();
				Vblk[i][j].SetDC(0);
			}//end for i & j...
		/// Transform & quantise Chr DC components.
		UDCblk.ForwardTransform(pFDC2x2T);
		VDCblk.ForwardTransform(pFDC2x2T);

		/// Quantise the main blocks.
		for(i = 0; i < 4; i++)	///< 16 Lum blocks + 4 U Chr and 4 V Chr blocks.
			for(j = 0; j < 4; j++)
			{
				/// Lum.
				Yblk[i][j].Quantise(pF4x4T, q);
				/// Chr.
				if((i < 2)&&(j < 2))
				{
					Ublk[i][j].Quantise(pF4x4T, q);
					Vblk[i][j].Quantise(pF4x4T, q);
				}//end if i...
			}//end for i & j...

		/// Encode the vlc blocks with the context of the neighborhood number of coeffs. Map
		/// to the required H.264 coding order of each block.
		pBsw->Reset();
		int numLumNeighborCoeffs	= 0;
		int numUChrNeighborCoeffs = 0;
		int numVChrNeighborCoeffs = 0;
		/// Lum DC first. Block num = -1.
		pCAVLC4x4->SetParameter(pCAVLC4x4->DC_SKIP_FLAG_ID, 0);											///< Include DC for DC Lum block.
		pCAVLC4x4->SetParameter(pCAVLC4x4->NUM_TOT_NEIGHBOR_COEFF_ID, 0);						///< This would be found from neighboring MBs.
		ret = pCAVLC4x4->Encode(YDCblk.GetBlk(), pBsw);															///< Put onto coded stream.
		if(ret <= 0)	///< Codec errors are detected from a negative return value.
			result = 0;
		YDCblk.SetNumCoeffs(pCAVLC4x4->GetParameter(pCAVLC4x4->NUM_TOT_COEFF_ID));	///< Set this block's coeff num. (Not used here.)
		///< 16 Lum blocks without DC value. Block num = 0..15
		pCAVLC4x4->SetParameter(pCAVLC4x4->DC_SKIP_FLAG_ID, 1);
		for(i = 0; i < 4; i++)	
			for(j = 0; j < 4; j++)	
			{
				int blkY = macroBlkCodingOrderY[i][j];
				int blkX = macroBlkCodingOrderX[i][j];
				/// Get num of neighbourhood coeffs as average of above and left block coeffs.
				int neighCoeffs = 0;
				if(Yblk[blkY][blkX]._blkAbove != NULL)
					neighCoeffs += (Yblk[blkY][blkX]._blkAbove)->GetNumCoeffs();
				if(Yblk[blkY][blkX]._blkLeft != NULL)
					neighCoeffs += (Yblk[blkY][blkX]._blkLeft)->GetNumCoeffs();
				if((Yblk[blkY][blkX]._blkAbove != NULL)&&(Yblk[blkY][blkX]._blkLeft != NULL))
					neighCoeffs = (neighCoeffs + 1)/2;
				pCAVLC4x4->SetParameter(pCAVLC4x4->NUM_TOT_NEIGHBOR_COEFF_ID, neighCoeffs);
				/// Put onto coded stream.
				ret = pCAVLC4x4->Encode(Yblk[blkY][blkX].GetBlk(), pBsw);										
				int numCoeffs = pCAVLC4x4->GetParameter(pCAVLC4x4->NUM_TOT_COEFF_ID);
				Yblk[blkY][blkX].SetNumCoeffs(numCoeffs);															///< Set this block's coeff num for later use.
				if(numCoeffs > 0)																											///< Set the coded/no coded flag
					Yblk[blkY][blkX].SetCoded(1);
				else
					Yblk[blkY][blkX].SetCoded(0);
			}//end for i & j...
		/// Chr DC next. Block num = 16 & 17.
		pCAVLC2x2->SetParameter(pCAVLC2x2->DC_SKIP_FLAG_ID, 0);											///< Include DC for DC Chr blocks.
		pCAVLC2x2->SetParameter(pCAVLC2x2->NUM_TOT_NEIGHBOR_COEFF_ID, -1);					///< Fixed at -1 for chroma_format_idc = 1.
		ret = pCAVLC2x2->Encode(UDCblk.GetBlk(), pBsw);															///< Put Chr U onto coded stream.
		UDCblk.SetNumCoeffs(pCAVLC2x2->GetParameter(pCAVLC2x2->NUM_TOT_COEFF_ID));	///< Set this block's coeff num. (Not used here.)
		ret = pCAVLC2x2->Encode(VDCblk.GetBlk(), pBsw);															///< Put Chr V onto coded stream.
		VDCblk.SetNumCoeffs(pCAVLC2x2->GetParameter(pCAVLC2x2->NUM_TOT_COEFF_ID));	///< Set this block's coeff num. (Not used here.)
		///< 4 U Chr blocks without DC value. Block num = 18..21
		pCAVLC4x4->SetParameter(pCAVLC4x4->DC_SKIP_FLAG_ID, 1);
		for(i = 0; i < 2; i++)	
			for(j = 0; j < 2; j++)	
			{
				/// Get num of neighbourhood coeffs as average of above and left block coeffs.
				int neighCoeffs = 0;
				if(Ublk[i][j]._blkAbove != NULL)
					neighCoeffs += (Ublk[i][j]._blkAbove)->GetNumCoeffs();
				if(Ublk[i][j]._blkLeft != NULL)
					neighCoeffs += (Ublk[i][j]._blkLeft)->GetNumCoeffs();
				if((Ublk[i][j]._blkAbove != NULL)&&(Ublk[i][j]._blkLeft != NULL))
					neighCoeffs = (neighCoeffs + 1)/2;
				pCAVLC4x4->SetParameter(pCAVLC4x4->NUM_TOT_NEIGHBOR_COEFF_ID, neighCoeffs);
				/// Put onto coded stream.
				ret = pCAVLC4x4->Encode(Ublk[i][j].GetBlk(), pBsw);																		
				int numCoeffs = pCAVLC4x4->GetParameter(pCAVLC4x4->NUM_TOT_COEFF_ID);
				Ublk[i][j].SetNumCoeffs(numCoeffs);															///< Set this block's coeff num for later use.
				if(numCoeffs > 0)																								///< Set the coded/no coded flag
					Ublk[i][j].SetCoded(1);
				else
					Ublk[i][j].SetCoded(0);
			}//end for i & j...
		///< 4 V Chr blocks without DC value. Block num = 22..25
		pCAVLC4x4->SetParameter(pCAVLC4x4->DC_SKIP_FLAG_ID, 1);
		for(i = 0; i < 2; i++)	
			for(j = 0; j < 2; j++)	
			{
				/// Get num of neighbourhood coeffs as average of above and left block coeffs.
				int neighCoeffs = 0;
				if(Vblk[i][j]._blkAbove != NULL)
					neighCoeffs += (Vblk[i][j]._blkAbove)->GetNumCoeffs();
				if(Ublk[i][j]._blkLeft != NULL)
					neighCoeffs += (Vblk[i][j]._blkLeft)->GetNumCoeffs();
				if((Vblk[i][j]._blkAbove != NULL)&&(Vblk[i][j]._blkLeft != NULL))
					neighCoeffs = (neighCoeffs + 1)/2;
				pCAVLC4x4->SetParameter(pCAVLC4x4->NUM_TOT_NEIGHBOR_COEFF_ID, neighCoeffs);
				/// Put onto coded stream.
				ret = pCAVLC4x4->Encode(Vblk[i][j].GetBlk(), pBsw);
				int numCoeffs = pCAVLC4x4->GetParameter(pCAVLC4x4->NUM_TOT_COEFF_ID);
				Vblk[i][j].SetNumCoeffs(numCoeffs);															///< Set this block's coeff num for later use.
				if(numCoeffs > 0)																								///< Set the coded/no coded flag.
					Vblk[i][j].SetCoded(1);
				else
					Vblk[i][j].SetCoded(0);
			}//end for i & j...

		/// *********** Clear all ***********
		for(i = 0; i < 256; i++)
		{
			Y[i] = 0;
			if(i < 64)
			{
				U[i] = 0;
				V[i] = 0;
			}//end if i...
		}//end for i...

		for(i = 0; i < 4; i++)
			for(j = 0; j < 4; j++)
			{
				Yblk[i][j].Zero();
				if((i < 2)&&(j < 2))
				{
					Ublk[i][j].Zero();
					Vblk[i][j].Zero();
				}//end if i...
			}//end for i & j...
		YDCblk.Zero();
		UDCblk.Zero();
		VDCblk.Zero();

		/// *********** Decode ***********

		/// Decode the vlc blocks with the context of the neighborhood number of coeffs.
		pBsr->Reset();
		numLumNeighborCoeffs	= 0;
		numUChrNeighborCoeffs = 0;
		numVChrNeighborCoeffs = 0;
		/// Lum DC first. Block num = -1.
		pCAVLC4x4->SetParameter(pCAVLC4x4->DC_SKIP_FLAG_ID, 0);											///< Include DC for DC Lum block.
		pCAVLC4x4->SetParameter(pCAVLC4x4->NUM_TOT_NEIGHBOR_COEFF_ID, 0);						///< This would be found from neighboring MBs.
		ret = pCAVLC4x4->Decode(pBsr, YDCblk.GetBlk());															///< Get from coded stream.
		YDCblk.SetNumCoeffs(pCAVLC4x4->GetParameter(pCAVLC4x4->NUM_TOT_COEFF_ID));	///< Set this block's coeff num. (Not used here.)
		///< 16 Lum blocks without DC value. Block num = 0..15
		pCAVLC4x4->SetParameter(pCAVLC4x4->DC_SKIP_FLAG_ID, 1);
		for(i = 0; i < 4; i++)	
			for(j = 0; j < 4; j++)	
			{
				int blkY = macroBlkCodingOrderY[i][j];
				int blkX = macroBlkCodingOrderX[i][j];
				/// Get num of neighbourhood coeffs as average of above and left block coeffs.
				int neighCoeffs = 0;
				if(Yblk[blkY][blkX]._blkAbove != NULL)
					neighCoeffs += (Yblk[blkY][blkX]._blkAbove)->GetNumCoeffs();
				if(Yblk[blkY][blkX]._blkLeft != NULL)
					neighCoeffs += (Yblk[blkY][blkX]._blkLeft)->GetNumCoeffs();
				if((Yblk[blkY][blkX]._blkAbove != NULL)&&(Yblk[blkY][blkX]._blkLeft != NULL))
					neighCoeffs = (neighCoeffs + 1)/2;
				pCAVLC4x4->SetParameter(pCAVLC4x4->NUM_TOT_NEIGHBOR_COEFF_ID, neighCoeffs);
				/// Get from coded stream.
				ret = pCAVLC4x4->Decode(pBsr, Yblk[blkY][blkX].GetBlk());										
				Yblk[blkY][blkX].SetNumCoeffs(pCAVLC4x4->GetParameter(pCAVLC4x4->NUM_TOT_COEFF_ID));	///< Set this block's coeff num for later use.
			}//end for i & j...
		/// Chr DC next. Block num = 16 & 17.
		pCAVLC2x2->SetParameter(pCAVLC2x2->DC_SKIP_FLAG_ID, 0);											///< Include DC for DC Chr blocks.
		pCAVLC2x2->SetParameter(pCAVLC2x2->NUM_TOT_NEIGHBOR_COEFF_ID, -1);					///< Fixed at -1 for chroma_format_idc = 1.
		ret = pCAVLC2x2->Decode(pBsr, UDCblk.GetBlk());															///< Get Chr U from coded stream.
		UDCblk.SetNumCoeffs(pCAVLC2x2->GetParameter(pCAVLC2x2->NUM_TOT_COEFF_ID));	///< Set this block's coeff num. (Not used here.)
		ret = pCAVLC2x2->Decode(pBsr, VDCblk.GetBlk());															///< Get Chr V from coded stream.
		VDCblk.SetNumCoeffs(pCAVLC2x2->GetParameter(pCAVLC2x2->NUM_TOT_COEFF_ID));	///< Set this block's coeff num. (Not used here.)
		///< 4 U Chr blocks without DC value. Block num = 18..21
		pCAVLC4x4->SetParameter(pCAVLC4x4->DC_SKIP_FLAG_ID, 1);
		for(i = 0; i < 2; i++)	
			for(j = 0; j < 2; j++)	
			{
				/// Get num of neighbourhood coeffs as average of above and left block coeffs.
				int neighCoeffs = 0;
				if(Ublk[i][j]._blkAbove != NULL)
					neighCoeffs += (Ublk[i][j]._blkAbove)->GetNumCoeffs();
				if(Ublk[i][j]._blkLeft != NULL)
					neighCoeffs += (Ublk[i][j]._blkLeft)->GetNumCoeffs();
				if((Ublk[i][j]._blkAbove != NULL)&&(Ublk[i][j]._blkLeft != NULL))
					neighCoeffs = (neighCoeffs + 1)/2;
				pCAVLC4x4->SetParameter(pCAVLC4x4->NUM_TOT_NEIGHBOR_COEFF_ID, neighCoeffs);
				/// Get from coded stream.
				ret = pCAVLC4x4->Decode(pBsr, Ublk[i][j].GetBlk());
				Ublk[i][j].SetNumCoeffs(pCAVLC4x4->GetParameter(pCAVLC4x4->NUM_TOT_COEFF_ID));	///< Set this block's coeff num for later use.
			}//end for i & j...
		///< 4 V Chr blocks without DC value. Block num = 22..25
		pCAVLC4x4->SetParameter(pCAVLC4x4->DC_SKIP_FLAG_ID, 1);
		for(i = 0; i < 2; i++)	
			for(j = 0; j < 2; j++)	
			{
				/// Get num of neighbourhood coeffs as average of above and left block coeffs.
				int neighCoeffs = 0;
				if(Vblk[i][j]._blkAbove != NULL)
					neighCoeffs += (Vblk[i][j]._blkAbove)->GetNumCoeffs();
				if(Ublk[i][j]._blkLeft != NULL)
					neighCoeffs += (Vblk[i][j]._blkLeft)->GetNumCoeffs();
				if((Vblk[i][j]._blkAbove != NULL)&&(Vblk[i][j]._blkLeft != NULL))
					neighCoeffs = (neighCoeffs + 1)/2;
				pCAVLC4x4->SetParameter(pCAVLC4x4->NUM_TOT_NEIGHBOR_COEFF_ID, neighCoeffs);
				/// Get from coded stream.
				ret = pCAVLC4x4->Decode(pBsr, Vblk[i][j].GetBlk());																		
				Vblk[i][j].SetNumCoeffs(pCAVLC4x4->GetParameter(pCAVLC4x4->NUM_TOT_COEFF_ID));	///< Set this block's coeff num.
			}//end for i & j...

		/// Inverse transform & inverse quantise Lum DC components.
		YDCblk.InverseTransform(pIDC4x4T);
		/// Inverse transform & inverse quantise Chr DC components.
		UDCblk.InverseTransform(pIDC2x2T);
		VDCblk.InverseTransform(pIDC2x2T);

		/// Inverse quantise the main blocks.
		for(i = 0; i < 4; i++)	///< 16 Lum blocks + 4 U Chr and 4 V Chr blocks.
			for(j = 0; j < 4; j++)
			{
				/// Lum.
				Yblk[i][j].InverseQuantise(pI4x4T, q);
				/// Chr.
				if((i < 2)&&(j < 2))
				{
					Ublk[i][j].InverseQuantise(pI4x4T, q);
					Vblk[i][j].InverseQuantise(pI4x4T, q);
				}//end if i...
			}//end for i & j...

		/// Put the Lum DC components in position with a 4x scale factor.
		pYDC = YDCblk.GetBlk();
		for(i = 0; i < 4; i++)
			for(j = 0; j < 4; j++)
				Yblk[i][j].SetDC(4 * pYDC[4*i + j]);
		/// Put the Chr DC components in position.
		pUDC = UDCblk.GetBlk();
		pVDC = VDCblk.GetBlk();
		for(i = 0; i < 2; i++)
			for(j = 0; j < 2; j++)
			{
				Ublk[i][j].SetDC(4 * pUDC[2*i + j]);
				Vblk[i][j].SetDC(4 * pVDC[2*i + j]);
			}//end for i & j...

		for(i = 0; i < 4; i++)	///< 16 Lum blocks + 4 U Chr and 4 V Chr blocks.
			for(j = 0; j < 4; j++)
			{
				/// Inverse transform.
				Yblk[i][j].InverseTransform(pI4x4T);
				/// Lum.
				pY->SetOrigin(Yblk[i][j]._offX, Yblk[i][j]._offY);	///< Set overlay origin for macroblock.
				/// Copy from block to macroblock.
				pY->Write(*(Yblk[i][j].GetBlkOverlay()));

				/// Chr.
				if((i < 2)&&(j < 2))
				{
					/// Inverse transform.
					Ublk[i][j].InverseTransform(pI4x4T);
					pU->SetOrigin(Ublk[i][j]._offX, Ublk[i][j]._offY);
					/// Copy from block to macroblock.
					pU->Write(*(Ublk[i][j].GetBlkOverlay()));

					/// Inverse transform.
					Vblk[i][j].InverseTransform(pI4x4T);
					pV->SetOrigin(Vblk[i][j]._offX, Vblk[i][j]._offY);
					/// Copy from block to macroblock.
					pV->Write(*(Vblk[i][j].GetBlkOverlay()));
				}//end if i...

			}//end for i & j...

		/// ************ Evaluate ******************
		for(i = 0; i < 256; i++)
		{
			if( abs(Y[i] - Yo[i]) > qErr )	///< Allow rounding error.
				result = 0;
			if(i < 64)
			{
				if( abs(U[i] - Uo[i]) > qErr )
					result = 0;
				if( abs(V[i] - Vo[i]) > qErr )
					result = 0;
			}//end if i...
		}//end for i...

	}//end for test...

	/// Chuck out everything in reverse order of instantiation.
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
	delete pV;
	delete pU;
	delete pY;

	return(result);
}//end TestBlockLayer.

