/** @file

MODULE				: CStatusInterface

FILE NAME			: CStatusInterface.cpp

DESCRIPTION			: 
					  
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
#include "stdafx.h"

#include <DirectShow/CStatusInterface.h>
#include <DirectShow/NotifyCodes.h>

CStatusInterface::CStatusInterface()
: m_sLastError(""),
m_pMediaEventSink(NULL),
m_lFriendlyId(-1)
{}

STDMETHODIMP CStatusInterface::SetLastError( std::string sError, bool bNotifyApplication)
{
	m_sLastError = sError;
	if (bNotifyApplication)
	{
		NotifyApplication(WM_TM_GRAPHNOTIFY, m_lFriendlyId);
	}
	return S_OK;
}

STDMETHODIMP CStatusInterface::GetLastError( std::string& sError )
{
	sError = m_sLastError;
	return S_OK;
}

STDMETHODIMP CStatusInterface::SetMediaEventSink( IMediaEventSink* pEventSink )
{
	m_pMediaEventSink = pEventSink;
	return S_OK;
}

STDMETHODIMP CStatusInterface::SetFriendlyID( long lId )
{
	m_lFriendlyId = lId;
	return S_OK;
}

STDMETHODIMP CStatusInterface::GetFriendlyID( long& lId )
{
	lId = m_lFriendlyId;
	return S_OK;
}

void CStatusInterface::NotifyApplication( long lEventCode, LONG_PTR lEventParam1 )
{
	//Notify sending the friendly ID as a parameter
	if (m_pMediaEventSink)
		m_pMediaEventSink->Notify(lEventCode, m_lFriendlyId, lEventParam1);
}

STDMETHODIMP CStatusInterface::GetNotificationMessage( char* szMessage, int nBufferSize)
{
  int nLength = strlen(m_szNotification);
  if (nLength >= nBufferSize)
  {
    // If this happens we know that we have to increase the buffer size
    return E_FAIL;
  }
  memcpy(szMessage, m_szNotification, nLength);
  return S_OK;
}

STDMETHODIMP CStatusInterface::SetNotificationMessage( const char* szMessage )
{
  if (strlen(szMessage) > MAX_MESSAGE_LENGTH)
    return E_FAIL;
  // Clear memory first
  ZeroMemory(m_szNotification, MAX_MESSAGE_LENGTH);
  memcpy(m_szNotification, szMessage, strlen(szMessage));
  NotifyApplication(WM_TM_GRAPHNOTIFY, m_lFriendlyId);
  return S_OK;
}
