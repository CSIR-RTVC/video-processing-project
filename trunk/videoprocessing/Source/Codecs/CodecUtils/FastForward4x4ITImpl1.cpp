/** @file

MODULE				: FastForward4x4ITImpl1

TAG						: FF4ITI1

FILE NAME			: FastForward4x4ITImpl1.cpp

DESCRIPTION		: A class to implement a fast forward 4x4 2-D integer

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

======================================================================================
*/
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include <string.h>
#include "FastForward4x4ITImpl1.h"

/*
---------------------------------------------------------------------------
	Constants.
---------------------------------------------------------------------------
*/
const int FastForward4x4ITImpl1::NormAdjust[6][3] =
{
	{13107, 5243, 8066 },
	{11916, 4660, 7490 },
	{10082, 4194, 6554 },
	{ 9362, 3647, 5825 },
	{ 8192, 3355, 5243 },
	{ 7282, 2893, 4559 }
};

const int FastForward4x4ITImpl1::ColSelector[16] =
{
	0, 2, 0, 2 ,
	2, 1, 2, 1 ,
	0, 2, 0, 2 ,
	2, 1, 2, 1 
};

FastForward4x4ITImpl1::FastForward4x4ITImpl1()	
{ 
	_mode		= TransformAndQuant; 
	_intra	= 1;
	_q			= 1;
	_qm			= _q % 6;
	_qe			= _q/6;
	if(_intra)
		_f = (1 << (15+_qe))/3;
	else
		_f = (1 << (15+_qe))/6;
	_scale	= 15+_qe;
}//end constructor.

/*
---------------------------------------------------------------------------
	Interface Methods.
---------------------------------------------------------------------------
*/
/** In-place forward Integer Transform.
The 2-D IT is performed on the input and replaces it with the coeffs. A 1-D
transform is performed on the rows first and then the cols.
@param ptr	: Data to transform.
@return			:	none.
*/
void FastForward4x4ITImpl1::Transform(void* ptr)
{
	short* block = (short *)ptr;
	int j;

	/// Only do IT modes = 0 & 1.
	if(_mode != QuantOnly)	///< 2 = Q only.
	{
		/// 1-D forward horiz direction.
		for(j = 0; j < 16; j += 4)
		{
			/// 1st stage transform.
			int s0 = (int)(block[j]		+ block[j+3]);
			int s3 = (int)(block[j]		- block[j+3]);
			int s1 = (int)(block[j+1] + block[j+2]);
			int s2 = (int)(block[j+1] - block[j+2]);

			/// 2nd stage transform.
			block[j]		= (short)(s0 + s1);
			block[j+2]	= (short)(s0 - s1);
			block[j+1]	= (short)(s2 + (s3 << 1));
			block[j+3]	= (short)(s3 - (s2 << 1));
		}//end for j...

		/// 1-D forward vert direction.

		if(_mode == TransformAndQuant)	///< 0 = IT+Q.
		{
			for(j = 0; j < 4; j++)
			{
				/// 1st stage transform.
				int s0 = (int)block[j]		+ (int)block[j+12];
				int s3 = (int)block[j]		- (int)block[j+12];
				int s1 = (int)block[j+4]	+ (int)block[j+8];
				int s2 = (int)block[j+4]	- (int)block[j+8];

				/// 2nd stage transform with scaling and quantisation.
				int x0 = s0 + s1;
				int r	= ColSelector[j];
				if(x0 < 0)
					block[j] = (short)(-( (((-x0) * NormAdjust[_qm][r]) + _f) >> _scale ));
				else
					block[j] = (short)( ((x0 * NormAdjust[_qm][r]) + _f) >> _scale );

				int x2 = s0 - s1;
				r	= ColSelector[j+8];
				if(x2 < 0)
					block[j+8] = (short)(-( (((-x2) * NormAdjust[_qm][r]) + _f) >> _scale ));
				else
					block[j+8] = (short)( ((x2 * NormAdjust[_qm][r]) + _f) >> _scale );

				int x1 = s2 + (s3 << 1);
				r	= ColSelector[j+4];
				if(x1 < 0)
					block[j+4] = (short)(-( (((-x1) * NormAdjust[_qm][r]) + _f) >> _scale ));
				else
					block[j+4] = (short)( ((x1 * NormAdjust[_qm][r]) + _f) >> _scale );

				int x3 = s3 - (s2 << 1);
				r	= ColSelector[j+12];
				if(x3 < 0)
					block[j+12] = (short)(-( (((-x3) * NormAdjust[_qm][r]) + _f) >> _scale ));
				else
					block[j+12] = (short)( ((x3 * NormAdjust[_qm][r]) + _f) >> _scale );
			}//end for j...
		}//end if _mode...
		else	///< mode = 1 i.e. IT only.
		{
			for(j = 0; j < 4; j++)
			{
				/// 1st stage transform.
				int s0 = (int)(block[j]		+ block[j+12]);
				int s3 = (int)(block[j]		- block[j+12]);
				int s1 = (int)(block[j+4] + block[j+8]);
				int s2 = (int)(block[j+4] - block[j+8]);

				/// 2nd stage transform.
				block[j]		= (short)(s0 + s1);
				block[j+8]	= (short)(s0 - s1);
				block[j+4]	= (short)(s2 + (s3 << 1));
				block[j+12]	= (short)(s3 - (s2 << 1));
			}//end for j...
		}//end else...
	}//end if _mode != 2...
	else ///< mode = 2 i.e. QuantOnly.
	{
		/// Scaling and quantisation are combined.
		int i;
		for(i = 0; i < 4; i++)
			for(j = 0; j < 4; j++)
			{
				int x = (int)block[i*4 + j];
				int r = ColSelector[i*4 + j];
				if( x < 0 )
					block[i*4 + j] = (short)(-( (((-x) * NormAdjust[_qm][r]) + _f) >> _scale ));
				else
					block[i*4 + j] = (short)( ((x * NormAdjust[_qm][r]) + _f) >> _scale );
			}//end for i & j...
	}//end else...

}//end Transform.

