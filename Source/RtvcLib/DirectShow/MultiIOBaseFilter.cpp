/** @file

MODULE				: MultiIOBaseFilter

FILE NAME			: MultiIOBaseFilter.cpp

DESCRIPTION			: 
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008 - 2011, CSIR
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
#include "MultiIOBaseFilter.h"

#include "MultiIOInputPin.h"
#include "MultiIOOutputPin.h"

#include <Shared/StringUtil.h>
#include <Shared/Conversion.h>

//#include <atlconv.h>
//#pragma comment(lib, "atls.lib")

CMultiIOBaseFilter::CMultiIOBaseFilter(TCHAR *pObjectName, LPUNKNOWN lpUnk, CLSID clsid)
: CBaseFilter(pObjectName, lpUnk, &m_csFilter, clsid),
m_bInitialised(false)
{}

CMultiIOBaseFilter::~CMultiIOBaseFilter(void)
{
	// Release all the COM interfaces
	for (size_t i = 0; i < m_vInputPins.size(); i++)
	{
		m_vInputPins[i]->Release();
	}
	for (size_t i = 0; i < m_vOutputPins.size(); i++)
	{
		m_vOutputPins[i]->Release();
	}

	// Clean up input type map
	CleanupTypeMap(m_mInputTypeMap);
	// Clean up Output type map
	CleanupTypeMap(m_mOutputTypeMap);
	// Clean up generic types
	for (size_t i = 0; i < m_vInputTypes.size(); i++)
	{
		delete m_vInputTypes.at(i);
	}
	for (size_t i = 0; i < m_vOutputTypes.size(); i++)
	{
		delete m_vOutputTypes.at(i);
	}
}

void CMultiIOBaseFilter::CleanupTypeMap(MEDIA_TYPE_MAP mTypeMap)
{
	MEDIA_TYPE_MAP_ITERATOR it;
	for (it = mTypeMap.begin(); it != mTypeMap.end(); it++)
	{
		//Delete the input type list for that pin
		MEDIA_TYPE_LIST* pInputList = (*it).second;
		//Iterate over list and delete all input types
		for (size_t i = 0; i < pInputList->size(); i++)
		{
			delete pInputList->at(i);
		}
		delete pInputList;
	}
	mTypeMap.clear();
}


void CMultiIOBaseFilter::Initialise()
{
	if (!m_bInitialised)
	{
		//Create initial number of inputs/outputs
		for (int i = 0; i < InitialNumberOfInputPins(); i++)
		{
			CreateInputPin();
		}

		for (int i = 0; i < InitialNumberOfOutputPins(); i++)
		{
			CreateOutputPin();
		}
		InitialiseInputTypes();
		InitialiseOutputTypes();
	}
	m_bInitialised = true;
}

void CMultiIOBaseFilter::AddInputType( const GUID* pType, const GUID* pSubType, const GUID* pFormat, int nPin  )
{
	CIOMediaType* pMediaType = new CIOMediaType(pType, pSubType, pFormat);
	AddMediaType(pMediaType, m_vInputTypes, m_mInputTypeMap, nPin );
}

void CMultiIOBaseFilter::AddOutputType( const GUID* pType, const GUID* pSubType, const GUID* pFormat, int nPin   )
{
	CIOMediaType* pMediaType = new CIOMediaType(pType, pSubType, pFormat);
	AddMediaType(pMediaType, m_vOutputTypes, m_mOutputTypeMap, nPin );
}

void CMultiIOBaseFilter::AddMediaType(CIOMediaType* pMediaType, MEDIA_TYPE_LIST& typeList, MEDIA_TYPE_MAP& typeMap, int nPin  )
{
	if (nPin == -1)
	{
		// Generic types
		typeList.push_back(pMediaType);
	}
	else
	{
		// Add the mapping of the pin number to the type
		MEDIA_TYPE_MAP_ITERATOR it;
		it = typeMap.find(nPin);
		if (it == m_mInputTypeMap.end())
		{
			//No list for pin, create one
			typeMap[nPin] = new MEDIA_TYPE_LIST;
			typeMap[nPin]->push_back(pMediaType);
		}
		else
		{
			//list already exists for pin
			(*it).second->push_back(pMediaType);
		}
	}
}

int CMultiIOBaseFilter::GetPinCount()
{
	return (int)(m_vInputPins.size() + m_vOutputPins.size());
}

CBasePin * CMultiIOBaseFilter::GetPin( int n )
{
	if ((n >= GetPinCount()) || (n < 0))
		return NULL;

	if (n < (int) m_vInputPins.size())
	{
		//Return the input pin
		return m_vInputPins[n];
	}
	else
	{
		return m_vOutputPins[n - m_vInputPins.size() ];
	}
}

STDMETHODIMP CMultiIOBaseFilter::FindPin( LPCWSTR Id, IPin **ppPin )
{
	//Find pin according to it's name
	CheckPointer(ppPin,E_POINTER);
	ValidateReadWritePtr(ppPin,sizeof(IPin *));

	//Parse Id and get string number
  std::string sId = StringUtil::wideToStl(Id);
  std::string sType = sId.substr(0, 5);
	if (sType == "Input")
	{
    std::string sIdCount = sId.substr(6);
    int nId = convert<int>(sIdCount);
		*ppPin = GetPin(nId);
	}
	else if (sType == "Outpu")
	{
    std::string sIdCount = sId.substr(7);
    int nId = convert<int>(sIdCount);
		*ppPin = GetPin((int)m_vInputPins.size() + nId);
	} 
	else 
	{
		*ppPin = NULL;
		return VFW_E_NOT_FOUND;
	}

	HRESULT hr = NOERROR;
	//  AddRef() returned pointer - but GetPin could fail if memory is low.
	if (*ppPin) 
	{
		(*ppPin)->AddRef();
	} 
	else 
	{
		hr = E_OUTOFMEMORY;  // probably.  There's no pin anyway.
	}
	return hr;
}

void CMultiIOBaseFilter::CreateInputPin()
{
	HRESULT hr;
  std::ostringstream ostr;
  ostr << "Input " << m_vInputPins.size();
  wchar_t* wszId = StringUtil::stlToWide(ostr.str());
	CMultiIOInputPin* pInputPin = new CMultiIOInputPin(this, &m_csFilter, &hr, wszId, (int)m_vInputPins.size());
  delete wszId;
	pInputPin->AddRef();
	m_vInputPins.push_back(pInputPin);
	// ensure enumerator is refreshed
	IncrementPinVersion();	
}

void CMultiIOBaseFilter::CreateOutputPin()
{
	HRESULT hr;
  std::ostringstream ostr;
  ostr << "Output " << m_vInputPins.size();
  wchar_t* wszId = StringUtil::stlToWide(ostr.str());
	CMultiIOOutputPin* pOutputPin = new CMultiIOOutputPin(this, &m_csFilter, &hr, wszId, (int)m_vOutputPins.size());
  delete wszId;

  pOutputPin->AddRef();
	m_vOutputPins.push_back(pOutputPin);
	// ensure enumerator is refreshed
	IncrementPinVersion();	
}

HRESULT CMultiIOBaseFilter::CheckInputType( const CMediaType *pMediaType )
{
	//RG 8/01/2008
	//Changing this to check all input types
	return FindMediaTypeInLists(pMediaType, &m_vInputTypes, m_mInputTypeMap);
}

HRESULT CMultiIOBaseFilter::FindMediaTypeInLists(const CMediaType* pMediaType, MEDIA_TYPE_LIST* pTypeList, MEDIA_TYPE_MAP& typeMap)
{
	// Search for media type in global list
	HRESULT hr = FindMediaType(pMediaType, pTypeList);
	if (SUCCEEDED(hr))
	{
		return hr;
	}

	//We haven't found it: Iterate over all pins own media types
	MEDIA_TYPE_MAP_ITERATOR it;
	for ( it = typeMap.begin(); it != typeMap.end(); it++)
	{
		MEDIA_TYPE_LIST* pTypeList = (*it).second;
		HRESULT hr = FindMediaType(pMediaType, pTypeList);
		if (SUCCEEDED(hr))
		{
			return hr;
		}
	}
	// We didn't find the type anywhere
	return VFW_E_TYPE_NOT_ACCEPTED;
}

void CMultiIOBaseFilter::OnConnect( int nIndex, RTVC_DIRECTION eDirection )
{
	switch (eDirection)
	{
	case RTVC_IN:
		{
			// if this is the last one, make a new one: taken from Geraint's Mpeg Parser
			if (OnFullCreateMoreInputs())
			{
				if (nIndex == (int)(m_vInputPins.size()-1))
				{
					CreateInputPin();
				}
			}
			break;
		}
	case RTVC_OUT:
		{
			// if this is the last one, make a new one: taken from Geraint's Mpeg Parser
			if (OnFullCreateMoreOutputs())
			{
				if (nIndex == (int)(m_vOutputPins.size()-1))
				{
					CreateOutputPin();
				}
			}
			break;
		}
	}
}

void CMultiIOBaseFilter::OnDisconnect( int nIndex )
{
	if (OnFullCreateMoreInputs())
	{
		// if index is the last but one, and
		// the last one is still unconnected, then
		// remove the last one
		if ((nIndex == (int)(m_vInputPins.size() - 2)) &&
			(!m_vInputPins[m_vInputPins.size()-1]->IsConnected()))
		{
			// Now make sure that we can't disconnect more than the initial number of inputs
			// You cannot remove a pin to have less pins than you started with
			if (m_vInputPins.size() > (size_t)InitialNumberOfInputPins())
			{
				m_vInputPins[m_vInputPins.size()-1]->Release();
				m_vInputPins.pop_back();
				// refresh enumerator
				IncrementPinVersion();
			}
		}
	}
}


int CMultiIOBaseFilter::InitialNumberOfInputPins()
{
	return 1;
}

int CMultiIOBaseFilter::InitialNumberOfOutputPins()
{
	return 1;
}

bool CMultiIOBaseFilter::OnFullCreateMoreInputs()
{
	return false;
}

bool CMultiIOBaseFilter::OnFullCreateMoreOutputs()
{
	return false;
}

//////////////////////////////////////////////////////////////////////////
// Output pin stuff
HRESULT CMultiIOBaseFilter::CheckOutputType( const CMediaType* pMediaType )
{
	return FindMediaTypeInLists(pMediaType, &m_vOutputTypes, m_mOutputTypeMap);
}

/**
* 
* 
* Method:    InitializeOutputSample
* FullName:  CMultiIOBaseFilter::InitializeOutputSample
* Access:    virtual public 
* Qualifier:
* @return:   HRESULT
* @param: 	 IMediaSample *pSample
* @param: 	 IMediaSample **ppOutSample
* @param: 	 int nInputIndex: this is required when initialising an out sample so that we can get the properties such as sample start time of the specified input and use this for the specified out sample
* @param: 	 int nOutputIndex: this needs to be specified so that we can get the correct allocator
*/
HRESULT CMultiIOBaseFilter::InitializeOutputSample( IMediaSample *pSample, IMediaSample **ppOutSample, int nInputIndex, int nOutputIndex )
{
	AM_SAMPLE2_PROPERTIES * const pProperties = m_vInputPins[nInputIndex]->SampleProps();

	IMediaSample *pOutSample;
	//Let's get rid of the const

	AM_SAMPLE2_PROPERTIES* pProps = (AM_SAMPLE2_PROPERTIES*)pProperties;
	// default - times are the same
	DWORD dwFlags = 0;
	// This will prevent the image renderer from switching us to DirectDraw
	// when we can't do it without skipping frames because we're not on a
	// keyframe.  If it really has to switch us, it still will, but then we
	// will have to wait for the next keyframe
	if (!(pProps->dwSampleFlags & AM_SAMPLE_SPLICEPOINT)) {
		dwFlags |= AM_GBF_NOTASYNCPOINT;
	}

	ASSERT(m_vOutputPins[nOutputIndex]->GetAllocator() != NULL);
	HRESULT hr = m_vOutputPins[nOutputIndex]->GetAllocator()->GetBuffer(
		&pOutSample
		, pProps->dwSampleFlags & AM_SAMPLE_TIMEVALID ?
		&pProps->tStart : NULL
		, pProps->dwSampleFlags & AM_SAMPLE_STOPVALID ?
		&pProps->tStop : NULL
		, dwFlags
		);
	*ppOutSample = pOutSample;
	if (FAILED(hr)) {
		return hr;
	}

	ASSERT(pOutSample);
	IMediaSample2 *pOutSample2;
	if (SUCCEEDED(pOutSample->QueryInterface(IID_IMediaSample2,
		(void **)&pOutSample2))) {
			/*  Modify it */
			AM_SAMPLE2_PROPERTIES OutProps;
			EXECUTE_ASSERT(SUCCEEDED(pOutSample2->GetProperties(
				FIELD_OFFSET(AM_SAMPLE2_PROPERTIES, tStart), (PBYTE)&OutProps)
				));
			OutProps.dwTypeSpecificFlags = pProps->dwTypeSpecificFlags;
			OutProps.dwSampleFlags =
				(OutProps.dwSampleFlags & AM_SAMPLE_TYPECHANGED) |
				(pProps->dwSampleFlags & ~AM_SAMPLE_TYPECHANGED);
			OutProps.tStart = pProps->tStart;
			OutProps.tStop  = pProps->tStop;
			OutProps.cbData = FIELD_OFFSET(AM_SAMPLE2_PROPERTIES, dwStreamId);
			hr = pOutSample2->SetProperties(
				FIELD_OFFSET(AM_SAMPLE2_PROPERTIES, dwStreamId),
				(PBYTE)&OutProps
				);
			pOutSample2->Release();
	} else {
		if (pProps->dwSampleFlags & AM_SAMPLE_TIMEVALID) {
			pOutSample->SetTime(&pProps->tStart,
				&pProps->tStop);
		}
		if (pProps->dwSampleFlags & AM_SAMPLE_SPLICEPOINT) {
			pOutSample->SetSyncPoint(TRUE);
		}
		if (pProps->dwSampleFlags & AM_SAMPLE_DATADISCONTINUITY) {
			pOutSample->SetDiscontinuity(TRUE);
		}
		// Copy the media times

		LONGLONG MediaStart, MediaEnd;
		if (pSample->GetMediaTime(&MediaStart,&MediaEnd) == NOERROR) {
			pOutSample->SetMediaTime(&MediaStart,&MediaEnd);
		}
	}
	return S_OK;
}

