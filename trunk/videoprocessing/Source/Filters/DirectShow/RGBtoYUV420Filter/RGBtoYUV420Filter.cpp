/** @file

MODULE				: RGBtoYUV420Filter

FILE NAME			: RGBtoYUV420Filter.cpp

DESCRIPTION			: 
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008 - 2014, CSIR
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
#include "RGBtoYUV420Filter.h"
#include <DirectShow/CommonDefs.h>
#include <DirectShow/CustomMediaTypes.h>
#include <Image/RealRGB24toYUV420Converter.h>
#include <Image/RealRGB32toYUV420Converter.h>

DEFINE_GUID(MEDIASUBTYPE_I420, 0x30323449, 0x0000, 0x0010, 0x80, 0x00,
0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71); 

RGBtoYUV420Filter::RGBtoYUV420Filter()
  : CCustomBaseFilter(NAME("CSIR VPP RGB 2 YUV420P Converter"), 0, CLSID_VPP_RGBtoYUV420ColorConverter),
  m_pConverter(NULL),
  m_nSizeYUV(0),
  m_bInvert(true)
{
  //Call the initialise input method to load all acceptable input types for this filter
  InitialiseInputTypes();
  // Init parameters
  initParameters();
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
      // Determine if we need to flip the image or not based on it's orientation
      VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER*)pmt->Format();
      if (pvi->bmiHeader.biHeight < 0 )
      {
        // don't need to invert
        m_bInvert = false;
      }
      else
      {
        m_bInvert = true;
      }

			//The converter might already exist if the filter has been connected previously
			if (m_pConverter)
			{
				delete m_pConverter;
				m_pConverter = NULL;
			}
			if (pmt->subtype == MEDIASUBTYPE_RGB24)
			{
				m_pConverter = new RealRGB24toYUV420Converter(m_nInWidth, m_nInHeight, m_nChrominanceOffset);
        m_pConverter->SetFlip(m_bInvert);
        m_pConverter->SetChrominanceOffset(m_nChrominanceOffset);
			}
			else if (pmt->subtype == MEDIASUBTYPE_RGB32)
			{
				m_pConverter = new RealRGB32toYUV420Converter(m_nInWidth, m_nInHeight);
        m_pConverter->SetFlip(m_bInvert);
        m_pConverter->SetChrominanceOffset(m_nChrominanceOffset);
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
		// Change the output format to MEDIASUBTYPE_I420 
		pMediaType->SetType(&MEDIATYPE_Video);
		pMediaType->SetSubtype(&MEDIASUBTYPE_I420);
		pMediaType->SetFormatType(&FORMAT_VideoInfo);

		// Get bitmap info header and modify
		VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER*)pMediaType->Format();
		pvi->bmiHeader.biBitCount = 12;
		pvi->bmiHeader.biCompression = MAKEFOURCC('I', '4', '2', '0');

    // YUV420 is naturally top to bottom
    // YUV DIBS have no sign and ignore it
    if (pvi->bmiHeader.biHeight < 0 )
      pvi->bmiHeader.biHeight =  -1 * pvi->bmiHeader.biHeight; // get rid of -1

    // update sample sizes
    unsigned uiSampleSize = static_cast<unsigned>(m_nInPixels * BYTES_PER_PIXEL_YUV420P);
    pvi->bmiHeader.biSizeImage = uiSampleSize;
    pMediaType->SetSampleSize( uiSampleSize );
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

HRESULT RGBtoYUV420Filter::ApplyTransform(BYTE* pBufferIn, long lInBufferSize, long lActualDataLength, BYTE* pBufferOut, long lOutBufferSize, long& lOutActualDataLength)
{
  ASSERT(m_pConverter);
  ASSERT(pBufferIn);
  ASSERT(pBufferOut);
  
  //Map everything into yuvType pointers
  yuvType* pYUV = (yuvType*)pBufferOut;
  //Set pointer offsets into YUV array
  yuvType* pY = pYUV;
  yuvType* pU = pYUV + m_nInPixels;
  yuvType* pV = pU + m_nSizeYUV;
  //Convert to YUV
  m_pConverter->Convert((void*)pBufferIn, (void*)pY, (void*)pU, (void*)pV);
  DbgLog((LOG_TRACE, 0, TEXT("Converted to YUV directly")));
  lOutActualDataLength = (m_nInPixels + (2 * m_nSizeYUV))*sizeof(yuvType);;
  return S_OK;
}

HRESULT RGBtoYUV420Filter::CheckTransform( const CMediaType *mtIn, const CMediaType *mtOut )
{
	// Check the major type.
	if (mtOut->majortype != MEDIATYPE_Video)
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}
	// Adding advert media type to this method
	if ((mtOut->subtype != MEDIASUBTYPE_YUV420P) && (mtOut->subtype != MEDIASUBTYPE_I420) )
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}
	else
	{
		VIDEOINFOHEADER *pVih1 = (VIDEOINFOHEADER*)mtIn->pbFormat;
		VIDEOINFOHEADER *pVih2 = (VIDEOINFOHEADER*)mtOut->pbFormat;

		if ( pVih1 && pVih2 )
		{
			BITMAPINFOHEADER* pBi1 = &(pVih1->bmiHeader);
			BITMAPINFOHEADER* pBi2 = &(pVih2->bmiHeader);

			//if (pBi1->biWidth != pBi2->biWidth )
			//{
			//	return VFW_E_TYPE_NOT_ACCEPTED;
			//}

      // TODO: refactor same as in GetMediaType
      if (pVih1->bmiHeader.biHeight < 0 )
      {
        if (pVih1->bmiHeader.biHeight != (-1 * pVih2->bmiHeader.biHeight))
        {
          return VFW_E_TYPE_NOT_ACCEPTED;
        }
      }
      else
      {
        if (pVih1->bmiHeader.biHeight != pVih2->bmiHeader.biHeight)
        {
          return VFW_E_TYPE_NOT_ACCEPTED;
        }
      }
		}
	}

	if (mtOut->formattype != FORMAT_VideoInfo)
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}
	// Everything is good.
	return S_OK;
}

STDMETHODIMP RGBtoYUV420Filter::SetParameter( const char* type, const char* value )
{
  if (SUCCEEDED(CSettingsInterface::SetParameter(type, value)))
  {
    if (m_pConverter)
    {
      m_pConverter->SetChrominanceOffset(m_nChrominanceOffset);
      m_pConverter->SetFlip(m_bInvert);
    }
    return S_OK;
  }
  else
  {
    return E_FAIL;
  }
}
