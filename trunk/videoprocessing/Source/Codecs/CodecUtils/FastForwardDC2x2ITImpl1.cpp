/** @file

MODULE				: FastForwardDC2x2ITImpl1

TAG						: FFDC2ITI1

FILE NAME			: FastForwardDC2x2ITImpl1.cpp

DESCRIPTION		: A class to implement a fast forward 2x2 2-D integer
								Hardamard transform defined by the H.264 (03/2005) 
								standard on the input. It implements the 
								IForwardTransform interface. The scaling is part of 
								the quantisation process.

COPYRIGHT			:	(c)CSIR 2007-2010 all rights resevered

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

======================================================================================
*/
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include <string.h>
#include "FastForwardDC2x2ITImpl1.h"

/*
---------------------------------------------------------------------------
	Constants.
---------------------------------------------------------------------------
*/

const int FastForwardDC2x2ITImpl1::NormAdjust[6] =
{
	13107, 
	11916, 
	10082, 
	 9362, 
	 8192, 
	 7282
};

/*
---------------------------------------------------------------------------
	Construction and Destruction.
---------------------------------------------------------------------------
*/
FastForwardDC2x2ITImpl1::FastForwardDC2x2ITImpl1()
{ 
	_mode = 0; 
	_q			= 1;
	_qm			= _q % 6;
	_qe			= _q/6;
//	_f			= (1 << (14+_qe))/6;
//	_scale	= 14+_qe;
	_f			= 2 * ((1 << (15+_qe))/3);
	_scale	= 16 + _qe;
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
void FastForwardDC2x2ITImpl1::Transform(void* ptr)
{
	short* block = (short *)ptr;

	/// Only do IT modes = 0 & 1.
	if(_mode != QuantOnly)	///< 2 = Q only.
	{
		/// 1st stage transform.
		int s0 = (int)(block[0]	+ block[1]);
		int s2 = (int)(block[0] - block[1]);
		int s1 = (int)(block[2] + block[3]);
		int s3 = (int)(block[2]	- block[3]);

		if(_mode == TransformAndQuant)	///< 0 = IT+Q.
		{
			/// 2nd stage transform with scaling and quantisation.
			int x0 = s0 + s1;
			if(x0 < 0)
				block[0] = (short)(-( (((-x0) * NormAdjust[_qm]) + _f) >> _scale ));
			else
				block[0] = (short)( ((x0 * NormAdjust[_qm]) + _f) >> _scale );

			int x2 = s0 - s1;
			if(x2 < 0)
				block[2] = (short)(-( (((-x2) * NormAdjust[_qm]) + _f) >> _scale ));
			else
				block[2] = (short)( ((x2 * NormAdjust[_qm]) + _f) >> _scale );

			int x1 = s2 + s3;
			if(x1 < 0)
				block[1] = (short)(-( (((-x1) * NormAdjust[_qm]) + _f) >> _scale ));
			else
				block[1] = (short)( ((x1 * NormAdjust[_qm]) + _f) >> _scale );

			int x3 = s2 - s3;
			if(x3 < 0)
				block[3] = (short)(-( (((-x3) * NormAdjust[_qm]) + _f) >> _scale ));
			else
				block[3] = (short)( ((x3 * NormAdjust[_qm]) + _f) >> _scale );
		}//end if _mode...
		else	///< mode = 1 i.e. IT only.
		{
			/// 2nd stage transform.
			block[0]	= (short)(s0 + s1);
			block[2]	= (short)(s0 - s1);
			block[1]	= (short)(s2 + s3);
			block[3]	= (short)(s2 - s3);
		}//end else...
	}//end if _mode != 2...
	else ///< mode = 2 i.e. Q only.
	{
		/// Scaling and quantisation are combined.
		int i,j;
		for(i = 0; i < 2; i++)
			for(j = 0; j < 2; j++)
			{
				int x = (int)block[i*2 + j];
				if( x < 0 )
					block[i*2 + j] = (short)(-( (((-x) * NormAdjust[_qm]) + _f) >> _scale ));
				else
					block[i*2 + j] = (short)( ((x * NormAdjust[_qm]) + _f) >> _scale );
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
void FastForwardDC2x2ITImpl1::Transform(void* pIn, void* pCoeff)
{
	/// Copy to output and then do in-place inverse transform.
	memcpy(pCoeff, pIn, sizeof(short) * 4);
	Transform(pCoeff);
}//end Transform.

/** Set and get parameters for the implementation.
@param paramID	: Parameter to set/get.
@param paramVal	: Parameter value.
@return					: None (Set) or the param value (Get).
*/
void FastForwardDC2x2ITImpl1::SetParameter(int paramID, int paramVal)
{
	switch(paramID)
	{
		case QUANT_ID:
			{
				// TODO: Put these calcs into a table.
				if(paramVal != _q)	///< Do we need to change member values or keep previous?
				{
					_qm			= paramVal % 6;
					_qe			= paramVal/6;
//					_f			= (1 << (14+_qe))/6;
//					_scale	= 14+_qe;
	        _f			= 2 * ((1 << (15+_qe))/3);
	        _scale	= 16 + _qe;
					_q			= paramVal;
				}//end if q...
			}//end case QUANT_ID...
			break;
		default :
			/// Do nothing.
			break;
	}//end switch paramID...
}//end SetParameter.

int FastForwardDC2x2ITImpl1::GetParameter(int paramID)
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

