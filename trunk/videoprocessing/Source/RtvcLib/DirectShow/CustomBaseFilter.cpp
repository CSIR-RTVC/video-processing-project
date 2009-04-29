/** @file

MODULE				: CustomBaseFilter

FILE NAME			: CustomBaseFilter.cpp

DESCRIPTION			: 
					  
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
#include <DirectShow/stdafx.h>
#include "CustomBaseFilter.h"

CCustomBaseFilter::CCustomBaseFilter(TCHAR *pObjectName, LPUNKNOWN lpUnk, CLSID clsid)
: CTransformFilter(pObjectName, lpUnk, clsid),
m_nInHeight(0),
m_nInWidth(0),
m_nInPixels(0),
m_nOutHeight(0),
m_nOutWidth(0),
m_nOutPixels(0),
m_nBitCount(0)
{
}

CCustomBaseFilter::~CCustomBaseFilter(void)
{
}

void CCustomBaseFilter::AddInputType( const GUID* pType, const GUID* pSubType, const GUID* pFormat )
{
	m_vInTypes.push_back((GUID*)pType);
	m_vInSubTypes.push_back((GUID*)pSubType);
	m_vInFormats.push_back((GUID*)pFormat);
}

HRESULT CCustomBaseFilter::CheckInputType( const CMediaType *pMediaType )
{
	for (int i = 0; i < m_vInTypes.size(); i++)
	{
		if (((pMediaType->majortype == *m_vInTypes[i]) && (m_vInTypes[i]))||(!m_vInTypes[i]))
		{
			//Compare sub type
			if (((pMediaType->subtype == *m_vInSubTypes[i]) && (m_vInSubTypes[i]))||(!m_vInSubTypes[i]))
			{
				//Compare formats
				if (((pMediaType->formattype == *m_vInFormats[i])&&(m_vInFormats[i]))||(!m_vInFormats[i]))
				{
					return S_OK;
				}
			}
		}
	}
	return VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CCustomBaseFilter::SetMediaType( PIN_DIRECTION direction, const CMediaType *pmt )
{
	if (direction == PINDIR_INPUT)
	{
		if (pmt->formattype == FORMAT_VideoInfo)
		{
			// WARNING! In general you cannot just copy a VIDEOINFOHEADER
			// struct, because the BITMAPINFOHEADER member may be followed by
			// random amounts of palette entries or color masks. (See VIDEOINFO
			// structure in the DShow SDK docs.) Here it's OK because we just
			// want the information that's in the VIDEOINFOHEADER struct itself.

			VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*) pmt->pbFormat;
			CopyMemory(&m_videoInHeader, pVih, sizeof(VIDEOINFOHEADER));
			//Set these values now so we don't have to repeat it for each transform
			m_nInHeight = abs(m_videoInHeader.bmiHeader.biHeight);
			m_nInWidth = m_videoInHeader.bmiHeader.biWidth;
			m_nInPixels = m_nInHeight * m_nInWidth;
			m_nBitCount = m_videoInHeader.bmiHeader.biBitCount;
		}
	}
	else   // output pin
	{
		ASSERT(direction == PINDIR_OUTPUT);
		if (pmt->formattype == FORMAT_VideoInfo)
		{
			// Store a copy of the video header info
			VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pmt->pbFormat;
			CopyMemory(&m_videoOutHeader, pVih, sizeof(VIDEOINFOHEADER));
			//Set these values now so we don't have to repeat it for each transform
			m_nOutHeight = abs(m_videoOutHeader.bmiHeader.biHeight);
			m_nOutWidth = m_videoOutHeader.bmiHeader.biWidth;
			m_nOutPixels = m_nOutHeight * m_nOutWidth;
		}
	}
	return S_OK;
}

HRESULT CCustomBaseFilter::Transform( IMediaSample *pSource, IMediaSample *pDest )
{
	// Get pointers to the underlying buffers.
	long lSrcDataLen = pSource->GetSize();
	long lDestDataLen = pDest->GetSize();

	BYTE *pBufferIn, *pBufferOut;
	HRESULT hr = pSource->GetPointer(&pBufferIn);
	if (FAILED(hr))
	{
		return hr;
	}
	
	hr = pDest->GetPointer(&pBufferOut);
	if (FAILED(hr))
	{
		return hr;
	}
	// Process the data.

	DWORD cbDest = ApplyTransform(pBufferIn, pBufferOut);
	
	KASSERT((long)cbDest <= pDest->GetSize());

	pDest->SetActualDataLength(cbDest);
	pDest->SetSyncPoint(TRUE);
	return S_OK;
}

STDMETHODIMP CCustomBaseFilter::NonDelegatingQueryInterface( REFIID riid, void **ppv )
{
	if(riid == (IID_ISettingsInterface))
	{
		return GetInterface((ISettingsInterface*) this, ppv);
	}
	else if (riid == IID_IStatusInterface)
	{
		return GetInterface((IStatusInterface*) this, ppv);
	}
	else
	{
		return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
	}
}

// TOREMOVE? Obsolete since we are now inheriting CSettingsInterface
//STDMETHODIMP CCustomBaseFilter::GetParameter( const char* szParamName, char* szResult , int& nResultLength)
//{
//	return S_FALSE;
//}
//
//STDMETHODIMP CCustomBaseFilter::SetParameter( const char* szParamName, const char* szValue )
//{
//	return S_FALSE;
//}
