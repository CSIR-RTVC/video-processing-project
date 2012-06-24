/** @file

MODULE				: FramerateDisplayProperties

FILE NAME			: FramerateDisplayProperties.h

DESCRIPTION			: Properties for FramerateDisplay filter.
					  
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
#include <cassert>
#include <DirectShow/FilterPropertiesBase.h>

#include "resource.h"

#define BUFFER_SIZE 256
#define UF_MODE "mode"
#define UF_FPS_TS "FPS Timestamp (TS)"
#define UF_FPS_ST "FPS Timestamp (ST)"

class FramerateDisplayProperties : public FilterPropertiesBase
{
public:

	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr) 
	{
		FramerateDisplayProperties *pNewObject = new FramerateDisplayProperties(pUnk);
		if (pNewObject == NULL) 
		{
			*pHr = E_OUTOFMEMORY;
		}
		return pNewObject;
	}

	FramerateDisplayProperties::FramerateDisplayProperties(IUnknown *pUnk) : 
		FilterPropertiesBase(NAME("FramerateDisplay Properties"), pUnk, IDD_FRAMERATE_DIALOG, IDS_FRAMERATE_CAPTION)
	{;}

	HRESULT ReadSettings()
	{
    initialiseControls();

    int nLength = 0;
    char szBuffer[BUFFER_SIZE];

    // Mode of operation
    HRESULT hr = m_pSettingsInterface->GetParameter("mode", sizeof(szBuffer), szBuffer, &nLength);
    if (SUCCEEDED(hr))
    {
      unsigned uiMode = atoi(szBuffer);
      switch (uiMode)
      {
      case 0:
        {
          SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE), CB_SELECTSTRING,  0, (LPARAM)UF_FPS_TS);
          break;
        }
      case 1:
        {
          SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE), CB_SELECTSTRING,  0, (LPARAM)UF_FPS_ST);
          break;
        }
      }
    }
    else
    {
      return E_FAIL;
    }

    // X
    hr = setEditTextFromIntFilterParameter("X", IDC_EDIT_X);
    if (FAILED(hr)) return hr;

    // Y
    hr = setEditTextFromIntFilterParameter("Y", IDC_EDIT_Y);
    if (FAILED(hr)) return hr;
		
		return hr;
	}

	HRESULT OnApplyChanges(void)
	{
		int nLength = 0;
		char szBuffer[BUFFER_SIZE];

    // mode of operation
    int index = ComboBox_GetCurSel(GetDlgItem(m_Dlg, IDC_CMB_MODE));
    ASSERT (index != CB_ERR);
    itoa(index, szBuffer, 10);
    m_pSettingsInterface->SetParameter(UF_MODE, szBuffer);
    //nLength = GetDlgItemText(m_Dlg, IDC_CMB_MODE, szBuffer, BUFFER_SIZE);
    //if (strcmp(szBuffer, UF_FPS_TS_DATE_TIME) == 0)
    //{

    //}
    m_pSettingsInterface->SetParameter(UF_MODE, szBuffer);

        // chrom offset
    HRESULT hr = setIntFilterParameterFromEditText("X", IDC_EDIT_X);
    assert(SUCCEEDED(hr));

    hr = setIntFilterParameterFromEditText("Y", IDC_EDIT_Y);
    assert(SUCCEEDED(hr));

		return S_OK;
	} 

private:
  void initialiseControls()
  {
    InitCommonControls();

    // mode of operation
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE), CB_RESETCONTENT, 0, 0);
    //Add default option
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE), CB_ADDSTRING,	   0, (LPARAM)UF_FPS_TS);
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE), CB_SELECTSTRING,  0, (LPARAM)UF_FPS_TS);
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE), CB_INSERTSTRING,  1, (LPARAM)UF_FPS_ST);
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE), CB_SETMINVISIBLE, 2, 0);

    setSpinBoxRange(IDC_SPIN1, 0, SHRT_MAX);
    setSpinBoxRange(IDC_SPIN2, 0, SHRT_MAX);
  }

	unsigned m_uiX;
	unsigned m_uiY;
};

