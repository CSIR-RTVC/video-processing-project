/** @file

MODULE				: UtilityProperties

FILE NAME			: UtilityProperties.h

DESCRIPTION			: Properties for FramerateDisplay filter.
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2015, Meraka Institute
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

#include <DirectShow/FilterPropertiesBase.h>

#include "resource.h"

#define BUFFER_SIZE 256

#define UF_MODE "mode"
#define UF_FPS_TS "FPS Timestamp (TS)"
#define UF_FPS_ST "FPS Timestamp (ST)"
#define UF_TIME "Time"
#define UF_DATE_TIME "DateTime"
#define UF_FPS_TS_DATE_TIME "FPS TS + DateTime"
#define UF_FPS_ST_DATE_TIME "FPS ST + DateTime"
#define UF_MEASURE_BITRATE "Measure bitrate"
#define UF_DETECT_UPSTREAM_SOURCE "Detect upstream source"
#define UF_NONE "None"


class UtilityProperties : public FilterPropertiesBase
{
public:

	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr) 
	{
		UtilityProperties *pNewObject = new UtilityProperties(pUnk);
		if (pNewObject == NULL) 
		{
			*pHr = E_OUTOFMEMORY;
		}
		return pNewObject;
	}

	UtilityProperties::UtilityProperties(IUnknown *pUnk) : 
		FilterPropertiesBase(NAME("Utility Properties"), pUnk, IDD_UTILITY_DIALOG, IDS_UTILITY_CAPTION)
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
      case 2:
        {
          SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE), CB_SELECTSTRING,  0, (LPARAM)UF_TIME);
          break;
        }
      case 3:
        {
          SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE), CB_SELECTSTRING,  0, (LPARAM)UF_DATE_TIME);
          break;
        }
      case 4:
        {
          SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE), CB_SELECTSTRING,  0, (LPARAM)UF_FPS_TS_DATE_TIME);
          break;
        }
      case 5:
        {
          SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE), CB_SELECTSTRING,  0, (LPARAM)UF_FPS_ST_DATE_TIME);
          break;
        }
      case 6:
        {
          SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE), CB_SELECTSTRING, 0, (LPARAM)UF_MEASURE_BITRATE);
          break;
      }
      case 7:
        {
          SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE), CB_SELECTSTRING, 0, (LPARAM)UF_DETECT_UPSTREAM_SOURCE);
        break;
        }
      case 8:
        {
        SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE), CB_SELECTSTRING, 0, (LPARAM)UF_NONE);
        break;
        }
      }
    }
    else
    {
      return E_FAIL;
    }

		hr = m_pSettingsInterface->GetParameter("X", sizeof(szBuffer), szBuffer, &nLength);
		if (SUCCEEDED(hr))
		{
			SetDlgItemText(m_Dlg, IDC_EDIT_X, szBuffer);
			m_uiX = atoi(szBuffer);
		}
		else
		{
			return E_FAIL;
		}
		hr = m_pSettingsInterface->GetParameter("Y", sizeof(szBuffer), szBuffer, &nLength);
		if (SUCCEEDED(hr))
		{
			SetDlgItemText(m_Dlg, IDC_EDIT_Y, szBuffer);
			m_uiY = atoi(szBuffer);
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

		nLength = GetDlgItemText(m_Dlg, IDC_EDIT_X, szBuffer, BUFFER_SIZE);
		m_uiX = atoi(szBuffer);
		m_pSettingsInterface->SetParameter("X", szBuffer);
		nLength = GetDlgItemText(m_Dlg, IDC_EDIT_Y, szBuffer, BUFFER_SIZE);
		m_uiY = atoi(szBuffer);
		m_pSettingsInterface->SetParameter("Y", szBuffer);
		return S_OK;
	} 

  void initialiseControls()
  {
    InitCommonControls();

    // mode of operation
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE), CB_RESETCONTENT, 0, 0);
    //Add default option
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE), CB_ADDSTRING,	   0, (LPARAM)UF_FPS_TS);
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE), CB_SELECTSTRING,  0, (LPARAM)UF_FPS_TS);
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE), CB_INSERTSTRING,  1, (LPARAM)UF_FPS_ST);
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE), CB_INSERTSTRING,  2, (LPARAM)UF_TIME);
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE), CB_INSERTSTRING,  3, (LPARAM)UF_DATE_TIME);
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE), CB_INSERTSTRING,  4, (LPARAM)UF_FPS_TS_DATE_TIME);
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE), CB_INSERTSTRING,  5, (LPARAM)UF_FPS_ST_DATE_TIME);
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE), CB_INSERTSTRING,  6, (LPARAM)UF_MEASURE_BITRATE);
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE), CB_INSERTSTRING,  7, (LPARAM)UF_DETECT_UPSTREAM_SOURCE);
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE), CB_INSERTSTRING,  8, (LPARAM)UF_NONE);
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE), CB_SETMINVISIBLE, 9, 0);

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
  }
private:

	unsigned m_uiX;
	unsigned m_uiY;
};

