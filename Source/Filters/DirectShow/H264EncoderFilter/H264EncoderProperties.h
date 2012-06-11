/** @file

MODULE				: H264EncoderProperties

FILE NAME			: H264EncoderProperties.h

DESCRIPTION			: Properties for H264 encoder filter.

LICENSE: Software License Agreement (BSD License)

Copyright (c) 2012, Meraka Institute
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

#include <climits>
#include "resource.h"

#define BUFFER_SIZE 256

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

  BOOL OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

    // use MS H264 decoder
    hr = setCheckBoxFromBoolFilterParameter(USE_MS_H264, IDC_USE_MS_H264);
    ASSERT(SUCCEEDED(hr));

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

    // use MS H264 decoder
    hr = setBoolFilterParameterFromCheckBox(USE_MS_H264, IDC_USE_MS_H264);

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

