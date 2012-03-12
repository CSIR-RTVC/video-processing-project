/** @file

MODULE				: VideoMixingProperties

FILE NAME			: VideoMixingProperties.h

DESCRIPTION			: 
					  
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

#include <DirectShow/CommonDefs.h>
#include <DirectShow/FilterPropertiesBase.h>
#include <Shared/Conversion.h>

#include "resource.h"
#define BUFFER_SIZE 256

const int RADIO_BUTTON_IDS[] = {IDC_HORIZONTAL, IDC_VERTICAL};

/**
 * \ingroup DirectShowFilters
 * Video mixing filter property dialog
 */
class VideoMixingProperties : public FilterPropertiesBase
{
public:

	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr) 
	{
		VideoMixingProperties *pNewObject = new VideoMixingProperties(pUnk);
		if (pNewObject == NULL) 
		{
			*pHr = E_OUTOFMEMORY;
		}
		return pNewObject;
	}

	VideoMixingProperties::VideoMixingProperties(IUnknown *pUnk) : 
	FilterPropertiesBase(NAME("Video Mixing Properties"), pUnk, IDD_VIDEO_MIXING_DIALOG, IDS_VM_CAPTION)
	{;}

	HRESULT ReadSettings()
	{
		int nLength = 0;
		char szBuffer[BUFFER_SIZE];
		HRESULT hr = m_pSettingsInterface->GetParameter(ORIENTATION, sizeof(szBuffer), szBuffer, &nLength);
		if (SUCCEEDED(hr))
		{
			int nMixingMode = atoi(szBuffer);
			int nRadioID = RADIO_BUTTON_IDS[nMixingMode];
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
		for (int i = 0; i < 2; ++i)
		{
			int nRadioID = RADIO_BUTTON_IDS[i];
			int iCheck = SendMessage( GetDlgItem(m_Dlg, nRadioID),	(UINT) BM_GETCHECK,	0, 0);
			if (iCheck != 0)
			{
				std::string sID = toString(i);
				HRESULT hr = m_pSettingsInterface->SetParameter(ORIENTATION, sID.c_str());
				break;
			}
		}
		return S_OK;
	} 
};

