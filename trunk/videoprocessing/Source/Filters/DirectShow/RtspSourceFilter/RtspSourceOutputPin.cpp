/** @file

MODULE				: RtspSourceOutputPin

FILE NAME			: RtspSourceOutputPin.cpp

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
#include "RtspSourceOutputPin.h"
#include "RtspSourceFilter.h"

// DS // For MP3 header
#include <mmreg.h>

// STL
#include <iostream>

// LiveMedia
#include <liveMedia.hh>

// RTVC
#include <Shared/MediaSample.h>

// {726D6173-0000-0010-8000-00AA00389B71}
DEFINE_GUID(MEDIASUBTYPE_AMR, 
			0x726d6173, 0x000, 0x0010, 0x80, 0x00, 0x0, 0xaa, 0x00, 0x38, 0x9b, 0x71);

RtspSourceOutputPin::RtspSourceOutputPin( HRESULT* phr, RtspSourceFilter* pFilter, MediaSubsession* pMediaSubsession, int nID )
: CSourceStream(NAME("Audio Source Filter Output Pin"), phr, pFilter, L"Out"),
	m_pFilter(pFilter), // Parent filter
	m_pMediaType(NULL), //
	m_nBitsPerSample(0),
	m_nChannels(0),
	m_nBytesPerSecond(0),
	m_nSamplesPerSecond(0),
	m_nID(nID),
	m_bOffsetSet(false),
	m_bHasBeenSynced(false),
	m_dGlobalStartTime(-1),
	m_dLocalStartTime(-1)
{
	// Make sure the CSourceStream constructor was successful
	if (!SUCCEEDED(*phr))
	{
		return;
	}

	initialiseMediaType(pMediaSubsession, phr);
}

RtspSourceOutputPin::~RtspSourceOutputPin(void)
{
	// Free media type
	if (m_pMediaType)
		FreeMediaType(*m_pMediaType);
}

void RtspSourceOutputPin::Reset()
{
	m_bOffsetSet = false;
	m_bHasBeenSynced = false;
	m_dGlobalStartTime = -1;
	m_dLocalStartTime = -1;
}

void RtspSourceOutputPin::initialiseMediaType(MediaSubsession* pSubsession, HRESULT* phr)
{
	// Now iterate over all media subsessions and create our RTVC media attributes
	// Get medium name
	const char* szMedium = pSubsession->mediumName();
	const char* szCodec = pSubsession->codecName();

	if (pSubsession)
	{
		// Audio
		if (strcmp(szMedium, "audio")==0)
		{
			m_pMediaType = new CMediaType();

			// Setup media type for 8 bit PCM
			if (strcmp(szCodec, "L8")==0)
			{
				m_pMediaType->SetType(&MEDIATYPE_Audio);

				// For PCM
				m_pMediaType->SetFormatType(&FORMAT_WaveFormatEx);
				m_pMediaType->SetSubtype(&MEDIASUBTYPE_PCM);
				// 8 bit audio
				m_nBitsPerSample = 8;
				// Get sample rate
				m_nSamplesPerSecond = pSubsession->rtpTimestampFrequency();

				// Get channels
				m_nChannels = pSubsession->numChannels();

				m_nBytesPerSecond = m_nSamplesPerSecond * (m_nBitsPerSample >> 3) * m_nChannels;

				WAVEFORMATEX *pWavHeader = (WAVEFORMATEX*) m_pMediaType->AllocFormatBuffer(sizeof(WAVEFORMATEX));
				if (pWavHeader == 0)
					*phr = (E_OUTOFMEMORY);

				// Initialize the video info header
				ZeroMemory(pWavHeader, m_mt.cbFormat);   

				pWavHeader->wFormatTag = WAVE_FORMAT_PCM;
				pWavHeader->nChannels = m_nChannels;
				pWavHeader->nSamplesPerSec = m_nSamplesPerSecond;
				pWavHeader->wBitsPerSample = m_nBitsPerSample;

				// From MSDN: http://msdn.microsoft.com/en-us/library/ms713497(VS.85).aspx
				// Block alignment, in bytes. The block alignment is the minimum atomic unit of data for the wFormatTag format type. If wFormatTag is WAVE_FORMAT_PCM or WAVE_FORMAT_EXTENSIBLE, nBlockAlign must be equal to the product of nChannels and wBitsPerSample divided by 8 (bits per byte).
				//OLD pWavHeader->nBlockAlign = 1;
				pWavHeader->nBlockAlign = m_nChannels * (m_nBitsPerSample >> 3);

				// From MSDN: Required average data-transfer rate, in bytes per second, for the format tag. If wFormatTag is WAVE_FORMAT_PCM, nAvgBytesPerSec should be equal to the product of nSamplesPerSec and nBlockAlign. For non-PCM formats, this member must be computed according to the manufacturer's specification of the format tag. 
				// OLD pWavHeader->nAvgBytesPerSec = m_nChannels*m_nSamplesPerSecond;
				pWavHeader->nAvgBytesPerSec =  m_nSamplesPerSecond * pWavHeader->nBlockAlign;

				pWavHeader->cbSize = 0;

				m_pMediaType->SetTemporalCompression(FALSE);
				// From using graph studio to look at the pins media types
				//m_pMediaType->SetSampleSize(1);
				m_pMediaType->SetSampleSize(4);
			}
			// Setup media type for 16 bit PCM
			else if (strcmp(szCodec, "L16")==0)
			{
				m_pMediaType->SetType(&MEDIATYPE_Audio);
				// For PCM
				m_pMediaType->SetFormatType(&FORMAT_WaveFormatEx);
				m_pMediaType->SetSubtype(&MEDIASUBTYPE_PCM);
				// 16-bit audio
				m_nBitsPerSample = 16;
				// Get sample rate
				m_nSamplesPerSecond = pSubsession->rtpTimestampFrequency();

				// Get channels
				m_nChannels = pSubsession->numChannels();

				m_nBytesPerSecond = m_nSamplesPerSecond * (m_nBitsPerSample >> 3) * m_nChannels;

				WAVEFORMATEX *pWavHeader = (WAVEFORMATEX*) m_pMediaType->AllocFormatBuffer(sizeof(WAVEFORMATEX));
				if (pWavHeader == 0)
					*phr = (E_OUTOFMEMORY);

				// Initialize the video info header
				ZeroMemory(pWavHeader, m_mt.cbFormat);   

				pWavHeader->wFormatTag = WAVE_FORMAT_PCM;
				pWavHeader->nChannels = m_nChannels;
				pWavHeader->nSamplesPerSec = m_nSamplesPerSecond;
				pWavHeader->wBitsPerSample = m_nBitsPerSample;

				// From MSDN: http://msdn.microsoft.com/en-us/library/ms713497(VS.85).aspx
				// Block alignment, in bytes. The block alignment is the minimum atomic unit of data for the wFormatTag format type. If wFormatTag is WAVE_FORMAT_PCM or WAVE_FORMAT_EXTENSIBLE, nBlockAlign must be equal to the product of nChannels and wBitsPerSample divided by 8 (bits per byte).
				//OLD pWavHeader->nBlockAlign = 1;
				pWavHeader->nBlockAlign = m_nChannels * (m_nBitsPerSample >> 3);

				// From MSDN: Required average data-transfer rate, in bytes per second, for the format tag. If wFormatTag is WAVE_FORMAT_PCM, nAvgBytesPerSec should be equal to the product of nSamplesPerSec and nBlockAlign. For non-PCM formats, this member must be computed according to the manufacturer's specification of the format tag. 
				// OLD pWavHeader->nAvgBytesPerSec = m_nChannels*m_nSamplesPerSecond;
				pWavHeader->nAvgBytesPerSec =  m_nSamplesPerSecond * pWavHeader->nBlockAlign;

				pWavHeader->cbSize = 0;

				m_pMediaType->SetTemporalCompression(FALSE);
				// From using graph studio to look at the pins media types
				//m_pMediaType->SetSampleSize(1);
				m_pMediaType->SetSampleSize(4);
			}

			else if (strcmp(szCodec, "AMR")==0)
			{
				//m_pMediaType->SetType(&MEDIATYPE_Stream);
				//m_pMediaType->SetFormatType(&GUID_NULL);
				//m_pMediaType->SetSubtype(&GUID_NULL);
				
				m_pMediaType->SetType(&MEDIATYPE_Audio);
				// For PCM
				m_pMediaType->SetFormatType(&FORMAT_WaveFormatEx);
				m_pMediaType->SetSubtype(&MEDIASUBTYPE_AMR);
				// 16-bit audio
				m_nBitsPerSample = 0;
				// Get sample rate
				m_nSamplesPerSecond = pSubsession->rtpTimestampFrequency();

				// Get channels
				m_nChannels = pSubsession->numChannels();

				m_nBytesPerSecond = m_nSamplesPerSecond * (m_nBitsPerSample >> 3) * m_nChannels;

				WAVEFORMATEX *pWavHeader = (WAVEFORMATEX*) m_pMediaType->AllocFormatBuffer(sizeof(WAVEFORMATEX));
				if (pWavHeader == 0)
					*phr = (E_OUTOFMEMORY);

				// Initialize the video info header
				ZeroMemory(pWavHeader, m_mt.cbFormat);   

				pWavHeader->wFormatTag = 0;
				pWavHeader->nChannels = m_nChannels;
				pWavHeader->nSamplesPerSec = m_nSamplesPerSecond;
				pWavHeader->wBitsPerSample = m_nBitsPerSample;

				// From MSDN: http://msdn.microsoft.com/en-us/library/ms713497(VS.85).aspx
				// Block alignment, in bytes. The block alignment is the minimum atomic unit of data for the wFormatTag format type. If wFormatTag is WAVE_FORMAT_PCM or WAVE_FORMAT_EXTENSIBLE, nBlockAlign must be equal to the product of nChannels and wBitsPerSample divided by 8 (bits per byte).
				//OLD pWavHeader->nBlockAlign = 1;
				pWavHeader->nBlockAlign = m_nChannels * (m_nBitsPerSample >> 3);
				pWavHeader->nBlockAlign = 1;

				// From MSDN: Required average data-transfer rate, in bytes per second, for the format tag. If wFormatTag is WAVE_FORMAT_PCM, nAvgBytesPerSec should be equal to the product of nSamplesPerSec and nBlockAlign. For non-PCM formats, this member must be computed according to the manufacturer's specification of the format tag. 
				// OLD pWavHeader->nAvgBytesPerSec = m_nChannels*m_nSamplesPerSecond;
				pWavHeader->nAvgBytesPerSec =  m_nSamplesPerSecond * pWavHeader->nBlockAlign;

				pWavHeader->cbSize = 0;

				m_pMediaType->SetTemporalCompression(FALSE);
				// From using graph studio to look at the pins media types
				//m_pMediaType->SetSampleSize(1);
				m_pMediaType->SetSampleSize(1024);
			}
			// This section caters for MP3 audio but does NOT work yet
			else if (strcmp(szCodec, "MPA")==0)
			{
				// MP3-bit audio
				//MSDN
				m_pMediaType->SetType(&MEDIATYPE_Audio);
				// For MP3
				m_pMediaType->SetFormatType(&FORMAT_WaveFormatEx);
				m_pMediaType->subtype = FOURCCMap(WAVE_FORMAT_MPEGLAYER3);

				m_nSamplesPerSecond = pSubsession->rtpTimestampFrequency();
				// Get channels
				m_nChannels = pSubsession->numChannels();
				
				MPEGLAYER3WAVEFORMAT mp3WaveFormat;
				ZeroMemory(&mp3WaveFormat, sizeof(MPEGLAYER3WAVEFORMAT));
				mp3WaveFormat.wID = MPEGLAYER3_ID_MPEG;
				mp3WaveFormat.fdwFlags =  MPEGLAYER3_FLAG_PADDING_ISO;
				mp3WaveFormat.nFramesPerBlock = 1;
				mp3WaveFormat.nBlockSize = 1;
				mp3WaveFormat.nCodecDelay = 0;

				WAVEFORMATEX* waveFormat = &mp3WaveFormat.wfx;
				waveFormat->cbSize = MPEGLAYER3_WFX_EXTRA_BYTES;
				waveFormat->nAvgBytesPerSec = 24000;
				waveFormat->nAvgBytesPerSec = 0;
				waveFormat->nBlockAlign = 0;
				waveFormat->nChannels = 2;
				waveFormat->nSamplesPerSec = 44100;
				//waveFormat->nSamplesPerSec = 0;
				waveFormat->wBitsPerSample = 0;
				waveFormat->wFormatTag = 85;

				// Apply format
				m_pMediaType->SetFormat((BYTE*)&mp3WaveFormat, sizeof(MPEGLAYER3WAVEFORMAT));
				m_pMediaType->bFixedSizeSamples = TRUE;
				m_pMediaType->bTemporalCompression = FALSE;


				//pmt->subtype = FOURCCMap(WAVE_FORMAT_MPEGLAYER3);
				//MPEGLAYER3WAVEFORMAT mp3WaveFormat;
				//ZeroMemory(&mp3WaveFormat, sizeof(MPEGLAYER3WAVEFORMAT));
				//mp3WaveFormat.wID = MPEGLAYER3_ID_MPEG;
				//mp3WaveFormat.fdwFlags =  MPEGLAYER3_FLAG_PADDING_OFF;
				//mp3WaveFormat.nFramesPerBlock = 1;
				//mp3WaveFormat.nBlockSize = 417;
				//mp3WaveFormat.nCodecDelay = 1393;

				//WAVEFORMATEX* waveFormat = &mp3WaveFormat.wfx;
				//waveFormat->cbSize = MPEGLAYER3_WFX_EXTRA_BYTES;
				//waveFormat->nAvgBytesPerSec = 16000;
				//waveFormat->nBlockAlign = 1;
				//waveFormat->nChannels = 2;
				//waveFormat->nSamplesPerSec = 22050;
				//waveFormat->wBitsPerSample = 0;
				//waveFormat->wFormatTag = 85;

				//// Apply format
				//pmt->SetFormatType(&FORMAT_WaveFormatEx);
				//pmt->SetFormat((BYTE*)&mp3WaveFormat, sizeof(MPEGLAYER3WAVEFORMAT));

				//pmt->bFixedSizeSamples = FALSE;
				//pmt->bTemporalCompression = FALSE;
			}

			else
			{
				// Unsupported
				//m_sLastError = "Unsupported audio codec: " + std::string(szMedium) + std::string(szCodec);
				*phr = VFW_E_INVALIDSUBTYPE;
			}	
		}
		else
		{
			// TODO: cater for video
			*phr = VFW_E_INVALIDMEDIATYPE;
		}
	}
	else
	{
		// Invalid pointer
		*phr = E_POINTER;
	}
}

HRESULT RtspSourceOutputPin::GetMediaType( CMediaType* pMediaType )
{
	CAutoLock cAutoLock(m_pFilter->pStateLock());
	CheckPointer(pMediaType, E_POINTER);
	if (!m_pMediaType)
	{
		return E_UNEXPECTED;
	}
	// First try to free the format block just in case it has already been allocated according to MSDN doc
	FreeMediaType(*pMediaType);

	// Copy media type
	HRESULT hr = CopyMediaType(pMediaType, m_pMediaType);
	if (FAILED(hr))
	{
		return E_UNEXPECTED;
	}
	else
	{
		return S_OK;
	}
}

HRESULT RtspSourceOutputPin::DecideBufferSize( IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pRequest )
{
	HRESULT hr;
	CAutoLock cAutoLock(m_pFilter->pStateLock());

	CheckPointer(pAlloc, E_POINTER);
	CheckPointer(pRequest, E_POINTER);

	// Ensure a minimum number of buffers
	if (pRequest->cBuffers == 0)
	{
		pRequest->cBuffers = 2;
	}

	if (m_pMediaType->formattype == FORMAT_VideoInfo)
	{
		VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)m_pMediaType->pbFormat;
		// Now get size of video
		pRequest->cbBuffer = pVih->bmiHeader.biSizeImage;
	}
	else if (m_pMediaType->formattype == FORMAT_WaveFormatEx)
	{
		// Audio buffer size
		//Buffer size calculation for PCM
		if (m_nBitsPerSample > 0)
			pRequest->cbBuffer = m_nSamplesPerSecond * m_nChannels * (m_nBitsPerSample >> 3);
		else
		{
			pRequest->cbBuffer		= 4 * 1024;
			pRequest->cBuffers		= 3;
			pRequest->cbAlign		= 2;		
			pRequest->cbPrefix		= 0;
		}
		// Not sure what buffer size to use for MP3
		//pRequest->cbBuffer = 200000;

	}

	ALLOCATOR_PROPERTIES Actual;
	hr = pAlloc->SetProperties(pRequest, &Actual);
	if (FAILED(hr)) 
	{
		return hr;
	}

	// Is this allocator unsuitable?
	if (Actual.cbBuffer < pRequest->cbBuffer) 
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT RtspSourceOutputPin::FillBuffer( IMediaSample* pSample )
{
	BYTE *pData;
	long cbData;

	CheckPointer(pSample, E_POINTER);
	CAutoLock cAutoLockShared(&m_cSharedState);

	// Access the sample's data buffer
	pSample->GetPointer(&pData);

	MediaSample* pSampleData = m_pFilter->m_rtpPacketManager.getNextSample(m_nID);
	if (!pSampleData) return S_FALSE;

	// Set the output pin's local start time: i.e. the time of the first sample that this pin has received
	if ( m_dLocalStartTime == -1)
	{
		m_dLocalStartTime = pSampleData->StartTime();
	}

	// Set the global start time: this occurs when the first sample is received for a pin
	// This sets the global base starting time before RTCP sync kicks in
	if (!m_bOffsetSet)
	{
		m_pFilter->notifyFilterAboutOffset(pSampleData->StartTime());
	}
	else
	{
		// If this is the first sample time has already been synchronised using RTCP, we update the base time globally
		if (pSampleData->isSynchronisationPoint())
		{
			m_bHasBeenSynced = true;
			// Notify the filter about the "new" offset since the base starting time usually jumps per media stream once RTCP synchronisation kicks in
			m_pFilter->notifyFilterAboutOffset(pSampleData->StartTime());
		}
	}

	// Get buffer size
	cbData = pSampleData->getSize();

	// Copy the data from the sample into the buffer
	if (m_nBitsPerSample == 16)
	{
		// 16 bit audio is always deliver in network byte order by the liveMedia library, we need to swap the byte order first
		// before delivering the media downstream
		
		// Swap the byte order of the 16-bit values that we have just read:
		unsigned numValues = cbData >> 1;
		
		short* pDest = (short*)pData;
		short* pSrc = (short*)pSampleData->getData();

		for (unsigned i = 0; i < numValues; ++i) 
		{
			short const orig = pSrc[i];
			pDest[i] = ((orig&0xFF)<<8) | ((orig&0xFF00)>>8);
		}
	}
	else
	{
		// 8 bit PCM audio: simply copy the media buffer
		if (pSampleData)
		{
			memcpy(pData, pSampleData->getData(), (DWORD) cbData);
		}
		else
		{
			// Error!!!! What to do?
			return S_FALSE;
		}
	}

	// Calculate adjusted times: 
	// 1)	Subtract the first timestamp to make our time zero-based (the time on the RTSP server is gettimeofday-based) 
	//		And the DirectShow time is zero-based. Pick the first timestamp according to the following criteria:
	//		If this pin has been offset but is unsynced: use the local offset
	//		Once this pin has been syned:				 use the global offset
	//
	// 2) Add the last stream offset time (Since sync occurs after DS time zero, we need to remember when sync last occurred so that we can offset the time by at least that amount into the future
	//    If this is not done, the samples would arrive at the renderer late 

	double dOffset = (m_bHasBeenSynced) ? m_dGlobalStartTime : m_dLocalStartTime;
	double dAdjustedTime = pSampleData->StartTime() - dOffset + m_pFilter->m_dStreamTimeOffset;
	REFERENCE_TIME rtStart = dAdjustedTime * 1000000 * 10;

	// Set stop timestamp to be slightly in the future
	// The commented out NULL time stamp for the stop time seemed to work the same
	REFERENCE_TIME rtStop = rtStart + 1;
	pSample->SetTime(&rtStart, &rtStop);

	// USE THIS IN ORDER TO GET THE RENDERER TO RENDER SAMPLES IMMEDIATELY AS IT RECEIVES THEM
//#define RTVC_DEBUG_SET_NULL_TIMESTAMPS
#ifdef RTVC_DEBUG_SET_NULL_TIMESTAMPS
	pSample->SetTime(NULL, NULL);
#endif

	if (m_pMediaType->subtype == MEDIASUBTYPE_PCM)
	{
		// All samples are sync points
		pSample->SetSyncPoint(TRUE);
	}

	// Set length of media sample
	pSample->SetActualDataLength(cbData);

	// Free memory used by sample
	delete pSampleData;

	return S_OK;
}

HRESULT RtspSourceOutputPin::DoBufferProcessingLoop( void )
{
	Command com;

	OnThreadStartPlay();

	m_bOffsetSet = false;
	do {
		while (!CheckRequest(&com)) 
		{
			if (m_pFilter->m_rtpPacketManager.hasSamples(m_nID))
			{
				// Get buffer
				IMediaSample *pSample = NULL;
				HRESULT hr = GetDeliveryBuffer(&pSample,NULL,NULL,0);
				if (FAILED(hr)) {
					Sleep(1);
					continue;	// go round again. Perhaps the error will go away
					// or the allocator is decommited & we will be asked to
					// exit soon.
				}

				// Virtual function user will override.
				hr = FillBuffer(pSample);

				if (hr == S_OK) {
					hr = Deliver(pSample);
					pSample->Release();

					// downstream filter returns S_FALSE if it wants us to
					// stop or an error if it's reporting an error.
					if(hr != S_OK)
					{
						DbgLog((LOG_TRACE, 2, TEXT("Deliver() returned %08x; stopping"), hr));
						//return S_OK;
					}

				} else if (hr == S_FALSE) {
					// derived class wants us to stop pushing data
					pSample->Release();

					// Should I use this to signal when there is no data
					continue;
				} else {
					// derived class encountered an error
					pSample->Release();
					DbgLog((LOG_ERROR, 1, TEXT("Error %08lX from FillBuffer!!!"), hr));
					DeliverEndOfStream();
					m_pFilter->NotifyEvent(EC_ERRORABORT, hr, 0);
					return hr;
				}
			}
			else
			{
				Sleep(10);
			}
		}

		// For all commands sent to us there must be a Reply call!
		if (com == CMD_RUN || com == CMD_PAUSE) 
		{
			Reply(NOERROR);
		} else if (com != CMD_STOP) 
		{
			Reply((DWORD) E_UNEXPECTED);
			DbgLog((LOG_ERROR, 1, TEXT("Unexpected command!!!")));
		}
	} while (com != CMD_STOP);

	return S_FALSE;
}

void RtspSourceOutputPin::setOffset( double dOffset)
{
	m_dGlobalStartTime = dOffset;
	m_bOffsetSet = true;
}
