/** @file

MODULE				: VideoMixingFilter

FILE NAME			: VideoMixingFilter.h

DESCRIPTION			: Video Mixing Filter for RGB24 and RGB32 Media
					  
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
#pragma once

#include <DirectShow/VideoMixingBase.h>
#include <DirectShow/CSettingsInterface.h>

// {48E4D730-389D-4bdc-BEC1-F178FA6DCDBA}
static const GUID CLSID_VideoMixingFilter = 
{ 0x48e4d730, 0x389d, 0x4bdc, { 0xbe, 0xc1, 0xf1, 0x78, 0xfa, 0x6d, 0xcd, 0xba } };

// {EECA4B39-B8A9-4d38-A4AD-37D091C47403}
static const GUID CLSID_VideoMixingProperties = 
{ 0xeeca4b39, 0xb8a9, 0x4d38, { 0xa4, 0xad, 0x37, 0xd0, 0x91, 0xc4, 0x74, 0x3 } };

#ifdef _BUILD_FOR_SHORT
typedef short yuvType;
#else
typedef signed char yuvType ;
#endif

// Forward declarations
class PicConcatBase;

/**
 * \ingroup DirectShowFilters
 * Video Mixing Filter for RGB24 and RGB32 Media
 */
class VideoMixingFilter	:	public VideoMixingBase,
							public CSettingsInterface,
							public ISpecifyPropertyPages
{
public:
	DECLARE_IUNKNOWN;

	/// Constructor
	VideoMixingFilter();
	/// Destructor
	~VideoMixingFilter();

	/// Static object-creation method (for the class factory)
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr); 
	
	///  From CBasePin
	virtual HRESULT CheckOutputType(const CMediaType* pMediaType);

	/// Overridden from VideoMixingBase 
	virtual HRESULT ReceiveFirstSample(IMediaSample *pSample);
	/// Overridden from VideoMixingBase 
	virtual HRESULT ReceiveSecondSample(IMediaSample *pSample);
	/// Overridden from VideoMixingBase 
	virtual HRESULT CreateVideoMixer(const CMediaType *pMediaType, int nIndex);
	/// Overridden from VideoMixingBase 
	virtual HRESULT SetOutputDimensions(BITMAPINFOHEADER* pBmih1, BITMAPINFOHEADER* pBmih2);

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
		pPages->pElems[0] = CLSID_VideoMixingProperties;
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
			return VideoMixingBase::NonDelegatingQueryInterface(riid, ppv);
		}
	}

private:

	/// Pointers to our last received Media Sample
	BYTE* m_pSampleBuffers[2];
	// Sizes
	int m_nSampleSizes[2];

	/// 0 = horizontal, 1 = vertical
	int m_nOrientation;

	// Picture concatenation class
	PicConcatBase* m_pPicConcat;

	/// Stores bytes per needed to store pixel according to media type
	double m_nBytesPerPixel;
};
