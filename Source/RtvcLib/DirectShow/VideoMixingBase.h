/** @file

MODULE                : VideoMixingBase

FILE NAME            : VideoMixingBase.h

DESCRIPTION            : Video mixing base class for RGB24 and RGB32 media
                     
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
#pragma once

#include <DirectShow/MultiIOBaseFilter.h>

/**
 * \ingroup DirectShowLib
 * Base class for video mixing renderers. 
 * Implements common video mixing functionality for RGB24/32 mixers with two inputs and one output.
 */
class VideoMixingBase : public CMultiIOBaseFilter
{
public:
	/// Destructor
	~VideoMixingBase();

	/// Subclass must override these methods: it is up to the implementer whether the samples should be copied
	/// to a temporary buffer etc.
	/// The user must also decide on how the output of the filter is regulated.
	virtual HRESULT ReceiveFirstSample(IMediaSample *pSample) = 0;
	virtual HRESULT ReceiveSecondSample(IMediaSample *pSample) = 0;

	/// This method allows the subclass to create the appropriate video mixer based on the input media types
	/// Implementations that do not require this functionality should return S_OK;
	virtual HRESULT CreateVideoMixer(const CMediaType *pMediaType, int nIndex) = 0;

	/// This method is responsible for setting the values of 
	/// m_nOutputWidth;
	/// m_nOutputHeight;
	/// m_nOutputSize;
	/// The values set in this method will be used in GetMediaType and hence in DecideBufferSize
	/// The Subclass should also reject invalid media types in this method: e.g. a simple concatenation
	/// filter should return E_FAIL from this method if the image dimensions do not match
	virtual HRESULT SetOutputDimensions(BITMAPINFOHEADER* pBmih1, BITMAPINFOHEADER* pBmih2) = 0;

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
		AddInputType(&MEDIATYPE_Video, &MEDIASUBTYPE_RGB24, &FORMAT_VideoInfo);
		AddInputType(&MEDIATYPE_Video, &MEDIASUBTYPE_RGB32, &FORMAT_VideoInfo);
	}
	virtual void InitialiseOutputTypes()
	{
		AddOutputType(&MEDIATYPE_Video, &MEDIASUBTYPE_RGB24, &FORMAT_VideoInfo);
		AddOutputType(&MEDIATYPE_Video, &MEDIASUBTYPE_RGB32, &FORMAT_VideoInfo);
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
  //virtual STDMETHODIMP NewSegment( REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate, int nIndex);
protected:
	/// Constructor
	VideoMixingBase(TCHAR *pObjectName, LPUNKNOWN lpUnk, CLSID clsid);

	/// Stores video info headers
	VIDEOINFOHEADER m_VideoInHeader[2];
	
	int m_nOutputWidth;
	int m_nOutputHeight;
	int m_nOutputSize;


	/// Start time of the sample
	REFERENCE_TIME m_tStartTime1;
	REFERENCE_TIME m_tStartTime2;
	/// End time of the sample
	REFERENCE_TIME m_tStopTime1;
	REFERENCE_TIME m_tStopTime2;	

	/// Stores bytes per needed to store pixel according to media type
	double m_nBytesPerPixel;

  unsigned m_uiEndOfStreamCount;
  unsigned m_uiBeginFlushCount;
  unsigned m_uiEndFlushCount;
};
