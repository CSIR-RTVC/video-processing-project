/** @file

MODULE				: PicInPicFilter

FILE NAME			: PicInPicFilter.cpp

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
#include "PicInPicFilter.h"
#include <DirectShow/CommonDefs.h>
#include <Image/PicInPicRGB24Impl.h>
#include <Image/PicInPicRGB32Impl.h>
#include <Image/PicScalerRGB24Impl.h>

PicInPicFilter::PicInPicFilter()
	:VideoMixingBase(NAME("CSIR VPP Picture in Picture Filter"), 0, CLSID_PicInPicFilter),
	m_pPicInPic(NULL),
	m_pTargetPicScaler(NULL),
	m_pSubPicScaler(NULL),
  m_pBufferForScaledSecondaryImage(NULL)
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
    m_pTargetPicScaler = NULL;
	}
	if (m_pSubPicScaler)
	{
		delete m_pSubPicScaler;
    m_pSubPicScaler = NULL;
	}
  if (m_pBufferForScaledSecondaryImage)
  {
    delete[]m_pBufferForScaledSecondaryImage;
    m_pBufferForScaledSecondaryImage = NULL;
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
  // currently unused: supposed to be one of the standard positions to insert 
  // the picture i.e. top left or top right corner.
	// addParameter(SUB_PICTURE_POSITION, &m_nPosition, (int)SUB_PIC_POSITION_1);
  // width output picture is scaled to
	addParameter(TARGET_WIDTH, &m_nTargetWidth, 0);
  // height output picture is scaled to
  addParameter(TARGET_HEIGHT, &m_nTargetHeight, 0);
  // width inner picture is scaled to
  addParameter(SUB_PIC_WIDTH, &m_nSubPictureWidth, 0);
  // height inner picture is scaled to
  addParameter(SUB_PIC_HEIGHT, &m_nSubPictureHeight, 0);
  // x offset from bottom left corner
  addParameter(OFFSET_X, &m_nCustomOffsetX, 0);
  // y offset from bottom left corner
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
		// the target scaler is only created if the input dimensions of the primary picture don't match the target dimensions
		if ( m_pTargetPicScaler )
		{
      // scale the primary image into the out buffer
      m_pTargetPicScaler->Scale((void*)pBufferOut, (void*)m_pSampleBuffers[0]);
		}
		else
		{
      // copy the primary image
      memcpy(pBufferOut, m_pSampleBuffers[0], m_pPicInPic->GetWidth() * m_pPicInPic->GetHeight() * (size_t)m_nBytesPerPixel);
		}
    
    if (m_pSubPicScaler)
    {
      m_pSubPicScaler->Scale((void*)m_pBufferForScaledSecondaryImage, (void*)m_pSampleBuffers[1]);
      m_pPicInPic->Insert((void*)m_pBufferForScaledSecondaryImage, pBufferOut);
    }
    else
    {
      // copy direct from buffer
      m_pPicInPic->Insert((void*)m_pSampleBuffers[1], pBufferOut);
    }
	}
 	else if (m_pSampleBuffers[0])
 	{
		if ( m_pTargetPicScaler )
		{
      m_pTargetPicScaler->Scale((void*)pBufferOut, (void*)m_pSampleBuffers[0]);
		}
		else
		{
      memcpy(pBufferOut, m_pSampleBuffers[0], m_pPicInPic->GetWidth() * m_pPicInPic->GetHeight() * (size_t)m_nBytesPerPixel);
		}
 	}
 	else
 	{
    int nWidth = m_nOutputWidth;
    int nHeight = m_nOutputHeight;
		// TODO: is zeros the right filler
		memset(pBufferOut, 0, nWidth * nHeight* m_nBytesPerPixel);

    if (m_pSubPicScaler)
    {
      m_pSubPicScaler->Scale((void*)m_pBufferForScaledSecondaryImage, (void*)m_pSampleBuffers[1]);
      m_pPicInPic->Insert((void*)m_pBufferForScaledSecondaryImage, pBufferOut);
    }
    else
    {
      // copy direct from buffer
      m_pPicInPic->Insert((void*)m_pSampleBuffers[1], pBufferOut);
    }
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
    ASSERT(pMediaType->subtype == MEDIASUBTYPE_RGB24);
    m_pPicInPic = new PicInPicRGB24Impl();
    m_nBytesPerPixel = BYTES_PER_PIXEL_RGB24;
	}

	return S_OK;
}

HRESULT PicInPicFilter::SetOutputDimensions(BITMAPINFOHEADER* pBmih1, BITMAPINFOHEADER* pBmih2, int& nOutputWidth, int& nOutputHeight, int& nOutputSize)
{
  // target picture scaler
  if (m_nTargetWidth > 0 && m_nTargetHeight > 0)
  {
    // if target width and height have been set use those dimensions
    nOutputWidth = m_nTargetWidth;
    nOutputHeight = m_nTargetHeight;
    nOutputSize = m_nOutputWidth * m_nOutputHeight * m_nBytesPerPixel;
  }
  else
  {
    ASSERT(pBmih1);
    // Use size of first input
    nOutputSize = m_nSampleSizes[0];
    nOutputWidth = pBmih1->biWidth;
    nOutputHeight = pBmih1->biHeight;
  }

  reconfigure();
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
	return S_FALSE;
}

// Override to control state dependent behaviour
STDMETHODIMP PicInPicFilter::SetParameter( const char* type, const char* value )
{
	// For now, one cannot set any parameters once the output has been connected -> this will affect the buffersize
  if (m_vOutputPins[0] && m_vOutputPins[0]->IsConnected())
	{
  	if ( parameterChangeAffectsOutput( type ) )
  		return E_FAIL;
	}

	if (SUCCEEDED(CSettingsInterface::SetParameter(type, value)))
	{
		reconfigure();
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
  BITMAPINFOHEADER* pBmih1 = &m_VideoInHeader[0].bmiHeader;
  BITMAPINFOHEADER* pBmih2 = &m_VideoInHeader[1].bmiHeader;
  // target picture scaler
  if (m_nTargetWidth > 0 && m_nTargetHeight > 0)
  {
    if (!m_pTargetPicScaler)
    {
      m_pTargetPicScaler = new PicScalerRGB24Impl();
    }
    m_pTargetPicScaler->SetInDimensions(pBmih1->biWidth, pBmih1->biHeight);
    m_pTargetPicScaler->SetOutDimensions(m_nTargetWidth, m_nTargetHeight);
  }
  else
  {
    if (m_pTargetPicScaler)
    {
      delete m_pTargetPicScaler;
      m_pTargetPicScaler = NULL;
    }
  }

  // Check sub picture dimensions.
  int iSecondaryWidth = pBmih2->biWidth;
  int iSecondaryHeight = pBmih2->biHeight;
  int iSubWidth = 0;
  int iSubHeight = 0;
  if (!((m_nSubPictureWidth > 0 && m_nSubPictureWidth < getOutputWidth()) &&
       (m_nSubPictureHeight > 0 && m_nSubPictureHeight < getOutputHeight()))
    )
  {
    m_nSubPictureWidth = iSecondaryWidth;
    m_nSubPictureHeight = iSecondaryHeight;
    // if the inner picture is bigger than half the target picture, scale it by a factor of 0.4
    // TODO: do we need to scale to multiples of 4?
    if (m_nSubPictureWidth > getOutputWidth() / 2)
      m_nSubPictureWidth = m_nSubPictureWidth * 0.4;
    if (m_nSubPictureHeight > getOutputHeight() / 2)
      m_nSubPictureHeight = m_nSubPictureHeight * 0.4;
  }

  if (iSecondaryWidth != m_nSubPictureWidth || iSecondaryHeight != m_nSubPictureHeight)
  {
    ASSERT(m_nBytesPerPixel == BYTES_PER_PIXEL_RGB24);
    if (!m_pSubPicScaler)
    {
      m_pSubPicScaler = new PicScalerRGB24Impl();
    }
    m_pSubPicScaler->SetInDimensions(iSecondaryWidth, iSecondaryHeight);
    m_pSubPicScaler->SetOutDimensions(m_nSubPictureWidth, m_nSubPictureHeight);

    // recreate picture buffer
    
    if (m_pBufferForScaledSecondaryImage)
    {
      delete m_pBufferForScaledSecondaryImage;
      m_pBufferForScaledSecondaryImage = NULL;
    }
    m_pBufferForScaledSecondaryImage = new BYTE[m_nSubPictureWidth * m_nSubPictureHeight * m_nBytesPerPixel];
  }
  else
  {
    if (m_pSubPicScaler)
    {
      delete m_pSubPicScaler;
      m_pSubPicScaler = NULL;
    }
    if (m_pBufferForScaledSecondaryImage)
    {
      delete[]m_pBufferForScaledSecondaryImage;
      m_pBufferForScaledSecondaryImage = NULL;
    }
  }

  // Check validity of sub picture position else use default value
  if (!((m_nCustomOffsetX + m_nSubPictureWidth < getOutputWidth()) &&
    (m_nCustomOffsetY + m_nSubPictureHeight < getOutputHeight())))
  {
    // otherwise just use 10 pixels
    m_nCustomOffsetX = 10;
    m_nCustomOffsetY = 10;
  }

  // Setup the picture in picture
  m_pPicInPic->SetDimensions(getOutputWidth(), getOutputHeight());
  m_pPicInPic->SetSubDimensions(m_nSubPictureWidth, m_nSubPictureHeight);
  m_pPicInPic->SetPos(m_nCustomOffsetX, m_nCustomOffsetY);
}
