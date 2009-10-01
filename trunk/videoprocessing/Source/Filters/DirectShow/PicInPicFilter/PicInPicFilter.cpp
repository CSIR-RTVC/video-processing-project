/** @file

MODULE				: PicInPicFilter

FILE NAME			: PicInPicFilter.cpp

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

// CSIR includes
#include "PicInPicFilter.h"

#include <DirectShow/CommonDefs.h>
#include <Image/PicInPicRGB24Impl.h>
#include <Image/PicInPicRGB32Impl.h>
#include <Image/PicScalerRGB24Impl.h>

PicInPicFilter::PicInPicFilter()
	:VideoMixingBase(NAME("CSIR RTVC PicInPic Video Mixing Multiplexer"), 0, CLSID_PicInPicFilter),
	m_pPicInPic(NULL),
	m_pSubPicScaler(NULL)
{
	m_pSampleBuffers[0] = NULL;
	m_pSampleBuffers[1] = NULL;

	m_nSampleSizes[0] = 0;
	m_nSampleSizes[1] = 0;

	// Init parameters
	initParameters();
}

PicInPicFilter::~PicInPicFilter()
{
	if (m_pPicInPic)
	{
		delete m_pPicInPic;
		m_pPicInPic = NULL;
	}

	if (m_pSubPicScaler)
	{
		delete m_pSubPicScaler;
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

CUnknown * WINAPI PicInPicFilter::CreateInstance( LPUNKNOWN pUnk, HRESULT *pHr )
{
	PicInPicFilter* pFilter = new PicInPicFilter();
	return pFilter;
}

void PicInPicFilter::initParameters()
{
	//addParameter(ORIENTATION, &m_nOrientation, 0);
	//addParameter(TARGET_WIDTH, &m_nOutWidth, 0);
	//addParameter(TARGET_HEIGHT, &m_nOutHeight, 0);
}

HRESULT PicInPicFilter::GenerateOutputSample(IMediaSample *pSample, int nIndex)
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

	// do pic in pic transform
 	if (m_pSampleBuffers[0] && m_pSampleBuffers[1])
	{
		// if the input dimensions of the primary picture don't match the target dimensions
		// Scale the primary image into the out buffer
		// TODO
		// For now just copy the primary image
		int nWidth = m_pPicInPic->GetWidth();
		int nHeight = m_pPicInPic->GetHeight();
		memcpy(pBufferOut, m_pSampleBuffers[0], nWidth * nHeight* m_nBytesPerPixel);

		// Now scale the secondary image into the sub picture dimensions
		// TODO: make dynamic: for now using hard coded aspect ration of (1: 4/10, 1:4/10)
		int iSubWidth = nWidth * 0.4;
		int iSubHeight = nHeight * 0.4;
		BYTE* pScaledSecondaryImage = new BYTE[iSubWidth * iSubHeight * m_nBytesPerPixel];

		// TODO: replace with in dimensions of secondary image
		m_pSubPicScaler->SetInDimensions(nWidth, nHeight);
		m_pSubPicScaler->SetOutDimensions(iSubWidth, iSubHeight);
		m_pSubPicScaler->Scale((void*)pScaledSecondaryImage, (void*)m_pSampleBuffers[1]);

		m_pPicInPic->Insert((void*)pScaledSecondaryImage, pBufferOut);
	}
 	else if (m_pSampleBuffers[0])
 	{
		int nWidth = m_pPicInPic->GetWidth();
		int nHeight = m_pPicInPic->GetHeight();
 		memcpy(pBufferOut, m_pSampleBuffers[0], nWidth * nHeight* m_nBytesPerPixel );
 	}
 	else
 	{
		int nWidth = m_pPicInPic->GetWidth();
		int nHeight = m_pPicInPic->GetHeight();
		// TODO: is zeros the right filler
		memset(pBufferOut, 0, nWidth * nHeight* m_nBytesPerPixel);

		int iSubWidth = nWidth * 0.4;
		int iSubHeight = nHeight * 0.4;
		BYTE* pScaledSecondaryImage = new BYTE[iSubWidth * iSubHeight * m_nBytesPerPixel];

		// TODO: replace with in dimensions of secondary image
		m_pSubPicScaler->SetInDimensions(nWidth, nHeight);
		m_pSubPicScaler->SetOutDimensions(iSubWidth, iSubHeight);
		m_pSubPicScaler->Scale((void*)pScaledSecondaryImage, (void*)m_pSampleBuffers[1]);

		m_pPicInPic->Insert((void*)pScaledSecondaryImage, pBufferOut);
 	}

	// Concat the pictures
// 	if (m_pSampleBuffers[0] && m_pSampleBuffers[1])
// 		m_pPicInPic->Concat(m_pSampleBuffers[0], m_pSampleBuffers[1], pBufferOut);
// 	else if (m_pSampleBuffers[0])
// 	{
// 		int nWidth = m_pPicInPic->Get1stWidth();
// 		int nHeight = m_pPicInPic->Get1stHeight();
// 		memcpy(pBufferOut, m_pSampleBuffers[0], nWidth * nHeight* m_nBytesPerPixel);
// 	}
// 	else
// 	{
// 		int nWidth = m_pPicInPic->Get2ndWidth();
// 		int nHeight = m_pPicInPic->Get2ndHeight();
// 		memcpy(pBufferOut, m_pSampleBuffers[1], nWidth * nHeight* m_nBytesPerPixel);
// 	}

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

HRESULT PicInPicFilter::ReceiveFirstSample( IMediaSample *pSample )
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
}

HRESULT PicInPicFilter::ReceiveSecondSample( IMediaSample *pSample )
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
}

HRESULT PicInPicFilter::CreateVideoMixer( const CMediaType *pMediaType, int nIndex )
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
	if (!m_pPicInPic)
	{
		if (pMediaType->subtype == MEDIASUBTYPE_RGB24)
		{
			m_pPicInPic = new PicInPicRGB24Impl();
			m_nBytesPerPixel = BYTES_PER_PIXEL_RGB24;
		}
		// TODO: cater for RGB32
		else if (pMediaType->subtype == MEDIASUBTYPE_RGB32)
		{
			m_pPicInPic = new PicInPicRGB32Impl();
			m_nBytesPerPixel = BYTES_PER_PIXEL_RGB32;
		}
	}

	// Configure 2nd input scaler
	switch( nIndex )
	{
	case 1:
		{
			if ( !m_pSubPicScaler )
			{
				if (m_nBytesPerPixel == BYTES_PER_PIXEL_RGB24)
				{
					m_pSubPicScaler = new PicScalerRGB24Impl();
				}
				// TODO: cater for RGB32
// 				else if (m_nBytesPerPixel == BYTES_PER_PIXEL_RGB32)
// 				{
// 					m_pSubPicScaler = new PicScalerRGB32Impl();
// 				}
			}
		}
	}

	return S_OK;
}

HRESULT PicInPicFilter::SetOutputDimensions( BITMAPINFOHEADER* pBmih1, BITMAPINFOHEADER* pBmih2 )
{
	if (pBmih1 && pBmih2)
	{
		// Verify that the dimensions match and set output width and height
		//switch (m_nOrientation)
		//{
		//case 0: 
		//	{
		//		// Height must be the same
		//		if (pBmih1->biHeight != pBmih2->biHeight) return E_FAIL;
		//		m_nOutputWidth = pBmih1->biWidth + pBmih2->biWidth;
		//		m_nOutputHeight = pBmih1->biHeight;
		//		break;
		//	}
		//case 1: 
		//	{
		//		// Width must be the same
		//		if (pBmih1->biWidth!= pBmih2->biWidth) return E_FAIL; 
		//		m_nOutputWidth = pBmih1->biWidth;
		//		m_nOutputHeight = pBmih1->biHeight + pBmih2->biHeight;
		//		break;
		//	}
		//}

		m_nOutputSize =  m_nSampleSizes[0];

		m_nOutputWidth = pBmih1->biWidth;
		m_nOutputHeight = pBmih1->biHeight;
		// Setup the picture concatenator
		if (m_pPicInPic)
		{
			m_pPicInPic->SetDimensions(pBmih1->biWidth, pBmih1->biHeight);

			int iSubWidth = pBmih1->biWidth * 0.4;
			int iSubHeight = pBmih1->biHeight * 0.4;

			m_pPicInPic->SetSubDimensions(iSubWidth, iSubHeight);
			m_pPicInPic->SetPos(0, 0);
		}
	}
	else
	{
		if (pBmih1)
		{
			//m_pPicInPic->Set1stDimensions(pBmih1->biWidth, pBmih1->biHeight);
			m_pPicInPic->SetDimensions(pBmih1->biWidth, pBmih1->biHeight);
		}
		else if (pBmih2)
		{
			//m_pPicInPic->Set2ndDimensions(pBmih2->biWidth, pBmih2->biHeight);
			int iSubWidth = pBmih1->biWidth * 0.4;
			int iSubHeight = pBmih1->biHeight * 0.4;

			m_pPicInPic->SetSubDimensions(iSubWidth, iSubHeight);

		}
	}

	return S_OK;
}

HRESULT PicInPicFilter::CheckOutputType( const CMediaType* pMediaType )
{
	if (m_nBytesPerPixel == BYTES_PER_PIXEL_RGB24)
	{
		if (*(pMediaType->Subtype()) == MEDIASUBTYPE_RGB24)
		{
			return S_OK;
		}
	}
	// TODO: add RGB32
	//else if (m_nBytesPerPixel == BYTES_PER_PIXEL_RGB32)
	//{
	//	if (*(pMediaType->Subtype()) == MEDIASUBTYPE_RGB32)
	//	{
	//		return S_OK;
	//	}
	//}
	return S_FALSE;
}

