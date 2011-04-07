/** @file

MODULE				: AudioMixingFilter

FILE NAME			: AudioMixingFilter.cpp

DESCRIPTION			: 
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2010, CSIR
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

#include <numeric>
#include <algorithm>

// CSIR includes
#include "AudioMixingFilter.h"

#include <DirectShow/CommonDefs.h>

AudioMixingFilter::AudioMixingFilter()
  :CMultiIOBaseFilter("CSIR VPP Audio Mixer", 0, CLSID_AudioMixingFilter),
  //m_pSample1(NULL),
  //m_pSample2(NULL),
  m_uiBitsPerSample(0),
  m_uiSamplesPerSecond(0),
  m_uiChannels(0),
  m_uiEndOfStreamCount(0),
  m_uiBeginFlushCount(0),
  m_uiEndFlushCount(0)
{
	m_pSampleBuffers[0] = NULL;
	m_pSampleBuffers[1] = NULL;

	m_nSampleSizes[0] = 0;
	m_nSampleSizes[1] = 0;

	// Init parameters
	initParameters();

  Initialise();

  // Create Live Media Event loop handle - this will be used to notify the main thread that the live Media RTSP thread has finished
  m_hSyncEvent = CreateEvent(
    NULL,                       // default security attributes
    FALSE,                      // auto-reset event
    FALSE,                      // initial state is nonsignaled
    TEXT("SyncEvent")  // object name
    );
}

AudioMixingFilter::~AudioMixingFilter()
{
	for (int i = 0; i < 2; i++)
	{
		if (m_pSampleBuffers[i])
		{
			delete[] m_pSampleBuffers[i];
			m_pSampleBuffers[i] = NULL;
		}
	}
}

CUnknown * WINAPI AudioMixingFilter::CreateInstance( LPUNKNOWN pUnk, HRESULT *pHr )
{
	AudioMixingFilter* pFilter = new AudioMixingFilter();
	return pFilter;
}

void AudioMixingFilter::initParameters()
{
}

HRESULT AudioMixingFilter::GenerateOutputSample(IMediaSample *pSample, int nIndex)
{
	// Prepare output sample
	AM_SAMPLE2_PROPERTIES * const pProps = m_vInputPins[nIndex]->SampleProps();
	DbgLog((LOG_TRACE,0,TEXT("Audio Mixer: Sample Received from input 0: start time: %I64d end time: %I64d"), pProps->tStart, pProps->tStop));
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

  // TODO: do transform!

  // TODO: set size: use same size as input
	pOutSample->SetActualDataLength( 0 );
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

HRESULT AudioMixingFilter::CheckOutputType( const CMediaType* pMediaType )
{
	if (*(pMediaType->Subtype()) == MEDIASUBTYPE_PCM)
	{
		return S_OK;
	}
	return S_FALSE;
}

HRESULT AudioMixingFilter::Receive( IMediaSample *pSample, int nIndex )
{
  CAutoLock lck(&m_csReceive);
  ASSERT(nIndex >= 0);
  ASSERT (nIndex < m_vInputPins.size());
  ASSERT(pSample);
  ASSERT (m_vOutputPins[0] != NULL);

  if (nIndex == 0)
  {
    pSample->AddRef();
    m_qSamples1.push_back(pSample);
  }
  else
  {
    pSample->AddRef();
    m_qSamples2.push_back(pSample);
  }

  if (!m_qSamples1.empty() && !m_qSamples2.empty())
  {
    IMediaSample* pSample1 = m_qSamples1.front();
    m_qSamples1.pop_front();
    BYTE *pBuffer1 = NULL;
    HRESULT hr = pSample1->GetPointer(&pBuffer1);
    if (FAILED(hr))
    {
      pSample1->Release();
      return hr;
    }

    IMediaSample* pSample2 = m_qSamples2.front();
    m_qSamples2.pop_front();
    BYTE *pBuffer2 = NULL;
    hr = pSample2->GetPointer(&pBuffer2);
    if (FAILED(hr))
    {
      pSample2->Release();
      return hr;
    }

    IMediaSample * pOutSample;
    // Set up the output sample
    hr = InitializeOutputSample(pSample1, &pOutSample, 0, 0);

    if (FAILED(hr)) 
    {
      return hr;
    }

    BYTE *pBufferOut = NULL;
    hr = pOutSample->GetPointer(&pBufferOut);
    if (FAILED(hr))
    {
      return hr;
    }

    // adding both samples
    unsigned uiLen = 0;
    unsigned uiRemainder = 0;
    unsigned uiLen1 = pSample1->GetActualDataLength();
    unsigned uiLen2 = pSample2->GetActualDataLength();
    
    // This will only be the case if the one track has finished already
    if (uiLen1 != uiLen2)
    {
      uiLen = min(uiLen1, uiLen2);
      uiRemainder = std::abs((int)uiLen2 - (int)uiLen1);
    }
    else
    {
      uiLen = uiLen1;
    }

    if (m_uiBitsPerSample == 8)
    {
      for (size_t i = 0; i < uiLen; ++i)
      {
        // try dividing by 2?
        pBufferOut[i] = (pBuffer1[i] >> 1) + (pBuffer2[i] >> 1);
      }
      if (uiRemainder > 0)
      {
        // copy the remaining ones straight from the buffer
        if (uiLen1 > uiLen2)
          memcpy(pBufferOut + uiLen, pBuffer1 + uiLen, uiRemainder);
        else
          memcpy(pBufferOut + uiLen, pBuffer2 + uiLen, uiRemainder);
      }
    }
    else if (m_uiBitsPerSample == 16)
    {
      short* pIn1 = reinterpret_cast<short*>(pBuffer1);
      short* pIn2 = reinterpret_cast<short*>(pBuffer2);
      short* pOut = reinterpret_cast<short*>(pBufferOut);
      unsigned uiTotalLen = uiLen >> 1;
      for (size_t i = 0; i < uiTotalLen; ++i)
      {
        // try dividing by 2?
        pOut[i] = (pIn1[i] >> 1) + (pIn2[i] >> 1);
      }
      if (uiRemainder > 0)
      {
        // copy the remaining ones straight from the buffer
        if (uiLen1 > uiLen2)
          memcpy(pOut + uiTotalLen, pIn1 + uiTotalLen, uiRemainder >> 1);
        else
          memcpy(pOut + uiTotalLen, pIn2 + uiTotalLen, uiRemainder >> 1);
      }
    }
    else
    {
      ASSERT(false);
    }

    // TODO: set size: use same size as input
    pOutSample->SetActualDataLength( uiLen );
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

          pSample1->Release();
          pSample2->Release();
          
          return NOERROR;
        }
      }
    }
    // release the output buffer. If the connected pin still needs it,
    // it will have addrefed it itself.
    pOutSample->Release();

    pSample1->Release();
    pSample2->Release();
    
    return hr;
  }
  else
  {
    // one stream has stopped already so just play out the remaining other stream
    if (m_uiEndOfStreamCount > 0)
    {
      IMediaSample* pSample(NULL);
      if (!m_qSamples1.empty())
      {
        pSample = m_qSamples1.front();
        m_qSamples1.pop_front();
      }
      else if (!m_qSamples2.empty())
      {
        pSample = m_qSamples2.front();
        m_qSamples2.pop_front();
      }

      BYTE *pBuffer = NULL;
      HRESULT hr = pSample->GetPointer(&pBuffer);
      if (FAILED(hr))
      {
        pSample->Release();
        return hr;
      }

      IMediaSample * pOutSample(NULL);
      // Set up the output sample
      hr = InitializeOutputSample(pSample, &pOutSample, 0, 0);

      if (FAILED(hr)) 
      {
        return hr;
      }

      BYTE *pBufferOut = NULL;
      hr = pOutSample->GetPointer(&pBufferOut);
      if (FAILED(hr))
      {
        return hr;
      }

      memcpy(pBufferOut, pBuffer, pSample->GetActualDataLength());

      // TODO: set size: use same size as input
      pOutSample->SetActualDataLength( pSample->GetActualDataLength() );
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

            pSample->Release();
            return NOERROR;
          }
        }
      }
      // release the output buffer. If the connected pin still needs it,
      // it will have addrefed it itself.
      pOutSample->Release();
      pSample->Release();
      return hr;
    }
  }
  return S_OK;
}

HRESULT AudioMixingFilter::DecideBufferSize( IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pRequestProperties, int m_nIndex )
{
  // Get media type of the output pin
  AM_MEDIA_TYPE mt;
  HRESULT hr = m_vOutputPins[m_nIndex]->ConnectionMediaType(&mt);
  if (FAILED(hr))
  {
    return hr;
  } 

  WAVEFORMATEX *pWavHeader = (WAVEFORMATEX*)mt.pbFormat;
  pRequestProperties->cbBuffer = pWavHeader->nSamplesPerSec * pWavHeader->nChannels * (pWavHeader->wBitsPerSample >> 3);

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

HRESULT AudioMixingFilter::GetMediaType( int iPosition, CMediaType* pmt, int nOutputPinIndex )
{
  if (iPosition < 0)
  {
    return E_INVALIDARG;
  }
  else if (iPosition == 0)
  {
    // To keep things simple: we assume that both inputs will have the same type for now
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

    HRESULT hr = pConnectedPin->ConnectionMediaType(pmt);
    if (FAILED(hr))
    {
      return hr;
    }
    return S_OK;
  }
  return VFW_S_NO_MORE_ITEMS;
}

HRESULT AudioMixingFilter::SetMediaType( PIN_DIRECTION direction, const CMediaType *pmt, int nIndex )
{
  // Check if our base class accepts the connection
  HRESULT hr = CMultiIOBaseFilter::SetMediaType(direction, pmt, nIndex);
  if (FAILED(hr))
  {
    return hr;
  }

  if (direction == PINDIR_INPUT)
  {
    // Reject connection if output is already connected
    if (m_vOutputPins[0]->IsConnected())
    {
      return E_FAIL;
    }

    WAVEFORMATEX* pWavHeader = (WAVEFORMATEX*)pmt->pbFormat;
    
    if (m_uiBitsPerSample == 0 && m_uiSamplesPerSecond == 0 && m_uiChannels == 0)
    {
      m_uiBitsPerSample = pWavHeader->wBitsPerSample;
      m_uiSamplesPerSecond = pWavHeader->nSamplesPerSec;
      m_uiChannels = pWavHeader->nChannels;
    }
    else
    {
      // TODO: check if input types match and reject if not
    }
  }
  return S_OK;
}

STDMETHODIMP AudioMixingFilter::Stop()
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

  CAutoLock lck(&m_csReceive);
  // Iterate over both queues and release all samples
  for (MediaSampleQueue_t::iterator it = m_qSamples1.begin(); it != m_qSamples1.end(); ++it)
  {
    (*it)->Release();
  }
  m_qSamples1.clear();

  for (MediaSampleQueue_t::iterator it = m_qSamples2.begin(); it != m_qSamples2.end(); ++it)
  {
    (*it)->Release();
  }
  m_qSamples2.clear();
  
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

STDMETHODIMP AudioMixingFilter::Pause()
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

HRESULT AudioMixingFilter::StartStreaming()
{
  // reset counters
  m_uiEndOfStreamCount = 0;
  m_uiBeginFlushCount = 0;
  m_uiEndFlushCount = 0;
  return NO_ERROR;
}

HRESULT AudioMixingFilter::StopStreaming()
{
  return NO_ERROR;
}

STDMETHODIMP AudioMixingFilter::EndOfStream( int nIndex )
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

STDMETHODIMP AudioMixingFilter::BeginFlush( int nIndex )
{
  ++m_uiBeginFlushCount;
  if (m_uiBeginFlushCount == 2)
  {
    m_vOutputPins[0]->DeliverBeginFlush();
  }
  return S_OK;
}

STDMETHODIMP AudioMixingFilter::EndFlush( int nIndex )
{
  ++m_uiEndFlushCount;
  if (m_uiEndFlushCount == 2)
  {
    m_vOutputPins[0]->DeliverEndFlush();
  }
  return S_OK;
}
