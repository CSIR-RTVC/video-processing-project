/** @file

MODULE				: FastInverse4x4On16x16ITImpl1

TAG						: FII4O16TI1

FILE NAME			: FastInverse4x4On16x16ITImpl1.cpp

DESCRIPTION		: A class to implement a fast inverse 4x4 2-D integer
								transform defined by the H.264 standard on the 16x16
                macroblock input. It implements the IInverseTransform 
                interface. The pre-scaling is part of the inverse 
                quantisation process.

COPYRIGHT			:	(c)CSIR 2007-2011 all rights resevered

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

#include <string.h>
#include "FastInverse4x4On16x16ITImpl1.h"

/*
---------------------------------------------------------------------------
	Constants.
---------------------------------------------------------------------------
*/
const int FastInverse4x4On16x16ITImpl1::NormAdjust[6][3] =
{
	{10, 16, 13 },
	{11, 18, 14 },
	{13, 20, 16 },
	{14, 23, 18 },
	{16, 25, 20 },
	{18, 29, 23 }
};

const int FastInverse4x4On16x16ITImpl1::ColSelector[16] =
{
	0, 2, 0, 2 ,
	2, 1, 2, 1 ,
	0, 2, 0, 2 ,
	2, 1, 2, 1 
};

/*
--------------------------------------------------------------------------
	Construction.
--------------------------------------------------------------------------
*/
FastInverse4x4On16x16ITImpl1::FastInverse4x4On16x16ITImpl1(void)
{
	int i,j,qm;

	for(i = 0; i < 16; i++)
		_weightScale[i] = 16;	///< For baseline Flat_4x4_16 is default.

	for(qm = 0; qm < 6; qm++)
		for(i = 0; i < 4; i++)
			for(j = 0; j < 4; j++)
			{
				int pos = 4*i + j;
				_levelScale[qm][pos] = NormAdjust[qm][ColSelector[pos]] * _weightScale[pos];
			}//end for qm & i & j...

	_mode				= TransformAndQuant;
	_q					= 1;
	_qm					= _q % 6;
	_qe					= _q/6;
	_f					= 1 << (3-_qe);
	_leftScale	= _qe-4;
	_rightScale	= 4-_qe;

}//end constructor.

