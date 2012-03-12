/** @file

MODULE				: RGB2YUVProperties

FILE NAME			: RGB2YUVProperties.h

DESCRIPTION			: Properties for RGB2YUV420 conversion filter.
					  
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
 * RGB2YUV property dialog class
 */
class RGB2YUVProperties : public FilterPropertiesBase
{
public:

	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr) 
	{
		RGB2YUVProperties *pNewObject = new RGB2YUVProperties(pUnk);
		if (pNewObject == NULL) 
		{
			*pHr = E_OUTOFMEMORY;
		}
		return pNewObject;
	}

	RGB2YUVProperties::RGB2YUVProperties(IUnknown *pUnk) : 
		FilterPropertiesBase(NAME("RGB 2 YUV Properties"), pUnk, IDD_RGB2YUV_DIALOG, IDS_RGB2YUV_CAPTION)
	{;}

	HRESULT ReadSettings()
	{
    setSpinBoxRange(IDC_SPIN1, 0, SHRT_MAX);

    // chrominance offset
    HRESULT hr = setEditTextFromIntFilterParameter(CHROMINANCE_OFFSET, IDC_EDIT_CHR_OFFSET);
    if (FAILED(hr)) return hr;

    // invert
    hr = setCheckBoxFromBoolFilterParameter(INVERT, IDC_CHECK_INVERT);

		return hr;
	}

	HRESULT OnApplyChanges(void)
	{
    // chrom offset
    HRESULT hr = setIntFilterParameterFromEditText(CHROMINANCE_OFFSET, IDC_EDIT_CHR_OFFSET);
    assert(SUCCEEDED(hr));
    // invert
    hr = setBoolFilterParameterFromCheckBox(INVERT, IDC_CHECK_INVERT);
    assert(SUCCEEDED(hr));
		return S_OK;
	} 
};

