/** @file

MODULE				: FastInverse4x4On16x16ITImpl1

TAG						: FII4O16TI1

FILE NAME			: FastInverse4x4On16x16ITImpl1.h

DESCRIPTION		: A class to implement a fast inverse 4x4 2-D integer
								transform defined by the H.264 standard on the 16x16
                macroblock input. It implements the IInverseTransform 
                interface. The pre-scaling is part of the inverse 
                quantisation process.

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

=========================================================================================
*/
#ifndef _FASTINVERSE4X4ON16X16ITIMPL1_H
#define _FASTINVERSE4X4ON16X16ITIMPL1_H

#pragma once

#include "IInverseTransform.h"

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class FastInverse4x4On16x16ITImpl1 : public IInverseTransform
{
	public:
		FastInverse4x4On16x16ITImpl1(void);
		virtual ~FastInverse4x4On16x16ITImpl1()	{ }

	/// Interface implementation.
	public:
		/** In-place inverse IT.
		The inverse IT is performed on the input and replaces it with the coeffs.
		@param ptr	: Data to transform.
		@return			:	none.
		*/
		virtual void InverseTransform(void* ptr);

		/** Transfer inverse IT.
		The inverse IT is performed on the coeffs and are written to 
		the output.
		@param pCoeff	: Input coeffs.
		@param pOut		: Output data.
		@return				:	none.
		*/
		virtual void InverseTransform(void* pCoeff, void* pOut);

		/** Set scaling array.
		Each coefficient may be scaled before transforming and therefore 
		requires setting up.
		@param	pScale:	Scale factor array.
		@return				:	none.
		*/
		virtual void	SetScale(void* pScale);
		virtual void* GetScale(void);

		/** Set/Get the mode of operation.
		Allow an implementation to define modes of operation or
		interpretation of the quantisation process.
		@param mode	: Mode to set.
		@return			: none.
		*/
		virtual void SetMode(int mode) { _mode = mode; }
		virtual int  GetMode(void) { return(_mode); }

		/** Set and get parameters for the implementation.
		@param paramID	: Parameter to set/get.
		@param paramVal	: Parameter value.
		@return					: None (Set) or the param value (Get).
		*/
		virtual void	SetParameter(int paramID, int paramVal);
		virtual int		GetParameter(int paramID);

		/// Constants.
	protected:
		static const int NormAdjust[6][3];
		static const int ColSelector[16];
		
	protected:
		int _mode;	///< 0 = IT+Q, 1 = IT only, 2 = Q only.

		int _weightScale[16];
		int _levelScale[6][16];	///< Pre-calc for each QP % 6.

		/// Hold these members that will only change when q changes.
		int	_q;						///< Current quantisation parameter.
		int	_qm;					///< _q % 6.
		int _qe;					///< _q / 6.
		int _f;						///< Rounding shift = 1 << (3-_qe).
		int _leftScale;		///< Scaling left shift = _qe-4;
		int _rightScale;	///< Scaling right shift = 4-_qe;

};// end class FastInverse4x4On16x16ITImpl1.

#endif	//_FASTINVERSE4X4ON16X16ITIMPL1_H
