/** @file

MODULE				: SettingsInterface

FILE NAME			: SettingsInterface.h

DESCRIPTION			: COM settings interface to set and get filter parameters.
					Supports using smart COM pointers
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008 - 2012, CSIR
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of the CSIR nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

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

===========================================================================
*/
#pragma once

//This file needs to be included - else we get a linker error for the GUID
#include <INITGUID.H>

/*! \interface ISettingsInterface
\brief COM Settings interface for RTVC DirectShow filters.

The settings interface is used to configure filters before they are connected downstream. 
The downstream connection finalises the media type and often, such as in the scale and crop 
filter, the media type is modified when configuring the filter.

Usage:

\code
IBaseFilterPtr pScaleFilter = NULL;
...
ISettingsInterfacePtr pSettingsInterface = pScaleFilter;
char szParamValue[255];
int nLength = 0;
HRESULT hr;
hr = pSettingsInterface->GetParameter("targetwidth", 255, szParamValue, nLength);
hr = pSettingsInterface->GetParameter("targetheight", 255, szParamValue, nLength);
hr = pSettingsInterface->SetParameter("targetwidth", "800");
hr = pSettingsInterface->SetParameter("targetheight", "600");
\endcode
*/

//For Smart pointers
DEFINE_GUID( IID_ISettingsInterface, /* 388EEF20-40CC-4752-A0FF-66AA5C4AF8FA */
			0x388eef20, 
			0x40cc, 
			0x4752, 
			0xa0, 0xff, 0x66, 0xaa, 0x5c, 0x4a, 0xf8, 0xfa
			);

#undef  INTERFACE
#define INTERFACE   ISettingsInterface
DECLARE_INTERFACE_( ISettingsInterface, IUnknown )
{
	// *** methods ***
	/// Method to retrieve parameters named type. The result will be stored in value and the length of the result in length
	STDMETHOD(GetParameter)( const char* type, int buffersize, char* value, int* length ) = 0;
	/// Method to set parameter named type to value
	STDMETHOD(SetParameter)( const char* type, const char* value) = 0;
	/// Method to retrieve ALL parameters in szResult. nSize should contain the size of the buffer passed in
	STDMETHOD(GetParameterSettings)(char* szResult, int nSize) = 0;

};
