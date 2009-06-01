/** @file

MODULE				: IInverseTransform

TAG						: IIT

FILE NAME			: IInverseTransform.h

DESCRIPTION		: An interface to inverse transform implementations. The 
								interface has 2 options as either in-place or from an 
								input to an output parameter argument. Further, they 
								may include or exclude the quantisation. The 
								implementations must define the mem type (e.g. short, 
								int) and whether it is a 2-D on a block or 1-D on an 
								array.

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

============================================================================================
*/
#ifndef _IINVERSETRANSFORM_H
#define _IINVERSETRANSFORM_H

/*
---------------------------------------------------------------------------
	Interface definition.
---------------------------------------------------------------------------
*/
class IInverseTransform
{
public:
	virtual ~IInverseTransform() {}
	
	/** In-place inverse Dct.
	The inverse Dct is performed on the input and replaces it with the coeffs.
	@param p	: Data to transform.
	@return		:	none.
	*/
	virtual void InverseTransform(void* ptr) = 0;

	/** Transfer inverse Dct.
	The inverse Dct is performed on the coeffs and are written to 
	the output.
	@param pCoeff	: Input coeffs.
	@param pOut		: Output data.
	@return				:	none.
	*/
	virtual void InverseTransform(void* pCoeff, void* pOut) = 0;

	/** Set scaling array.
	Each coefficient may be scaled before transforming and therefore 
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

	/// Modes.
	static const int TransformAndQuant	= 0;
	static const int TransformOnly			= 1;
	static const int QuantOnly					= 2;

};//end IInverseTransform.


#endif	// _IINVERSETRANSFORM_H
