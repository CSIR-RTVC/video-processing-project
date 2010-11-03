/** @file

MODULE				: AudioMixingFilter

FILE NAME			: AudioMixingFilter.h

DESCRIPTION			: Audio Mixing Filter for PCM
					  
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
#pragma once

#include <DirectShow/MultiIOBaseFilter.h>
#include <DirectShow/CSettingsInterface.h>

#include <deque>

// {D4F39FA5-3E76-44fc-B8FE-FC62545BA730}
static const GUID CLSID_AudioMixingFilter = 
{ 0xd4f39fa5, 0x3e76, 0x44fc, { 0xb8, 0xfe, 0xfc, 0x62, 0x54, 0x5b, 0xa7, 0x30 } };

// {D4F39FA5-3E76-44fc-B8FE-FC62545BA730}
static const GUID CLSID_AudioMixingProperties = 
{ 0xd4f39fa5, 0x3e76, 0x44fc, { 0xb8, 0xfe, 0xfc, 0x62, 0x54, 0x5b, 0xa7, 0x30 } };

/**
 * \ingroup DirectShowFilters
 * Audio Mixing Filter for PCM Mono Media
 */
class AudioMixingFilter	:	public CMultiIOBaseFilter,
							            public CSettingsInterface,
							            public ISpecifyPropertyPages
{
  typedef std::deque<IMediaSample*> MediaSampleQueue_t; 
public:
	DECLARE_IUNKNOWN;

	/// Constructor
	AudioMixingFilter();
	/// Destructor
	~AudioMixingFilter();

	/// Static object-creation method (for the class factory)
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr); 

	///  From CBasePin
	virtual HRESULT CheckOutputType(const CMediaType* pMediaType);

  /// Delegates call to virtual ReceiveFirstSample and ReceiveSecondSample methods.
  virtual HRESULT Receive(IMediaSample *pSample, int nIndex );

  /// From CBaseOutputPin
  virtual HRESULT DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pRequestProperties, int m_nIndex);

  /// To Check media type
  virtual HRESULT GetMediaType(int iPosition, CMediaType* pmt, int nOutputPinIndex) ;

  /// Overriding this so that we can set whether this is an RGB24 or an RGB32 Filter and create
  /// the picture concatenator dynamically. This method also tells us if the advert stream is tagged
  HRESULT SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt, int nIndex);

  STDMETHODIMP Stop();
  STDMETHODIMP Pause();

  virtual void InitialiseInputTypes()
  {
    AddInputType(&MEDIATYPE_Audio, &MEDIASUBTYPE_PCM, &FORMAT_WaveFormatEx);
  }
  virtual void InitialiseOutputTypes()
  {
    AddOutputType(&MEDIATYPE_Audio, &MEDIASUBTYPE_PCM, &FORMAT_WaveFormatEx);
  }

  virtual int InitialNumberOfInputPins()	{ return 2; }
  virtual int InitialNumberOfOutputPins() { return 1; }
  virtual bool OnFullCreateMoreInputs()	{ return false; }
  virtual bool OnFullCreateMoreOutputs()	{ return false; }

  // you can also override these if you want to know about streaming
  virtual HRESULT StartStreaming();
  virtual HRESULT StopStreaming();

  // Streaming calls delegated from input pins
  virtual STDMETHODIMP EndOfStream(int nIndex);
  virtual STDMETHODIMP BeginFlush(int nIndex);
  virtual STDMETHODIMP EndFlush(int nIndex);

	/// Overridden from AudioMixingBase 
	virtual HRESULT ReceiveFirstSample(IMediaSample *pSample);
	/// Overridden from AudioMixingBase 
	virtual HRESULT ReceiveSecondSample(IMediaSample *pSample);

	HRESULT GenerateOutputSample(IMediaSample *pSample, int nIndex);

	virtual void initParameters();

	/// For configuration dialog
	STDMETHODIMP GetPages(CAUUID *pPages)
	{
		if (pPages == NULL) return E_POINTER;
		pPages->cElems = 1;
		pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
		if (pPages->pElems == NULL) 
		{
			return E_OUTOFMEMORY;
		}
		pPages->pElems[0] = CLSID_AudioMixingProperties;
		return S_OK;
	}

	/// For configuration dialog
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv)
	{
		if (riid == IID_ISpecifyPropertyPages)
		{
			return GetInterface(static_cast<ISpecifyPropertyPages*>(this), ppv);
		}
		else if(riid == (IID_ISettingsInterface))
		{
			return GetInterface((ISettingsInterface*) this, ppv);
		}
		else
		{
			// Call the parent class.
			return CMultiIOBaseFilter::NonDelegatingQueryInterface(riid, ppv);
		}
	}

private:

	/// Pointers to our last received Media Sample
	BYTE* m_pSampleBuffers[2];
	// Sizes
	int m_nSampleSizes[2];
  MediaSampleQueue_t m_qSamples1;
  MediaSampleQueue_t m_qSamples2;

  unsigned m_uiBitsPerSample;
  unsigned m_uiSamplesPerSecond;
  unsigned m_uiChannels;

  // Vars needed for eos behaviour
  unsigned m_uiEndOfStreamCount;
  unsigned m_uiBeginFlushCount;
  unsigned m_uiEndFlushCount;

  HANDLE m_hSyncEvent;
};
