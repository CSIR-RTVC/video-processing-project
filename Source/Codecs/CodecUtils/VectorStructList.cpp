/** @file

MODULE				: VectorStructList

TAG						: VSL

FILE NAME			: VectorStructList.cpp

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
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include <memory.h>
#include <string.h>
#include <stdlib.h>

#include "VectorStructList.h"
#include "MeasurementTable.h"

/*
--------------------------------------------------------------------------
  Constants. 
--------------------------------------------------------------------------
*/
const int	VectorStructList::SIMPLE2D	= 0;
const int	VectorStructList::COMPLEX2D	= 1;

/*
--------------------------------------------------------------------------
  Construction and destruction. 
--------------------------------------------------------------------------
*/
/** Construction.
No mem is alloc in the constructor and is left for the SetLength()
member.
*/
VectorStructList::VectorStructList(int type, int dim, int maxVecsPerStruct)
{
	_type							= type;
	_dim							= dim;
	_maxVecsPerStruct	= maxVecsPerStruct;
	_length						= 0;
	_pData						= NULL;
}//end constructor.

VectorStructList::~VectorStructList(void)
{
	Delete();
}//end destructor.

void VectorStructList::Delete(void)
{
	if(_pData != NULL)
	{
		if(_type == COMPLEX2D)
		{
			VCL_COMPLEX_2D_TYPE* p = (VCL_COMPLEX_2D_TYPE *)_pData;
			for(int i = 0; i < _length; i++)
				delete[] p[i].vec;

                        delete[] p;
		}//end if _type.i..
                else
                {
                  VCL_2D_TYPE* pData = (VCL_2D_TYPE*)_pData;
		  delete[] pData;
                }
	}//end if _pData...
	_pData	= NULL;
	_length = 0;
}//end Delete.

/*
--------------------------------------------------------------------------
  Public member interface. 
--------------------------------------------------------------------------
*/
/** Set mem size dependent on type and length of vector list.
There is no checking on the validity of the members set during construction.
@param	length	: Num of structs in _pData;
@return					: 1 = success, 0 = failure.
*/
int VectorStructList::SetLength(int length)
{
	if(length < 0)
		return(0);

	// Clear out old mem.
	if(_pData != NULL)
		Delete();

	// Create new space on construction parameters.
	if(_type == SIMPLE2D)
	{
		_pData = new VCL_2D_TYPE[length];
		_maxVecsPerStruct = 1;	// Forced to 1 for simple.
	}//end if _type...
	else if(_type == COMPLEX2D)
		_pData = new VCL_COMPLEX_2D_TYPE[length];
	if(_pData == NULL)
		return(0);

	_length = length;
	_dim		= 2;		// Forced to 2D.

	// Complex type requires filling in.
	if(_type == COMPLEX2D)
	{
		VCL_COMPLEX_2D_TYPE* p = (VCL_COMPLEX_2D_TYPE *)_pData;
		for(int i = 0; i < _length; i++)
		{
			p[i].pattern		= 0;
			p[i].numVectors = 0;
			p[i].vec				= new VCL_2D_TYPE[_maxVecsPerStruct];
		}//end for i...
	}//end if _type...

	return(1);
}//end SetLength.

/** Get a ptr to the desired struct in the list.
@param pos	: Position in the list.
@return			: Generic ptr to struct at position pos.
*/
void* VectorStructList::GetStructPtr(int pos)
{
	if( (_pData == NULL)||(pos >= _length) )
		return(NULL);

	void* p = NULL;

	if(_type == SIMPLE2D)
	{
		VCL_2D_TYPE* ps = (VCL_2D_TYPE *)_pData;
		p = &(ps[pos]);
	}//end if _type...
	else if(_type == COMPLEX2D)
	{
		VCL_COMPLEX_2D_TYPE* pc = (VCL_COMPLEX_2D_TYPE *)_pData;
		p = &(pc[pos]);
	}//end else...

	return(p); 
}//end GetStructPtr.

/** Set the element in a simple vector list.
No bounds checking is done so be careful.
@param pos			: Struct position in the list.
@param element	: Element within the vector.
@param val			: Value to set.
*/
void VectorStructList::SetSimpleElement(int pos, int element, int val)
{
	VCL_2D_TYPE* p = (VCL_2D_TYPE *)_pData;
	if(element == 0)
		p[pos].x = (short)val;
	else if(element == 1)
		p[pos].y = (short)val;
}//end SetSimpleElement.

int	VectorStructList::GetSimpleElement(int pos, int element)
{
	VCL_2D_TYPE* p = (VCL_2D_TYPE *)_pData;
	int val = 0;
	if(element == 0)
		val = (int)(p[pos].x);
	else if(element == 1)
		val = (int)(p[pos].y);
	return(val);
}//end GetSimpleElement.

/** Set the element in a complex vector list.
No bounds checking is done so be careful.
@param pos			: Struct position in the list.
@param vec			: Vector in the pattern.
@param element	: Element within the vector.
@param val			: Value to set.
*/
void VectorStructList::SetComplexElement(int pos, int vec, int element, int val)
{
	VCL_COMPLEX_2D_TYPE* p = (VCL_COMPLEX_2D_TYPE *)_pData;
	if(element == 0)
		p[pos].vec[vec].x = (short)val;
	else if(element == 1)
		p[pos].vec[vec].y = (short)val;
}//end SetComplexElement.

int	VectorStructList::GetComplexElement(int pos, int vec, int element)
{
	VCL_COMPLEX_2D_TYPE* p = (VCL_COMPLEX_2D_TYPE *)_pData;
	int val = 0;
	if(element == 0)
		val = (int)(p[pos].vec[vec].x);
	else if(element == 1)
		val = (int)(p[pos].vec[vec].y);
	return(val);
}//end GetComplexElement.

/** Dump the list into a file.
Treat all elements as type integer.
@param filename : File name to use.
@param title    : Title to use in the text file.
@return         : None.
*/
void VectorStructList::Dump(char* filename, const char* title)
{
  int i; 

  /// Currently only support simple type.
	if(_type != SIMPLE2D)
    return;

  MeasurementTable* pT = new MeasurementTable();
	pT->SetTitle(title);

	pT->Create(2, _length);
  /// Motion X
  pT->SetHeading(0, "mv x");
  pT->SetDataType(0, MeasurementTable::INT);
  /// Motion Y
  pT->SetHeading(1, "mv y");
  pT->SetDataType(1, MeasurementTable::INT);

	for(i = 0; i < _length; i++)
  {
		pT->WriteItem(0, i, GetSimpleElement(i, 0));
		pT->WriteItem(1, i, GetSimpleElement(i, 1));
  }//end for i...

  pT->Save(filename, ",", 1);

  delete pT;
}//end Dump.

