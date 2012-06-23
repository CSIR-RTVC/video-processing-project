/** @file

MODULE				: SkewingProperties

FILE NAME			: SkewingProperties.h

DESCRIPTION			: Properties for Skewing filter.

LICENSE: Software License Agreement (BSD License)

Copyright (c) 2011, CSIR
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

#include <DirectShow/FilterPropertiesBase.h>

#include <climits>
#include "resource.h"

#define BUFFER_SIZE 256

/**
 * \ingroup DirectShowFilters
 * Skewing Filter Property Dialog
 */
class SkewingProperties : public FilterPropertiesBase
{
public:

	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr) 
	{
		SkewingProperties *pNewObject = new SkewingProperties(pUnk);
		if (pNewObject == NULL) 
		{
			*pHr = E_OUTOFMEMORY;
		}
		return pNewObject;
	}

	SkewingProperties::SkewingProperties(IUnknown *pUnk) : 
	FilterPropertiesBase(NAME("Skewing Properties"), pUnk, IDD_SKEWING_DIALOG, IDS_SKEWING_CAPTION)
	{;}

	INT_PTR OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// Let the parent class handle the message.
		return FilterPropertiesBase::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
	}

	HRESULT ReadSettings()
	{
		short lower = -SHRT_MAX;
		short upper = SHRT_MAX;

		// Init UI
		long lResult = SendMessage(			// returns LRESULT in lResult
			GetDlgItem(m_Dlg, IDC_SPIN1),	// handle to destination control
			(UINT) UDM_SETRANGE,			// message ID
			(WPARAM) 0,						// = 0; not used, must be zero
			(LPARAM) MAKELONG ( upper, lower)      // = (LPARAM) MAKELONG ((short) nUpper, (short) nLower)
			);

		int nLength = 0;
		char szBuffer[BUFFER_SIZE];
		HRESULT hr = m_pSettingsInterface->GetParameter("skewTimestamp", sizeof(szBuffer), szBuffer, &nLength);
		if (SUCCEEDED(hr))
		{
			SetDlgItemText(m_Dlg, IDC_EDIT_SKEW_TIMESTAMP, szBuffer);
			m_skewTimeStamp = atoi(szBuffer);
		}
		else
		{
			return E_FAIL;
		}
		return hr;
	}
 
	HRESULT OnApplyChanges(void)
	{
		int nLength = 0;
		char szBuffer[BUFFER_SIZE];
		nLength = GetDlgItemText(m_Dlg, IDC_EDIT_SKEW_TIMESTAMP, szBuffer, BUFFER_SIZE);
		m_skewTimeStamp = atoi(szBuffer);
		m_pSettingsInterface->SetParameter("skewTimestamp", szBuffer);
		return S_OK;
	} 

private:

	unsigned m_skewTimeStamp;
};

