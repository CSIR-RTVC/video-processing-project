/** @file

MODULE				: H264

FILE NAME			: H264DecoderFilter.cpp

DESCRIPTION			: H.264 decoder filter implementation
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2012, CSIR
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
#include "H264DecoderFilter.h"

// RTVC
#include <Codecs/H264v2/H264v2.h>
#include <Codecs/CodecUtils/ICodecv2.h>

#include <Shared/CommonDefs.h>
#include <Shared/Conversion.h>

// Define used to switch between the actual switching version of the codec and the non switching
#ifdef _DEBUG
#pragma comment(lib, "H264v2d.lib")
#else
#pragma comment(lib, "H264v2.lib")
#endif

H264DecoderFilter::H264DecoderFilter()
  : CCustomBaseFilter(NAME("CSIR VPP H264 Decoder"), 0, CLSID_RTVC_VPP_H264Decoder),
  m_pCodec(NULL),
  m_pSeqParamSet(NULL),
  m_uiSeqParamSetLen(0),
  m_pPicParamSet(NULL),
  m_uiPicParamSetLen(0)
{
	//Call the initialise input method to load all acceptable input types for this filter
	InitialiseInputTypes();
	// Init parameters
	initParameters();
	H264v2Factory factory;
	m_pCodec = factory.GetCodecInstance();
	// Set default codec properties 
	if (m_pCodec)
	{
		//16 = YUV
		//m_pCodec->SetParameter(IN_COLOUR, "16");
		//0 = RGB only RGB is supported as an output
		m_pCodec->SetParameter("outcolour", "0");
	}
	else
	{
		SetLastError("Unable to create H264 Encoder from Factory.", true);
	}
}

H264DecoderFilter::~H264DecoderFilter()
{
  SafeDeleteArray(m_pSeqParamSet);
  SafeDeleteArray(m_pPicParamSet);

	if (m_pCodec)
	{
		m_pCodec->Close();
		H264v2Factory factory;
		factory.ReleaseCodecInstance(m_pCodec);
	}
}

CUnknown * WINAPI H264DecoderFilter::CreateInstance( LPUNKNOWN pUnk, HRESULT *pHr )
{
	H264DecoderFilter *pFilter = new H264DecoderFilter();
	if (pFilter== NULL) 
	{
		*pHr = E_OUTOFMEMORY;
	}
	return pFilter;
}

void H264DecoderFilter::InitialiseInputTypes()
{
	AddInputType(&MEDIATYPE_Video, &MEDIASUBTYPE_VPP_H264,		&FORMAT_VideoInfo);
	//RG Test 18012008
	AddInputType(&MEDIATYPE_Stream, &MEDIASUBTYPE_VPP_H264,		&FORMAT_VideoInfo);
}

HRESULT H264DecoderFilter::SetMediaType( PIN_DIRECTION direction, const CMediaType *pmt )
{
	HRESULT hr = CCustomBaseFilter::SetMediaType(direction, pmt);
	if (direction == PINDIR_INPUT)
	{
		// Configure codec
		m_pCodec->SetParameter(IMAGE_WIDTH, toString(m_nInWidth).c_str());
		m_pCodec->SetParameter(IMAGE_HEIGHT, toString(m_nInHeight).c_str());
    // always flip the image on windows to get it into the desired YUV format
    m_pCodec->SetParameter("flip",            "1");

#define RTVC_NEW_MOTION_EST
#ifdef RTVC_NEW_MOTION_EST
		// NB: New codec settings for auto-iframe detection: These settings need to correlate to the settings of the DECODER
		m_pCodec->SetParameter("unrestrictedmotion",	"1");
		m_pCodec->SetParameter("extendedpicturetype",	"1");
		m_pCodec->SetParameter("unrestrictedmotion",	"1");
		m_pCodec->SetParameter("advancedintracoding",	"1");
		m_pCodec->SetParameter("alternativeintervlc",	"1");
		m_pCodec->SetParameter("modifiedquant",			"1");
		m_pCodec->SetParameter("autoipicture",			"1");
		m_pCodec->SetParameter("ipicturefraction",		"0");
#endif

    // Check for private codec data
    // Store SPS and PPS in media format header
    int nSize = pmt->cbFormat;
    BYTE* pFormat = pmt->Format();
    unsigned uiAdditionalSize(0);
    memcpy(&uiAdditionalSize, pFormat + nSize - sizeof(int), sizeof(int));

    if (uiAdditionalSize > 0)
    {
      unsigned char startCode[4] = { 0, 0, 0, 1 };  
      char* pBuffer = new char[uiAdditionalSize];
      memcpy(pBuffer, pFormat + (nSize - uiAdditionalSize), uiAdditionalSize);
      // check for first start code
      if (memcmp(startCode, pBuffer, 4) == 0)
      {
        int index = 0;
        // adjust to compensate for start code
        for (size_t i = 4; i < uiAdditionalSize - 8; ++i) // skip 1st start code and since start code is 4 bytes long don't cmp past the end
        {
          // search for second start code
          if (memcmp(startCode, &pBuffer[i], 4) == 0)
          {
            index = i;
            break;
          }
        }

        // if we found the second start code
        if (index != 0)
        {
          SafeDeleteArray(m_pSeqParamSet);
          SafeDeleteArray(m_pPicParamSet);
          m_uiSeqParamSetLen = index /*- 4*/; // subtract
          m_pSeqParamSet = new unsigned char[m_uiSeqParamSetLen];
          memcpy(m_pSeqParamSet, pBuffer /*+ 4*/, m_uiSeqParamSetLen);
          m_uiPicParamSetLen = uiAdditionalSize /*- 8 */- m_uiSeqParamSetLen - sizeof(int);
          m_pPicParamSet = new unsigned char[m_uiPicParamSetLen];
          memcpy(m_pPicParamSet, pBuffer + index /*+ 4*/, m_uiPicParamSetLen);