/*
---------------------------------------------------------------------------
	Interface Methods.
---------------------------------------------------------------------------
*/
/** In-place inverse Integer Transform.
The 2-D inverse IT is performed on the input coeffs and replaces them. A 1-D
inverse transform is performed on the rows first and then the cols. 
@param ptr	: Data to transform.
@return			:	none.
*/
void FastInverse4x4On16x16ITImpl1::InverseTransform(void* ptr)
{
	short* block = (short *)ptr;
	int j;

	if(_mode == QuantOnly)	///< 2 = Q only.
	{
		/// Pre-scaling and quantisation are combined.
		for(int i = 0; i < 16; i++)
    {
      int k = i % 4;
 		  for(j = 0; j < 16; j++)
		  {
        int l = j % 4;
        int x = (int)block[i*16 + j] * _levelScale[_qm][k*4 +l];

			  if( _q < 24 )
					block[i*16 + j] = (short)( (x + _f) >> _rightScale );
				else
					block[i*16 + j] = (short)( x << _leftScale );

		  }//end for j...
    }//end for i...
	}//end if _mode == 2...
	else	///< IT&Q and IT only.
	{
		/// 1-D inverse IT in vert direction.

		if(_mode == TransformAndQuant)	///< 0 = IT&Q.
		{
			for(j = 0; j < 16; j++)
			{
				/// 1st stage with pre-scaling and quantisation.
        int k = j % 4;

				int x0 = (int)block[j]		* _levelScale[_qm][k];
				int x1 = (int)block[j+16]	* _levelScale[_qm][k+4];
				int x2 = (int)block[j+32]	* _levelScale[_qm][k+8];
				int x3 = (int)block[j+48]	* _levelScale[_qm][k+12];

				int x4 = (int)block[j+64]	  * _levelScale[_qm][k];
				int x5 = (int)block[j+80]	  * _levelScale[_qm][k+4];
				int x6 = (int)block[j+96]	  * _levelScale[_qm][k+8];
				int x7 = (int)block[j+112]	* _levelScale[_qm][k+12];

				int x8  = (int)block[j+128]	* _levelScale[_qm][k];
				int x9  = (int)block[j+144]	* _levelScale[_qm][k+4];
				int x10 = (int)block[j+160]	* _levelScale[_qm][k+8];
				int x11 = (int)block[j+176]	* _levelScale[_qm][k+12];

				int x12 = (int)block[j+192]	* _levelScale[_qm][k];
				int x13 = (int)block[j+208]	* _levelScale[_qm][k+4];
				int x14 = (int)block[j+224]	* _levelScale[_qm][k+8];
				int x15 = (int)block[j+240]	* _levelScale[_qm][k+12];

				if(_q < 24)
				{
					x0  = (x0 + _f) >> _rightScale;
					x1  = (x1 + _f) >> _rightScale;
					x2  = (x2 + _f) >> _rightScale;
					x3  = (x3 + _f) >> _rightScale;
					x4  = (x4 + _f) >> _rightScale;
					x5  = (x5 + _f) >> _rightScale;
					x6  = (x6 + _f) >> _rightScale;
					x7  = (x7 + _f) >> _rightScale;
					x8  = (x8 + _f) >> _rightScale;
					x9  = (x9 + _f) >> _rightScale;
					x10 = (x10 + _f) >> _rightScale;
					x11 = (x11 + _f) >> _rightScale;
					x12 = (x12 + _f) >> _rightScale;
					x13 = (x13 + _f) >> _rightScale;
					x14 = (x14 + _f) >> _rightScale;
					x15 = (x15 + _f) >> _rightScale;
				}//end if _q...
				else
				{
					x0  = x0 << _leftScale;
					x1  = x1 << _leftScale;
					x2  = x2 << _leftScale;
					x3  = x3 << _leftScale;
					x4  = x4 << _leftScale;
					x5  = x5 << _leftScale;
					x6  = x6 << _leftScale;
					x7  = x7 << _leftScale;
					x8  = x8 << _leftScale;
					x9  = x9 << _leftScale;
					x10 = x10 << _leftScale;
					x11 = x11 << _leftScale;
					x12 = x12 << _leftScale;
					x13 = x13 << _leftScale;
					x14 = x14 << _leftScale;
					x15 = x15 << _leftScale;
				}//end else...

				int s0 = x0					+ x2;
				int s1 = x0					- x2;
				int s2 = (x1 >> 1)	- x3;
				int s3 = x1					+ (x3 >> 1);

				int s4 = x4					+ x6;
				int s5 = x4					- x6;
				int s6 = (x5 >> 1)	- x7;
				int s7 = x5					+ (x7 >> 1);

				int s8  = x8				+ x10;
				int s9  = x8				- x10;
				int s10 = (x9 >> 1)	- x11;
				int s11 = x9				+ (x11 >> 1);

				int s12 = x12				 + x14;
				int s13 = x12				 - x14;
				int s14 = (x13 >> 1) - x15;
				int s15 = x13				 + (x15 >> 1);

				/// 2nd stage.
				block[j]		= (short)(s0 + s3);
				block[48+j]	= (short)(s0 - s3);
				block[16+j]	= (short)(s1 + s2);
				block[32+j]	= (short)(s1 - s2);

				block[j+64]	  = (short)(s4 + s7);
				block[112+j]	= (short)(s4 - s7);
				block[80+j]	  = (short)(s5 + s6);
				block[96+j]	  = (short)(s5 - s6);

				block[j+128]	= (short)(s8 + s11);
				block[176+j]	= (short)(s8 - s11);
				block[144+j]	= (short)(s9 + s10);
				block[160+j]	= (short)(s9 - s10);

				block[j+192]	= (short)(s12 + s15);
				block[240+j]	= (short)(s12 - s15);
				block[208+j]	= (short)(s13 + s14);
				block[224+j]	= (short)(s13 - s14);

			}//end for j...
		}//end if _mode == 0...
		else	///< _mode == 1 IT only.
		{
			for(j = 0; j < 16; j++)
			{
				/// 1st stage.
				int s0 = (int)block[j]					  + (int)block[32+j];
				int s1 = (int)block[j]					  - (int)block[32+j];
				int s2 = (int)(block[16+j] >> 1)	- (int)block[48+j];
				int s3 = (int)block[16+j]				  + (int)(block[48+j] >> 1);

				int s4 = (int)block[j+64]					+ (int)block[96+j];
				int s5 = (int)block[j+64]					- (int)block[96+j];
				int s6 = (int)(block[80+j] >> 1)	- (int)block[112+j];
				int s7 = (int)block[80+j]				  + (int)(block[112+j] >> 1);

				int s8  = (int)block[j+128]					+ (int)block[160+j];
				int s9  = (int)block[j+128]					- (int)block[160+j];
				int s10 = (int)(block[144+j] >> 1)	- (int)block[176+j];
				int s11 = (int)block[144+j]				  + (int)(block[176+j] >> 1);

				int s12 = (int)block[j+192]					+ (int)block[224+j];
				int s13 = (int)block[j+192]					- (int)block[224+j];
				int s14 = (int)(block[208+j] >> 1)	- (int)block[240+j];
				int s15 = (int)block[208+j]				  + (int)(block[240+j] >> 1);

				/// 2nd stage.
				block[j]		= (short)(s0 + s3);
				block[48+j]	= (short)(s0 - s3);
				block[16+j]	= (short)(s1 + s2);
				block[32+j]	= (short)(s1 - s2);

				block[64+j]		= (short)(s4 + s7);
				block[112+j]	= (short)(s4 - s7);
				block[80+j]	  = (short)(s5 + s6);
				block[96+j]	  = (short)(s5 - s6);

				block[128+j]	= (short)(s8 + s11);
				block[176+j]	= (short)(s8 - s11);
				block[144+j]	= (short)(s9 + s10);
				block[160+j]	= (short)(s9 - s10);

				block[192+j]	= (short)(s12 + s15);
				block[240+j]	= (short)(s12 - s15);
				block[208+j]	= (short)(s13 + s14);
				block[224+j]	= (short)(s13 - s14);

			}//end for j...
		}//end else...

		/// 1-D inverse IT in horiz direction.
		for(j = 0; j < 256; j += 16)
		{
			/// 1st stage.
			int s0 = (int)block[j]					+ (int)block[2+j];
			int s1 = (int)block[j]					- (int)block[2+j];
			int s2 = (int)(block[1+j] >> 1)	- (int)block[3+j];
			int s3 = (int)block[1+j]				+ (int)(block[3+j] >> 1);

			int s4 = (int)block[j+4]					+ (int)block[6+j];
			int s5 = (int)block[j+4]					- (int)block[6+j];
			int s6 = (int)(block[5+j] >> 1)	  - (int)block[7+j];
			int s7 = (int)block[5+j]				  + (int)(block[7+j] >> 1);

			int s8  = (int)block[j+8]					+ (int)block[10+j];
			int s9  = (int)block[j+8]					- (int)block[10+j];
			int s10 = (int)(block[9+j] >> 1)	- (int)block[11+j];
			int s11 = (int)block[9+j]				  + (int)(block[11+j] >> 1);

			int s12 = (int)block[j+12]				+ (int)block[14+j];
			int s13 = (int)block[j+12]				- (int)block[14+j];
			int s14 = (int)(block[13+j] >> 1)	- (int)block[15+j];
			int s15 = (int)block[13+j]				+ (int)(block[15+j] >> 1);

			/// 2nd stage.
			block[j]		= (short)((s0 + s3 + 32) >> 6);
			block[3+j]	= (short)((s0 - s3 + 32) >> 6);
			block[1+j]	= (short)((s1 + s2 + 32) >> 6);
			block[2+j]	= (short)((s1 - s2 + 32) >> 6);

			block[4+j]	= (short)((s4 + s7 + 32) >> 6);
			block[7+j]	= (short)((s4 - s7 + 32) >> 6);
			block[5+j]	= (short)((s5 + s6 + 32) >> 6);
			block[6+j]	= (short)((s5 - s6 + 32) >> 6);

			block[8+j]	= (short)((s8 + s11 + 32) >> 6);
			block[11+j]	= (short)((s8 - s11 + 32) >> 6);
			block[9+j]	= (short)((s9 + s10 + 32) >> 6);
			block[10+j]	= (short)((s9 - s10 + 32) >> 6);

			block[12+j]	= (short)((s12 + s15 + 32) >> 6);
			block[15+j]	= (short)((s12 - s15 + 32) >> 6);
			block[13+j]	= (short)((s13 + s14 + 32) >> 6);
			block[14+j]	= (short)((s13 - s14 + 32) >> 6);

		}//end for j...
	}//end else...

}//end InverseTransform.

