/** @file

MODULE				: RotateFilter

FILE NAME			: RotateFilter.cpp

DESCRIPTION			: 
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008, CSIR
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

#include "RotateFilter.h"

#include <DirectShow/CommonDefs.h>

#include <Image/PicRotateRGB24Impl.h>
#include <Image/PicRotateRGB32Impl.h>

RotateFilter::RotateFilter()
: CCustomBaseFilter(NAME("CSIR RTVC Rotate Filter"), 0, CLSID_RotateFilter),
	m_pRotate(NULL),
	m_nBytesPerPixel(BYTES_PER_PIXEL_RGB24),
	m_nOutWidth(0),
	m_nOutHeight(0),
	m_nStride(0),
	m_nPadding(0)
{
	//Call the initialise input method to load all acceptable input types for this filter
	InitialiseInputTypes();
	// Init parameters
	initParameters();
}

RotateFilter::~RotateFilter()
{
	if (m_pRotate)
	{
		delete m_pRotate;
		m_pRotate = NULL;
	}
}

CUnknown * WINAPI RotateFilter::CreateInstance( LPUNKNOWN pUnk, HRESULT *pHr )
{
	RotateFilter *pFilter = new RotateFilter();
	if (pFilter== NULL) 
	{
		*pHr = E_OUTOFMEMORY;
	}
	return pFilter;
}


void RotateFilter::InitialiseInputTypes()
{
	AddInputType(&MEDIATYPE_Video, &MEDIASUBTYPE_RGB24, &FORMAT_VideoInfo);
	AddInputType(&MEDIATYPE_Video, &MEDIASUBTYPE_RGB32, &FORMAT_VideoInfo);
}

HRESULT RotateFilter::SetMediaType( PIN_DIRECTION direction, const CMediaType *pmt )
{
	HRESULT hr = CCustomBaseFilter::SetMediaType(direction, pmt);
	if (direction == PINDIR_INPUT)
	{
		//Determine whether we are connected to a RGB24 or 32 source
		if (pmt->majortype == MEDIATYPE_Video)
		{
			//The converter might already exist if the filter has been connected previously
			if (m_pRotate)
			{
				delete m_pRotate;
				m_pRotate = NULL;
			}
			if (pmt->subtype == MEDIASUBTYPE_RGB24)
			{
				m_pRotate = new PicRotateRGB24Impl();
				m_nBytesPerPixel = BYTES_PER_PIXEL_RGB24;
			}
			else if (pmt->subtype == MEDIASUBTYPE_RGB32)
			{
				m_pRotate = new PicRotateRGB32Impl();
				m_nBytesPerPixel = BYTES_PER_PIXEL_RGB32;
			}
		}
	}
	return hr;
}

HRESULT RotateFilter::GetMediaType( int iPosition, CMediaType *pMediaType )
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
		
		// Get the bitmap info header and adapt the cropped 
		//make sure that it's a video info header
		ASSERT(pMediaType->formattype == FORMAT_VideoInfo);
		VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pMediaType->pbFormat;
		//Now we need to calculate the size of the output image
		BITMAPINFOHEADER* pBi = &(pVih->bmiHeader);
		
		switch (m_nRotation)
		{
		case ROTATE_NONE:
		case ROTATE_180_DEGREES_CLOCKWISE:
		case ROTATE_FLIP_HORIZONTAL:
		case ROTATE_FLIP_VERTICAL:
			{
				m_nOutWidth = m_nInWidth;
				m_nOutHeight = m_nInHeight;
				break;
			}
		case ROTATE_90_DEGREES_CLOCKWISE:
		case ROTATE_270_DEGREES_CLOCKWISE:
		case ROTATE_FLIP_DIAGONALLY:
			{
				m_nOutWidth = m_nInHeight;
				m_nOutHeight = m_nInWidth;
				break;
			}
		}
		pBi->biHeight = m_nOutHeight;
		ASSERT(pBi->biHeight > 0);

		pBi->biWidth = m_nOutWidth;
		ASSERT(pBi->biWidth > 0);
		pBi->biSizeImage = pBi->biWidth * pBi->biHeight * m_nBytesPerPixel;

		pVih->rcSource.top = 0;
		pVih->rcSource.left = 0;
		pVih->rcSource.right = pBi->biWidth;
		pVih->rcSource.bottom = pBi->biHeight;

		pVih->rcTarget.top = 0;
		pVih->rcTarget.left = 0;
		pVih->rcTarget.right = pBi->biWidth;
		pVih->rcTarget.bottom = pBi->biHeight;

		pMediaType->lSampleSize = pBi->biSizeImage;

		RecalculateFilterParameters();
		return S_OK;
	}
	return VFW_S_NO_MORE_ITEMS;
}

HRESULT RotateFilter::DecideBufferSize( IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProp )
{
	// Adding padding to take stride into account
	pProp->cbBuffer = (m_nOutWidth + m_nPadding) * m_nOutHeight * m_nBytesPerPixel;

	if (pProp->cbAlign == 0)
	{
		pProp->cbAlign = 1;
	}
	if (pProp->cBuffers == 0)
	{
		pProp->cBuffers = 1;
	}
	
	// Set allocator properties.
	ALLOCATOR_PROPERTIES Actual;
	HRESULT hr = pAlloc->SetProperties(pProp, &Actual);
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

HRESULT RotateFilter::CheckTransform( const CMediaType *mtIn, const CMediaType *mtOut )
{
	//Make sure the input and output types are related
	if (mtOut->majortype != MEDIATYPE_Video)
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}
	// RG: 27/08/2008 Bug FIX: Resulted in incorrect image in VMR9
	/*if (!
		((mtIn->subtype == MEDIASUBTYPE_RGB24)&&(mtOut->subtype == MEDIASUBTYPE_RGB24))
		||
		((mtIn->subtype == MEDIASUBTYPE_RGB32)&&(mtOut->subtype == MEDIASUBTYPE_RGB32))
		)*/

	if (mtIn->subtype == MEDIASUBTYPE_RGB24)
		if (mtOut->subtype != MEDIASUBTYPE_RGB24)
			return VFW_E_TYPE_NOT_ACCEPTED;
		else
		{
			VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)mtOut->pbFormat;
			//Now we need to calculate the size of the output image
			BITMAPINFOHEADER* pBi = &(pVih->bmiHeader);
			int iNewStride = pBi->biWidth;
			// TEMP HACK TO TEST IF IMAGE RENDERS CORRECTLY
			if (iNewStride != m_nOutWidth)
			{
				//m_nStrideRenderer = iNewStride;
				// FOR NOW JUST REJECT THE TYPE AND LET THE COLOR CONVERTER HANDLE IT
				return VFW_E_TYPE_NOT_ACCEPTED;
			}
			int iNewWidth = pVih->rcSource.right - pVih->rcSource.left;
			int iNewHeight = pBi->biHeight;
			if (iNewHeight < 0)
			{
				// REJECT INVERTED PICTURES
				return VFW_E_TYPE_NOT_ACCEPTED;
			}
		}

	if (mtOut->subtype == MEDIASUBTYPE_RGB32)
	{
		VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)mtOut->pbFormat;
		//Now we need to calculate the size of the output image
		BITMAPINFOHEADER* pBi = &(pVih->bmiHeader);
		int iNewStride = pBi->biWidth;
		// TEMP HACK TO TEST IF IMAGE RENDERS CORRECTLY
		if (iNewStride != m_nOutWidth)
		{
			//m_nStrideRenderer = iNewStride;
			// FOR NOW JUST REJECT THE TYPE AND LET THE COLOR CONVERTER HANDLE IT
			return VFW_E_TYPE_NOT_ACCEPTED;
		}
		int iNewWidth = pVih->rcSource.right - pVih->rcSource.left;
		int iNewHeight = pBi->biHeight;
		if (iNewHeight < 0)
		{
			// REJECT INVERTED PICTURES
			return VFW_E_TYPE_NOT_ACCEPTED;
		}
	}

