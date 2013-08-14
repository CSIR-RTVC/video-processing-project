/** @file

MODULE				: RtspSourceOutputPin

FILE NAME			: RtspSourceOutputPin.cpp

DESCRIPTION			: DirectShow RTSP output pin
					  
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
#include "RtspSourceOutputPin.h"
#include "RtspSourceFilter.h"

// DS // For MP3 header
#include <mmreg.h>
#include <dvdmedia.h>
#include <wmcodecdsp.h>

// STL
#include <iostream>

// RTVC
#include <Shared/MediaSample.h>

#include <MPEG4LATMAudioRTPSource.hh>

// {726D6173-0000-0010-8000-00AA00389B71}
DEFINE_GUID(MEDIASUBTYPE_AMR, 
			0x726d6173, 0x000, 0x0010, 0x80, 0x00, 0x0, 0xaa, 0x00, 0x38, 0x9b, 0x71);

// Taken from Monogram
// {000001FF-0000-0010-8000-00AA00389B71}
DEFINE_GUID(MEDIASUBTYPE_LATM_AAC,
      0x000001ff, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

RtspSourceOutputPin::RtspSourceOutputPin( HRESULT* phr, RtspSourceFilter* pFilter, MediaSubsession* pMediaSubsession, const StringMap_t& rParams, int nID )
: CSourceStream(NAME("Audio Source Filter Output Pin"), phr, pFilter, L"Out"),
	m_pFilter(pFilter), // Parent filter
	m_pMediaType(NULL), //
	m_nBitsPerSample(0),
	m_nChannels(0),
	m_nBytesPerSecond(0),
	m_nSamplesPerSecond(0),
	m_nID(nID),
  m_bFirstSample(false),
  m_bFirstSyncedSample(false),
  m_dStreamTimeOffset(0.0)
{
	// Make sure the CSourceStream constructor was successful
	if (!SUCCEEDED(*phr))
	{
		return;
	}

	initialiseMediaType(pMediaSubsession, rParams, phr);
}

RtspSourceOutputPin::~RtspSourceOutputPin(void)
{
	// Free media type
	if (m_pMediaType)
		FreeMediaType(*m_pMediaType);
}

FOURCCMap H264(DWORD('462H'));
void RtspSourceOutputPin::initialiseMediaType( MediaSubsession* pSubsession, const StringMap_t& rParams, HRESULT* phr )
{
  // Now iterate over all media subsessions and create our RTVC media attributes
  // Get medium name
  const char* szMedium = pSubsession->mediumName();
  const char* szCodec = pSubsession->codecName();

  if (pSubsession)
  {
    m_pMediaType = new CMediaType();
    // Audio
    if (strcmp(szMedium, "audio")==0)
    {
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

        // Expecting width and height parameters to have been passed in
        StringMap_t::const_iterator it;
        it = rParams.find(SAMPLING_RATE);
        if(it != rParams.end()) 
        {
          std::string const& sSamplingRate = it->second;
          m_uiSamplingRate = atoi(sSamplingRate.c_str());
        }
        waveFormat->nSamplesPerSec = m_uiSamplingRate;
        //waveFormat->nSamplesPerSec = 44100;
        //waveFormat->nSamplesPerSec = 0;
        waveFormat->wBitsPerSample = 0;
        waveFormat->wFormatTag = 85;

        // Apply format
        m_pMediaType->SetFormat((BYTE*)&mp3WaveFormat, sizeof(MPEGLAYER3WAVEFORMAT));
        m_pMediaType->bFixedSizeSamples = TRUE;
        m_pMediaType->bTemporalCompression = FALSE;
      }
      // AAC audio
      else if ( strcmp(szCodec, "MPEG4-GENERIC")==0 )
      {
        const int SamplingFrequencies[] = 
        {
          96000,
          88200,
          64000,
          48000,
          44100,
          32000,
          24000,
          22050,
          16000,
          12000,
          11025,
          8000,
          7350,
          0,
          0,
          0,
        };

        long m_cDecoderSpecific=2;
        const int WAVE_FORMAT_AAC = 0x00ff;
        // get AAC format string and convert from stringified hex 
        int hexConfig;
        sscanf(pSubsession->fmtp_config(), "%x", &hexConfig);
        unsigned char m_pDecoderSpecific[2];
        unsigned char* pConfig = (unsigned char*)&hexConfig;
        // sscanf inverts order
        m_pDecoderSpecific[0] = pConfig[1];
        m_pDecoderSpecific[1] = pConfig[0];

        m_pMediaType->InitMediaType();
        m_pMediaType->SetType(&MEDIATYPE_Audio);
        FOURCCMap faad(WAVE_FORMAT_AAC);
        m_pMediaType->SetSubtype(&faad);
        m_pMediaType->SetFormatType(&FORMAT_WaveFormatEx);
        unsigned uiSize = sizeof(WAVEFORMATEX) + m_cDecoderSpecific;
        WAVEFORMATEX* pwfx = (WAVEFORMATEX*)m_pMediaType->AllocFormatBuffer(uiSize);
        ZeroMemory(pwfx,  uiSize);
        pwfx->cbSize = WORD(m_cDecoderSpecific);
        CopyMemory((pwfx+1),  m_pDecoderSpecific,  m_cDecoderSpecific);

        // parse decoder-specific info to get rate/channels
        long samplerate = ((m_pDecoderSpecific[0] & 0x7) << 1) + ((m_pDecoderSpecific[1] & 0x80) >> 7);
        pwfx->nSamplesPerSec = SamplingFrequencies[samplerate];
        pwfx->nBlockAlign = 1;
        pwfx->wBitsPerSample = 16;
        pwfx->wFormatTag = WAVE_FORMAT_AAC;
        pwfx->nChannels = (m_pDecoderSpecific[1] & 0x78) >> 3;
      }
      // AAC LATM audio
      else if ( strcmp(szCodec, "MP4A-LATM")==0 )
      {
        const int SamplingFrequencies[] = 
        {
          96000,
          88200,
          64000,
          48000,
          44100,
          32000,
          24000,
          22050,
          16000,
          12000,
          11025,
          8000,
          7350,
          0,
          0,
          0,
        };

        MPEG4LATMAudioRTPSource* pSource = (MPEG4LATMAudioRTPSource*)pSubsession->rtpSource();

        unsigned audioSpecificConfigSize = 0;
        unsigned char* pAudioConfig = parseStreamMuxConfigStr(pSubsession->fmtp_config(),
				       audioSpecificConfigSize);

        m_pMediaType->InitMediaType();
        m_pMediaType->SetType(&MEDIATYPE_Audio);

#if 0
        // DOESN'T SEEM TO WORK
        // ATTEMPT: convert hex string to char representation
        const char* hex_string2 = "40002610";
        const char* hex_string2b = "3FC0";
        int hexConfig2;
        sscanf(hex_string2, "%x", &hexConfig2);
        unsigned char m_pDecoderSpecific2[7];
        unsigned char* pConfig2 = (unsigned char*)&hexConfig2;
        int hexConfig2b;
        sscanf(hex_string2b, "%x", &hexConfig2b);
        unsigned char* pConfig2b = (unsigned char*)&hexConfig2b;
        m_pDecoderSpecific2[0] = pConfig2[3];
        m_pDecoderSpecific2[1] = pConfig2[2];
        m_pDecoderSpecific2[2] = pConfig2[1];
        m_pDecoderSpecific2[3] = pConfig2[0];
        m_pDecoderSpecific2[4] = pConfig2b[1];
        m_pDecoderSpecific2[5] = pConfig2b[0];
        m_pDecoderSpecific2[6] = '\0';

        int m_cDecoderSpecific = 6;
        unsigned uiSize = sizeof(WAVEFORMATEX) + m_cDecoderSpecific;
        WAVEFORMATEX* pwfx = (WAVEFORMATEX*)m_pMediaType->AllocFormatBuffer(uiSize);
        ZeroMemory(pwfx,  uiSize);
        pwfx->cbSize = WORD(m_cDecoderSpecific);
        CopyMemory((pwfx+1),  m_pDecoderSpecific2,  m_cDecoderSpecific);

#else
        // hack for now: testing if we need to set the extra data for monogram aac to work

#if 1
        // Seems to get decoded in GS!
        long m_cDecoderSpecific = audioSpecificConfigSize;
        unsigned uiSize = sizeof(WAVEFORMATEX) + m_cDecoderSpecific;
        WAVEFORMATEX* pwfx = (WAVEFORMATEX*)m_pMediaType->AllocFormatBuffer(uiSize);
        ZeroMemory(pwfx,  uiSize);
        pwfx->cbSize = WORD(m_cDecoderSpecific);
        
        CopyMemory((pwfx+1),  pAudioConfig,  m_cDecoderSpecific);

#else
        // try not adding any decoder specific data
        unsigned uiSize = sizeof(WAVEFORMATEX);
        WAVEFORMATEX* pwfx = (WAVEFORMATEX*)m_pMediaType->AllocFormatBuffer(uiSize);
        ZeroMemory(pwfx,  uiSize);
        pwfx->cbSize = WORD(0);

#endif
#endif
        
        // option 1: raw aac
#define USE_RAW_AAC
#ifdef USE_RAW_AAC
        m_pMediaType->SetSubtype(&MEDIASUBTYPE_RAW_AAC1);
        m_pMediaType->SetFormatType(&FORMAT_WaveFormatEx);
        pwfx->wFormatTag = WAVE_FORMAT_RAW_AAC1;
        // pSource->omitLATMDataLengthField();
#endif

        // option 2: LATM
// #define USE_LATM
#ifdef USE_LATM
        // LATM?
        m_pMediaType->SetSubtype(&MEDIASUBTYPE_LATM_AAC);
        m_pMediaType->SetFormatType(&FORMAT_WaveFormatEx);
        // #define WAVE_FORMAT_LATM_AAC 0x01FF
        pwfx->wFormatTag = 0x01FF;

//        DWORD a = MAKEFOURCC('m','p','4','a');
//        DWORD b = MAKEFOURCC('a','4','p','m');
        // pSource->omitLATMDataLengthField();

#endif

        // OPTION 3: std aac
// #define USE_AAC
#ifdef USE_AAC
        // AAC?
        const int WAVE_FORMAT_AAC = 0x00ff;
        FOURCCMap faad(WAVE_FORMAT_AAC);
        m_pMediaType->SetSubtype(&faad);
        m_pMediaType->SetFormatType(&FORMAT_WaveFormatEx);
        pwfx->wFormatTag = WAVE_FORMAT_AAC;

        pSource->omitLATMDataLengthField();
#endif 

// #define USE_LOAS
#ifdef USE_LOAS
        m_pMediaType->SetSubtype(&MEDIASUBTYPE_MPEG_LOAS);
        m_pMediaType->SetFormatType(&FORMAT_WaveFormatEx);
        pwfx->wFormatTag = 0x1602;

#endif
        pwfx->nBlockAlign = 1;
        pwfx->wBitsPerSample = 16;
        // pwfx->wFormatTag = MAKEFOURCC('m','p','4','a');
        //pwfx->nChannels = (m_pDecoderSpecific[1] & 0x78) >> 3;
        pwfx->nChannels = pSubsession->numChannels();
        // pwfx->nChannels = 1;
        pwfx->nSamplesPerSec = 24000;
        // pwfx->nSamplesPerSec = 
        int t = 0;

        // https://github.com/Kovensky/mplayer-kovensky/blob/master/libmpdemux/demux_rtp_codec.cpp
        // wf->wFormatTag = sh_audio->format = mmioFOURCC('m','p','4','a');
        // For the codec to work correctly, it needs "AudioSpecificConfig"
    // data, which is parsed from the "StreamMuxConfig" string that
    // was present (hopefully) in the SDP description:
    //unsigned codecdata_len;
    //sh_audio->codecdata
    //  = parseStreamMuxConfigStr(subsession->fmtp_config(),
				//codecdata_len);
    //sh_audio->codecdata_len = codecdata_len;
    ////faad doesn't understand LATM's data length field, so omit it
    //((MPEG4LATMAudioRTPSource*)subsession->rtpSource())->omitLATMDataLengthField();
      }
      else
      {
        // Unsupported
        //m_sLastError = "Unsupported audio codec: " + std::string(szMedium) + std::string(szCodec);
        *phr = VFW_E_INVALIDSUBTYPE;
      }	
    }
    else if (strcmp(szMedium, "video")==0)
    {
      if (( strcmp(szCodec, "H263-1998") == 0 )||
        ( strcmp(szCodec, "H263-2000") == 0 ))
      {
        // TODO
        *phr = VFW_E_INVALIDSUBTYPE;
      }
      // INCOMPLETE: H264: hard-coding width and height doesn't seem to affect 
      // correctness. Other resolution video is still rendered correctly?
      // Should still parse parameter sets?
      else if (strcmp(szCodec, "H264") == 0 )
      {
        // TODO
        // test.264
        //const unsigned uiWidth = 720;
        //const unsigned uiHeight = 576;

        // test2.264
        const unsigned uiWidth = 384;
        const unsigned uiHeight = 288;
        m_pMediaType->InitMediaType();
        m_pMediaType->SetType(&MEDIATYPE_Video);
        m_pMediaType->SetSubtype(&MEDIASUBTYPE_H264);
        //MEDIASUBTYPE_h264
        //MEDIASUBTYPE_X264
        //MEDIASUBTYPE_x264
        //MEDIASUBTYPE_AVC1
        //FORMAT_MPEG2Video

        m_pMediaType->SetFormatType(&FORMAT_VideoInfo2);
        VIDEOINFOHEADER2* pvi2 = (VIDEOINFOHEADER2*)m_pMediaType->AllocFormatBuffer(sizeof(VIDEOINFOHEADER2));
        ZeroMemory(pvi2, sizeof(VIDEOINFOHEADER2));
        pvi2->bmiHeader.biBitCount = 24;
        pvi2->bmiHeader.biSize = 40;
        pvi2->bmiHeader.biPlanes = 1;
        pvi2->bmiHeader.biWidth = uiWidth;
        pvi2->bmiHeader.biHeight = uiHeight;
        pvi2->bmiHeader.biSize = uiWidth * uiHeight * 3;
        pvi2->bmiHeader.biSizeImage = DIBSIZE(pvi2->bmiHeader);
        pvi2->bmiHeader.biCompression = DWORD('1cva');
        //pvi2->AvgTimePerFrame = m_tFrame;
        //pvi2->AvgTimePerFrame = 1000000;
        const REFERENCE_TIME FPS_25 = UNITS / 25;
        pvi2->AvgTimePerFrame = FPS_25;
        //SetRect(&pvi2->rcSource, 0, 0, m_cx, m_cy);
        SetRect(&pvi2->rcSource, 0, 0, uiWidth, uiHeight);
        pvi2->rcTarget = pvi2->rcSource;

        pvi2->dwPictAspectRatioX = uiWidth;
        pvi2->dwPictAspectRatioY = uiHeight;
      }
      else
      {
        // Unsupported
        //m_sLastError = "Unsupported video codec: " + std::string(szMedium) + std::string(szCodec);
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
  else if (m_pMediaType->formattype == FORMAT_VideoInfo2)
  {
    VIDEOINFOHEADER2 *pVih = (VIDEOINFOHEADER2*)m_pMediaType->pbFormat;
    // Now get size of video
    pRequest->cbBuffer = pVih->bmiHeader.biSizeImage;
    //pRequest->cbBuffer = pVih->bmiHeader.
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
  CheckPointer(pSample, E_POINTER);
  CAutoLock cAutoLockShared(&m_cSharedState);

  // Get next sample from media queue
  MediaSample* pSampleData = m_pFilter->m_mediaPacketManager.getNextSample(m_nID);
  if (!pSampleData) return S_FALSE;

  copyMediaDataIntoSample(pSample, pSampleData );
  setSampleTimestamps( pSample, pSampleData );
  setSynchronisationMarker( pSample, pSampleData);

  // Free memory used by sample
  delete pSampleData;
  return S_OK;
}

HRESULT RtspSourceOutputPin::DoBufferProcessingLoop( void )
{
	Command com;

	OnThreadStartPlay();

	resetTimeStampOffsets();
	do {
		while (!CheckRequest(&com)) 
		{
      // The packet manager is ready once buffering has been completed
      if (( m_pFilter->m_mediaPacketManager.isReady() ) &&
        (m_pFilter->m_mediaPacketManager.hasSamples(m_nID)))
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
				if (m_pFilter->m_mediaPacketManager.eof())
				{
					DeliverEndOfStream();
					return S_OK;
				}
				//return S_FALSE;
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

void RtspSourceOutputPin::copyMediaDataIntoSample( IMediaSample* pSample, MediaSample* pSampleData )
{
  BYTE* pDestinationBuffer(NULL);
  // Access the sample's data buffer
  pSample->GetPointer(&pDestinationBuffer);

  if (m_pMediaType->subtype == MEDIASUBTYPE_H264)
  {
#if 0
    pDestinationBuffer[0] = 0x0;
    pDestinationBuffer[1] = 0x0;
    pDestinationBuffer[2] = 0x0;
    pDestinationBuffer[3] = 0x1;
    // try manually copying in start codes
    memcpy(pDestinationBuffer + 4, pSampleData->getData(), (DWORD) pSampleData->getSize());
    // Set length of media sample
    pSample->SetActualDataLength(pSampleData->getSize() + 4);
#endif    
    
    pDestinationBuffer[0] = 0x0;
    pDestinationBuffer[1] = 0x0;
    pDestinationBuffer[2] = 0x1;
    // try manually copying in start codes
    memcpy(pDestinationBuffer + 3, pSampleData->getData(), (DWORD) pSampleData->getSize());
    // Set length of media sample
    pSample->SetActualDataLength(pSampleData->getSize() + 3);
    return;
  }

  // Copy the data from the sample into the buffer
  if (m_nBitsPerSample == 16)
  {
    // 16 bit audio is always deliver in network byte order by the liveMedia library, we need to swap the byte order first
    // before delivering the media downstream

    // Swap the byte order of the 16-bit values that we have just read:
    unsigned numValues = pSampleData->getSize() >> 1;

    short* pDest = (short*)pDestinationBuffer;
    short* pSrc = (short*)pSampleData->getData();

    for (unsigned i = 0; i < numValues; ++i) 
    {
      short const orig = pSrc[i];
      pDest[i] = ((orig&0xFF)<<8) | ((orig&0xFF00)>>8);
    }
  }
  else
  {
    // 8 bit PCM audio or H263 video: simply copy the media buffer
    memcpy(pDestinationBuffer, pSampleData->getData(), (DWORD) pSampleData->getSize());
  }
  // Set length of media sample
  pSample->SetActualDataLength(pSampleData->getSize());
}

void RtspSourceOutputPin::setSampleTimestamps( IMediaSample* pSample, MediaSample* pSampleData )
{
  if ( !m_bFirstSample )
  {
    m_bFirstSample = true;
    m_pFilter->setInitialOffset(pSampleData->StartTime());
    m_dStreamTimeOffset = m_pFilter->m_dInitialStreamTimeOffset;
    DbgLog((LOG_ERROR, 1, TEXT("First sample received. Time offset: %f Stream offset: %f!"), pSampleData->StartTime(), m_dStreamTimeOffset));
  }
  else if (!m_bFirstSyncedSample && pSampleData->isMarkerSet())
  {
    m_bFirstSyncedSample = true;
    m_pFilter->setSynchronisedOffset(pSampleData->StartTime());
    m_dStreamTimeOffset = m_pFilter->m_dSynchronisedStreamTimeOffset;
    DbgLog((LOG_ERROR, 1, TEXT("Sync sample received. Time offset: %f Stream offset: %f!"), pSampleData->StartTime(), m_dStreamTimeOffset));
  }
#define ADJUST_IN_MEDIA_QUEUE
#ifdef ADJUST_IN_MEDIA_QUEUE
  double dStartTime = pSampleData->StartTime() + m_dStreamTimeOffset;
#endif

  //#define ADJUST
#ifdef ADJUST
  double dStartTime = pSampleData->StartTime() - getStartTimeOffset() + m_dStreamTimeOffset;
#endif
  //#define NO_ADJUST
#ifdef NO_ADJUST
  double dStartTime = pSampleData->StartTime();
#endif
  REFERENCE_TIME rtStart = (REFERENCE_TIME) (dStartTime * 1000000 * 10);

  // Codec specific
  if (m_pMediaType->subtype == MEDIASUBTYPE_H264)
  {
    // See:
    // http://social.msdn.microsoft.com/Forums/en-US/windowsdirectshowdevelopment/thread/b337c5dd-f87f-4624-92c8-59eb9755fbcf
    const BYTE* pData = pSampleData->getData();
    if (isIdrFrame(pData[0]))
    {
      // RG: TESTING ONLY SETTING THE START TIME
      pSample->SetTime(&rtStart, NULL);
    }
    else
    {
      pSample->SetTime(NULL, NULL);
    }

    // RG: setting timestamps according to the above link causes artifacts in video: still need to determine why
    // Remove the following line once issue is sorted out
    pSample->SetTime(NULL, NULL);
  }
  else
  {
    // Set stop timestamp to be slightly in the future
    // The commented out NULL time stamp for the stop time seemed to work the same
    REFERENCE_TIME rtStop = rtStart + 1;
    pSample->SetTime(&rtStart, &rtStop);
  }

  // If timestamps seem to cause strange problems, uncomment the define to see how the stream renders without timestamps
//#define RTVC_DEBUG_SET_NULL_TIMESTAMPS
#ifdef RTVC_DEBUG_SET_NULL_TIMESTAMPS
  pSample->SetTime(NULL, NULL);
#endif

}

void RtspSourceOutputPin::setSynchronisationMarker( IMediaSample* pSample, MediaSample* pSampleData )
{
  // Set SyncPoint property of media sample
  if (m_pMediaType->subtype == MEDIASUBTYPE_PCM)
  {
    // All samples are sync points
    pSample->SetSyncPoint(TRUE);
  }
  else if (m_pMediaType->subtype == MEDIASUBTYPE_AMR)
  {
    // All samples are sync points
    pSample->SetSyncPoint(TRUE);
  }
  else if (m_pMediaType->subtype == FOURCCMap(WAVE_FORMAT_MPEGLAYER3))
  {  
    // All samples are sync points
    pSample->SetSyncPoint(TRUE);
  }
  else if (m_pMediaType->subtype == MEDIASUBTYPE_H264)
  {
    // if is IDR set flag
    const BYTE* pData = pSampleData->getData();
    if (isIdrFrame(pData[0]))
    {
      pSample->SetSyncPoint(TRUE);
    }
  }
  else
  {
    // Default
    pSample->SetSyncPoint(FALSE);
  }
}

inline void RtspSourceOutputPin::resetTimeStampOffsets()
{
  m_bFirstSample = false;
  m_bFirstSyncedSample = false;
}
