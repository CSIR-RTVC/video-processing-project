/** @file

MODULE				: FastInverseDC2x2ITImpl1

TAG						: FIDC2ITI1

FILE NAME			: FastInverseDC2x2ITImpl1.cpp

DESCRIPTION		: A class to implement a fast inverse 2x2 2-D integer
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

========================================================================================
*/
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include <string.h>
#include "FastInverseDC2x2ITImpl1.h"

/*
---------------------------------------------------------------------------
	Constants.
---------------------------------------------------------------------------
*/
const int FastInverseDC2x2ITImpl1::NormAdjust[6] =
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
FastInverseDC2x2ITImpl1::FastInverseDC2x2ITImpl1(void)
{
	int i,j,qm;

	for(i = 0; i < 4; i++)
		_weightScale[i] = 16;	///< Flat_4x4_16 is default.

	for(qm = 0; qm < 6; qm++)
		for(i = 0; i < 2; i++)
			for(j = 0; j < 2; j++)
			{
				int pos = 2*i + j;
				_levelScale[qm][pos] = NormAdjust[qm] * _weightScale[pos];
			}//end for qm & i & j...

	_mode = 0;
	_q					= 1;
	_qm					= _q % 6;
	_qe					= _q/6;
	_leftScale	= _qe;
	_rightScale	= 5;
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
void FastInverseDC2x2ITImpl1::InverseTransform(void* ptr)
{
	short* block = (short *)ptr;

	if(_mode != QuantOnly)	///< IT&Q and IT only.
	{
		/// 1st stage inverse transform.
		int s0 = (int)(block[0]	+ block[1]);
		int s2 = (int)(block[0] - block[1]);
		int s1 = (int)(block[2] + block[3]);
		int s3 = (int)(block[2]	- block[3]);

		if(_mode == TransformAndQuant)	///< 0 = IT&Q.
		{
			/// 2nd stage with post-scaling and quantisation.
			int x0	= (s0 + s1) * _levelScale[_qm][0];
			int x2	= (s0 - s1) * _levelScale[_qm][2];
			int x1	= (s2 + s3) * _levelScale[_qm][1];
			int x3	= (s2 - s3) * _levelScale[_qm][3];

			block[0]	= (short)((x0 << _leftScale) >> _rightScale);
			block[1]	= (short)((x1 << _leftScale) >> _rightScale);
			block[2]	= (short)((x2 << _leftScale) >> _rightScale);
			block[3]	= (short)((x3 << _leftScale) >> _rightScale);

		}//end if _mode == 0...
		else	///< _mode == 1 IT only.
		{
			/// 2nd stage transform.
			block[0]	= (short)(s0 + s1);
			block[2]	= (short)(s0 - s1);
			block[1]	= (short)(s2 + s3);
			block[3]	= (short)(s2 - s3);
		}//end else...

	}//end if _mode != 2...
	else	///< _mode 2 = Q only.
	{
		/// Post-scaling and quantisation are combined.
		int i;
		for(i = 0; i < 4; i++)
		{
			int x = (int)block[i] * _levelScale[_qm][i];
			block[i]	= (short)((x << _leftScale) >> _rightScale);
		}//end for i...
	}//end else...

}//end InverseTransform.

/** Transfer inverse IT.
The inverse IT is performed on the coeffs and are written to 
the output.
@param pCoeff	: Input coeffs.
@param pOut		: Output data.
@return				:	none.
*/
void FastInverseDC2x2ITImpl1::InverseTransform(void* pCoeff, void* pOut)
{
	/// Copy to output and then do in-place inverse transform.
	memcpy(pOut, pCoeff, sizeof(short) * 4);
	InverseTransform(pOut);
}//end InverseTransform.

/** Set scaling array.
Each coefficient may be scaled before transforming and therefore 
requires setting up.
@param	pScale:	Scale factor array.
@return				:	none.
*/
void FastInverseDC2x2ITImpl1::SetScale(void* pScale)
{
	int		i,j,qm;
	int*	ptr = (int *)pScale;

	for(i = 0; i < 4; i++)
		_weightScale[i] = ptr[i];

	for(qm = 0; qm < 6; qm++)
		for(i = 0; i < 2; i++)
			for(j = 0; j < 2; j++)
			{
				int pos = 2*i + j;
				_levelScale[qm][pos] = NormAdjust[qm] * _weightScale[pos];
			}//end for qm & i & j...

}//end SetScale.


void* FastInverseDC2x2ITImpl1::GetScale(void)
{
	return( (void *)(_weightScale) );
}//end GetScale.

/** Set and get parameters for the implementation.
@param paramID	: Parameter to set/get.
@param paramVal	: Parameter value.
@return					: None (Set) or the param value (Get).
*/
void FastInverseDC2x2ITImpl1::SetParameter(int paramID, int paramVal)
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
					_leftScale	= _qe;
					_rightScale	= 5;
					_q					= paramVal;
				}//end if q...
			}//end case QUANT_ID...
			break;
		default :
			/// Do nothing.
			break;
	}//end switch paramID...
}//end SetParameter.

int FastInverseDC2x2ITImpl1::GetParameter(int paramID)
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


