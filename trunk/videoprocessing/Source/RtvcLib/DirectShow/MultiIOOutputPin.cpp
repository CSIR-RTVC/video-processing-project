/** @file

MODULE				: MultiIOOutputPin

FILE NAME			: MultiIOOutputPin.cpp

DESCRIPTION			: 
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008, Meraka Institute
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of the Meraka Institute nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

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
#include "MultiIOOutputPin.h"

#include "MultiIOBaseFilter.h"
#include "MultiIOInputPin.h"

CMultiIOOutputPin::CMultiIOOutputPin(CMultiIOBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr, LPCWSTR pName, int nIndex)
: CBaseOutputPin(NAME("TM Multi Output Pin"), pFilter, pLock, phr, pName),
m_pFilter(pFilter),
m_nIndex(nIndex),
m_pOutputQueue(NULL)
{
}

CMultiIOOutputPin::~CMultiIOOutputPin(void)
{
}

HRESULT CMultiIOOutputPin::CompleteConnect( IPin *pReceivePin )
{
	HRESULT hr = CBaseOutputPin::CompleteConnect(pReceivePin);
	if (SUCCEEDED(hr))
	{
		m_pFilter->OnConnect(m_nIndex, RTVC_OUT);
	}
	return hr;
}

HRESULT CMultiIOOutputPin::DecideBufferSize( IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pRequestProperties )
{
	//Pass the buffer allocation on to the mixer filter
	return m_pFilter->DecideBufferSize(pAlloc, pRequestProperties, m_nIndex);
}

HRESULT CMultiIOOutputPin::CheckMediaType( const CMediaType* pMediaType )
{
	return m_pFilter->CheckOutputType(pMediaType);
}

HRESULT CMultiIOOutputPin::GetMediaType( int iPosition, CMediaType* pMediaType )
{
	ASSERT(m_pFilter->m_vInputPins.size() > 0);

	//  We don't have any media types if our input is not connected

	if (m_pFilter->m_vInputPins[0]->IsConnected()) 
	{
		return m_pFilter->GetMediaType(iPosition, pMediaType, m_nIndex);
	} else 
	{
		return VFW_S_NO_MORE_ITEMS;
	}
}

STDMETHODIMP CMultiIOOutputPin::Notify( IBaseFilter * pSender, Quality q )
{
	UNREFERENCED_PARAMETER(q);
	UNREFERENCED_PARAMETER(pSender);
	return E_NOTIMPL;
}

HRESULT CMultiIOOutputPin::Active()
{
	CAutoLock lock_it(m_pLock);
	HRESULT hr = NOERROR;

	// Make sure that the pin is connected
	if(m_Connected == NULL)
		return NOERROR;

	// Create the output queue if we have to
	if(m_pOutputQueue == NULL)
	{
		m_pOutputQueue = new COutputQueue(m_Connected, &hr, TRUE, FALSE);
		if(m_pOutputQueue == NULL)
			return E_OUTOFMEMORY;

		// Make sure that the constructor did not return any error
		if(FAILED(hr))
		{
			delete m_pOutputQueue;
			m_pOutputQueue = NULL;
			return hr;
		}
	}

	// Pass the call on to the base class
	CBaseOutputPin::Active();
	return NOERROR;

} // Active

HRESULT CMultiIOOutputPin::Inactive()
{
	CAutoLock lock_it(m_pLock);

	// Delete the output queus associated with the pin.
	if(m_pOutputQueue)
	{
		delete m_pOutputQueue;
		m_pOutputQueue = NULL;
	}

	CBaseOutputPin::Inactive();
	return NOERROR;

} // Inactive

HRESULT CMultiIOOutputPin::Deliver(IMediaSample *pMediaSample)
{
	CheckPointer(pMediaSample,E_POINTER);

	// Make sure that we have an output queue
	if(m_pOutputQueue == NULL)
		return NOERROR;

	pMediaSample->AddRef();
	return m_pOutputQueue->Receive(pMediaSample);

} // Deliver

//
// DeliverEndOfStream
//
HRESULT CMultiIOOutputPin::DeliverEndOfStream()
{
	// Make sure that we have an output queue
	if(m_pOutputQueue == NULL)
		return NOERROR;

	m_pOutputQueue->EOS();
	return NOERROR;

} // DeliverEndOfStream


//
// DeliverBeginFlush
//
HRESULT CMultiIOOutputPin::DeliverBeginFlush()
{
	// Make sure that we have an output queue
	if(m_pOutputQueue == NULL)
		return NOERROR;

	m_pOutputQueue->BeginFlush();
	return NOERROR;

} // DeliverBeginFlush


//
// DeliverEndFlush
//
HRESULT CMultiIOOutputPin::DeliverEndFlush()
{
	// Make sure that we have an output queue
	if(m_pOutputQueue == NULL)
		return NOERROR;

	m_pOutputQueue->EndFlush();
	return NOERROR;

} // DeliverEndFlish

//
// DeliverNewSegment
//
HRESULT CMultiIOOutputPin::DeliverNewSegment(REFERENCE_TIME tStart, 
										 REFERENCE_TIME tStop,  
										 double dRate)
{
	// Make sure that we have an output queue
	if(m_pOutputQueue == NULL)
		return NOERROR;

	m_pOutputQueue->NewSegment(tStart, tStop, dRate);
	return NOERROR;

} // DeliverNewSegment