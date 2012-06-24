/** @file

MODULE				: CropProperties

FILE NAME			: CropProperties.h

DESCRIPTION			: Properties for crop filter.
					  
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
#include <cassert>
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
    setSpinBoxRange(IDC_SPIN1, 0, SHRT_MAX);
    setSpinBoxRange(IDC_SPIN2, 0, SHRT_MAX);
    setSpinBoxRange(IDC_SPIN3, 0, SHRT_MAX);
    setSpinBoxRange(IDC_SPIN4, 0, SHRT_MAX);

    // top
    HRESULT hr = setEditTextFromIntFilterParameter(TOP_CROP, IDC_EDIT_TOP);
    if (FAILED(hr)) return hr;
    // bottom
    hr = setEditTextFromIntFilterParameter(BOTTOM_CROP, IDC_EDIT_BOTTOM);
    if (FAILED(hr)) return hr;
    // left
    hr = setEditTextFromIntFilterParameter(LEFT_CROP, IDC_EDIT_LEFT);
    if (FAILED(hr)) return hr;
    // right
    hr = setEditTextFromIntFilterParameter(RIGHT_CROP, IDC_EDIT_RIGHT);
    if (FAILED(hr)) return hr;

    return hr;
	}

	HRESULT OnApplyChanges(void)
	{
    // top
    HRESULT hr = setIntFilterParameterFromEditText(TOP_CROP, IDC_EDIT_TOP);
    assert(SUCCEEDED(hr));
    // bottom
    hr = setIntFilterParameterFromEditText(BOTTOM_CROP, IDC_EDIT_BOTTOM);
    assert(SUCCEEDED(hr));
    // left
    hr = setIntFilterParameterFromEditText(LEFT_CROP, IDC_EDIT_LEFT);
    assert(SUCCEEDED(hr));
    // right
    hr = setIntFilterParameterFromEditText(RIGHT_CROP, IDC_EDIT_RIGHT);
    assert(SUCCEEDED(hr));
		return S_OK;
	} 

private:

};

