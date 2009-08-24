/** @file

MODULE				: Tee

FILE NAME			: Tee.cpp

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
#include "stdafx.h"

// STL
#include <cassert>

// CSIR includes
#include "Tee.h"

RtvcTee::RtvcTee() 
	:CMultiIOBaseFilter(NAME("CSIR RTVC Tee Filter"), 0, CLSID_RtvcTee)
{
	//Hack virtual method of subclass can't be called from base class contructor, hence must be called in child contructor
	Initialise();
}

RtvcTee::~RtvcTee()
{;}

CUnknown * WINAPI RtvcTee::CreateInstance( LPUNKNOWN pUnk, HRESULT *pHr )
{
	RtvcTee *pFilter = new RtvcTee();
	if (pFilter== NULL) 
	{
		*pHr = E_OUTOFMEMORY;
	}
	return pFilter;
}

HRESULT RtvcTee::Receive( IMediaSample *pSample, int nIndex )
{
	ASSERT(nIndex >= 0);
	ASSERT (nIndex < m_vInputPins.size());
	ASSERT(pSample);

	HRESULT hr;
	if (nIndex == 0)
	{
		//We have a live stream - display it
		CAutoLock lck(&m_csReceive);
		AM_SAMPLE2_PROPERTIES * const pProps = m_vInputPins[0]->SampleProps();
		if (pProps->dwStreamId != AM_STREAM_MEDIA) {
			return m_vInputPins[0]->Receive(pSample);
		}

		// Copy the input sample to all outputs
		for (size_t i = 0;  i < m_vOutputPins.size(); i++)
		{
			if (m_vOutputPins[i]->IsConnected())
			{
				IMediaSample * pOutSample;
				// Set up the output sample
				hr = InitializeOutputSample(pSample, &pOutSample, nIndex, i);

				if (FAILED(hr)) {
					return hr;
				}

				// Start timing the transform (if PERF is defined)
				MSR_START(m_idTransform);

				// Get pointers to the underlying buffers.
				BYTE *pBufferIn  = NULL;
				BYTE *pBufferOut = NULL;

				if (pSample)
				{
					HRESULT hr = pSample->GetPointer(&pBufferIn);
					if (FAILED(hr))
					{
						return hr;
					}
				}

				if (pOutSample)
				{
					HRESULT hr = pOutSample->GetPointer(&pBufferOut);
					if (FAILED(hr))
					{
						return hr;
					}
				}

				// Process the data.
				int nSize = pSample->GetActualDataLength();
				memcpy(pBufferOut, pBufferIn, nSize);

				pOutSample->SetActualDataLength(nSize);
				pOutSample->SetSyncPoint(TRUE);

				// Stop the clock and log it (if PERF is defined)
				MSR_STOP(m_idTransform);

				if (FAILED(hr)) {
					DbgLog((LOG_TRACE,1,TEXT("Error from transform")));
				} 
				else 
				{
					// the Transform() function can return S_FALSE to indicate that the
					// sample should not be delivered; we only deliver the sample if it's
					// really S_OK (same as NOERROR, of course.)
					if (hr == NOERROR) 
					{
						hr = m_vOutputPins[i]->Deliver(pOutSample);
						//m_bSampleSkipped = FALSE;	// last thing no longer dropped
					} 
					else 
					{
						// S_FALSE returned from Transform is a PRIVATE agreement
						// We should return NOERROR from Receive() in this cause because returning S_FALSE
						// from Receive() means that this is the end of the stream and no more data should
						// be sent.
						if (S_FALSE == hr) 
						{
							//  Release the sample before calling notify to avoid
							//  deadlocks if the sample holds a lock on the system
							//  such as DirectDraw buffers do
							pOutSample->Release();
							/*m_bSampleSkipped = TRUE;
							if (!m_bQualityChanged) {
							NotifyEvent(EC_QUALITY_CHANGE,0,0);
							m_bQualityChanged = TRUE;
							}*/
							return NOERROR;
						}
					}
				}
				// release the output buffer. If the connected pin still needs it,
				// it will have addrefed it itself.
				pOutSample->Release();
			}
		}
		return hr;
	}
	else
	{
		// There are only two possible indexes for this multiplexer
		return E_INVALIDARG;
	}
}

void RtvcTee::InitialiseInputTypes()
{
	AddInputType(NULL, NULL, NULL);
}

void RtvcTee::InitialiseOutputTypes()
{
	AddOutputType(NULL, NULL, NULL);
}

HRESULT RtvcTee::DecideBufferSize( IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pRequestProperties, int m_nIndex )
{
	// Get media type of the output pin
	AM_MEDIA_TYPE mt;
	HRESULT hr = m_vInputPins[0]->ConnectionMediaType(&mt);
	if (FAILED(hr))
	{
		return hr;
	} 

	// This simple tee will only cater for the following types
	assert(mt.formattype == FORMAT_VideoInfo || mt.formattype == FORMAT_WaveFormatEx);
	if(mt.formattype == FORMAT_VideoInfo)
	{
		BITMAPINFOHEADER *pbmi = HEADER(mt.pbFormat);
		pRequestProperties->cbBuffer = DIBSIZE(*pbmi);
	}
	else if (mt.formattype == FORMAT_WaveFormatEx)
	{
		WAVEFORMATEX *pWavHeader = (WAVEFORMATEX*)mt.pbFormat;
		pRequestProperties->cbBuffer = pWavHeader->nSamplesPerSec * pWavHeader->nChannels * (pWavHeader->wBitsPerSample >> 3);
	}

	if (pRequestProperties->cbAlign == 0)
	{
		pRequestProperties->cbAlign = 1;
	}
	if (pRequestProperties->cBuffers == 0)
	{
		pRequestProperties->cBuffers = 1;
	}
	// Release the format block.
	FreeMediaType(mt);

	// Set allocator properties.
	ALLOCATOR_PROPERTIES Actual;
	hr = pAlloc->SetProperties(pRequestProperties, &Actual);
	if (FAILED(hr)) 
	{
		return hr;
	}
	// Even when it succeeds, check the actual result.
	if (pRequestProperties->cbBuffer > Actual.cbBuffer) 
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT RtvcTee::GetMediaType( int iPosition, CMediaType* pMediaType, int nOutputPinIndex )
{
	if (iPosition < 0)
	{
		return E_INVALIDARG;
	}
	else if (iPosition == 0)
	{
		return m_vInputPins[0]->ConnectionMediaType(pMediaType);
	}
	return VFW_S_NO_MORE_ITEMS;
}

HRESULT RtvcTee::CheckOutputType( const CMediaType* pMediaType )
{
	AM_MEDIA_TYPE mediaType;
	HRESULT hr = m_vInputPins[0]->ConnectionMediaType(&mediaType);
	if ((*pMediaType->Type() == mediaType.majortype) &&
		(*pMediaType->Subtype() == mediaType.subtype) &&
		(*pMediaType->FormatType() == mediaType.formattype)
		)
	{
		return S_OK;
	}
	else
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}
}
