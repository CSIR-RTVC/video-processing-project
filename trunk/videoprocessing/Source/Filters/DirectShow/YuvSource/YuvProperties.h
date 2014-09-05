/** @file

MODULE				: YuvProperties

FILE NAME			: YuvProperties.h

DESCRIPTION			: Properties dialog for YUV Source Filter

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
#pragma once

#include <DirectShow/FilterPropertiesBase.h>
#include <Shared/Conversion.h>

#include "resource.h"

#define BUFFER_SIZE 256

const int RADIO_BUTTON_IDS[] = {IDC_YUV_420_PLANAR, IDC_YUV_444};

/**
* \ingroup DirectShowFilters
* Properties dialog for YUV Source Filter
*/
class YuvProperties : public FilterPropertiesBase
{
public:

  static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr) 
  {
    YuvProperties *pNewObject = new YuvProperties(pUnk);
    if (pNewObject == NULL) 
    {
      *pHr = E_OUTOFMEMORY;
    }
    return pNewObject;
  }

  YuvProperties::YuvProperties(IUnknown *pUnk) : 
  FilterPropertiesBase(NAME("YUV Source Properties"), pUnk, IDD_YUV_DIALOG, IDS_YUV_CAPTION)
  {;}

  HRESULT ReadSettings()
  {
    int nLength = 0;
    char szBuffer[BUFFER_SIZE];

    initialiseControls();

    HRESULT hr = m_pSettingsInterface->GetParameter(YUV_FORMAT, sizeof(szBuffer), szBuffer, &nLength);
    if (SUCCEEDED(hr))
    {
      int nYuvFormat = atoi(szBuffer);
      int nRadioID = RADIO_BUTTON_IDS[nYuvFormat];
      long lResult = SendMessage(				// returns LRESULT in lResult
        GetDlgItem(m_Dlg, nRadioID),		// handle to destination control
        (UINT)BM_SETCHECK,					// message ID
        (WPARAM)1,							// = 0; not used, must be zero
        (LPARAM)0							// = (LPARAM) MAKELONG ((short) nUpper, (short) nLower)
        );
    }
    else
    {
      return E_FAIL;
    }

    hr = m_pSettingsInterface->GetParameter(SOURCE_DIMENSIONS, sizeof(szBuffer), szBuffer, &nLength);
    if (SUCCEEDED(hr))
      SendMessage(GetDlgItem(m_Dlg, IDC_CMB_DIMENSIONS), CB_SELECTSTRING,  0, (LPARAM)szBuffer);
    else
      return E_FAIL;

    hr = m_pSettingsInterface->GetParameter(SOURCE_FPS, sizeof(szBuffer), szBuffer, &nLength);
    if (SUCCEEDED(hr))
      // Default value of 30 FPS
      SetDlgItemText(m_Dlg, IDC_FPS, szBuffer);

    return hr;
  }

  HRESULT OnApplyChanges(void)
  {
    int nLength = 0;
    char szBuffer[BUFFER_SIZE];

    for (int i = 0; i < 2; ++i)
    {
      int nRadioID = RADIO_BUTTON_IDS[i];
      int iCheck = SendMessage(GetDlgItem(m_Dlg, nRadioID), (UINT)BM_GETCHECK, 0, 0);
      if (iCheck != 0)
      {
        std::string sID = toString(i);
        HRESULT hr = m_pSettingsInterface->SetParameter(YUV_FORMAT, sID.c_str());
        ASSERT(SUCCEEDED(hr));
        break;
      }
    }

    nLength = GetDlgItemText(m_Dlg, IDC_CMB_DIMENSIONS, szBuffer, BUFFER_SIZE);
    HRESULT hr = m_pSettingsInterface->SetParameter(SOURCE_DIMENSIONS, szBuffer);

    if (FAILED(hr)) return E_FAIL;

    nLength = GetDlgItemText(m_Dlg, IDC_FPS, szBuffer, BUFFER_SIZE);
    int iFps = atoi(szBuffer);
    hr = m_pSettingsInterface->SetParameter(SOURCE_FPS, szBuffer);
    return hr;
  } 

private:
  void initialiseControls()
  {
    InitCommonControls();

    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_DIMENSIONS), CB_RESETCONTENT, 0, 0);
    //Add default option
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_DIMENSIONS), CB_ADDSTRING,	 0, (LPARAM)"Select");
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_DIMENSIONS), CB_SELECTSTRING,  0, (LPARAM)"Select");
    // Now populate the graphs
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_DIMENSIONS), CB_INSERTSTRING,  1, (LPARAM)"176x144");
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_DIMENSIONS), CB_INSERTSTRING,  2, (LPARAM)"320x240");
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_DIMENSIONS), CB_INSERTSTRING,  3, (LPARAM)"352x288");
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_DIMENSIONS), CB_INSERTSTRING,  4, (LPARAM)"704x576");
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_DIMENSIONS), CB_SETMINVISIBLE, 5, 0);

    // Init frames per second
    short lower = 1;
    short upper = 60;

    // Init UI
    long lResult = SendMessage(			// returns LRESULT in lResult
      GetDlgItem(m_Dlg, IDC_SPIN_FPS),	// handle to destination control
      (UINT) UDM_SETRANGE,			// message ID
      (WPARAM) 0,						// = 0; not used, must be zero
      (LPARAM) MAKELONG ( upper, lower)      // = (LPARAM) MAKELONG ((short) nUpper, (short) nLower)
      );
    // Default value of 30 FPS
    SetDlgItemText(m_Dlg, IDC_FPS, "30");

  }
};