/** Transfer inverse IT.
The inverse IT is performed on the coeffs and are written to 
the output.
@param pCoeff	: Input coeffs.
@param pOut		: Output data.
@return				:	none.
*/
void FastInverse4x4On16x16ITImpl1::InverseTransform(void* pCoeff, void* pOut)
{
	/// Copy to output and then do in-place inverse transform.
	memcpy(pOut, pCoeff, sizeof(short) * 256);
	InverseTransform(pOut);
}//end InverseTransform.

/** Set scaling array.
Each coefficient may be scaled before transforming and therefore 
requires setting up.
@param	pScale:	Scale factor array.
@return				:	none.
*/
void FastInverse4x4On16x16ITImpl1::SetScale(void* pScale)
{
	int		i,j,qm;
	int*	ptr = (int *)pScale;

	for(i = 0; i < 16; i++)
		_weightScale[i] = ptr[i];

	for(qm = 0; qm < 6; qm++)
		for(i = 0; i < 4; i++)
			for(j = 0; j < 4; j++)
			{
				int pos = 4*i + j;
				_levelScale[qm][pos] = NormAdjust[qm][ColSelector[pos]] * _weightScale[pos];
			}//end for qm & i & j...

}//end SetScale.


void* FastInverse4x4On16x16ITImpl1::GetScale(void)
{
	return( (void *)(_weightScale) );
}//end GetScale.

/** Set and get parameters for the implementation.
@param paramID	: Parameter to set/get.
@param paramVal	: Parameter value.
@return					: None (Set) or the param value (Get).
*/
void FastInverse4x4On16x16ITImpl1::SetParameter(int paramID, int paramVal)
{
	switch(paramID)
	{
		case QUANT_ID:
			{
				// TODO: Put these calcs into a table.
				if(paramVal != _q)	///< Do we need to change member values or keep previous?
				{
					_qm					= paramVal % 6;
					_qe					= paramVal/6;
					_f					= 1 << (3-_qe);
					_leftScale	= _qe-4;
					_rightScale	= 4-_qe;
					_q					= paramVal;
				}//end if q...
			}//end case QUANT_ID...
			break;
		default :
			/// Do nothing.
			break;
	}//end switch paramID...
}//end SetParameter.

int FastInverse4x4On16x16ITImpl1::GetParameter(int paramID)
{
	int ret;
	switch(paramID)
	{
		case QUANT_ID:
			ret = _q;
			break;
		default :
			ret = _q;	///< Quant value is returned as default.
			break;
	}//end switch paramID...

	return(ret);
}//end GetParameter.



