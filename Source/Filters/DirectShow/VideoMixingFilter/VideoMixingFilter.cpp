/** @file

MODULE				: VideoMixingFilter

FILE NAME			: VideoMixingFilter.cpp

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
#include "stdafx.h"
#include "VideoMixingFilter.h"
#include <DirectShow/CommonDefs.h>
#include <Image/PicConcatRGB24Impl.h>
#include <Image/PicConcatRGB32Impl.h>

VideoMixingFilter::VideoMixingFilter()
	:VideoMixingBase(NAME("CSIR VPP Video Mixer"), 0, CLSID_VideoMixingFilter),
	m_pPicConcat(NULL)
{
	m_pSampleBuffers[0] = NULL;
	m_pSampleBuffers[1] = NULL;

	m_nSampleSizes[0] = 0;
	m_nSampleSizes[1] = 0;

	// Init parameters
	initParameters();
}

VideoMixingFilter::~VideoMixingFilter()
{
	if (m_pPicConcat)
	{
		delete m_pPicConcat;
		m_pPicConcat = NULL;
	}

	for (int i = 0; i < 2; i++)
	{
		if (m_pSampleBuffers[i])
		{
			delete[] m_pSampleBuffers[i];
			m_pSampleBuffers[i] = NULL;
		}
	}
}

CUnknown * WINAPI VideoMixingFilter::CreateInstance( LPUNKNOWN pUnk, HRESULT *pHr )
{
	VideoMixingFilter* pFilter = new VideoMixingFilter();
	return pFilter;
}

void VideoMixingFilter::initParameters()
{
	addParameter(ORIENTATION, &m_nOrientation, 0);
}

HRESULT VideoMixingFilter::GenerateOutputSample(IMediaSample *pSample, int nIndex)
{
	// Prepare output sample
	AM_SAMPLE2_PROPERTIES * const pProps = m_vInputPins[nIndex]->SampleProps();
	DbgLog((LOG_TRACE,0,TEXT("Video Mixer: Sample Received from input 0: start time: %I64d end time: %I64d"), pProps->tStart, pProps->tStop));
	if (pProps->dwStreamId != AM_STREAM_MEDIA) {
		return m_vInputPins[nIndex]->Receive(pSample);
	}

	IMediaSample * pOutSample;
	// Set up the output sample
	HRESULT hr = InitializeOutputSample(pSample, &pOutSample, nIndex, 0);

	if (FAILED(hr)) {
		return hr;
	}

	BYTE *pBufferOut = NULL;
	hr = pOutSample->GetPointer(&pBufferOut);
	if (FAILED(hr))
	{
		return hr;
	}

	////Set orientation
	if (m_nOrientation == 0)
	{
		// Horizontal 3 (RIGHT)
		m_pPicConcat->SetOrientation(3);
	}
	else
	{
		// Vertical = 1 (BOTTOM)
		m_pPicConcat->SetOrientation(1);
	}

	// Concat the pictures
	if (m_pSampleBuffers[0] && m_pSampleBuffers[1])
		m_pPicConcat->Concat(m_pSampleBuffers[0], m_pSampleBuffers[1], pBufferOut);
	else if (m_pSampleBuffers[0])
	{
		int nWidth = m_pPicConcat->Get1stWidth();
		int nHeight = m_pPicConcat->Get1stHeight();
		memcpy(pBufferOut, m_pSampleBuffers[0], nWidth * nHeight* m_nBytesPerPixel);
	}
	else
	{
		int nWidth = m_pPicConcat->Get2ndWidth();
		int nHeight = m_pPicConcat->Get2ndHeight();
		memcpy(pBufferOut, m_pSampleBuffers[1], nWidth * nHeight* m_nBytesPerPixel);
	}

	pOutSample->SetActualDataLength(m_nOutputSize);
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
			hr = m_vOutputPins[0]->GetInputPin()->Receive(pOutSample);
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
				return NOERROR;
			}
		}
	}
	// release the output buffer. If the connected pin still needs it,
	// it will have addrefed it itself.
	pOutSample->Release();
	return hr;
}

HRESULT VideoMixingFilter::ReceiveFirstSample( IMediaSample *pSample )
{
	// Copy the secondary sample into our buffer
	if (pSample)
	{
		BYTE *pBuffer = NULL;
		HRESULT hr = pSample->GetPointer(&pBuffer);
		if (FAILED(hr))
		{
			return hr;
		}

		// Copy the buffer
		memcpy(m_pSampleBuffers[0], pBuffer, m_nSampleSizes[0]);
		return GenerateOutputSample(pSample, 0);
	}
	else
	{
		return E_POINTER;
	}

	// Copy sample into buffer
	
	// Output sample

	/*
	// We need to produce an output sample
	// Prepare output sample
	AM_SAMPLE2_PROPERTIES * const pProps = m_vInputPins[nIndex]->SampleProps();
	DbgLog((LOG_TRACE,0,TEXT("Video Mixer: Sample Received from input 0: start time: %I64d end time: %I64d"), pProps->tStart, pProps->tStop));
	if (pProps->dwStreamId != AM_STREAM_MEDIA) {
		return m_vInputPins[nIndex]->Receive(pSample);
	}

	IMediaSample * pOutSample;
	// Set up the output sample
	hr = InitializeOutputSample(pSample, &pOutSample, nIndex, 0);

	if (FAILED(hr)) {
		return hr;
	}

	BYTE *pBufferOut = NULL;
	hr = pOutSample->GetPointer(&pBufferOut);
	if (FAILED(hr))
	{
		return hr;
	}

	// Concatenate the images
	if (m_pSecondaryStreamSample)
	{
		AM_SAMPLE2_PROPERTIES * const pProps = m_vInputPins[nIndex]->SampleProps();
		m_tPrimarySampleStartTime = pProps->tStart;
		m_tPrimarySampleStopTime = pProps->tStop;
		//If the start time of the live sample is later than the start time of the current advert
		//if ((StopTime > m_tAdvertStopTime)) //||(tAd))
		{
			//Release the sample so that we can obtain the next one
			//m_pSecondarySampleBuffer->Release();
		}

		//m_pPicConcat->Set1stDimensions(nWidth, nHeight);
		//m_pPicConcat->Set2ndDimensions(nWidthAdvert, nHeightAdvert);
		//m_pPicConcat->SetOutDimensions(nOutWidth, nOutHeight);
		////Set orientation
		//m_pPicConcat->SetOrientation(m_eOrientation);
		//m_pPicConcat->Concat(pBufferIn, pBufferIn2, pBufferOut);

		// Mix the images
		m_pSecondaryStreamSample->Release();
		m_pSecondaryStreamSample = NULL;

		DWORD cbDest = 0;
		// There might 
		if (m_pSecondarySampleBuffer)
		{
			////Set orientation
			if (m_nOrientation == 0)
			{
				// Horizontal 3 (RIGHT)
				m_pPicConcat->SetOrientation(3);
			}
			else
			{
				// Vertical = 1 (BOTTOM)
				m_pPicConcat->SetOrientation(1);
			}

			int nTemp = m_pPicConcat->Get1stWidth();
			nTemp = m_pPicConcat->Get1stHeight();

			nTemp = m_pPicConcat->Get2ndWidth();
			nTemp = m_pPicConcat->Get2ndHeight();
			// Concat the pictures
			m_pPicConcat->Concat(pBufferIn, m_pSecondarySampleBuffer, pBufferOut);
			cbDest = m_nWidth * m_nHeight * m_nBytesPerPixel;

			//memset(pBufferOut, 0, cbDest);
		}
		else
		{
			// This means that there is only one pin connected
			// Simply copy the buffer
			int nLen = m_VideoInHeader[nIndex].bmiHeader.biWidth * m_VideoInHeader[nIndex].bmiHeader.biHeight * (m_VideoInHeader[nIndex].bmiHeader.biBitCount >> 3);
			CopyMemory(pBufferOut, pBufferIn, nLen);
			cbDest = nLen;
		}

		//hr = Transform(pBufferIn, m_vadvert_buffer.size()?&m_vadvert_buffer[0]:0, m_hadvert_buffer.size()?&m_hadvert_buffer[0]:0, pOutSample);
		KASSERT((long)cbDest <= pOutSample->GetSize());
		pOutSample->SetActualDataLength(cbDest);
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
				hr = m_vOutputPins[nIndex]->GetInputPin()->Receive(pOutSample);
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
					return NOERROR;
				}
			}
		}
		// release the output buffer. If the connected pin still needs it,
		// it will have addrefed it itself.
		pOutSample->Release();
		return hr;

	}
	*/
}