HRESULT CMultiIOBaseFilter::SetMediaType( PIN_DIRECTION direction,const CMediaType *pmt, int nIndex )
{
	UNREFERENCED_PARAMETER(direction);
	UNREFERENCED_PARAMETER(pmt);
	//Grip a copy of the media type
	if (direction == PINDIR_INPUT)
	{
		m_mMediaTypes[nIndex] = *pmt;
	}
	// Check if pin at position accepts the input type
	//First iterate over all common media types
	HRESULT hr = FindMediaType(pmt, &m_vInputTypes);
	if (SUCCEEDED(hr))
	{
		return hr;
	}

	//We haven't found it: Iterate over pins own media types
	MEDIA_TYPE_MAP_ITERATOR it;
	it = m_mInputTypeMap.find(nIndex);
	if (it == m_mInputTypeMap.end())
	{
		//We couldn't find it, reject the connection
		return E_FAIL;
	}
	else
	{
		MEDIA_TYPE_LIST* pInputTypeList = (*it).second;
		HRESULT hr = FindMediaType(pmt, pInputTypeList);
		return hr;
	}
}

HRESULT CMultiIOBaseFilter::FindMediaType(const CMediaType* pmt, MEDIA_TYPE_LIST* pInputTypeList)
{
	for (size_t i = 0; i < pInputTypeList->size(); i++)
	{
		//Compare major type
		const GUID* pType = (pInputTypeList->at(i))->MediaType;
		if ((!pType)||((pmt->majortype == *pType) && (pType)))
		{
			//Compare sub type
			const GUID* pSubtype = (pInputTypeList->at(i))->MediaSubtype;
			if ((!pSubtype) || ((pmt->subtype == *pSubtype) && (pSubtype)))
			{
				//Compare formats
				const GUID* pFormat = (pInputTypeList->at(i))->Format;
				if ((!pFormat) || ((pmt->formattype == *pFormat)&&(pFormat)))
				{
					return NOERROR;
				}
			}
		}
	}
	return E_FAIL;
}

// Basic implementation -> Just here for override purposes
STDMETHODIMP CMultiIOBaseFilter::EndOfStream( int nIndex )
{
	return S_OK;
}

STDMETHODIMP CMultiIOBaseFilter::BeginFlush( int nIndex )
{
	return S_OK;
}

STDMETHODIMP CMultiIOBaseFilter::EndFlush( int nIndex )
{
	return S_OK;
}

STDMETHODIMP CMultiIOBaseFilter::NewSegment( REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate, int nIndex )
{
	return S_OK;
}