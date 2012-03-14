/** @file

MODULE				: CAVLCH264Impl

TAG						: CLRLH264I

FILE NAME			: CAVLCH264Impl.cpp

DESCRIPTION		: A class to implement a CAVLC codec on the 2-D quantised 
								integer transformed and quantised coeffs of a coded block 
								as defined in the H.264 standard. It implements the 
								IRunLengthCodec interface.

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

#include <string.h>
#include "IBitStreamReader.h"
#include "IBitStreamWriter.h"
#include "CAVLCH264Impl.h"

/*
---------------------------------------------------------------------------
	Constants.
---------------------------------------------------------------------------
*/
const int CAVLCH264Impl::zigZag8x8Pos[64] =
{
	 0,	 1,	 8, 16,	 9,	 2,	 3,	10,
	17,	24,	32,	25,	18,	11,	 4,	 5,
	12,	19,	26,	33,	40,	48,	41,	34,
	27,	20,	13,	 6,	 7,	14,	21,	28,
	35,	42,	49,	56,	57,	50,	43,	36,
	29,	22,	15,	23,	30,	37,	44,	51,
	58,	59,	52,	45,	38,	31,	39,	46,
	53,	60,	61,	54,	47,	55,	62,	63
};

const int CAVLCH264Impl::zigZag4x4Pos[16] =
{
	 0,	 1,	 4,  8,
	 5,	 2,	 3,	 6,
	 9,	12,	13,	10,
	 7,	11,	14,	15
};

const int CAVLCH264Impl::zigZag2x2Pos[4] =
{
	 0,	 1,
	 2,	 3
};

/*
---------------------------------------------------------------------------
	Construction and Destruction.
---------------------------------------------------------------------------
*/
CAVLCH264Impl::CAVLCH264Impl(void)
{
	/// Default to 4x4.
	_mode					= Mode4x4;
	_zigZag				= zigZag4x4Pos;
	_maxNumCoeff	= 16;
	_numCoeff			= 0;

	_numTotNeighborCoeff	= 0;
	_dcSkip								= 0;

	/// Vlc encoders/decoders
	_pCoeffTokenVlcEncoder	= NULL;
	_pCoeffTokenVlcDecoder	= NULL;
	_pPrefixVlcEncoder			= NULL;;
	_pPrefixVlcDecoder			= NULL;;
	_pTotalZerosVlcEncoder	= NULL;;
	_pTotalZerosVlcDecoder	= NULL;;
	_pRunBeforeVlcEncoder		= NULL;;
	_pRunBeforeVlcDecoder		= NULL;;

}//end constructor.

CAVLCH264Impl::~CAVLCH264Impl(void)
{
}//end destructor.

