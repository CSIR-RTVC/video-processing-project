/** @file

MODULE				: VectorStructList

TAG						: VSL

FILE NAME			: VectorStructList.h

DESCRIPTION		: A generic base class to contain vectors in a contiguous 
								list of structs defined by the extended implementations.
								The structure of the vectors is determined by its type.

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
#ifndef _VECTORSTRUCTLIST_H
#define _VECTORSTRUCTLIST_H

/*
---------------------------------------------------------------------------
	Vector type description.
---------------------------------------------------------------------------
*/

// Two Structs are defined for general 2-D vectors and 2-D patterns. Each 
// implementation either uses these or defines its own.

// Simple structure. _type = SIMPLE:
typedef struct _VSL_2D_TYPE
{
	short x;
	short y;
} VCL_2D_TYPE;

// Complex structure.	_type = COMPLEX:
typedef struct _VSL_COMPLEX_2D_TYPE
{
	int						pattern;		// Describes the meaning of the pattern of vectors.
	int						numVectors;	// Valid vectors in vec.
	VCL_2D_TYPE*	vec;				// Array of length _maxVecsPerStruct.
} VCL_COMPLEX_2D_TYPE;

/*
---------------------------------------------------------------------------
	Class definitions.
---------------------------------------------------------------------------
*/
class VectorStructList
{
	public:
		VectorStructList(int type, int dim = 2, int maxVecsPerStruct = 1);
		virtual ~VectorStructList(void);

		// Set mem size dependent on type and length of list.
		virtual int	SetLength(int length);
		// Get the struct at a specific position in the list.
		virtual void* GetStructPtr(int pos);

		int		GetType(void)								{ return(_type); }
		int		GetDimension(void)					{ return(_dim); }
		int		GetMaxVecsPerStruct(void)		{ return(_maxVecsPerStruct); }
		int		GetLength(void)							{ return(_length); }
		void* GetListPtr(void)						{ return(_pData); }

		// Vector element access interface.
		virtual void	SetSimpleElement(int pos, int element, int val);
		virtual int		GetSimpleElement(int pos, int element);
		virtual void	SetComplexElement(int pos, int vec, int element, int val);
		virtual int		GetComplexElement(int pos, int vec, int element);
    virtual void  Dump(char* filename, const char* title);

	public:
		static const int	SIMPLE2D;
		static const int	COMPLEX2D;

	protected:
		virtual void Delete(void);

	protected:
		int		_type;							// SIMPLE/COMPLEX or implementation defined.
		int		_dim;								// Vector dimension.
		int		_maxVecsPerStruct;	// Max vectors per pattern. (Default = 1 for SIMPLE.)
		int		_length;						// Num of patterns of type _type.	( Is num of vecs for SIMPLE).

		// The data list is stored in an array whose structure 
		// is dependent on the type and length.
		void*	_pData;	
};//end VectorStructList.


#endif // _VECTORSTRUCTLIST_H

