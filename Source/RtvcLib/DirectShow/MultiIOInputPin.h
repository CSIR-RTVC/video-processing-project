/** @file

MODULE				: MultiIOInputPin

FILE NAME			: MultiIOInputPin.h

DESCRIPTION			: 
					  
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

#pragma warning(push)     // disable for this header only
#pragma warning(disable:4312)
// DirectShow
#include <streams.h>
#pragma warning(pop)      // restore original warning level

/**
 * \ingroup DirectShowLib
 * Base class for input pins of the CMultiIOBaseFilter
 */
class CMultiIOInputPin : public CBaseInputPin
{
	friend class CMultiIOBaseFilter;
	friend class CMultiIOOutputPin;

public:
	CMultiIOInputPin(CMultiIOBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr, LPCWSTR pName, int nIndex);
	virtual ~CMultiIOInputPin(void);

	// lifetime management for pins is normally delegated to the filter, but
	// we need to be able to create and delete them independently, so keep 
	// a separate refcount.
	STDMETHODIMP_(ULONG) NonDelegatingRelease()
	{
		return CUnknown::NonDelegatingRelease();
	}
	STDMETHODIMP_(ULONG) NonDelegatingAddRef()
	{
		return CUnknown::NonDelegatingAddRef();
	}

	// check that we can support this output type
	HRESULT CheckMediaType(const CMediaType* mtIn);

	// connection management -- used to maintain one free pin
	HRESULT BreakConnect();
	HRESULT CompleteConnect(IPin *pReceivePin);

	// do something with this media sample
	STDMETHODIMP Receive(IMediaSample *pSample);

	CMediaType& CurrentMediaType() { return m_mt; };

	// set the connection media type
	HRESULT SetMediaType(const CMediaType* mt);

	// Pass through calls downstream
	STDMETHODIMP EndOfStream();
	STDMETHODIMP BeginFlush();
	STDMETHODIMP EndFlush();
	STDMETHODIMP NewSegment(
		REFERENCE_TIME tStart,
		REFERENCE_TIME tStop,
		double dRate);
protected:
	CMultiIOBaseFilter* m_pFilter;
	int m_nIndex;
};