/*
---------------------------------------------------------------------------
	Interface Methods.
---------------------------------------------------------------------------
*/
/** Encode the input to a CAVLC bit stream.
Encode the input 2-D block of IT coeffs into the output run-level bit stream. 
A NULL stream will switch the encoder to count the number of bits used without 
writing to stream. Error codes are:
	-1 = Vlc error (no such symbol).
	-2 = Stream is full.
@param in		:	Input block to encode.
@param rle	:	Run-level encoded stream.
@return			: Total num of encoded bits. Negative values for errors.
*/
int CAVLCH264Impl::Encode(void* in, void* stream)
{
	/// Interpret the param list for this implementation.
	short*						coeffLevel	= (short *)in;
	IBitStreamWriter*	pBsw				= (IBitStreamWriter *)stream;

	int totalEncBits = 0;
	int level[64]; ///< Max size is for 8x8.
	int runBefore[64];
	int totalCoeff		= 0;
	int trailingOnes	= 0;
	int trailingOnesSignFlag[3];
	int totalZeros		= 0;
	int i;

	/// Scan the input in reverse zigzag order and determine the context-aware
	/// variables to code.
	int pos		= 0;	///< A forward array index.
	int count = 0;	///< Local zero counter.
	for(i = (_maxNumCoeff-1); i >= _dcSkip; i--)
	{
		int x = coeffLevel[_zigZag[i]];
		/// Last trailing zeros are ignored and not coded. Wait for the 1st
		/// non-zero coeff.
		if(totalCoeff > 0)
		{
			if(x == 0)
			{
				count++;
				totalZeros++;	///< Count all zeros including any at the head (low freq) end.
			}//end if x == 0...
			else	///< ...a non-zero coeff is detected.
			{
				/// Set the zero run for previous non-zero coeff and advance to next non-zero
				/// pos. Reset the local zero counter.
				runBefore[pos++] = count;
				count = 0;
				/// Load the next non-zero coeff.
				level[pos] = x;
				totalCoeff++;
				/// Only 3 trailing ones are permitted. There must be no non-one values in-between
				/// the trailing ones count.
				if((trailingOnes < 3)&&(trailingOnes == (totalCoeff-1)))
				{
					if((x == -1)||(x == 1))	///< Trailing one.
					{
						if(x == -1)
							trailingOnesSignFlag[trailingOnes] = 1;
						else	///< x == 1.
							trailingOnesSignFlag[trailingOnes] = 0;
						trailingOnes++;
					}//end if x...
				}//end if trailingOnes...
			}//end else...
		}//end if totalCoeff...
		else	
		{
			if(x != 0)	///< 1st non-zero coeff.
			{
				level[pos] = x;
				totalCoeff++;
				if((x == -1)||(x == 1))	///< Trailing one?
				{
					if(x == -1)
						trailingOnesSignFlag[0] = 1;
					else	///< x == 1.
						trailingOnesSignFlag[0] = 0;
					trailingOnes++;
				}//end if x...
			}//end if x != 0...
		}//end else...
	}//end for i...

	/// Get the Vlc codes for each context-aware variable are write them to
	/// the bit stream keeping count of the total no. of bits consumed.
	int lclNumBits;

	/// Write the coeff_token to the bit stream with error detection.
	lclNumBits = _pCoeffTokenVlcEncoder->Encode3(totalCoeff, trailingOnes, _numTotNeighborCoeff);
	if(pBsw != NULL)
	{
		if(pBsw->GetStreamBitsRemaining() >= lclNumBits)
			pBsw->Write(lclNumBits, _pCoeffTokenVlcEncoder->GetCode());
		else	///< Stream full error.
			return(STREAM_ACCESS_DENIED);
	}//end if pBsw...
	if(lclNumBits > 0)
		totalEncBits += lclNumBits;
	else	///< Vlc symbol not recognised error.
		return(VLC_SYMBOL_NOT_RECOGNISED);

	/// Proceed if there is anything to code.
	if(totalCoeff > 0)
	{
		int suffixLength	= 0;	///< Default and most likely case.
		if((totalCoeff > 10) && (trailingOnes < 3))	///< Special condition for spatially active blocks.
			suffixLength = 1;

		/// Write all the non-zero coeffs that are stored in reverse order from high freq 
		/// to low freq starting with the trailing ones.
		for(i = 0; i < totalCoeff; i++)
		{
			if(i < trailingOnes)
			{
				/// trailing_ones_sign_flag is written to the bit stream.
				if(pBsw != NULL)
				{
					if(pBsw->GetStreamBitsRemaining() >= 1)
						pBsw->Write(trailingOnesSignFlag[i]);
					else	///< Stream full error.
						return(STREAM_ACCESS_DENIED);
				}//end if pBsw...
				totalEncBits++;
			}//end if i...
			else
			{
				/// Translate the level +/- codes into even/odd levelCode.
				int levelCode;
				if(level[i] > 0)	///< + is even.
					levelCode = (2*level[i]) - 2;
				else							///< - is odd.
					levelCode = -((2*level[i]) + 1);

				if((i == trailingOnes) && (trailingOnes < 3))	///< Special condition for non-ones in the trailing 3 positions.
					levelCode -= 2;

				/// Determine the prefix, suffix length & level code before writing to the stream.
				int done						= 0;
				int levelPrefix			= 0;
				int levelSuffix			= 0;
				int levelSuffixSize = suffixLength;
				/// Use levelPrefix = 14 as an ESC code for 1st trailing large 
				/// levels from 14 - 29 range.
				if(suffixLength == 0)
				{
					if((levelCode >= 14)&&(levelCode <= 29))
					{
						levelSuffixSize = 4;
						levelSuffix			= (levelCode - 14) & 0x0000000F;
						levelPrefix			= 14;
						done						= 1;	///< Early exit.
					}//end if encLevelCode...
				}//end if !suffixLength...

				if(!done)
				{
					/// Max possible levelCode with this suffixLength and 
					/// levelPrefix less than 15.
					int maxLevelCode = (14 << suffixLength) + ~(0xFFFFFFFF << suffixLength);

					/// Extract level prefix and suffix from the levelCode.
					if(levelCode <= maxLevelCode)
					{
						levelPrefix = levelCode >> suffixLength;
						levelSuffix = levelCode & ~(0xFFFFFFFF << suffixLength);
					}//end if encLevelCode...
					else	///< Large code levels.
					{
						/// Entering here the levelCode is always greater than the level 
						/// associated with a max prefix = 15 therefore it can be subtracted 
						/// to promote smaller levels.
						levelCode -= (15 << suffixLength);
						/// For a suffixLength of zero the max levelCode can be further 
						/// reduced by 15 as levelCodes less than 30 are catered for by
						/// the ESC code with LevelPrefix = 14 described above.
						if(suffixLength == 0)
							levelCode -= 15;
						/// The levelPrefix to set defines the range between octaves in
						/// the following way:
						///	LevelPrefix		Range (2^(levelPrefix-3)) offset by (2^x - 1)(2^12)
						///	15						0..((2^12)-1)
						///	16												2^12..((2^13)+(2^12)-1)
						///	17																							((2^13)+(2^12))..((2^14)+(2^12)-1)
						///	etc.
						/// Note that the min value for each range can be subtracted to save
						/// 1 bit per range.
						levelPrefix = 15;
						/// Iterate the levelPrefix by checking if levelCode is larger than the min
						/// levelCode for the next larger range. Put a hard limit max bit size to 32 (= 35 - 3).
						while((levelCode >= ((1 << (levelPrefix-2)) - 4096))&&(levelPrefix < 35))
							levelPrefix++;
						if(levelPrefix >= 16)
							levelCode -= (1 << (levelPrefix-3)) - 4096;	///< Reduce by min value for this range.

						levelSuffixSize = levelPrefix - 3;
						levelSuffix			= levelCode & ~(0xFFFFFFFF << levelSuffixSize);
					}//end else...
				}//end if !done...

				/// Write the level_prefix and V to the bit stream.
				lclNumBits = _pPrefixVlcEncoder->Encode(levelPrefix);
				if(pBsw != NULL)
				{
					if(pBsw->GetStreamBitsRemaining() >= (lclNumBits + levelSuffixSize))
					{
						pBsw->Write(lclNumBits, _pPrefixVlcEncoder->GetCode());
						pBsw->Write(levelSuffixSize, levelSuffix);
					}//end if GetStreamBitsRemaining...
					else	///< Stream is full.
						return(STREAM_ACCESS_DENIED);
				}//end if pBsw...
				if(lclNumBits > 0)
					totalEncBits += (lclNumBits + levelSuffixSize);
				else
					return(VLC_SYMBOL_NOT_RECOGNISED);

				/// If the level is above a threshold then increment suffixLength for next write. Note
				/// suffixLength cannot be longer than 6 bits.
				if(suffixLength == 0)
					suffixLength = 1;
				int absLevel = level[i];
				if(absLevel < 0)
					absLevel = -absLevel;
				if((absLevel > (3 << (suffixLength-1))) && (suffixLength < 6))
					suffixLength++;
			}//end else...
		}//end for i...

		/// Encode and write the total number of zeros in-between the non-zero 
		/// coeffs to the bit stream.
		int zerosLeft = totalZeros;
		if(totalCoeff < (_maxNumCoeff - _dcSkip))	///< i.e. there are some zeros.
		{
			/// The total_zeros code is dependent on totalCoeff.
			lclNumBits = _pTotalZerosVlcEncoder->Encode2(totalZeros, totalCoeff);
			if(pBsw != NULL)
			{
				if(pBsw->GetStreamBitsRemaining() >= lclNumBits)
					pBsw->Write(lclNumBits, _pTotalZerosVlcEncoder->GetCode());
				else	///< Stream is full.
					return(STREAM_ACCESS_DENIED);
			}//end if pBsw...
			if(lclNumBits > 0)
				totalEncBits += lclNumBits;
			else
				return(VLC_SYMBOL_NOT_RECOGNISED);

			/// The variable pos = total length of runBefore[] array, one less than
			/// the level[] array.
			for(i = 0; (i < pos) && zerosLeft; i++)
			{
				lclNumBits = _pRunBeforeVlcEncoder->Encode2(runBefore[i], zerosLeft);
				if(pBsw != NULL)
				{
					if(pBsw->GetStreamBitsRemaining() >= lclNumBits)
						pBsw->Write(lclNumBits, _pRunBeforeVlcEncoder->GetCode());
					else	///< Stream is full.
						return(STREAM_ACCESS_DENIED);
				}//end if pBsw...
				if(lclNumBits > 0)
					totalEncBits += lclNumBits;
				else
					return(VLC_SYMBOL_NOT_RECOGNISED);
				/// Update the context.
				zerosLeft -= runBefore[i];
			}//end for i...
		}//end if totalCoeff...

	}//end if totalCoeff...

	/// Store total coeffs for this encode session.
	_numCoeff = totalCoeff;

	return(totalEncBits);
}//end Encode.