/** Transfer forward IT.
The IT is performed on the input and the coeffs are written to 
the output.
@param pIn		: Input data.
@param pCoeff	: Output coeffs.
@return				:	none.
*/
void FastForward4x4ITImpl1::Transform(void* pIn, void* pCoeff)
{
	/// Copy to output and then do in-place inverse transform.
	memcpy(pCoeff, pIn, sizeof(short) * 16);
	Transform(pCoeff);
}//end Transform.

/** Set and get parameters for the implementation.
An Intra and Inter parameter and quantisation parameter are the only 
requirement for this implementation.
@param paramID	: Parameter to set/get.
@param paramVal	: Parameter value.
@return					: None (Set) or the param value (Get).
*/
void FastForward4x4ITImpl1::SetParameter(int paramID, int paramVal)
{
	switch(paramID)
	{
		case QUANT_ID:
			{
				// TODO: Put these calcs into a table.
				if(paramVal != _q)	///< Do we need to change member values or keep previous?
				{
					_q = paramVal;
          SetNewQP();
				}//end if q...
			}//end case QUANT_ID...
			break;
		case INTRA_FLAG_ID:
			_intra = paramVal;
      SetNewQP();
			break;
		default :
			/// Do nothing.
			break;
	}//end switch paramID...
}//end SetParameter.

int FastForward4x4ITImpl1::GetParameter(int paramID)
{
	int ret;
	switch(paramID)
	{
		case QUANT_ID:
			ret = _q;
			break;
		case INTRA_FLAG_ID:
			ret = _intra;
			break;
		default :
			ret = _intra;	///< Intra flag is returned as default.
			break;
	}//end switch paramID...

	return(ret);
}//end GetParameter.

/** Set internal quant members based on _q.
@return: none.
*/
void FastForward4x4ITImpl1::SetNewQP(void)
{
	_qm			= _q % 6;
	_qe			= _q/6;
	if(_intra)
		_f = (1 << (15+_qe))/3;
	else
		_f = (1 << (15+_qe))/6;
	_scale	= 15+_qe;
}//end SetNewQP.
