/** @file

MODULE				: PicInPicProperties

FILE NAME			: PicInPicProperties.h

DESCRIPTION			: 
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008, Meraka Institute
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of the Meraka Institute nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

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

#include <DirectShow/CommonDefs.h>
#include <DirectShow/FilterPropertiesBase.h>
#include <Shared/StringUtil.h>

#include "resource.h"
#define BUFFER_SIZE 256

//const int RADIO_BUTTON_IDS[] = {IDC_HORIZONTAL, IDC_VERTICAL};

class PicInPicProperties : public FilterPropertiesBase
{
public:

	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr) 
	{
		PicInPicProperties *pNewObject = new PicInPicProperties(pUnk);
		if (pNewObject == NULL) 
		{
			*pHr = E_OUTOFMEMORY;
		}
		return pNewObject;
	}

	PicInPicProperties::PicInPicProperties(IUnknown *pUnk) : 
	FilterPropertiesBase(NAME("Pic in pic Properties"), pUnk, IDD_PIC_IN_PIC_DIALOG, IDS_PIC_IN_PIC_CAPTION)
	{;}

	HRESULT ReadSettings()
	{
		initialiseControls();
		return S_OK;
		//int nLength = 0;
		//char szBuffer[BUFFER_SIZE];
		//HRESULT hr = m_pSettingsInterface->GetParameter(ORIENTATION, sizeof(szBuffer), szBuffer, &nLength);
		//if (SUCCEEDED(hr))
		//{
		//	int nMixingMode = atoi(szBuffer);
		//	int nRadioID = RADIO_BUTTON_IDS[nMixingMode];
		//	long lResult = SendMessage(				// returns LRESULT in lResult
		//		GetDlgItem(m_Dlg, nRadioID),		// handle to destination control
		//		(UINT) BM_SETCHECK,					// message ID
		//		(WPARAM) 1,							// = 0; not used, must be zero
		//		(LPARAM) 0							// = (LPARAM) MAKELONG ((short) nUpper, (short) nLower)
		//		);
		//	return S_OK;
		//}
		//else
		//{
		//	return E_FAIL;
		//}
		//return hr;
	}

	HRESULT OnApplyChanges(void)
	{
		//for (int i = 0; i < 2; ++i)
		//{
		//	int nRadioID = RADIO_BUTTON_IDS[i];
		//	int iCheck = SendMessage( GetDlgItem(m_Dlg, nRadioID),	(UINT) BM_GETCHECK,	0, 0);
		//	if (iCheck != 0)
		//	{
		//		std::string sID = StringUtil::itos(i);
		//		HRESULT hr = m_pSettingsInterface->SetParameter(ORIENTATION, sID.c_str());
		//		break;
		//	}
		//}
		return S_OK;
	}

private:
	void initialiseControls()
	{
		InitCommonControls();

		SendMessage(GetDlgItem(m_Dlg, IDC_CMB_POSITION), CB_RESETCONTENT, 0, 0);
		//Add default option
		SendMessage(GetDlgItem(m_Dlg, IDC_CMB_POSITION), CB_ADDSTRING,	   0, (LPARAM)"Left bottom");
		SendMessage(GetDlgItem(m_Dlg, IDC_CMB_POSITION), CB_SELECTSTRING,  0, (LPARAM)"Left bottom");
		// Now populate the graphs
		SendMessage(GetDlgItem(m_Dlg, IDC_CMB_POSITION), CB_INSERTSTRING,  1, (LPARAM)"Left top");
		SendMessage(GetDlgItem(m_Dlg, IDC_CMB_POSITION), CB_INSERTSTRING,  2, (LPARAM)"Right top");
		SendMessage(GetDlgItem(m_Dlg, IDC_CMB_POSITION), CB_INSERTSTRING,  3, (LPARAM)"Right bottom");
		SendMessage(GetDlgItem(m_Dlg, IDC_CMB_POSITION), CB_INSERTSTRING,  4, (LPARAM)"Custom");
		SendMessage(GetDlgItem(m_Dlg, IDC_CMB_POSITION), CB_SETMINVISIBLE, 5, 0);

		//// Init frames per second
		//short lower = 1;
		//short upper = 60;

		//// Init UI
		//long lResult = SendMessage(			// returns LRESULT in lResult
		//	GetDlgItem(m_Dlg, IDC_SPIN_FPS),	// handle to destination control
		//	(UINT) UDM_SETRANGE,			// message ID
		//	(WPARAM) 0,						// = 0; not used, must be zero
		//	(LPARAM) MAKELONG ( upper, lower)      // = (LPARAM) MAKELONG ((short) nUpper, (short) nLower)
		//	);
		//// Default value of 30 FPS
		//SetDlgItemText(m_Dlg, IDC_FPS, "30");

	}
};
