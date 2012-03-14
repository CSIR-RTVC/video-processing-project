/** @file

MODULE				: IForwardTransform

TAG						: IFT

FILE NAME			: IForwardTransform.h

DESCRIPTION		: An interface to forward transform implementations. The 
								interface has 2 options as either in-place or from an 
								input to an output parameter argument. Further, they 
								may include or exclude the quantisation and scaling. 
								The implementations must define the mem type (e.g. short, 
								int) and whether it is a 2-D on a block or 1-D on an 
								array.

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

=====================================================================================
*/
#ifndef _IFORWARDTRANSFORM_H
#define _IFORWARDTRANSFORM_H

/*
---------------------------------------------------------------------------
	Interface definition.
---------------------------------------------------------------------------
*/
class IForwardTransform
{
public:
	virtual ~IForwardTransform() {}
	
	/** In-place forward transform.
	The transform is performed on the input and replaces it with the coeffs.
	@param ptr	: Data to transform.
	@return			:	none.
	*/
	virtual void Transform(void* ptr) = 0;

	/** Transfer forward transform.
	The transform is performed on the input and the coeffs are written to 
	the output.
	@param pIn		: Input data.
	@param pCoeff	: Output coeffs.
	@return				:	none.
	*/
	virtual void Transform(void* pIn, void* pCoeff) = 0;

	/** Set scaling array.
	Each coefficient may be scaled after transforming and therefore 
	requires setting up.
	@param	pScale:	Scale factor array.
	@return				:	none.
	*/
	virtual void	SetScale(void* pScale) = 0;
	virtual void* GetScale(void) = 0;

	/** Set the mode of operation.
	Allow an implementation to define modes of operation or
	interpretation of the quantisation process.
	@param mode	: Mode to set.
	@return			: none.
	*/
	virtual void SetMode(int mode) = 0;
	virtual int  GetMode(void) = 0;

	/** Set and get parameters for the implementation.
	Implementations require some flexibility in the interface	to provide extra
	parameters that are internal. Most implementations will have empty methods.
	@param paramID	: Parameter to set/get.
	@param paramVal	: Parameter value.
	@return					: None (Set) or the param value (Get).
	*/
	virtual void	SetParameter(int paramID, int paramVal) = 0;
	virtual int		GetParameter(int paramID) = 0;

	/// Global constants for all implementations.
public:

	/// Parameter IDs.
	static const int QUANT_ID						= 0;	///< Sets/Gets quantisation parameter.
	static const int INTRA_FLAG_ID			= 1;	///< Sets/Gets intra flag.

	/// Modes.
	static const int TransformAndQuant	= 0;
	static const int TransformOnly			= 1;
	static const int QuantOnly					= 2;

};//end IForwardTransform.


#endif	// _IFORWARDTRANSFORM_H