#if 1
          BYTE buffer[256];
          // now configure the codec
          int nResult = m_pCodec->Decode(m_pSeqParamSet, m_uiSeqParamSetLen * 8, buffer);
          if (nResult)
          {
            //Encoding was successful
            nResult = m_pCodec->Decode(m_pPicParamSet, m_uiPicParamSetLen * 8, buffer);
            if (nResult)
            {
              // success
              int t = 0;
            }
            else
            {
              //An error has occurred
              DbgLog((LOG_TRACE, 0, TEXT("H264 Decode Error: %s"), m_pCodec->GetErrorStr()));
              std::string sError = "H264 decode error has occurred: " + std::string(m_pCodec->GetErrorStr());
              SetLastError(sError.c_str(), true);
            }
          }
          else
          {
            //An error has occurred
            DbgLog((LOG_TRACE, 0, TEXT("H264 Decode Error: %s"), m_pCodec->GetErrorStr()));
            std::string sError = "H264 decode error has occurred: " + std::string(m_pCodec->GetErrorStr());
            SetLastError(sError.c_str(), true);
          }
#endif
        }
      }
      delete[] pBuffer;
    }


		if (!m_pCodec->Open())
		{
			//Houston: we have a failure
			char* szErrorStr = m_pCodec->GetErrorStr();
			printf("%s\n", szErrorStr);
			// report the error to the main application
			SetLastError(szErrorStr, true);
		}
	}
	return hr;
}

HRESULT H264DecoderFilter::GetMediaType( int iPosition, CMediaType *pMediaType )
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
		ASSERT(pMediaType->formattype == FORMAT_VideoInfo);
		VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pMediaType->pbFormat;
		//Now we need to calculate the size of the output image
		BITMAPINFOHEADER* pBi = &(pVih->bmiHeader);
		
    // We only support outputing 24bit RGB (This solves issues were RGB32 is encoded and decoded
    if (pBi->biBitCount != 24)
      pBi->biBitCount = 24;

    // in the case of YUV I420 input to the H264 encoder, we need to change this back to RGB
    pBi->biCompression = BI_RGB;

    // Set subtype of stream
  	pMediaType->subtype = MEDIASUBTYPE_RGB24;

    // get rid of decoder specific data
    // Store SPS and PPS in media format header
    int nSize = pMediaType->cbFormat;
    BYTE* pFormat = pMediaType->Format();
    unsigned uiAdditionalSize(0);
    memcpy(&uiAdditionalSize, pFormat + nSize - sizeof(int), sizeof(int));
    pMediaType->ReallocFormatBuffer(nSize - uiAdditionalSize);

		return S_OK;
	}
	return VFW_S_NO_MORE_ITEMS;
}

HRESULT H264DecoderFilter::DecideBufferSize( IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProp )
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
	pProp->cbBuffer = DIBSIZE(*pbmi);

	if (pProp->cbAlign == 0)
	{
		pProp->cbAlign = 1;
	}
	if (pProp->cBuffers == 0)
	{
		pProp->cBuffers = 30;
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

void H264DecoderFilter::ApplyTransform( BYTE* pBufferIn, long lInBufferSize, long lActualDataLength, BYTE* pBufferOut, long lOutBufferSize, long& lOutActualDataLength )
{
	//make sure we were able to initialise our converter
	if (m_pCodec)
	{
		if (m_pCodec->Ready())
		{
 			// Mult by 8 to get number of bits
			int nBitLength = lActualDataLength * 8;

      // check if starts with start code: in that case strip it out
      //unsigned char startCode[4] = { 0, 0, 0, 1 };  
      //if (memcmp(startCode, pBufferIn, 4) == 0)
      //{
      //  // adjust to compensate for start code
      //  pBufferIn += 4;
      //  nBitLength -= 32;
      //}

			//header.Extract((void*)pBufferIn, nBitLength);
			int nResult = m_pCodec->Decode(pBufferIn, nBitLength, pBufferOut);
			
			if (nResult)
			{
				//Encoding was successful
				lOutActualDataLength = m_nInWidth * m_nInHeight * BYTES_PER_PIXEL_RGB24;
			}
			else
			{
				//An error has occurred
				DbgLog((LOG_TRACE, 0, TEXT("H264 Decode Error: %s"), m_pCodec->GetErrorStr()));
				std::string sError = "H264 decode error has occurred: " + std::string(m_pCodec->GetErrorStr());
				SetLastError(sError.c_str(), true);
			}
		}
	}
}

STDMETHODIMP H264DecoderFilter::GetParameter( const char* type, int* length, void* value )
{
	if (m_pCodec)
	{
		if (m_pCodec->GetParameter(type, length, value))
		{
			return S_OK;
		}
	}
	return E_FAIL;
}

STDMETHODIMP H264DecoderFilter::SetParameter( const char* type, const char* value )
{
	if (m_pCodec)
	{
		if (m_pCodec->SetParameter(type, value))
		{
			return S_OK;
		}
	}
	return E_FAIL;
}


HRESULT H264DecoderFilter::CheckTransform( const CMediaType *mtIn, const CMediaType *mtOut )
{
	// Check the major type.
	if (mtOut->majortype != MEDIATYPE_Video)
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	// Check subtypes
	if (mtIn->subtype == MEDIASUBTYPE_VPP_H264)
	{
		if (mtOut->subtype != MEDIASUBTYPE_RGB24)
		{
			return VFW_E_TYPE_NOT_ACCEPTED;
		}
	}
	else
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	if (mtOut->formattype != FORMAT_VideoInfo)
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}
	return S_OK;
}
