/** @file

MODULE				: OpusDecoderFilter

FILE NAME			: OpusDecoderFilter.cpp

DESCRIPTION			:

LICENSE: Software License Agreement (BSD License)

Copyright (c) 2014, CSIR
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
#include "OpusDecoderFilter.h"
#include <Codecs/CodecUtils/ICodecv2.h>
#include <Codecs/Opus/OpusFactory.h>
#include <Shared/Conversion.h>

OpusDecoderFilter::OpusDecoderFilter()
	: CCustomBaseFilter(NAME("CSIR VPP Opus Decoder"), 0, CLSID_VPP_OpusDecoder), m_pCodec(NULL)
{
	//Call the initialise input method to load all acceptable input types for this filter
	InitialiseInputTypes();
	initParameters();
	OpusFactory factory;
	m_pCodec = factory.GetCodecInstance();
	// Set default codec properties 
	if (m_pCodec)
	{
	}
	else
	{
		SetLastError("Unable to create Opus Decoder from Factory.", true);
	}
}

OpusDecoderFilter::~OpusDecoderFilter()
{
	if (m_pCodec)
	{
		m_pCodec->Close();
		OpusFactory factory;
		factory.ReleaseCodecInstance(m_pCodec);
	}
}

CUnknown * WINAPI OpusDecoderFilter::CreateInstance( LPUNKNOWN pUnk, HRESULT *pHr )
{
	OpusDecoderFilter *pFilter = new OpusDecoderFilter();
	if (pFilter== NULL) 
	{
		*pHr = E_OUTOFMEMORY;
	}
	return pFilter;
}

void OpusDecoderFilter::InitialiseInputTypes()
{
	AddInputType(&MEDIATYPE_Audio, &MEDIASUBTYPE_OPUS, &FORMAT_WaveFormatEx);
}

HRESULT OpusDecoderFilter::SetMediaType( PIN_DIRECTION direction, const CMediaType *pmt )
{
	HRESULT hr = CCustomBaseFilter::SetMediaType(direction, pmt);
	if (direction == PINDIR_INPUT)
	{
		if (pmt->majortype != MEDIATYPE_Audio)
			return E_FAIL;
		if (pmt->subtype != MEDIASUBTYPE_OPUS)
			return E_FAIL;
		if (pmt->formattype != FORMAT_WaveFormatEx)
			return E_FAIL;

    WAVEFORMATEX *pWfx = (WAVEFORMATEX*)pmt->pbFormat;
    m_pCodec->SetParameter("samples_per_second", toString(pWfx->nSamplesPerSec).c_str());
    m_pCodec->SetParameter("channels", toString(pWfx->nChannels).c_str());
    m_pCodec->SetParameter("bits_per_sample", toString(pWfx->wBitsPerSample).c_str());

		if (!m_pCodec->Open())
		{
			//Houston: we have a failure
			char* szErrorStr = m_pCodec->GetErrorStr();
			printf("%s\n", szErrorStr);
			SetLastError(szErrorStr, true);
		}
	}
	return hr;
}

HRESULT OpusDecoderFilter::GetMediaType( int iPosition, CMediaType *pMediaType )
{
	if (iPosition < 0)
	{
		return E_INVALIDARG;
	}
	else if (iPosition == 0)
	{
		if (m_pInput->IsConnected() == FALSE)
			return E_FAIL;

#if 0
    m_outMt.lSampleSize = 0;
    m_outMt.pUnk = NULL;
    pWfex->nAvgBytesPerSec = 16000; //nSamplesPerSec × nBlockAlign
#endif
    HRESULT hr = m_pInput->ConnectionMediaType(pMediaType);
    if (FAILED(hr))
    {
      return hr;
    }

		// there is only one type we can produce
		pMediaType->majortype = MEDIATYPE_Audio;
		pMediaType->subtype = MEDIASUBTYPE_PCM;
		pMediaType->formattype = FORMAT_WaveFormatEx;
		pMediaType->SetTemporalCompression(FALSE);
    pMediaType->bFixedSizeSamples = TRUE;

		WAVEFORMATEX *wfx = (WAVEFORMATEX*)pMediaType->pbFormat;
		// memset(wfx, 0, sizeof(*wfx));
		wfx->wFormatTag	= WAVE_FORMAT_PCM;
		// wfx->cbSize = 0;
		// wfx->wBitsPerSample = 16;
		// wfx->nSamplesPerSec = 8000;
		wfx->nBlockAlign = wfx->nChannels*(wfx->wBitsPerSample>>3);
		wfx->nAvgBytesPerSec = wfx->nChannels * (wfx->wBitsPerSample>>3) * wfx->nSamplesPerSec;
		return NOERROR;
		
	}
	return VFW_S_NO_MORE_ITEMS;
}

HRESULT OpusDecoderFilter::DecideBufferSize( IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProp )
{
	AM_MEDIA_TYPE mt;
	HRESULT hr = m_pOutput->ConnectionMediaType(&mt);
	if (FAILED(hr))
	{
		return hr;
	}
	//Make sure that the format type is our custom format
	ASSERT(mt.formattype == FORMAT_WaveFormatEx);
  WAVEFORMATEX *pWfx = (WAVEFORMATEX*)mt.pbFormat;

	// decide the size
  pProp->cbBuffer = pWfx->nSamplesPerSec * pWfx->wBitsPerSample * pWfx->nChannels / 8;

  // update max compressed size of codec
  m_pCodec->SetParameter("max_compr_size", toString(pProp->cbBuffer).c_str());

	pProp->cBuffers		= 5;
  pProp->cbAlign = pWfx->nBlockAlign;
	pProp->cbPrefix		= 0;

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

HRESULT OpusDecoderFilter::ApplyTransform(BYTE* pBufferIn, long lInBufferSize, long lActualDataLength, BYTE* pBufferOut, long lOutBufferSize, long& lOutActualDataLength)
{
#if 0
  //  we are overriding Receive to control outgoing samples.
  // This should never get called
  return E_FAIL;
#else
  //make sure we were able to initialise our converter
  if (m_pCodec && m_pCodec->Ready())
  {
      //long lDataLength = lActualDataLength;
      // Mult by 8 to get number of bits
      int nBitLength = lActualDataLength * 8;
      int nResult = m_pCodec->Decode(pBufferIn, nBitLength, pBufferOut);
      if (nResult)
      {
        // Decoding was successful
        lOutActualDataLength = m_pCodec->GetCompressedByteLength();
        return S_OK;
      }
      else
      {
        //An error has occurred
        DbgLog((LOG_TRACE, 0, TEXT("Opus Decode Error: %s"), m_pCodec->GetErrorStr()));
        std::string sError = "Opus decode error has occurred: " + std::string(m_pCodec->GetErrorStr());
        SetLastError(sError.c_str(), true);
      }
  }
#endif
  return E_FAIL;
}

#if 0
HRESULT OpusDecoderFilter::Receive(IMediaSample *pSample)
{
	if (m_pCodec && m_pCodec->Ready())
	{
		BYTE *pSrc;
		long n;
    HRESULT hr;
#if 1
		std::vector<BYTE> dst(160*sizeof(short));

		pSample->GetPointer(&pSrc);
		n = pSample->GetActualDataLength();
		REFERENCE_TIME	rtStart, rtStop, rtOutput;
		HRESULT hr = pSample->GetTime(&rtStart, &rtStop);
		rtOutput = rtStart;

		while (n)
		{
			int processed = m_pCodec->Decode(pSrc, n, &dst[0]);
			
			if (processed < 0)
			{
				break;
			}

			IMediaSample	*osample = NULL;
			// deliver data ... 
			hr = m_pOutput->GetDeliveryBuffer(&osample, NULL, NULL, 0);
			if (FAILED(hr)) return hr;

			long out_size = 160 * 1 * sizeof(short);
			BYTE *out;
			osample->GetPointer(&out);
			memcpy(out, &dst[0], out_size);
			osample->SetActualDataLength(out_size);

			// timestamps
			REFERENCE_TIME	rtdur = 20*10000;
			REFERENCE_TIME	start, stop;

			start		= rtOutput;
			rtOutput   += rtdur;
			stop		= rtOutput;
			
			osample->SetTime(&start, &stop);

			// deliver downstream
			hr = m_pOutput->Deliver(osample);

			osample->Release();



			pSrc += processed;
			n -= processed;
		}
#else

#endif

		return hr;
	}

	return E_FAIL;
}
#endif

HRESULT OpusDecoderFilter::CheckTransform( const CMediaType *mtIn, const CMediaType *mtOut )
{
	// Check the major type.
	if (mtOut->majortype != MEDIATYPE_Audio)
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}
	if (mtOut->subtype != MEDIASUBTYPE_PCM)
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}
	if (mtOut->formattype != FORMAT_WaveFormatEx)
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}
	// Everything is good.
	return S_OK;
}

STDMETHODIMP OpusDecoderFilter::GetParameter( const char* szParamName, int nBufferSize, char* szValue, int* pLength )
{
	if (SUCCEEDED(CCustomBaseFilter::GetParameter(szParamName, nBufferSize, szValue, pLength)))
	{
		return S_OK;
	}
	else
	{
		// Check if its a codec parameter
		if (m_pCodec && m_pCodec->GetParameter(szParamName, pLength, szValue))
		{
			return S_OK;
		}
		return E_FAIL;
	}
}

STDMETHODIMP OpusDecoderFilter::SetParameter( const char* type, const char* value )
{
	if (SUCCEEDED(CCustomBaseFilter::SetParameter(type, value)))
	{
		return S_OK;
	}
	else
	{
		// Check if it's a codec parameter
		if (m_pCodec && m_pCodec->SetParameter(type, value))
		{
			return S_OK;
		}
		return E_FAIL;
	}
}

STDMETHODIMP OpusDecoderFilter::GetParameterSettings( char* szResult, int nSize )
{
	if (SUCCEEDED(CCustomBaseFilter::GetParameterSettings(szResult, nSize)))
	{
		// Now add the codec parameters to the output:
		int nLen = strlen(szResult);
		char szValue[10];
		int nParamLength = 0;
		std::string sCodecParams("Codec Parameters:\r\n");
		if( m_pCodec->GetParameter("parameters", &nParamLength, szValue))
		{
			int nParamCount = atoi(szValue);
			char szParamValue[256];
			memset(szParamValue, 0, 256);

			int nLenName = 0, nLenValue = 0;
			for (int i = 0; i < nParamCount; i++)
			{
				// Get parameter name
				const char* szParamName;
				m_pCodec->GetParameterName(i, &szParamName, &nLenName);
				// Now get the value
				m_pCodec->GetParameter(szParamName, &nLenValue, szParamValue);
				sCodecParams += "Parameter: " + std::string(szParamName) + " : Value:" + std::string(szParamValue) + "\r\n";
			}
			// now check if the buffer is big enough:
			int nTotalSize = sCodecParams.length() + nLen;
			if (nTotalSize < nSize)
			{
				memcpy(szResult + nLen, sCodecParams.c_str(), sCodecParams.length());
				// Set null terminator
				szResult[nTotalSize] = 0;
				return S_OK;
			}
			else
			{
				return E_FAIL;
			}
		}
		else
		{
			return E_FAIL;
		}
	}
	else
	{
		return E_FAIL;
	}
}