HRESULT VideoMixingFilter::ReceiveSecondSample( IMediaSample *pSample )
{
	// Copy the secondary sample into our buffer
	if (pSample)
	{
		BYTE *pBuffer = NULL;
		HRESULT hr = pSample->GetPointer(&pBuffer);
		if (FAILED(hr))
		{
			return hr;
		}

		// Copy the buffer
		memcpy(m_pSampleBuffers[1], pBuffer, m_nSampleSizes[1]);
		return GenerateOutputSample(pSample, 1);
	}
	else
	{
		return E_POINTER;
	}

	/*
	m_pSecondaryStreamSample = pSample;
	AM_SAMPLE2_PROPERTIES * const pProps = m_vInputPins[nIndex]->SampleProps();
	// Check if the samples end time is AFTER the following primary samples start-time indicating that this
	// is the sample we want to lock
	//Get the new start and stop times
	m_tSecondarySampleStartTime = pProps->tStart;
	m_tSecondarySampleStopTime = pProps->tStop;
	//if (m_tSecondarySampleStopTime >= m_tPrimarySampleStopTime)
	{
		// Add a reference so that we can lock the sample
		m_pSecondaryStreamSample->AddRef();

		BYTE *pBuffer = NULL;
		// Copy the secondary sample into our buffer
		if (pSample)
		{
			HRESULT hr = pSample->GetPointer(&pBuffer);
			if (FAILED(hr))
			{
				return hr;
			}

			// Copy the buffer
			memcpy(m_pSecondarySampleBuffer, pBuffer, m_nSecondaryBufferSize);
		}
	}
	return S_OK;
	*/
}