/** Decode a CAVLC bit stream to the output.
Decode the input run-level bit stream into the output 2-D block of IT coeffs. It
is unknown how many bits will come off the stream for the next read so the error
checking only looks for end of the stream. A read is non-destructive so reading
past the end of the stream will not do bad things. Error codes are:
	-1 = Vlc error (non-existent symbol).
	-2 = Stream is empty.
@param stream	:	Run-level stream to decode.
@param out		:	Output block generated by the decode.
@return				: Total num of decoded bits. Negative values for errors.
*/
int CAVLCH264Impl::Decode(void* stream, void* out)
{
	/// Interpret the param list for this implementation.
	IBitStreamReader* pBsr				= (IBitStreamReader *)stream;
	short*						coeffLevel	= (short *)out;

	int totalDecBits = 0;
	int lclNumBits;
	int level[64]; ///< Max size is for 8x8.
	int runBefore[64];
	int i;

	/// Clear output array.
	memset(coeffLevel, 0, sizeof(short) * _maxNumCoeff);

	/// coeff_token is parsed from the bit stream and decoded to give the totalCoeff
	/// and trailingOnes where the vlc table selection is indicated by the 3rd symbol.
	int totalCoeff,trailingOnes;
	if(pBsr->GetStreamBitsRemaining() > 0)
		lclNumBits = _pCoeffTokenVlcDecoder->Decode3(pBsr, &totalCoeff, &trailingOnes, &_numTotNeighborCoeff);
	else	///< Stream is empty.
		return(STREAM_ACCESS_DENIED);
	if(lclNumBits > 0)
		totalDecBits += lclNumBits;
	else	///< Vlc error.
		return(VLC_SYMBOL_NOT_RECOGNISED);

	/// Only requires decoding if there are any non-zero coeffs.
	if(totalCoeff > 0)
	{
		int suffixLength	= 0;	///< Default and most likely case.
		if((totalCoeff > 10) && (trailingOnes < 3))	///< Special condition for spatially active blocks.
			suffixLength = 1;

		/// Extract all the non-zero coeffs that are stored in reverse order from high freq 
		/// to low freq starting with the trailing ones.
		for(i = 0; i < totalCoeff; i++)
		{
			if(i < trailingOnes)
			{
				int trailingOnesSignFlag;
				/// trailing_ones_sign_flag is parsed from the bit stream.
				if(pBsr->GetStreamBitsRemaining() > 0)
					trailingOnesSignFlag = pBsr->Read();
				else	///< Stream is empty.
					return(STREAM_ACCESS_DENIED);
				totalDecBits++;
				level[i] = 1 - 2*trailingOnesSignFlag;
			}//end if i...
			else
			{
				int levelPrefix;
				/// level_prefix is parsed from the bit stream by counting zeros.
				if(pBsr->GetStreamBitsRemaining() > 0)
					levelPrefix = _pPrefixVlcDecoder->Decode(pBsr);
				else	///< Stream is empty.
					return(STREAM_ACCESS_DENIED);
				lclNumBits = _pPrefixVlcDecoder->GetNumDecodedBits();
				if(lclNumBits > 0)
					totalDecBits += lclNumBits;
				else	///< Vlc error.
					return(VLC_SYMBOL_NOT_RECOGNISED);

				int levelSuffixSize = suffixLength;
				/// Modify the suffix bits to extract large values.
				if(levelPrefix >= 15)
					levelSuffixSize = levelPrefix - 3;
				else if((levelPrefix == 14)&&(suffixLength == 0))
					levelSuffixSize = 4;

				int levelSuffix = 0;
				if(levelSuffixSize > 0)
				{
					/// level_suffix is parsed from the bit stream.
					if(pBsr->GetStreamBitsRemaining() >= levelSuffixSize)
						levelSuffix = pBsr->Read(levelSuffixSize);
					else	///< Stream is empty.
						return(STREAM_ACCESS_DENIED);
					totalDecBits += levelSuffixSize;
				}//end if levelSuffixSize...

				/// Shift levelPrefix left into bit position defined by suffixLength
				/// and add the levelSuffix.
				int levelCode;
				if(levelPrefix < 15)
					levelCode = (levelPrefix << suffixLength) + levelSuffix;
				else
					levelCode = (15 << suffixLength) + levelSuffix;

				if((levelPrefix >= 15) && (suffixLength == 0))	///< Special condition for large high freq trailing levels.
					levelCode += 15;

				if(levelPrefix >= 16)
					levelCode += (1 << (levelPrefix-3)) - 4096;	///< Special condition for all large values.

				if((i == trailingOnes) && (trailingOnes < 3))	///< Special condition for non-ones in the trailing 3 positions.
					levelCode += 2;

				/// Translate the levelCode into the +/- level stored as even/odd codes.
				if(levelCode & 1)	///< Odd is -.
					level[i] = (-levelCode - 1) >> 1;
				else							///< Even is +.
					level[i] = (levelCode + 2) >> 1;

				/// If the level is above a threshold then increment suffixLength for next parse. Note
				/// suffixLength cannot be longer than 6 bits.
				if(suffixLength == 0)
					suffixLength = 1;
				int absLevel = level[i];
				if(absLevel < 0)
					absLevel = -absLevel;
				if((absLevel > (3 << (suffixLength-1))) && (suffixLength < 6))
					suffixLength++;
			}//end else...
		}//end for i...

		/// Get the total no. of zeros inbetween the non-zero coeffs.
		int zerosLeft = 0;
		if(totalCoeff < (_maxNumCoeff - _dcSkip))	///< i.e. there are some zeros.
		{
			/// total_zeros is parsed from the bit stream and is dependent on totalCoeff.
			if(pBsr->GetStreamBitsRemaining() > 0)
				lclNumBits = _pTotalZerosVlcDecoder->Decode2(pBsr, &zerosLeft, &totalCoeff);
			else	///< Stream is empty.
				return(STREAM_ACCESS_DENIED);
			if(lclNumBits > 0)
				totalDecBits += lclNumBits;
			else	///< Vlc error.
				return(VLC_SYMBOL_NOT_RECOGNISED);
		}//end if totalCoeff...

		/// Set the run_before values for each non-zero coeff. Note that the last
		/// (low freq) non-zero coeff run_before does not need decoding because it 
		/// is implicit.
		for(i = 0; i < (totalCoeff-1); i++)
		{
			runBefore[i] = 0;
			if(zerosLeft > 0)
			{
				/// run_before is parsed from the bit stream and is dependent on the current zerosLeft.
				if(pBsr->GetStreamBitsRemaining() > 0)
					lclNumBits = _pRunBeforeVlcDecoder->Decode2(pBsr, &(runBefore[i]), &zerosLeft);
				else	///< Stream is empty.
					return(STREAM_ACCESS_DENIED);
				if(lclNumBits > 0)
					totalDecBits += lclNumBits;
				else	///< Vlc error.
					return(VLC_SYMBOL_NOT_RECOGNISED);
			}//end if zerosLeft...
			zerosLeft -= runBefore[i];
		}//end for i...
		runBefore[totalCoeff-1] = zerosLeft;	///< Set the implicit run_before.

		/// Load the level non-zero coeffs into the output array in reverse order and
		/// with zigzag conversion. The coeff level array was initialised to zeros so
		/// only non-zero coeffs need to be added.
		int coeffNum = -1;
		for(i = (totalCoeff-1); i >= 0; i--)
		{
			coeffNum += (runBefore[i] + 1);
			coeffLevel[_zigZag[coeffNum + _dcSkip]] = (short)(level[i]);
		}//end for i...

	}//end if totalCoeff...

	/// Store total coeffs for this decode session.
	_numCoeff = totalCoeff;

	return(totalDecBits);
}//end Decode.

