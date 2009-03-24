/** @file

MODULE				: RGBtoYUV420Filter

FILE NAME			: RGBtoYUV420Filter.cpp

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
#include "RGBtoYUV420Filter.h"

// Media Type
#include <DirectShow/CustomMediaTypes.h>

// Color converters
#include <Image/RealRGB24toYUV420Converter.h>
#include <Image/RealRGB32toYUV420Converter.h>

RGBtoYUV420Filter::RGBtoYUV420Filter()
: CCustomBaseFilter(NAME("Meraka RTVC RGB 2 YUV420P Converter"), 0, CLSID_RGBtoYUV420ColorConverter),
m_pConverter(NULL),
m_nSizeYUV(0)
{
	//Call the initialise input method to load all acceptable input types for this filter
	InitialiseInputTypes();
}

RGBtoYUV420Filter::~RGBtoYUV420Filter()
{
	if (m_pConverter)
	{
		delete m_pConverter;
		m_pConverter = NULL;
	}
}

CUnknown * WINAPI RGBtoYUV420Filter::CreateInstance( LPUNKNOWN pUnk, HRESULT *pHr )
{
	RGBtoYUV420Filter *pFilter = new RGBtoYUV420Filter();
	if (pFilter== NULL) 
	{
		*pHr = E_OUTOFMEMORY;
	}
	return pFilter;
}


void RGBtoYUV420Filter::InitialiseInputTypes()
{
	// Add standard input types
	AddInputType(&MEDIATYPE_Video, &MEDIASUBTYPE_RGB24, &FORMAT_VideoInfo);
	AddInputType(&MEDIATYPE_Video, &MEDIASUBTYPE_RGB32, &FORMAT_VideoInfo);
}

HRESULT RGBtoYUV420Filter::SetMediaType( PIN_DIRECTION direction, const CMediaType *pmt )
{
	HRESULT hr = CCustomBaseFilter::SetMediaType(direction, pmt);
	if (direction == PINDIR_INPUT)
	{
		m_nSizeYUV = m_nInPixels >> 2;
		//Determine whether we are connected to a RGB24 or 32 source
		if (pmt->majortype == MEDIATYPE_Video)
		{
			//The converter might already exist if the filter has been connected previously
			if (m_pConverter)
			{
				delete m_pConverter;
				m_pConverter = NULL;
			}
			if (pmt->subtype == MEDIASUBTYPE_RGB24)
			{
				m_pConverter = new RealRGB24toYUV420Converter(m_nInWidth, m_nInHeight);
			}
			else if (pmt->subtype == MEDIASUBTYPE_RGB32)
			{
				m_pConverter = new RealRGB32toYUV420Converter(m_nInWidth, m_nInHeight);
			}
		}
	}
	return hr;
}

HRESULT RGBtoYUV420Filter::GetMediaType( int iPosition, CMediaType *pMediaType )
{
	if (iPosition < 0)
	{
		return E_INVALIDARG;
	}
	else if (iPosition == 0)
	{
		// Get the input pin's media type and return this as the output media type - we want to retain
		// all the information about the image
		HRESULT hr = m_pInput->ConnectionMediaType(pMediaType);
		if (FAILED(hr))
		{
			return hr;
		}
		// Change the output format to MEDIASUBTYPE_YUV420P 
		pMediaType->SetType(&MEDIATYPE_Video);
		pMediaType->SetSubtype(&MEDIASUBTYPE_YUV420P);
		pMediaType->SetFormatType(&FORMAT_VideoInfo);
		return S_OK;
	}
	return VFW_S_NO_MORE_ITEMS;
}

HRESULT RGBtoYUV420Filter::DecideBufferSize( IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProp )
{
	AM_MEDIA_TYPE mt;
	HRESULT hr = m_pOutput->ConnectionMediaType(&mt);
	if (FAILED(hr))
	{
		return hr;
	}
	//Make sure that the format type is our custom format
	ASSERT(mt.formattype == FORMAT_VideoInfo);
	BITMAPINFOHEADER *pbmi = HEADER(mt.pbFormat);
	//For YUV: we need to amount of bytes needed to store a frame of YUV 
	//(Since our converters are using yuvType pointers for arithmetics we need to take the total * sizeof(yuvType)
	int nSizeYUV = (m_nInPixels + (2*m_nSizeYUV)) * sizeof(yuvType);
	pProp->cbBuffer = nSizeYUV;
	
	if (pProp->cbAlign == 0)
	{
		pProp->cbAlign = 1;
	}
	if (pProp->cBuffers == 0)
	{
		pProp->cBuffers = 1;
	}
	// Release the format block.
	FreeMediaType(mt);

	// Set allocator properties.
	ALLOCATOR_PROPERTIES Actual;
	hr = pAlloc->SetProperties(pProp, &Actual);
	if (FAILED(hr))
	{
		return hr;
	}
	// Even when it succeeds, check the actual result.
	if (pProp->cbBuffer > Actual.cbBuffer) 
	{
		return E_FAIL;
	}
	return S_OK;
}

DWORD RGBtoYUV420Filter::ApplyTransform( BYTE* pBufferIn, BYTE* pBufferOut )
{
	int nTotalSize = 0;
	//make sure we were able to initialise our converter
	if (m_pConverter)
	{
		//Map everything into yuvType pointers
		yuvType* pYUV = NULL;
		pYUV = (yuvType*)pBufferOut;
		if (pYUV)
		{
			nTotalSize = (m_nInPixels + (2*m_nSizeYUV))*sizeof(yuvType);
			//Set pointer offsets into YUV array
			yuvType* pY = pYUV;
			yuvType* pU = pYUV + m_nInPixels;
			yuvType* pV = pU + m_nSizeYUV;
			//Convert to YUV
			m_pConverter->Convert((void*)pBufferIn, (void*)pY, (void*)pU, (void*)pV);
			DbgLog((LOG_TRACE, 0, TEXT("Converted to YUV directly")));
		}
		else
		{
			DbgLog((LOG_TRACE, 0, TEXT("YUV array memory not valid")));
		}
	}
	else
	{
		// Add some kind of user notification
		DbgLog((LOG_TRACE, 0, TEXT("RGB to YUV Converter is not initialised - unable to transform")));
		//m_sLastError = "RGB to YUV Converter is not initialised - unable to transform";
	}
	return nTotalSize;
}

HRESULT RGBtoYUV420Filter::CheckTransform( const CMediaType *mtIn, const CMediaType *mtOut )
{
	// Check the major type.
	if (mtOut->majortype != MEDIATYPE_Video)
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}
	// Adding advert media type to this method
	if (mtOut->subtype != MEDIASUBTYPE_YUV420P)
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}
	if (mtOut->formattype != FORMAT_VideoInfo)
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}
	// Everything is good.
	return S_OK;
}