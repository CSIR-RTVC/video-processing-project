/** @file

MODULE				: FastInverseDC4x4ITImpl1

TAG						: FIDC4ITI1

FILE NAME			: FastInverseDC4x4ITImpl1.cpp

DESCRIPTION		: A class to implement a fast inverse 4x4 2-D integer
								Hardamard transform defined by the H.264 (03/2005) 
								standard on the input. It implements the 
								IInverseTransform interface. The scaling is part of 
								the quantisation process.

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

==========================================================================================
*/
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include <string.h>
#include "FastInverseDC4x4ITImpl1.h"

/*
---------------------------------------------------------------------------
	Constants.
---------------------------------------------------------------------------
*/
const int FastInverseDC4x4ITImpl1::NormAdjust[6] =
{
	10, 
	11, 
	13, 
	14, 
	16, 
	18
};

/*
--------------------------------------------------------------------------
	Construction.
--------------------------------------------------------------------------
*/
FastInverseDC4x4ITImpl1::FastInverseDC4x4ITImpl1(void)
{
	int i,j,qm;

	for(i = 0; i < 16; i++)
		_weightScale[i] = 16;	///< Flat_4x4_16 is default.

	for(qm = 0; qm < 6; qm++)
		for(i = 0; i < 4; i++)
			for(j = 0; j < 4; j++)
			{
				int pos = 4*i + j;
				_levelScale[qm][pos] = NormAdjust[qm] * _weightScale[pos];
			}//end for qm & i & j...

	_mode				= 0;
	_q					= 1;
	_qm					= _q % 6;
	_qe					= _q/6;
	_f					= 1 << (5-_qe);
	_leftScale	= _qe-6;
	_rightScale	= 6-_qe;
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
void FastInverseDC4x4ITImpl1::InverseTransform(void* ptr)
{
	short* block = (short *)ptr;
	int j;

	if(_mode != QuantOnly)	///< IT&Q and IT only.
	{
		/// 1-D inverse IT in vert direction.
		for(j = 0; j < 4; j++)
		{
			/// 1st stage.
			int s0 = (int)block[j]		+ (int)block[8+j];
			int s1 = (int)block[j]		- (int)block[8+j];
			int s2 = (int)block[4+j]	- (int)block[12+j];
			int s3 = (int)block[4+j]	+ (int)block[12+j];

			/// 2nd stage.
			block[j]		= (short)(s0 + s3);
			block[12+j]	= (short)(s0 - s3);
			block[4+j]	= (short)(s1 + s2);
			block[8+j]	= (short)(s1 - s2);
		}//end for j...

		if(_mode == TransformAndQuant)	///< 0 = IT&Q.
		{
			/// 1-D inverse IT in horiz direction.
			for(j = 0; j < 16; j += 4)
			{
				/// 1st stage.
				int s0 = (int)block[j]		+ (int)block[2+j];
				int s1 = (int)block[j]		- (int)block[2+j];
				int s2 = (int)block[1+j]	- (int)block[3+j];
				int s3 = (int)block[1+j]	+ (int)block[3+j];

				/// 2nd stage with post-scaling and quantisation.
				int x0	= (s0 + s3) * _levelScale[_qm][j];
				int x3	= (s0 - s3) * _levelScale[_qm][j+3];
				int x1	= (s1 + s2) * _levelScale[_qm][j+1];
				int x2	= (s1 - s2) * _levelScale[_qm][j+2];

				if(_q < 36)
				{
					block[j]		= (short)((x0 + _f) >> _rightScale);
					block[j+1]	= (short)((x1 + _f) >> _rightScale);
					block[j+2]	= (short)((x2 + _f) >> _rightScale);
					block[j+3]	= (short)((x3 + _f) >> _rightScale);
				}//end if _q...
				else
				{
					block[j]		= (short)(x0 << _leftScale);
					block[j+1]	= (short)(x1 << _leftScale);
					block[j+2]	= (short)(x2 << _leftScale);
					block[j+3]	= (short)(x3 << _leftScale);
				}//end else...
			}//end for j...
		}//end if _mode == 0...
		else	///< _mode == 1 IT only.
		{
			/// 1-D inverse IT in horiz direction.
			for(j = 0; j < 16; j += 4)
			{
				/// 1st stage.
				int s0 = (int)block[j]		+ (int)block[2+j];
				int s1 = (int)block[j]		- (int)block[2+j];
				int s2 = (int)block[1+j]	- (int)block[3+j];
				int s3 = (int)block[1+j]	+ (int)block[3+j];

				/// 2nd stage.
				block[j]		= (short)(s0 + s3);
				block[3+j]	= (short)(s0 - s3);
				block[1+j]	= (short)(s1 + s2);
				block[2+j]	= (short)(s1 - s2);
			}//end for j...
		}//end else...

	}//end if _mode != 2...
	else	///< _mode 2 = Q only.
	{
		/// Post-scaling and quantisation are combined.
		int i;
		for(i = 0; i < 4; i++)
			for(j = 0; j < 4; j++)
			{
				int x = (int)block[i*4 + j] * _levelScale[_qm][i*4 +j];
				if(_q < 36)
					block[i*4 + j] = (short)( (x + _f) >> _rightScale );
				else
					block[i*4 + j] = (short)( x << _leftScale );
			}//end for i & j...
	}//end else...

}//end InverseTransform.

/** Transfer inverse IT.
The inverse IT is performed on the coeffs and are written to 
the output.
@param pCoeff	: Input coeffs.
@param pOut		: Output data.
@return				:	none.
*/
void FastInverseDC4x4ITImpl1::InverseTransform(void* pCoeff, void* pOut)
{
	/// Copy to output and then do in-place inverse transform.
	memcpy(pOut, pCoeff, sizeof(short) * 16);
	InverseTransform(pOut);
}//end InverseTransform.

/** Set scaling array.
Each coefficient may be scaled before transforming and therefore 
requires setting up.
@param	pScale:	Scale factor array.
@return				:	none.
*/
void FastInverseDC4x4ITImpl1::SetScale(void* pScale)
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
				_levelScale[qm][pos] = NormAdjust[qm] * _weightScale[pos];
			}//end for qm & i & j...

}//end SetScale.


void* FastInverseDC4x4ITImpl1::GetScale(void)
{
	return( (void *)(_weightScale) );
}//end GetScale.

/** Set and get parameters for the implementation.
@param paramID	: Parameter to set/get.
@param paramVal	: Parameter value.
@return					: None (Set) or the param value (Get).
*/
void FastInverseDC4x4ITImpl1::SetParameter(int paramID, int paramVal)
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
					_f					= 1 << (5-_qe);
					_leftScale	= _qe-6;
					_rightScale	= 6-_qe;
					_q					= paramVal;
				}//end if q...
			}//end case QUANT_ID...
			break;
		default :
			/// Do nothing.
			break;
	}//end switch paramID...
}//end SetParameter.

int FastInverseDC4x4ITImpl1::GetParameter(int paramID)
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


