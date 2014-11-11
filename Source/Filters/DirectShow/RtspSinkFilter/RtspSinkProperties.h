/** @file

MODULE				: RtspSinkProperties

FILE NAME			: RtspSinkProperties.h

DESCRIPTION			: Properties for H264 encoder filter.

LICENSE	: GNU Lesser General Public License

Copyright (c) 2008 - 2014, CSIR
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
#include <climits>
#include "resource.h"

#define BUFFER_SIZE 256
#define RTSP_PORT "rtsp_port"

class RtspSinkProperties : public FilterPropertiesBase
{
public:

  static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr) 
  {
    RtspSinkProperties *pNewObject = new RtspSinkProperties(pUnk);
    if (pNewObject == NULL) 
    {
      *pHr = E_OUTOFMEMORY;
    }
    return pNewObject;
  }

  RtspSinkProperties::RtspSinkProperties(IUnknown *pUnk) : 
  FilterPropertiesBase(NAME("CSIR VPP RTSP Sink Properties"), pUnk, IDD_RTSP_SINK_DIALOG, IDS_RTSP_SINK_CAPTION)
  {;}

  INT_PTR OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
  {
    // Let the parent class handle the message.
    return FilterPropertiesBase::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
  }

  HRESULT ReadSettings()
  {
    initialiseControls();

    // RTSP port
    HRESULT hr = setEditTextFromIntFilterParameter(RTSP_PORT, IDC_EDIT_RTSP_PORT);
    return hr;
  }

  HRESULT OnApplyChanges(void)
  {
    // RTSP port
    HRESULT hr = setIntFilterParameterFromEditText(RTSP_PORT, IDC_EDIT_RTSP_PORT);
    return hr;
  } 

private:

  void initialiseControls()
  {
    InitCommonControls();
    // set spin box ranges
    // RTSP port
    setSpinBoxRange(IDC_SPIN1, 0, USHRT_MAX);
  }
};

