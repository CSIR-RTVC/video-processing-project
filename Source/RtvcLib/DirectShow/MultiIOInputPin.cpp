/** @file

MODULE				: MultiIOInputPin

FILE NAME			: MultiIOInputPin.cpp

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
#include "MultiIOInputPin.h"

#include "MultiIOBaseFilter.h"

CMultiIOInputPin::CMultiIOInputPin(CMultiIOBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr, LPCWSTR pName, int nIndex)
: CBaseInputPin(NAME("TM Multi Input Pin"), pFilter, pLock, phr, pName),
m_pFilter(pFilter),
m_nIndex(nIndex)
{
}

CMultiIOInputPin::~CMultiIOInputPin(void)
{
}

HRESULT CMultiIOInputPin::CheckMediaType( const CMediaType* mtIn )
{
	return m_pFilter->CheckInputType(mtIn);
}

HRESULT CMultiIOInputPin::BreakConnect()
{
	HRESULT hr = CBaseInputPin::BreakConnect();
	m_pFilter->OnDisconnect(m_nIndex);
	return hr;
}

HRESULT CMultiIOInputPin::CompleteConnect( IPin *pReceivePin )
{
	HRESULT hr = CBaseInputPin::CompleteConnect(pReceivePin);
	if (SUCCEEDED(hr))
	{
		m_pFilter->OnConnect(m_nIndex, RTVC_IN);
	}
	return hr;
}

STDMETHODIMP CMultiIOInputPin::Receive( IMediaSample *pSample )
{
	//Copied from CTransformInputPin
	HRESULT hr;
	CAutoLock lck(&m_pFilter->m_csReceive);
	ASSERT(pSample);

	// check all is well with the base class
	hr = CBaseInputPin::Receive(pSample);
	if (S_OK == hr) {
		hr = m_pFilter->Receive(pSample, m_nIndex);
	}
	return hr;
}

HRESULT CMultiIOInputPin::SetMediaType( const CMediaType* mtIn )
{
	// Set the base class media type (should always succeed)
	HRESULT hr = CBasePin::SetMediaType(mtIn);
	if (FAILED(hr)) {
		return hr;
	}

	// check the transform can be done (should always succeed)
	ASSERT(SUCCEEDED(m_pFilter->CheckInputType(mtIn)));

	return m_pFilter->SetMediaType(PINDIR_INPUT, mtIn, m_nIndex);
}

STDMETHODIMP CMultiIOInputPin::EndOfStream()
{
	HRESULT hr = CBaseInputPin::EndOfStream();
	if (FAILED(hr))
	{
		return hr;
	}
	return m_pFilter->EndOfStream(m_nIndex);
}

STDMETHODIMP CMultiIOInputPin::BeginFlush()
{
	HRESULT hr = CBaseInputPin::BeginFlush();
	if (FAILED(hr))
	{
		return hr;
	}
	return m_pFilter->BeginFlush(m_nIndex);
}

STDMETHODIMP CMultiIOInputPin::EndFlush()
{
	HRESULT hr = CBaseInputPin::EndFlush();
	if (FAILED(hr))
	{
		return hr;
	}
	return m_pFilter->EndFlush(m_nIndex);
}

STDMETHODIMP CMultiIOInputPin::NewSegment( REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate )
{
	HRESULT hr = CBaseInputPin::NewSegment(tStart, tStop, dRate);
	if (FAILED(hr))
	{
		return hr;
	}
	return m_pFilter->NewSegment(tStart, tStop, dRate, m_nIndex );
}