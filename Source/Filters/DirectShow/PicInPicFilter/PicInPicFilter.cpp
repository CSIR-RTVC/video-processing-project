/** @file

MODULE				: PicInPicFilter

FILE NAME			: PicInPicFilter.cpp

DESCRIPTION			: 
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008 - 2011, CSIR
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
	:VideoMixingBase(NAME("CSIR VPP Picture in Picture Filter"), 0, CLSID_PicInPicFilter),
	m_pPicInPic(NULL),
	m_pTargetPicScaler(NULL),
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

	if (m_pTargetPicScaler)
	{
		delete m_pTargetPicScaler;
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
	addParameter(SUB_PICTURE_POSITION, &m_nPosition, (int)SUB_PIC_POSITION_1);
	addParameter(TARGET_WIDTH, &m_nTargetWidth, 0);
	addParameter(TARGET_HEIGHT, &m_nTargetHeight, 0);
	addParameter(SUB_PIC_WIDTH, &m_nSubPictureWidth, 0);
	addParameter(SUB_PIC_HEIGHT, &m_nSubPictureHeight, 0);
	addParameter(OFFSET_X, &m_nCustomOffsetX, 0);
	addParameter(OFFSET_Y, &m_nCustomOffsetY, 0);
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
		if ( m_pTargetPicScaler )
		{
			m_pTargetPicScaler->Scale((void*)pBufferOut, (void*)m_pSampleBuffers[0]);
		}
		else
		{
			int nWidth = m_pPicInPic->GetWidth();
			int nHeight = m_pPicInPic->GetHeight();
			memcpy(pBufferOut, m_pSampleBuffers[0], nWidth * nHeight* m_nBytesPerPixel);
		}

		// Now scale the secondary image into the sub picture dimensions
		// TODO: make dynamic: for now using hard coded aspect ration of (1: 4/10, 1:4/10)
		// int iSubWidth = nWidth * 0.4;
		// int iSubHeight = nHeight * 0.4;
		BYTE* pScaledSecondaryImage = new BYTE[m_pPicInPic->GetSubWidth() * m_pPicInPic->GetSubHeight() * m_nBytesPerPixel];

		int iWidth2 = m_VideoInHeader[1].bmiHeader.biWidth;
		int iHeight2 = m_VideoInHeader[1].bmiHeader.biHeight;

		// TODO: replace with in dimensions of secondary image
		m_pSubPicScaler->SetInDimensions(iWidth2, iHeight2);
		m_pSubPicScaler->SetOutDimensions(m_pPicInPic->GetSubWidth(), m_pPicInPic->GetSubHeight());
		m_pSubPicScaler->Scale((void*)pScaledSecondaryImage, (void*)m_pSampleBuffers[1]);

		m_pPicInPic->Insert((void*)pScaledSecondaryImage, pBufferOut);

		delete[] pScaledSecondaryImage;
	}
 	else if (m_pSampleBuffers[0])
 	{
		if ( m_pTargetPicScaler )
		{
			m_pSubPicScaler->Scale((void*)pBufferOut, (void*)m_pSampleBuffers[0]);
		}
		else
		{
			int nWidth = m_pPicInPic->GetWidth();
			int nHeight = m_pPicInPic->GetHeight();
	 		memcpy(pBufferOut, m_pSampleBuffers[0], nWidth * nHeight* m_nBytesPerPixel );
		}
 	}
 	else
 	{
		int iWidth2 = m_VideoInHeader[1].bmiHeader.biWidth;
		int iHeight2 = m_VideoInHeader[1].bmiHeader.biHeight;

// 		int nWidth = m_pPicInPic->GetWidth();
// 		int nHeight = m_pPicInPic->GetHeight();
		int nWidth = m_nOutputWidth;
		int nHeight = m_nOutputHeight;

		// TODO: is zeros the right filler
		memset(pBufferOut, 0, nWidth * nHeight* m_nBytesPerPixel);

// 		int iSubWidth = nWidth * 0.4;
// 		int iSubHeight = nHeight * 0.4;
		int iSubWidth = m_nSubPictureWidth;
		int iSubHeight = m_nSubPictureHeight;

		BYTE* pScaledSecondaryImage = new BYTE[iSubWidth * iSubHeight * m_nBytesPerPixel];

		// TODO: replace with in dimensions of secondary image
		m_pSubPicScaler->SetInDimensions(iWidth2, iHeight2);
		m_pSubPicScaler->SetOutDimensions(iSubWidth, iSubHeight);
		m_pSubPicScaler->Scale((void*)pScaledSecondaryImage, (void*)m_pSampleBuffers[1]);

		m_pPicInPic->Insert((void*)pScaledSecondaryImage, pBufferOut);

		delete[] pScaledSecondaryImage;
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
	// Check if target width and height have been set and use those dimensions
	if ( m_nTargetWidth > 0 && m_nTargetHeight > 0 )
	{
		m_nOutputWidth = m_nTargetWidth;
		m_nOutputHeight = m_nTargetHeight;
		m_nOutputSize = m_nOutputWidth * m_nOutputHeight;

		if ( !m_pTargetPicScaler )
		{
			m_pTargetPicScaler = new PicScalerRGB24Impl();
		}
	}
	else
	{
		// Use size of first input
		m_nOutputSize =  m_nSampleSizes[0];
		m_nOutputWidth = pBmih1->biWidth;
		m_nOutputHeight = pBmih1->biHeight;

		if ( m_pTargetPicScaler )
		{
			delete m_pTargetPicScaler;
			m_pTargetPicScaler = NULL;
		}
	}

	// Check sub picture dimensions.
	// Only use these if valid
	int iSubWidth = 0;
	int iSubHeight = 0;
	if ( (m_nSubPictureWidth > 0 && m_nSubPictureWidth < m_nOutputWidth) && ( m_nSubPictureHeight > 0 && m_nSubPictureHeight < m_nOutputHeight) )
	{
		iSubWidth = m_nSubPictureWidth;
		iSubHeight = m_nSubPictureHeight;
	}
	else
	{
		iSubWidth = m_nOutputWidth * 0.4;
		iSubHeight = m_nOutputHeight * 0.4;
	}

	// Check sub picture position.
	// Only use these if valid
	int iSubPosX = 0;
	int iSubPosY = 0;
	// TODO: make these relative to the current picture position
	if ( (m_nCustomOffsetX + m_nSubPictureWidth < m_nOutputWidth ) &&
		 (m_nCustomOffsetY + m_nSubPictureHeight < m_nOutputHeight))
	{
		iSubPosX = m_nCustomOffsetX;
		iSubPosY = m_nCustomOffsetY;
	}
	else
	{
		iSubPosX = m_nOutputWidth * 0.1;
		iSubPosY = m_nOutputHeight * 0.1;
	}

	if (pBmih1 && pBmih2)
	{
		if ( m_pTargetPicScaler )
		{
			m_pTargetPicScaler->SetInDimensions( pBmih1->biWidth, pBmih1->biHeight );
			m_pTargetPicScaler->SetOutDimensions( m_nOutputWidth, m_nOutputHeight );
		}

		// Setup the picture in picture
		if ( m_pPicInPic )
		{
			m_pPicInPic->SetDimensions(m_nOutputWidth, m_nOutputHeight);
			m_pPicInPic->SetSubDimensions(iSubWidth, iSubHeight);
			m_pPicInPic->SetPos(iSubPosX, iSubPosY);
		}
	}
	else
	{
		if (pBmih1)
		{
			m_pPicInPic->SetDimensions(m_nOutputWidth, m_nOutputHeight);
		}
		else if (pBmih2)
		{
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

// Override to control state dependent behaviour
STDMETHODIMP PicInPicFilter::SetParameter( const char* type, const char* value )
{
	// For now, one cannot set any parameters once the output has been connected -> this will affect the buffersize
	if (m_vOutputPins[0])
	{
		if (m_vOutputPins[0]->IsConnected())
		{
			if ( parameterChangeAffectsOutput( type ) )
				return E_FAIL;
		}
	}

	// TODO: get rid of enum or get rid of string!!!!
	if (SUCCEEDED(CSettingsInterface::SetParameter(type, value)))
	{
		reconfigure();
		//m_nOutPixels = m_nOutWidth * m_nOutHeight;
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}

bool PicInPicFilter::parameterChangeAffectsOutput( const char* szParam )
{
	if ( ( strcmp(szParam, TARGET_WIDTH) == 0) ||
		 ( strcmp(szParam, TARGET_HEIGHT) == 0)
		)
	{
		return true;
	}
	return false;
}

void PicInPicFilter::reconfigure()
{
	//m_pTargetPicScaler->SetOutDimensions( m_nOutputWidth, m_nOutputHeight );
}