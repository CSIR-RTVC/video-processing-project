/** @file

MODULE				: FastInverse4x4ITImpl1

TAG						: FII4TI1

FILE NAME			: FastInverse4x4ITImpl1.cpp

DESCRIPTION		: A class to implement a fast inverse 4x4 2-D integer
								transform defined by the H.264 standard on the input. 
								It implements the IInverseDct interface. The pre-scaling
								is part of the inverse quantisation process.

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
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include <string.h>
#include "FastInverse4x4ITImpl1.h"

/*
---------------------------------------------------------------------------
	Constants.
---------------------------------------------------------------------------
*/
const int FastInverse4x4ITImpl1::NormAdjust[6][3] =
{
	{10, 16, 13 },
	{11, 18, 14 },
	{13, 20, 16 },
	{14, 23, 18 },
	{16, 25, 20 },
	{18, 29, 23 }
};

const int FastInverse4x4ITImpl1::ColSelector[16] =
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
FastInverse4x4ITImpl1::FastInverse4x4ITImpl1(void)
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
void FastInverse4x4ITImpl1::InverseTransform(void* ptr)
{
	short* block = (short *)ptr;
	int j;

	if(_mode == QuantOnly)	///< 2 = Q only.
	{
		/// Pre-scaling and quantisation are combined.
		int i;
		for(i = 0; i < 4; i++)
			for(j = 0; j < 4; j++)
			{
				int x = (int)block[i*4 + j] * _levelScale[_qm][i*4 +j];
				if(_q < 24)
					block[i*4 + j] = (short)( (x + _f) >> _rightScale );
				else
					block[i*4 + j] = (short)( x << _leftScale );
			}//end for i & j...
	}//end if _mode == 2...
	else if(_mode == TransformAndQuant)	///< 0 = IT&Q.
	{
		/// 1-D inverse IT in horiz direction.
		for(j = 0; j < 16; j += 4)
		{
			/// 1st stage with pre-scaling and quantisation.
			int x0 = (int)block[j]		* _levelScale[_qm][j];
			int x1 = (int)block[j+1]	* _levelScale[_qm][j+1];
			int x2 = (int)block[j+2]	* _levelScale[_qm][j+2];
			int x3 = (int)block[j+3]	* _levelScale[_qm][j+3];
			if(_q < 24)
			{
				x0 = (x0 + _f) >> _rightScale;
				x1 = (x1 + _f) >> _rightScale;
				x2 = (x2 + _f) >> _rightScale;
				x3 = (x3 + _f) >> _rightScale;
			}//end if _q...
			else
			{
				x0 = x0 << _leftScale;
				x1 = x1 << _leftScale;
				x2 = x2 << _leftScale;
				x3 = x3 << _leftScale;
			}//end else...

			int s0 = x0					+ x2;
			int s1 = x0					- x2;
			int s2 = (x1 >> 1)	- x3;
			int s3 = x1					+ (x3 >> 1);

			/// 2nd stage.
			block[j]		= (short)(s0 + s3);
			block[3+j]	= (short)(s0 - s3);
			block[1+j]	= (short)(s1 + s2);
			block[2+j]	= (short)(s1 - s2);
		}//end for j...

		/// 1-D inverse IT in vert direction.
		for(j = 0; j < 4; j++)
		{
			/// 1st stage.
			int s0 = (int)block[j]					+ (int)block[j+8];
			int s1 = (int)block[j]					- (int)block[j+8];
			int s2 = ((int)block[j+4] >> 1)	- (int)block[j+12];
			int s3 = (int)block[j+4]				+ ((int)block[j+12] >> 1);

			/// 2nd stage.
			block[j]		= (short)((s0 + s3 + 32) >> 6);
			block[12+j]	= (short)((s0 - s3 + 32) >> 6);
			block[4+j]	= (short)((s1 + s2 + 32) >> 6);
			block[8+j]	= (short)((s1 - s2 + 32) >> 6);
		}//end for j...

  }//end if _mode == 0...
	else	///< _mode == 1 IT only.
	{
		/// 1-D inverse IT in horiz direction.
		for(j = 0; j < 16; j += 4)
		{
			/// 1st stage.
			int s0 = (int)block[j]					+ (int)block[2+j];
			int s1 = (int)block[j]					- (int)block[2+j];
			int s2 = (int)(block[1+j] >> 1)	- (int)block[3+j];
			int s3 = (int)block[1+j]				+ (int)(block[3+j] >> 1);

			/// 2nd stage.
			block[j]		= (short)(s0 + s3);
			block[3+j]	= (short)(s0 - s3);
			block[1+j]	= (short)(s1 + s2);
			block[2+j]	= (short)(s1 - s2);
		}//end for j...

		/// 1-D inverse IT in vert direction.
		for(j = 0; j < 4; j++)
		{
			/// 1st stage.
			int s0 = (int)block[j]					+ (int)block[8+j];
			int s1 = (int)block[j]					- (int)block[8+j];
			int s2 = (int)(block[4+j] >> 1)	- (int)block[12+j];
			int s3 = (int)block[4+j]				+ (int)(block[12+j] >> 1);

			/// 2nd stage.
			block[j]		= (short)((s0 + s3 + 32) >> 6);
			block[12+j]	= (short)((s0 - s3 + 32) >> 6);
			block[4+j]	= (short)((s1 + s2 + 32) >> 6);
			block[8+j]	= (short)((s1 - s2 + 32) >> 6);

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
void FastInverse4x4ITImpl1::InverseTransform(void* pCoeff, void* pOut)
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
void FastInverse4x4ITImpl1::SetScale(void* pScale)
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


void* FastInverse4x4ITImpl1::GetScale(void)
{
	return( (void *)(_weightScale) );
}//end GetScale.

/** Set and get parameters for the implementation.
@param paramID	: Parameter to set/get.
@param paramVal	: Parameter value.
@return					: None (Set) or the param value (Get).
*/
void FastInverse4x4ITImpl1::SetParameter(int paramID, int paramVal)
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

int FastInverse4x4ITImpl1::GetParameter(int paramID)
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



