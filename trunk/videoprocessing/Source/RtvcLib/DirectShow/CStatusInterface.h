/** @file

MODULE				: CStatusInterface

FILE NAME			: CStatusInterface.h

DESCRIPTION			: Implementation of COM IStatusInterface

					This interface enables communication between a DirectShow filter and the main 
					application using the GetLastError and SetLastError methods.

					The GetLastError, GetFriendlyID and SetFriendlyID methods should be called from the application code.
					The friendly ID is used purely for filter identification purposes. It is the responsibility of the developer
					to assign unique IDs to each filter that supports this interface on filter creation.
					The last error that occurred in the filter can be obtained by calling GetLastError.
					
					The SetLastError should be called from within the filter code itself. If a media event sink has
					been set, the main application will be notified of the error via the Windows event WM_TM_GRAPHNOTIFY.
					The main application can then use the friendly ID to determine which filter caused the event.
							
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008 - 2011, CSIR
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

#include <DirectShow/StatusInterface.h>
#include <Windows.h>

#define MAX_MESSAGE_LENGTH 256

/**
* \ingroup DirectShowLib
*  IStatusInterface implementation.
*/
class CStatusInterface : public IStatusInterface
{
public:
	/// Constructor
	CStatusInterface();
	/// Sets the error string and notifies the application using the media event sink if set
	STDMETHODIMP SetLastError( std::string sError, bool bNotifyApplication = false);
	/// Returns the last error that occurred in the filter
	STDMETHODIMP GetLastError( std::string& sError );
	/// Mutator for media event sink which enables notification via Windows message passing
	STDMETHODIMP SetMediaEventSink( IMediaEventSink* pEventSink );
	/// Mutator for friendly ID
	STDMETHODIMP SetFriendlyID( long lId );
	/// Accessor for friendly ID
	STDMETHODIMP GetFriendlyID( long& lId );

  STDMETHODIMP GetNotificationMessage( char* szMessage, int nBufferSize );
  STDMETHODIMP SetNotificationMessage( const char* szMessage );

protected:
	/// This method notifies the application with the specified params
	void NotifyApplication(long lEventCode, LONG_PTR lEventParam1);

	/// String to store last error
	std::string m_sLastError;
	/// Event sink used for windows message passing notification
	IMediaEventSink* m_pMediaEventSink;
	/// Friendly id of the filter
	long m_lFriendlyId;
  /// String to store last error
  char m_szNotification[MAX_MESSAGE_LENGTH];

};