// 	if (mtIn->subtype == MEDIASUBTYPE_RGB32)
// 		if (mtOut->subtype != MEDIASUBTYPE_RGB32)
// 			return VFW_E_TYPE_NOT_ACCEPTED;

	if (mtOut->formattype != FORMAT_VideoInfo)
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}
	return S_OK;
}

STDMETHODIMP RotateFilter::SetParameter( const char* type, const char* value )
{
	// For now, one cannot set any parameters once the output has been connected -> this will affect the buffer size
	if (m_pOutput)
	{
		if (m_pOutput->IsConnected())
		{
			return E_FAIL;
		}
	}

	if (SUCCEEDED(CSettingsInterface::SetParameter(type, value)))
	{
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}

DWORD RotateFilter::ApplyTransform( BYTE* pBufferIn, BYTE* pBufferOut )
{
	int nTotalSize = 0;
	//make sure we were able to initialise our converter
	if (m_pRotate)
	{
		// Create temp buffer for crop
		// TODO: Add stride parameter for rotate class to avoid this extra mem alloc
		BYTE* pBuffer = new BYTE[m_nOutWidth * m_nOutHeight * m_nBytesPerPixel];

		m_pRotate->SetInDimensions(m_nInWidth, m_nInHeight);
		m_pRotate->SetRotateMode((ROTATE_MODE)m_nRotation);
		m_pRotate->Rotate((void*)pBufferIn, (void*)pBuffer);

		// Copy the cropped image with stride padding
		BYTE* pFrom = pBuffer;
		BYTE* pTo = pBufferOut;

		int nBytesPerLine = m_nOutWidth * m_nBytesPerPixel;
		for (size_t i = 0; i < m_nOutHeight; i++)
		{
			memcpy(pTo, pFrom, nBytesPerLine);
			pFrom += nBytesPerLine;
			pTo += nBytesPerLine;

			if (m_nBytesPerPixel == BYTES_PER_PIXEL_RGB24)
			{
				for (size_t j = 0; j < m_nPadding; j++)
				{
					//*pTo = 0;
					//pTo++;
				}
			}
			else if (m_nBytesPerPixel == BYTES_PER_PIXEL_RGB32)
			{
				// TESTING SO FAR HAS SHOWN THAT NO PADDING IS NECCESSARY FOR RGB32???
				//int nPadding = 0;//(m_nStrideRenderer - m_nOutWidth)*4;
 			//	for (size_t j = 0; j < nPadding; j++)
 			//	{
 			//		*pTo = 0;
 			//		pTo++;
 			//	}
			}
		}
		nTotalSize = (m_nOutWidth + m_nPadding) * m_nOutHeight * m_nBytesPerPixel;
		delete[] pBuffer;
	}
	else
	{
		DbgLog((LOG_TRACE, 0, TEXT("Rotator is not initialised - unable to rotate")));
	}
	return nTotalSize;
}

void RotateFilter::RecalculateFilterParameters()
{
// 	m_nStride =  (m_nOutWidth * (m_nBitCount / 8) + 3) & ~3;
// 	m_nPadding = m_nStride - (m_nBytesPerPixel * m_nOutWidth);

	if (m_nBytesPerPixel == BYTES_PER_PIXEL_RGB24)
	{
		m_nStride =  (m_nOutWidth * (m_nBitCount / 8) + 3) & ~3;
		m_nPadding = m_nStride - (m_nBytesPerPixel * m_nOutWidth);
	}
	else if(m_nBytesPerPixel == BYTES_PER_PIXEL_RGB32)
	{
		m_nStride =  (m_nOutWidth * (m_nBitCount >> 3));
		m_nPadding = 32 - (m_nStride%32);
		//m_nStride += m_nStride % 32;
	}
}