HRESULT VideoMixingFilter::CreateVideoMixer( const CMediaType *pMediaType, int nIndex )
{
	// Create temporary sample buffers
	VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*) pMediaType->pbFormat;
	BITMAPINFOHEADER bmih = pVih->bmiHeader;

	m_nSampleSizes[nIndex] = DIBSIZE(bmih);
	if (m_pSampleBuffers[nIndex])
	{
		// Recreate in case dimensions have changed
		delete[] m_pSampleBuffers[nIndex];
	}
	m_pSampleBuffers[nIndex] = new BYTE[m_nSampleSizes[nIndex]];


	// Create appropriate picture concatenator
	if (!m_pPicConcat)
	{
		if (pMediaType->subtype == MEDIASUBTYPE_RGB24)
		{
			m_pPicConcat = new PicConcatRGB24Impl();
			m_nBytesPerPixel = BYTES_PER_PIXEL_RGB24;
		}
		else if (pMediaType->subtype == MEDIASUBTYPE_RGB32)
		{
			m_pPicConcat = new PicConcatRGB32Impl();
			m_nBytesPerPixel = BYTES_PER_PIXEL_RGB32;
		}
	}
	return S_OK;
}

HRESULT VideoMixingFilter::SetOutputDimensions(BITMAPINFOHEADER* pBmih1, BITMAPINFOHEADER* pBmih2, int& nOutputWidth, int& nOutputHeight, int& nOutputSize)
{
	if (pBmih1 && pBmih2)
	{
		// Verify that the dimensions match and set output width and height
		switch (m_nOrientation)
		{
		case 0: 
			{
				// Height must be the same
				if (pBmih1->biHeight != pBmih2->biHeight) return E_FAIL;
				nOutputWidth = pBmih1->biWidth + pBmih2->biWidth;
				nOutputHeight = pBmih1->biHeight;
				break;
			}
		case 1: 
			{
				// Width must be the same
				if (pBmih1->biWidth!= pBmih2->biWidth) return E_FAIL; 
				nOutputWidth = pBmih1->biWidth;
				nOutputHeight = pBmih1->biHeight + pBmih2->biHeight;
				break;
			}
		}
		nOutputSize =  m_nSampleSizes[0] + m_nSampleSizes[1];

		// Setup the picture concatenator
		if (m_pPicConcat)
		{
			m_pPicConcat->Set1stDimensions(pBmih1->biWidth, pBmih1->biHeight);
			m_pPicConcat->Set2ndDimensions(pBmih2->biWidth, pBmih2->biHeight);
			m_pPicConcat->SetOutDimensions(nOutputWidth, nOutputHeight);
		}
	}
	else
	{
		if (pBmih1)
			m_pPicConcat->Set1stDimensions(pBmih1->biWidth, pBmih1->biHeight);
		else if (pBmih2)
			m_pPicConcat->Set2ndDimensions(pBmih2->biWidth, pBmih2->biHeight);
	}

	return S_OK;
}

HRESULT VideoMixingFilter::CheckOutputType( const CMediaType* pMediaType )
{
	if (m_nBytesPerPixel == BYTES_PER_PIXEL_RGB24)
	{
		if (*(pMediaType->Subtype()) == MEDIASUBTYPE_RGB24)
		{
			return S_OK;
		}
	}
	else if (m_nBytesPerPixel == BYTES_PER_PIXEL_RGB32)
	{
		if (*(pMediaType->Subtype()) == MEDIASUBTYPE_RGB32)
		{
			return S_OK;
		}
	}
	return S_FALSE;
}

