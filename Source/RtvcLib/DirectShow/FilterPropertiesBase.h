/** @file

MODULE				: FilterPropertiesBase

FILE NAME			: FilterPropertiesBase.h

DESCRIPTION			: Filter Property base class that has a pointer to the filter's ISettingInterface COM interface.
					Subclasses must then still implement the ReadSettings and OnApplyChanges to read and write the filter settings.
					  
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

#pragma warning(push)     // disable for this header only
#pragma warning(disable:4312)
// DirectShow
#include <streams.h>
#pragma warning(pop)      // restore original warning level

#include <Commctrl.h>

#include <DirectShow/SettingsInterface.h>

#include "resource.h"

#define BUFFER_SIZE 256

/**
* Property dialog base class. This is the base classes for the property pages of filters that provide the ISettingsInterface COM interface.
* Sub classes must implement 
* - ReadSettings in which the filter settings need to be read and the dialog needs to be setup.
* - OnApplyChanges in which the dialog values must be stored in the filter.
* 
* From: http://msdn.microsoft.com/en-us/library/dd375010(VS.85).aspx Creating a Filter Property Page.
* - OnConnect is called when the client creates the property page. It sets the IUnknown pointer to the filter.
* - OnActivate is called when the dialog is created.
* - OnReceiveMessage is called when the dialog receives a window message.
* - OnApplyChanges is called when the user commits the property changes by clicking the OK or Apply button.
* - OnDisconnect is called when the user dismisses the property sheet.
*/
class FilterPropertiesBase : public CBasePropertyPage
{
public:
	HRESULT OnConnect(IUnknown *pUnk)
	{
		if (pUnk == NULL)
		{
			return E_POINTER;
		}
		ASSERT(m_pSettingsInterface == NULL);

		return pUnk->QueryInterface(IID_ISettingsInterface, 
			reinterpret_cast<void**>(&m_pSettingsInterface));
	}

	HRESULT OnActivate(void)
	{
		INITCOMMONCONTROLSEX icc;
		icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
		icc.dwICC = ICC_BAR_CLASSES;
		if (InitCommonControlsEx(&icc) == FALSE)
		{
			return E_FAIL;
		}

		ASSERT(m_pSettingsInterface != NULL);

		return ReadSettings();
	}

	BOOL OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		//TODO: only SetDirty if the spin message was intercepted
		SetDirty();
		// Let the parent class handle the message.
		return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
	} 

	HRESULT OnApplyChanges(void) = 0;

	HRESULT OnDisconnect(void)
	{
		if (m_pSettingsInterface)
		{
			m_pSettingsInterface->Release();
			m_pSettingsInterface = NULL;
		}
		return S_OK;
	}
protected:

	FilterPropertiesBase::FilterPropertiesBase(char* szName, IUnknown *pUnk, int nDialogId, int nTitleId) : 
	CBasePropertyPage(szName, pUnk, nDialogId, nTitleId),
		m_pSettingsInterface(NULL)
	{;}

	virtual HRESULT ReadSettings() = 0;


	void SetDirty()
	{
		m_bDirty = TRUE;
		if (m_pPageSite)
		{
			m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
		}
	}

  void setSpinBoxRange(int iSpinBoxId, short lower, short upper)
  {
    long lResult = SendMessage(			    // returns LRESULT in lResult
			GetDlgItem(m_Dlg, iSpinBoxId),	    // handle to destination control
			(UINT) UDM_SETRANGE,			        // message ID
			(WPARAM) 0,						            // = 0; not used, must be zero
			(LPARAM) MAKELONG ( upper, lower) // = (LPARAM) MAKELONG ((short) nUpper, (short) nLower)
			);
  }

  HRESULT setEditTextFromIntFilterParameter(const char* szParam, int iEditId)
  {
    int nLength = 0;
		char szBuffer[BUFFER_SIZE];
		HRESULT hr = m_pSettingsInterface->GetParameter(szParam, sizeof(szBuffer), szBuffer, &nLength);
		if (SUCCEEDED(hr))
		{
			SetDlgItemText(m_Dlg, iEditId, szBuffer);
      return S_OK;
		}
		else
		{
			return E_FAIL;
		}
  }

  HRESULT setComboTextFromFilterParameter(const char* szParam, int iCmbId)
  {
    int nLength = 0;
    char szBuffer[BUFFER_SIZE];
    HRESULT hr = m_pSettingsInterface->GetParameter(szParam, sizeof(szBuffer), szBuffer, &nLength);
    if (SUCCEEDED(hr))
    {
      SendMessage(GetDlgItem(m_Dlg, iCmbId), CB_SELECTSTRING,  0, (LPARAM)szBuffer);
    }
    return hr;
  }

  HRESULT setIntFilterParameterFromEditText(const char* szParam, int iEditId)
  {
    int nLength = 0;
		char szBuffer[BUFFER_SIZE];
		nLength = GetDlgItemText(m_Dlg, iEditId, szBuffer, BUFFER_SIZE);
		return m_pSettingsInterface->SetParameter(szParam, szBuffer);
  }

  HRESULT setCheckBoxFromBoolFilterParameter(const char* szParam, int iCheckBoxId)
  {
    int nLength = 0;
    char szBuffer[BUFFER_SIZE];
    HRESULT hr = m_pSettingsInterface->GetParameter(szParam, sizeof(szBuffer), szBuffer, &nLength);
		if (SUCCEEDED(hr))
		{
			WPARAM wParam = (szBuffer[0] == '0') ? 0 : 1;
			long lResult = SendMessage(				      // returns LRESULT in lResult
				GetDlgItem(m_Dlg, iCheckBoxId),	// handle to destination control
				(UINT) BM_SETCHECK,					          // message ID
				(WPARAM) wParam,							        // = 0; not used, must be zero
				(LPARAM) 0							              // = (LPARAM) MAKELONG ((short) nUpper, (short) nLower)
				);
      return S_OK;
		}
		else
		{
			return E_FAIL;
		}
  }

  HRESULT setBoolFilterParameterFromCheckBox(const char* szParam, int iCheckBoxId)
  {
    HRESULT hr(S_OK);
    int iCheck = SendMessage( GetDlgItem(m_Dlg, iCheckBoxId),	(UINT) BM_GETCHECK,	0, 0);
		if (iCheck == 0)
		{
      hr = m_pSettingsInterface->SetParameter(szParam, "false");
		}
		else
		{
			hr = m_pSettingsInterface->SetParameter(szParam, "true");
		}
    return hr;
  }

	/// Pointer to the filter's custom interface.
	/// This pointer allows subclasses access to the filter for setting configuration.
	ISettingsInterface* m_pSettingsInterface;
};

