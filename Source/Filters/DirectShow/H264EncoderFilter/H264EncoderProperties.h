/** @file

MODULE				: H264EncoderProperties

FILE NAME			: H264EncoderProperties.h

DESCRIPTION			: Properties for H264 encoder filter.

LICENSE	: GNU Lesser General Public License

Copyright (c) 2008 - 2012, CSIR
All rights reserved.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

===========================================================================
*/
#pragma once

#include <DirectShow/FilterPropertiesBase.h>
#include <Shared/Conversion.h>

#include <climits>
#include "resource.h"

#define BUFFER_SIZE 256

const int RADIO_BUTTON_IDS[] = {IDC_RADIO_VPP, IDC_RADIO_H264, IDC_RADIO_AVC1};

class H264EncoderProperties : public FilterPropertiesBase
{
public:

  static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr) 
  {
    H264EncoderProperties *pNewObject = new H264EncoderProperties(pUnk);
    if (pNewObject == NULL) 
    {
      *pHr = E_OUTOFMEMORY;
    }
    return pNewObject;
  }

  H264EncoderProperties::H264EncoderProperties(IUnknown *pUnk) : 
  FilterPropertiesBase(NAME("H264 Properties"), pUnk, IDD_SWITCH_DIALOG, IDS_SWITCH_CAPTION)
  {;}

  INT_PTR OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
  {
    // Let the parent class handle the message.
    return FilterPropertiesBase::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
  }

  HRESULT ReadSettings()
  {
    initialiseControls();

    // Mode of operation
    HRESULT hr = setComboTextFromFilterParameter(MODE_OF_OPERATION_H264, IDC_CMB_MODE_OF_OPERATION);

    // frame bit limit
    hr = setEditTextFromIntFilterParameter(FRAME_BIT_LIMIT, IDC_EDIT_BITRATE_LIMIT);
    if (FAILED(hr)) return hr;

    // Quality
    hr = setEditTextFromIntFilterParameter(QUALITY, IDC_EDIT_QUALITY);
    if (FAILED(hr)) return hr;

    // I-Frame period
    hr = setEditTextFromIntFilterParameter(IFRAME_PERIOD, IDC_EDIT_IFRAME_PERIOD);
    if (FAILED(hr)) return hr;

    // notify
    hr = setCheckBoxFromBoolFilterParameter(NOTIFYONIFRAME, IDC_CHECK_NOTIFY);
    ASSERT(SUCCEEDED(hr));

    // H264 type
    int nLength = 0;
    char szBuffer[BUFFER_SIZE];
    hr = m_pSettingsInterface->GetParameter(H264_TYPE, sizeof(szBuffer), szBuffer, &nLength);
    if (SUCCEEDED(hr))
    {
      int nH264Type = atoi(szBuffer);
      int nRadioID = RADIO_BUTTON_IDS[nH264Type];
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

    return hr;
  }

  HRESULT OnApplyChanges(void)
  {
    // mode of operation
    HRESULT hr = setIntFilterParameterFromEditText(MODE_OF_OPERATION_H264, IDC_CMB_MODE_OF_OPERATION);
    if (FAILED(hr)) return hr;

    // frame bit limit
    hr = setIntFilterParameterFromEditText(FRAME_BIT_LIMIT, IDC_EDIT_BITRATE_LIMIT);
    if (FAILED(hr)) return hr;

    // quality
    hr = setIntFilterParameterFromEditText(QUALITY, IDC_EDIT_QUALITY);
    if (FAILED(hr)) return hr;

    // i-frame period
    hr = setIntFilterParameterFromEditText(IFRAME_PERIOD, IDC_EDIT_IFRAME_PERIOD);
    if (FAILED(hr)) return hr;

    // notify
    hr = setBoolFilterParameterFromCheckBox(NOTIFYONIFRAME, IDC_CHECK_NOTIFY);
    if (FAILED(hr)) return hr;

    //// use MS H264 decoder
    //hr = setBoolFilterParameterFromCheckBox(USE_MS_H264, IDC_USE_MS_H264);
    for (int i = 0; i <= 2; ++i)
    {
      int nRadioID = RADIO_BUTTON_IDS[i];
      int iCheck = SendMessage( GetDlgItem(m_Dlg, nRadioID),	(UINT) BM_GETCHECK,	0, 0);
      if (iCheck != 0)
      {
        std::string sID = toString(i);
        HRESULT hr = m_pSettingsInterface->SetParameter(H264_TYPE, sID.c_str());
        break;
      }
    }

    return hr;
  } 

private:

  void initialiseControls()
  {
    InitCommonControls();

    // mode of operation
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE_OF_OPERATION), CB_RESETCONTENT, 0, 0);
    //Add default option
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE_OF_OPERATION), CB_ADDSTRING,	    0, (LPARAM)"0");
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE_OF_OPERATION), CB_SELECTSTRING,  0, (LPARAM)"0");
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE_OF_OPERATION), CB_INSERTSTRING,  1, (LPARAM)"1");
    SendMessage(GetDlgItem(m_Dlg, IDC_CMB_MODE_OF_OPERATION), CB_SETMINVISIBLE, 2, 0);

    // set spin box ranges
    // Frame bit limit
    setSpinBoxRange(IDC_SPIN1, 0, SHRT_MAX);
    // I-frame period
    setSpinBoxRange(IDC_SPIN2, 0, SHRT_MAX);
    // Quality
    setSpinBoxRange(IDC_SPIN3, 0, MAX_QP);
  }
};