/** Set the codec mode.
The mode defines the block size choice in this implementation.
@param mode	:	Block size selection defined by class consts.
@return			: none.
*/
void CAVLCH264Impl::SetMode(int mode)
{
	_mode = mode;
	switch(mode)
	{
		case Mode2x2:
			_zigZag				= zigZag2x2Pos;
			_maxNumCoeff	= 4;
			break;
		case Mode4x4:
			_zigZag				= zigZag4x4Pos;
			_maxNumCoeff	= 16;
			break;
		case Mode8x8:
			_zigZag				= zigZag8x8Pos;
			_maxNumCoeff	= 64;
			break;
		default:
			_mode					= Mode4x4;
			_zigZag				= zigZag4x4Pos;
			_maxNumCoeff	= 16;
			break;
	}//end switch mode...
}//end SetMode.

/** Set the codec parameters.
The IDs are defined by the class consts and values by the 
members. Do nothing if the param is not defined in this 
implementation.
@param paramID	: Parameter to set.
@param paramVal	: Parameter value.
@return					: none.
*/
void CAVLCH264Impl::SetParameter(int paramID, int paramVal)
{
	switch(paramID)
	{
		case NUM_TOT_NEIGHBOR_COEFF_ID:
			_numTotNeighborCoeff = paramVal;
			break;
		case DC_SKIP_FLAG_ID:
			_dcSkip = paramVal;
			break;
	}//end switch paramID...
}//end SetParameter.

