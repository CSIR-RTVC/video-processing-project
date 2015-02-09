/** @file

MODULE				: CodecControlInterface

FILE NAME			: CodecControlInterface.h

DESCRIPTION			: COM settings interface to control the codec.
Supports using smart COM pointers

LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008 - 2014, CSIR
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
\brief COM Settings interface for VPP DirectShow filters.
*/

//For Smart pointers

// {A33BCB92-5EF6-4A66-9988-5EEF2B8C8944}
DEFINE_GUID(IID_ICodecControlInterface ,
  0xa33bcb92, 0x5ef6, 0x4a66, 0x99, 0x88, 0x5e, 0xef, 0x2b, 0x8c, 0x89, 0x44);

#undef  INTERFACE
#define INTERFACE   ICodecControlInterface
DECLARE_INTERFACE_(ICodecControlInterface, IUnknown)
{
  /// Method to query frame bit limit
  STDMETHOD(GetFramebitLimit)(int& uiFrameBitLimit) = 0;
  /// Method to set frame bit limit
  STDMETHOD(SetFramebitLimit)(int uiFrameBitLimit) = 0;
  /// Method to query group id
  STDMETHOD(GetGroupId)(int& iGroupId) = 0;
  /// Method to set group id
  STDMETHOD(SetGroupId)(int iGroupId) = 0;
  /// Method to trigger IDR frame
  STDMETHOD(GenerateIdr)() = 0;
};
