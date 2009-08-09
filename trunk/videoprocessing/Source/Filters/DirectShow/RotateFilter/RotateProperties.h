/** @file

MODULE				: RotateProperties

FILE NAME			: RotateProperties.h

DESCRIPTION			: Properties for rotate filter.
					  
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

#include <string>

#include <DirectShow/FilterPropertiesBase.h>
#include <Shared/StringUtil.h>

#include "resource.h"

#define BUFFER_SIZE 256

const int RADIO_BUTTON_IDS[] = {IDC_RADIO1, IDC_RADIO2, IDC_RADIO3, IDC_RADIO4, IDC_RADIO5, IDC_RADIO6};

/**
 * \ingroup DirectShowFilters
 * Rotate property dialog class
 */
class RotateProperties : public FilterPropertiesBase
{
public:

	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr) 
	{
		RotateProperties *pNewObject = new RotateProperties(pUnk);
		if (pNewObject == NULL) 
		{
			*pHr = E_OUTOFMEMORY;
		}
		return pNewObject;
	}

	RotateProperties::RotateProperties(IUnknown *pUnk) : 
		FilterPropertiesBase(NAME("Rotate Properties"), pUnk, IDD_ROTATE_DIALOG, IDS_ROTATE_CAPTION)
	{;}

	HRESULT ReadSettings()
	{
		int nLength = 0;
		char szBuffer[BUFFER_SIZE];
		HRESULT hr = m_pSettingsInterface->GetParameter(ROTATION_MODE, sizeof(szBuffer), szBuffer, &nLength);
		if (SUCCEEDED(hr))
		{
			int nRotationMode = atoi(szBuffer);
			int nRadioID = RADIO_BUTTON_IDS[nRotationMode];
			long lResult = SendMessage(				// returns LRESULT in lResult
				GetDlgItem(m_Dlg, nRadioID),		// handle to destination control
				(UINT) BM_SETCHECK,					// message ID
				(WPARAM) 1,							// = 0; not used, must be zero
				(LPARAM) 0							// = (LPARAM) MAKELONG ((short) nUpper, (short) nLower)
				);
			return S_OK;
		}
		else
		{
			return E_FAIL;
		}
	}

	HRESULT OnApplyChanges(void)
	{
		for (int i = 0; i < 6; ++i)
		{
			int nRadioID = RADIO_BUTTON_IDS[i];
			int iCheck = SendMessage( GetDlgItem(m_Dlg, nRadioID),	(UINT) BM_GETCHECK,	0, 0);
			if (iCheck != 0)
			{
				std::string sID = StringUtil::itos(i);
				HRESULT hr = m_pSettingsInterface->SetParameter(ROTATION_MODE, sID.c_str());
				break;
			}
		}
		return S_OK;
	} 

private:

	int m_rotate;
};

