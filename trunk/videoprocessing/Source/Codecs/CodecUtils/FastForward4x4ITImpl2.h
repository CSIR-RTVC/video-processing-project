/** @file

MODULE				: FastForward4x4ITImpl2

TAG						: FF4ITI2

FILE NAME			: FastForward4x4ITImpl2.h

DESCRIPTION		: A class to implement a fast forward 4x4 2-D integer
								transform defined by the H.264 standard on the input. 
								It implements the IForwardTransform interface. The 
								scaling	is part of the quantisation process.

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

===========================================================================================
*/
#ifndef _FASTFORWARD4X4ITIMPL2_H
#define _FASTFORWARD4X4ITIMPL2_H

#pragma once

#include "IForwardTransform.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class FastForward4x4ITImpl2 : public IForwardTransform
{
	public:
		FastForward4x4ITImpl2();
		virtual ~FastForward4x4ITImpl2()	{ }

	// Interface implementation.
	public:
		/** In-place forward IT.
		The IT is performed on the input and replaces it with the coeffs.
		@param ptr	: Data to transform.
		@return			:	none.
		*/
		virtual void Transform(void* ptr);

		/** Transfer forward IT.
		The IT is performed on the input and the coeffs are written to 
		the output.
		@param pIn		: Input data.
		@param pCoeff	: Output coeffs.
		@return				:	none.
		*/
		virtual void Transform(void* pIn, void* pCoeff);

		/** Set scaling array.
		Each coefficient may be scaled after transforming and therefore 
		requires setting up. Only the default scaling is used and 
		therefore this method is not implemented.
		@param	pScale:	Scale factor array.
		@return				:	none.
		*/
		virtual void	SetScale(void* pScale) { }
		virtual void* GetScale(void) { return(NULL); }

		/** Set/Get the mode of operation.
		The modes for this implementation set the combinations of 
		including or excluding the transform with or without quantisation.
		@param mode	: Mode to set.
		@return			: none.
		*/
		virtual void SetMode(int mode) { _mode = mode; }
		virtual int  GetMode(void) { return(_mode); }

		/** Set and get parameters for the implementation.
		An Intra and Inter parameter and the quantisation parameter are the 
		only requirement for this	implementation.
		@param paramID	: Parameter to set/get.
		@param paramVal	: Parameter value.
		@return					: None (Set) or the param value (Get).
		*/
		virtual void	SetParameter(int paramID, int paramVal);
		virtual int		GetParameter(int paramID);

    /// Internal methods
  private:
    /** Set internal quant members based on _q.
    @return: none.
    */
    void SetNewQP(void);

		/// Constants.
	protected:
		static const int NormAdjust[6][3];
		static const int ColSelector[16];

	protected:
		int _mode;	///< 0 = IT+Q, 1 = IT only, 2 = Q only.
		int _intra;	///< 0 = Inter, 1 = Intra.

		/// Hold these members that will only change when q or intra flag changes.
		int	_q;				///< Current quantisation parameter.
		int	_qm;			///< _q % 6.
		int _qe;			///< _q / 6.
		int _f;				///< Rounding shift = (1 << (15+_qe))/6.
		int _scale;		///< Scaling shift = 15+_qe;

};// end class FastForward4x4ITImpl2.

#endif	//_FASTFORWARD4X4ITIMPL2_H
