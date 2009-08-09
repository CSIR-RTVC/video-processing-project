/** @file

MODULE				: CropProperties

FILE NAME			: CropProperties.h

DESCRIPTION			: Properties for crop filter.
					  
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

#include <DirectShow/FilterPropertiesBase.h>

#include "resource.h"

#define BUFFER_SIZE 256

/**
 * \ingroup DirectShowFilters
 * Crop property dialog class
 */
class CropProperties : public FilterPropertiesBase
{
public:

	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr) 
	{
		CropProperties *pNewObject = new CropProperties(pUnk);
		if (pNewObject == NULL) 
		{
			*pHr = E_OUTOFMEMORY;
		}
		return pNewObject;
	}

	CropProperties::CropProperties(IUnknown *pUnk) : 
		FilterPropertiesBase(NAME("Crop Properties"), pUnk, IDD_CROP_DIALOG, IDS_CROP_CAPTION)
	{;}

	HRESULT ReadSettings()
	{
		short lower = 0;
		short upper = SHRT_MAX;

		// Init UI
		long lResult = SendMessage(			// returns LRESULT in lResult
			GetDlgItem(m_Dlg, IDC_SPIN1),	// handle to destination control
			(UINT) UDM_SETRANGE,			// message ID
			(WPARAM) 0,						// = 0; not used, must be zero
			(LPARAM) MAKELONG ( upper, lower)      // = (LPARAM) MAKELONG ((short) nUpper, (short) nLower)
			);
		lResult = SendMessage(			// returns LRESULT in lResult
			GetDlgItem(m_Dlg, IDC_SPIN2),	// handle to destination control
			(UINT) UDM_SETRANGE,			// message ID
			(WPARAM) 0,						// = 0; not used, must be zero
			(LPARAM) MAKELONG ( upper, lower)      // = (LPARAM) MAKELONG ((short) nUpper, (short) nLower)
			);
		lResult = SendMessage(			// returns LRESULT in lResult
			GetDlgItem(m_Dlg, IDC_SPIN3),	// handle to destination control
			(UINT) UDM_SETRANGE,			// message ID
			(WPARAM) 0,						// = 0; not used, must be zero
			(LPARAM) MAKELONG ( upper, lower)      // = (LPARAM) MAKELONG ((short) nUpper, (short) nLower)
			);
		lResult = SendMessage(			// returns LRESULT in lResult
			GetDlgItem(m_Dlg, IDC_SPIN4),	// handle to destination control
			(UINT) UDM_SETRANGE,			// message ID
			(WPARAM) 0,						// = 0; not used, must be zero
			(LPARAM) MAKELONG ( upper, lower)      // = (LPARAM) MAKELONG ((short) nUpper, (short) nLower)
			);


		int nLength = 0;
		char szBuffer[BUFFER_SIZE];
		HRESULT hr = m_pSettingsInterface->GetParameter(TOP_CROP, sizeof(szBuffer), szBuffer, &nLength);
		if (SUCCEEDED(hr))
		{
			SetDlgItemText(m_Dlg, IDC_EDIT_TOP, szBuffer);
			m_topCrop = atoi(szBuffer);
		}
		else
		{
			return E_FAIL;
		}
		hr = m_pSettingsInterface->GetParameter(BOTTOM_CROP, sizeof(szBuffer), szBuffer, &nLength);
		if (SUCCEEDED(hr))
		{
			SetDlgItemText(m_Dlg, IDC_EDIT_BOTTOM, szBuffer);
			m_bottomCrop = atoi(szBuffer);
		}
		else
		{
			return E_FAIL;
		}
		hr = m_pSettingsInterface->GetParameter(LEFT_CROP, sizeof(szBuffer), szBuffer, &nLength);
		if (SUCCEEDED(hr))
		{
			SetDlgItemText(m_Dlg, IDC_EDIT_LEFT, szBuffer);
			m_leftCrop = atoi(szBuffer);
		}
		else
		{
			return E_FAIL;
		}
		hr = m_pSettingsInterface->GetParameter(RIGHT_CROP, sizeof(szBuffer), szBuffer, &nLength);
		if (SUCCEEDED(hr))
		{
			SetDlgItemText(m_Dlg, IDC_EDIT_RIGHT, szBuffer);
			m_rightCrop = atoi(szBuffer);
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
		nLength = GetDlgItemText(m_Dlg, IDC_EDIT_TOP, szBuffer, BUFFER_SIZE);
		m_topCrop = atoi(szBuffer);
		m_pSettingsInterface->SetParameter(TOP_CROP, szBuffer);
		nLength = GetDlgItemText(m_Dlg, IDC_EDIT_BOTTOM, szBuffer, BUFFER_SIZE);
		m_bottomCrop = atoi(szBuffer);
		m_pSettingsInterface->SetParameter(BOTTOM_CROP, szBuffer);
		nLength = GetDlgItemText(m_Dlg, IDC_EDIT_LEFT, szBuffer, BUFFER_SIZE);
		m_leftCrop = atoi(szBuffer);
		m_pSettingsInterface->SetParameter(LEFT_CROP, szBuffer);
		nLength = GetDlgItemText(m_Dlg, IDC_EDIT_RIGHT, szBuffer, BUFFER_SIZE);
		m_rightCrop = atoi(szBuffer);
		m_pSettingsInterface->SetParameter(RIGHT_CROP, szBuffer);
		return S_OK;
	} 

private:

	unsigned m_topCrop;
	unsigned m_bottomCrop;
	unsigned m_leftCrop;
	unsigned m_rightCrop;
};