/** Get the codec parameters.
Return -1 if the param is not defined in this implementation.
@param paramID	: Parameter to get.
@return					: Param value.
*/
int CAVLCH264Impl::GetParameter(int paramID)
{
	int res = -1;
	switch(paramID)
	{
		case NUM_TOT_NEIGHBOR_COEFF_ID:
			res = _numTotNeighborCoeff;
			break;
		case DC_SKIP_FLAG_ID:
			res = _dcSkip;
			break;
		case NUM_TOT_COEFF_ID:
			res = _numCoeff;
			break;
	}//end switch paramID...

	return(res);
}//end GetParameter.

/*
---------------------------------------------------------------------------
	Unit Test Methods.
---------------------------------------------------------------------------
*/
/** Test level prefix & suffix combinations.
Iterate through a large range of level codes for all possible combinations
of suffix lengths. This method tests the algorithm not in-place.
@return	: 1 = success, 0 = failure.
*/
int CAVLCH264Impl::TestLevelPrefixSuffix(void)
{
	int result = 1;

	/// Variables to emulate.
	int levelPrefix, levelSuffix, levelSuffixSize;
	int encLevelCode, decLevelCode, suffixLength;
	int upper, lower;

	for(suffixLength = 0; suffixLength <= 6; suffixLength++)
		for(encLevelCode = 1; encLevelCode < 65536; encLevelCode++)
		{
			/// --- Encode suffix length & level code pairs ---

			int done = 0;
			levelSuffixSize = suffixLength;
			/// Use levelPrefix = 14 as an ESC code for 1st trailing large 
			/// levels from 14 - 29 range.
			if(suffixLength == 0)
			{
				if((encLevelCode >= 14)&&(encLevelCode <= 29))
				{
					levelSuffixSize = 4;
					levelSuffix			= (encLevelCode - 14) & 0x0000000F;
					levelPrefix			= 14;
					done						= 1;
				}//end if encLevelCode...
			}//end if !suffixLength...

			if(!done)
			{
				/// Max possible levelCode with this suffixLength and 
				/// levelPrefix less than 15.
				int maxLevelCode = (14 << suffixLength) + ~(0xFFFFFFFF << suffixLength);

				/// Extract level prefix and suffix.
				if(encLevelCode <= maxLevelCode)
				{
					levelPrefix = encLevelCode >> suffixLength;
					levelSuffix = encLevelCode & ~(0xFFFFFFFF << suffixLength);
				}//end if encLevelCode...
				else	///< Large code levels.
				{
					int lev = encLevelCode;
					/// Entering here the levelCode is always greater than the level 
					/// associated with a max prefix = 15 therefore it can be subtracted 
					/// to promote smaller levels.
					lev -= (15 << suffixLength);
					/// For a suffixLength of zero the max levelCode can be further 
					/// reduced by 15 as levelCodes less than 30 are catered for by
					/// the ESC code with LevelPrefix = 14 described above.
					if(suffixLength == 0)
						lev -= 15;
					/// The levelPrefix to set defines the range between octaves in
					/// the following way:
					///	LevelPrefix		Range (2^(levelPrefix-3)) offset by (2^x - 1)(2^12)
					///	15						0..((2^12)-1)
					///	16												2^12..((2^13)+(2^12)-1)
					///	17																							((2^13)+(2^12))..((2^14)+(2^12)-1)
					///	etc.
					/// Note that the min value for each range can be subtracted to save
					/// 1 bit per range.
					levelPrefix = 15;
					/// Iterate the levelPrefix by checking if levelCode is larger than the min
					/// levelCode for the next larger range. Put a hard limit max bit size to 32 (= 35 - 3).
					while((lev >= ((1 << (levelPrefix-2)) - 4096))&&(levelPrefix < 35))
						levelPrefix++;
					if(levelPrefix >= 16)
						lev -= (1 << (levelPrefix-3)) - 4096;	///< Reduce by min value for this range.

					levelSuffixSize = levelPrefix - 3;
					levelSuffix			= lev & ~(0xFFFFFFFF << levelSuffixSize);
				}//end else...
			}//end if !done...

			/// Emulate writing to stream.
			upper = levelPrefix;
			lower = levelSuffix;

			/// --- Decode suffix length & level code pairs ---

			/// Emulate reading levelPrefix from stream.
			levelPrefix = upper;

			levelSuffixSize = suffixLength;
			/// Modify the suffix bits to extract large values.
			if(levelPrefix >= 15)
				levelSuffixSize = levelPrefix - 3;
			else if((levelPrefix == 14)&&(suffixLength == 0))
				levelSuffixSize = 4;

			levelSuffix = 0;
			if(levelSuffixSize > 0)
			{
				/// Emulate level_suffix is parsed from the bit stream.
				levelSuffix = lower & ~(0xFFFFFFFF << levelSuffixSize);
			}//end if levelSuffixSize...

			/// Shift levelPrefix left into bit position defined by suffixLength
			/// and add the levelSuffix.
			if(levelPrefix < 15)
				decLevelCode = (levelPrefix << suffixLength) + levelSuffix;
			else
				decLevelCode = (15 << suffixLength) + levelSuffix;

			if((levelPrefix >= 15) && (suffixLength == 0))	///< Special condition for large high freq trailing levels.
				decLevelCode += 15;

			if(levelPrefix >= 16)
				decLevelCode += (1 << (levelPrefix-3)) - 4096;	///< Special condition for all large values.

			if(decLevelCode != encLevelCode)
				result = 0;
		}//end for suffixLength & encLevelCode...

		return(result);
}//end TestLevelPrefixSuffix.

