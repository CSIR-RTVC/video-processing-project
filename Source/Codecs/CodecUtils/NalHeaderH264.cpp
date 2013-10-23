/** @file

MODULE				: NalHeaderH264

TAG						: NHH264

FILE NAME			: NalHeaderH264.cpp

DESCRIPTION		: A class to hold H.264 NAL header data for use in the 
								H264v2Codec class.

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
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include "NalHeaderH264.h"

/*
---------------------------------------------------------------------------
	Construction and destruction.
---------------------------------------------------------------------------
*/
NalHeaderH264::NalHeaderH264(void)
{
	/// Defualt to an IDR NAL unit.
	_ref_idc		= 3;					///< Indicates reference parameter sets or IDR coded slices in the NAL payload. 
	_unit_type	= IDR_Slice;	///< NAL contents type.

}//end constructor.

NalHeaderH264::~NalHeaderH264(void)
{
}//end destructor.

