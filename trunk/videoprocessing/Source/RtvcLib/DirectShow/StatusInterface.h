/** @file

MODULE				: StatusInterface

FILE NAME			: StatusInterface.h

DESCRIPTION			: COM Status interface class for application - filter communication
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008, CSIR
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

#include <Windows.h>
#include <string>

#pragma warning(push)     // disable for this header only
#pragma warning(disable:4312)
// DirectShow
#include <streams.h>
#pragma warning(pop)      // restore original warning level

/*! \interface IStatusInterface
\brief COM status interface for RTVC DirectShow filters.

The status interface is used for main application - filter communication.
The SetLastError method should be called from within the filter code when an error occurs.
The main application can then retrieve this information using GetLastError.

The SetMediaEventSink method provides us with a way of letting the filter notify the application
of an event asynchronously via Windows events. If an error occurs in the DirectShow pipeline during 
processing, this can be communicated to the main application provided that the Media Event Sink has been set.
The friendly ID is passed as a part of the windows message and allows the main application to determine
in from which filter the notification came.
*/

//Smart Pointers
DEFINE_GUID( IID_IStatusInterface, /* 60178ec0-c670-11d0-837a-0000f80220b9 */
			0x8bd6af85, 
			0xd262, 
			0x4dbe, 
			0xb2, 0x53, 0xfa, 0x8, 0xb4, 0xe3, 0x12, 0x5b
			);

#undef  INTERFACE
#define INTERFACE   IStatusInterface
DECLARE_INTERFACE_( IStatusInterface, IUnknown )
{
	// *** methods ***
	STDMETHOD(GetLastError)( std::string& sError ) = 0;
	STDMETHOD(SetLastError)( std::string sError, bool bNotifyApplication) = 0;
	STDMETHOD(SetMediaEventSink) (IMediaEventSink* pEventSink) = 0;
	//We need an id for when we perform a callback
	STDMETHOD(SetFriendlyID) ( long lId) = 0;
	STDMETHOD(GetFriendlyID) ( long& lId) = 0;
};
