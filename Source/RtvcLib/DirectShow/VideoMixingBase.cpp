/** @file

MODULE                : VideoMixingBase

FILE NAME            : VideoMixingBase.cpp

DESCRIPTION            :
                     
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008 - 2012, CSIR
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
#include "VideoMixingBase.h"

#include <DirectShow/CommonDefs.h>

VideoMixingBase::VideoMixingBase(TCHAR *pObjectName, LPUNKNOWN lpUnk, CLSID clsid)
:CMultiIOBaseFilter(pObjectName, lpUnk, clsid),
m_nOutputWidth(0),
m_nOutputHeight(0),
m_nOutputSize(0),
m_tStartTime1(0),
m_tStartTime2(0),
m_tStopTime1(0),
m_tStopTime2(0),
m_uiEndOfStreamCount(0),
m_uiBeginFlushCount(0),
m_uiEndFlushCount(0)
{
	//Hack virtual method of subclass can't be called from base class contructor, hence must be called in child contructor
	Initialise();
}

VideoMixingBase::~VideoMixingBase()
{}

HRESULT VideoMixingBase::Receive( IMediaSample *pSample, int nIndex )
{
	CAutoLock lck(&m_csReceive);
	ASSERT(nIndex >= 0);
	ASSERT (nIndex < (int)m_vInputPins.size());
	ASSERT(pSample);
	ASSERT (m_vOutputPins[0] != NULL);
	
	if (nIndex == 0)
	{
		return ReceiveFirstSample(pSample);
	}
	else
	{
		return ReceiveSecondSample(pSample);
	}
}

HRESULT VideoMixingBase::DecideBufferSize( IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pRequestProperties, int m_nIndex )
{
	// Get media type of the output pin
	AM_MEDIA_TYPE mt;
	HRESULT hr = m_vOutputPins[m_nIndex]->ConnectionMediaType(&mt);
	if (FAILED(hr))
	{
		return hr;
	} 
	BITMAPINFOHEADER *pbmi = HEADER(mt.pbFormat);

	//For RGB formats, hence we can just use the DIBSIZE of the bitmapinfoheader
	pRequestProperties->cbBuffer = DIBSIZE(*pbmi);

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

HRESULT VideoMixingBase::GetMediaType( int iPosition, CMediaType* pMediaType, int nOutputPinIndex )
{
	if (iPosition < 0)
	{
		return E_INVALIDARG;
	}
	else if (iPosition == 0)
	{

		CMultiIOInputPin* pConnectedPin = NULL;
		CMultiIOInputPin* pOtherPin = NULL;

		if (m_vInputPins[0]->IsConnected())
		{
			pConnectedPin = m_vInputPins[0];
			if (m_vInputPins[1]->IsConnected())
			{
				pOtherPin = m_vInputPins[1];
			}
		}
		else
		{
			if (m_vInputPins[1]->IsConnected())
			{
				pConnectedPin = m_vInputPins[1];
			}
			else
			{
				// Error: no connected input pins
				return E_FAIL;
			}
		}

		HRESULT hr = pConnectedPin->ConnectionMediaType(pMediaType);
		if (FAILED(hr))
		{
			return hr;
		}

		if (pOtherPin)
		{
			// We need to recalculate the new width and height
			hr = SetOutputDimensions(&(m_VideoInHeader[0].bmiHeader), &(m_VideoInHeader[1].bmiHeader), m_nOutputWidth, m_nOutputHeight, m_nOutputSize);
			VIDEOINFOHEADER* pVih1 = (VIDEOINFOHEADER*)pMediaType->pbFormat;
			BITMAPINFOHEADER* bmh1 = &(pVih1->bmiHeader);

			// Adjust media parameters
			bmh1->biWidth = m_nOutputWidth;
			bmh1->biHeight = m_nOutputHeight;

			bmh1->biSizeImage = m_nOutputSize;
			//Set sample size
			pMediaType->SetSampleSize(m_nOutputSize);
      if (m_nBytesPerPixel == BYTES_PER_PIXEL_RGB24)
        pMediaType->SetSubtype(&MEDIASUBTYPE_RGB24);
#ifdef RTVC_SUPPORT_RGB32
      else if (m_nBytesPerPixel == BYTES_PER_PIXEL_RGB32)
        pMediaType->SetSubtype(&MEDIASUBTYPE_RGB32);
#endif
      bmh1->biCompression = BI_RGB;

			// Set source rect
			pVih1->rcSource.left = 0;
			pVih1->rcSource.top = 0;
			pVih1->rcSource.right = m_nOutputWidth;
			pVih1->rcSource.bottom = m_nOutputHeight;

			// Set target rect
			pVih1->rcTarget.left = 0;
			pVih1->rcTarget.top = 0;
			pVih1->rcTarget.right = m_nOutputWidth;
			pVih1->rcTarget.bottom = m_nOutputHeight;
		}
		return S_OK;
	}
	return VFW_S_NO_MORE_ITEMS;
}

HRESULT VideoMixingBase::SetMediaType( PIN_DIRECTION direction, const CMediaType *pmt, int nIndex )
{
	// Check if our base class accepts the connection
	HRESULT hr = CMultiIOBaseFilter::SetMediaType(direction, pmt, nIndex);
	if (FAILED(hr))
	{
		return hr;
	}
	//TODO: Check if the sizes match: The images must correlate in dimension!!!!!!!!!!!
	if (direction == PINDIR_INPUT)
	{
		// Copy the video info header
		VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*) pmt->pbFormat;
		BITMAPINFOHEADER bmih = pVih->bmiHeader;
		CopyMemory(&m_VideoInHeader[nIndex], pVih, sizeof(VIDEOINFOHEADER));
		
		// Reject connection if output is already connected
		if (m_vOutputPins[0]->IsConnected())
		{
			return E_FAIL;
		}

		hr = CreateVideoMixer(pmt, nIndex);
		if (FAILED(hr))
		{
			return hr;
		}

		// Get first input dimensions
		AM_MEDIA_TYPE mediaType1;
		BITMAPINFOHEADER* pBmih1 = NULL;
		if (m_vInputPins[0]->IsConnected())
		{
			hr = m_vInputPins[0]->ConnectionMediaType(&mediaType1);
			if (FAILED(hr))
			{
				return hr;
			}
			VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*) mediaType1.pbFormat;
			pBmih1 = &pVih->bmiHeader;
		}

		// Get second input dimensions
		BITMAPINFOHEADER* pBmih2 = NULL;
		AM_MEDIA_TYPE mediaType2;
		if (m_vInputPins[1]->IsConnected())
		{
			hr = m_vInputPins[1]->ConnectionMediaType(&mediaType2);
			if (FAILED(hr))
			{
				return hr;
			}
			VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*) mediaType2.pbFormat;
			pBmih2 = &pVih->bmiHeader;
		}

		// Leave the output dimensions up to the sub class
		HRESULT hr = SetOutputDimensions(pBmih1, pBmih2, m_nOutputWidth, m_nOutputHeight, m_nOutputSize);

		// Free format blocks
		if (pBmih1)
			FreeMediaType(mediaType1);
		if (pBmih2)
			FreeMediaType(mediaType2);
		return hr;
	}
	return S_OK;
}

STDMETHODIMP VideoMixingBase::Stop()
{
	CAutoLock lck1(&m_csFilter);
	if (m_State == State_Stopped) {
		return NOERROR;
	}
	// If the pins are not connected
	if (((m_vInputPins[0] == NULL || m_vInputPins[0]->IsConnected() == FALSE) &&
		(m_vInputPins[1] == NULL || m_vInputPins[1]->IsConnected() == FALSE))||
		m_vOutputPins[0]->IsConnected() == FALSE) 
	{
		m_State = State_Stopped;
		return NOERROR;
	}

	ASSERT(m_vOutputPins[0]);

	// decommit the input pin before locking or we can deadlock
	m_vInputPins[0]->Inactive();
	m_vInputPins[1]->Inactive();

	// synchronize with Receive calls
	CAutoLock lck2(&m_csReceive);
	m_vOutputPins[0]->Inactive();

	// allow a class derived from VideoMixingBase
	// to know about starting and stopping streaming
	// complete the state transition
  HRESULT hr = StopStreaming();
  if (SUCCEEDED(hr)) {
    // complete the state transition
    m_State = State_Stopped;
    //m_bEOSDelivered = FALSE;
  }

	return S_OK;
}

STDMETHODIMP VideoMixingBase::Pause()
{
	CAutoLock lck(&m_csFilter);
	HRESULT hr = NOERROR;

	if (m_State == State_Paused) {
		// (This space left deliberately blank)
	}

	// If we have no input pin or it isn't yet connected then when we are
	// asked to pause we deliver an end of stream to the downstream filter.
	// This makes sure that it doesn't sit there forever waiting for
	// samples which we cannot ever deliver without an input connection.

	else if (m_vInputPins[0] == NULL || m_vInputPins[0]->IsConnected() == FALSE) 
	{
		if (m_vInputPins[1] == NULL || m_vInputPins[1]->IsConnected() == FALSE)
		{
			if (m_vOutputPins[0]) 
			{
				m_vOutputPins[0]->DeliverEndOfStream();
				//m_bEOSDelivered = TRUE;
			}
			m_State = State_Paused;
		}
	}

	// We may have an input connection but no output connection
	// However, if we have an input pin we do have an output pin
	else if (m_vOutputPins[0]->IsConnected() == FALSE) {
		// TOREVISE: This step is taken from the CBaseFilter but doesn't prevent the graph from running
		// m_State = State_Paused;
		// However this line prevents the graph from running if the output pin of the multiplexer is not connected
		return E_FAIL;

	}
	else 
	{
		if (m_State == State_Stopped) {
			// allow a class derived from CTransformFilter
			// to know about starting and stopping streaming
			CAutoLock lck2(&m_csReceive);
			hr = StartStreaming();
		}
		if (SUCCEEDED(hr)) {
			hr = CBaseFilter::Pause();
		}
	}
	return hr;
}

STDMETHODIMP VideoMixingBase::EndOfStream( int nIndex )
{
  ++m_uiEndOfStreamCount;
  // Check if we have received the same number of EndOfStream notifications
  // as there are input pins.
  if (m_uiEndOfStreamCount == 2)
  {
    m_vOutputPins[0]->DeliverEndOfStream();
  }
  return S_OK;
}

STDMETHODIMP VideoMixingBase::BeginFlush( int nIndex )
{
  ++m_uiBeginFlushCount;
  if (m_uiBeginFlushCount == 2)
  {
    m_vOutputPins[0]->DeliverBeginFlush();
  }
  return S_OK;
}

STDMETHODIMP VideoMixingBase::EndFlush( int nIndex )
{
  ++m_uiEndFlushCount;
  if (m_uiEndFlushCount == 2)
  {
    m_vOutputPins[0]->DeliverEndFlush();
  }
  return S_OK;
}

HRESULT VideoMixingBase::StartStreaming()
{
  // reset counters
  m_uiEndOfStreamCount = 0;
  m_uiBeginFlushCount = 0;
  m_uiEndFlushCount = 0;
  return NO_ERROR;
}

HRESULT VideoMixingBase::StopStreaming()
{
  return NO_ERROR;
}

