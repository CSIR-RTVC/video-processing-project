/** @file

MODULE				: H264

TAG					: H264

FILE NAME			: H264v2.h

DESCRIPTION			: H264v2Factory header	

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

===========================================================================
*/

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the H264V2_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// H264V2_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#pragma once

#ifdef H264V2_EXPORTS
#define H264V2_API __declspec(dllexport)
#else
#define H264V2_API __declspec(dllimport)
#endif

/// This dll has only one purpose to instantiate a H264v2Codec instance. The
/// obligation of scope management is left to the calling functions.
#include "H264v2Codec.h"

// This class is exported from the H264v2.dll
class H264V2_API H264v2Factory 
{
public:
	H264v2Factory(void);
	~H264v2Factory(void);

	/// Interface.
	H264v2Codec* GetCodecInstance(void);
	void				 ReleaseCodecInstance(ICodecv2* pInst);
};	///end H264v2Factory.
