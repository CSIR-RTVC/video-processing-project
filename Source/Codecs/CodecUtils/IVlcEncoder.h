/** @file

MODULE				: IVlcEncoder

TAG						: IVE

FILE NAME			: IVlcEncoder.h

DESCRIPTION		: A IVlcEncoder Interface as an abstract base class. The
								optional interface methods are not abstract but have
								trivial default implementations.

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
