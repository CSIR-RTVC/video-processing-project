/** @file

MODULE                : Stereo2MonoFilter

FILE NAME             : Stereo2MonoFilter.h

DESCRIPTION           :  This filter outputs mono output from a stereo input

LICENSE: Software License Agreement (BSD License)

Copyright (c) 2014, CSIR
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
#include <streams.h>

#include <DirectShow/CustomBaseFilter.h>

// {E0734A05-F25B-4F42-9CBF-7F1F610266C6}
static const GUID CLSID_RTVCStereo2MonoFilter = 
{ 0xe0734a05, 0xf25b, 0x4f42, { 0x9c, 0xbf, 0x7f, 0x1f, 0x61, 0x2, 0x66, 0xc6 } };

// {740C164C-B778-4604-A97E-DABDEED47861}
static const GUID CLSID_Stereo2MonoProperties = 
{ 0x740c164c, 0xb778, 0x4604, { 0xa9, 0x7e, 0xda, 0xbd, 0xee, 0xd4, 0x78, 0x61 } };

// Forward declarations

class Stereo2MonoFilter : public CCustomBaseFilter,
                            public ISpecifyPropertyPages
{
public:
  DECLARE_IUNKNOWN

  /// Constructor
  Stereo2MonoFilter(LPUNKNOWN pUnk, HRESULT *pHr);
  /// Destructor
  ~Stereo2MonoFilter();

  /// Static object-creation method (for the class factory)
  static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr); 

  /// override this to publicize our interfaces
  STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

	HRESULT CheckInputType(const CMediaType *pMediaType);
	HRESULT SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt);
	HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
	HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProp);
	HRESULT CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut);

  STDMETHODIMP GetPages(CAUUID *pPages) // For the Property Page
	{
		if (pPages == NULL) return E_POINTER;
		pPages->cElems = 1;
		pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
		if (pPages->pElems == NULL) 
		{
			return E_OUTOFMEMORY;
		}
		pPages->pElems[0] = CLSID_Stereo2MonoProperties;
		return S_OK;
	}

  /// Overridden from CSettingsInterface
  virtual void initParameters() { addParameter("channel", &m_channel, 0); }

private:
  	/// Overridden from CCustomBaseFilter
	  virtual void InitialiseInputTypes();
    virtual HRESULT ApplyTransform(BYTE* pBufferIn, long lInBufferSize, long lActualDataLength, BYTE* pBufferOut, long lOutBufferSize, long& lOutActualDataLength);

  int m_channel;
  int m_nBitsPerSample;
  int m_nSamplesPerSecond;
};
