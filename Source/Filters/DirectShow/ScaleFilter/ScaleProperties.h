/** @file

MODULE				: ScaleProperties

FILE NAME			: ScaleProperties.h

DESCRIPTION			: Properties for scale filter.

LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008 - 2015, CSIR
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

#include <climits>
#include "resource.h"

#define BUFFER_SIZE 256

/**
 * \ingroup DirectShowFilters
 * Scale Filter Property Dialog
 */
class ScaleProperties : public FilterPropertiesBase
{
public:

  static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr)
  {
    ScaleProperties *pNewObject = new ScaleProperties(pUnk);
    if (pNewObject == NULL)
    {
      *pHr = E_OUTOFMEMORY;
    }
    return pNewObject;
  }

  ScaleProperties::ScaleProperties(IUnknown *pUnk) :
    FilterPropertiesBase(NAME("Scale Properties"), pUnk, IDD_SCALE_DIALOG, IDS_SCALE_CAPTION)
  {
    ;
  }

  HRESULT ReadSettings()
  {
    setSpinBoxRange(IDC_SPIN1, 0, SHRT_MAX);
    setSpinBoxRange(IDC_SPIN2, 0, SHRT_MAX);
    // width
    HRESULT hr = setEditTextFromIntFilterParameter(TARGET_HEIGHT, IDC_EDIT_TARGET_HEIGHT);
    if (FAILED(hr)) return hr;
    // height
    hr = setEditTextFromIntFilterParameter(TARGET_WIDTH, IDC_EDIT_TARGET_WIDTH);
    if (FAILED(hr)) return hr;
    return hr;
  }

  HRESULT OnApplyChanges(void)
  {
    // width
    HRESULT hr = setIntFilterParameterFromEditText(TARGET_WIDTH, IDC_EDIT_TARGET_WIDTH);
    if (FAILED(hr)) return hr;
    // height
    hr = setIntFilterParameterFromEditText(TARGET_HEIGHT, IDC_EDIT_TARGET_HEIGHT);
    if (FAILED(hr)) return hr;
    return S_OK;
  }
};

