/** @file

MODULE				: IVlcEncoder

TAG						: IVE

FILE NAME			: IVlcEncoder.h

DESCRIPTION		: A IVlcEncoder Interface as an abstract base class. The
								optional interface methods are not abstract but have
								trivial default implementations.

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

========================================================================================
*/
#ifndef _IVLCENCODER_H
#define _IVLCENCODER_H

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class IVlcEncoder
{
public:
	virtual ~IVlcEncoder() { }

	/*
	--------------------------------------------------------------------------
	Interface implementation.
	--------------------------------------------------------------------------
	*/
	virtual int GetNumCodedBits(void)	= 0;
	virtual int GetCode(void)					= 0;
	virtual int Encode(int symbol)		= 0;

	/*
	--------------------------------------------------------------------------
	Optional interface implementation.
	--------------------------------------------------------------------------
	*/
	virtual int		Encode2(int symbol1, int symbol2)								{return(0);}
	virtual int		Encode3(int symbol1, int symbol2, int symbol3)	{return(0);}
	virtual void	SetEsc(int numEscBits, int escMask)							{ }
	virtual void	SetMode(int mode)																{ }
	virtual int		GetMode(void)																		{ return(0); }

};// end class IVlcEncoder.

#endif
