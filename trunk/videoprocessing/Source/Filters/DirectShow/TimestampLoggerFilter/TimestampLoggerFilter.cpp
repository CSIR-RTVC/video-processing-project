/** @file

MODULE                : TimestampLoggerFilter

FILE NAME			  : TimestampLoggerFilter.cpp

DESCRIPTION           :
                     
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
#include "stdafx.h"

//**** Adding the TimingFilter
#include "TimestampLoggerFilter.h"
#include <Shared/StringUtil.h>

//To calculate the time now
#include <iostream>
#include <time.h>
#include <ctime>


TimestampLoggerFilter::TimestampLoggerFilter(LPUNKNOWN pUnk, HRESULT *pHr)
: CTransInPlaceFilter(NAME("CSIR VPP Timestamp Logger"), pUnk, CLSID_RTVCTimeStampLoggerFilter, pHr, false)
{;}

TimestampLoggerFilter::~TimestampLoggerFilter()
{;}

CUnknown * WINAPI TimestampLoggerFilter::CreateInstance( LPUNKNOWN pUnk, HRESULT *pHr )
{
	TimestampLoggerFilter *pFilter = new TimestampLoggerFilter(pUnk, pHr);
	if (pFilter== NULL) 
	{
		*pHr = E_OUTOFMEMORY;
	}
	return pFilter;
}

STDMETHODIMP TimestampLoggerFilter::NonDelegatingQueryInterface( REFIID riid, void **ppv )
{
	if (riid == IID_IFileSourceFilter)
	{
		return GetInterface((IFileSourceFilter*) this, ppv);
	}
	else
	{
		return CTransInPlaceFilter::NonDelegatingQueryInterface(riid, ppv);
	}
}

HRESULT TimestampLoggerFilter::Transform(IMediaSample *pSample) // Overrriding the receive method. This method receives a media sample, processes it, and delivers it to the downstream filter.
{
	// Output time to file
	REFERENCE_TIME tStart;
	REFERENCE_TIME tStop;

	HRESULT hr = pSample->GetTime(&tStart,&tStop);
	if (FAILED(hr))
	{
		return hr;
	}
  m_vTimeInfo.push_back(TimeInfo(tStart, tStop, pSample->GetActualDataLength()));
	return S_OK;
}

HRESULT TimestampLoggerFilter::CheckInputType(const CMediaType* mtIn)
{
	// Accept any input type
	return S_OK;
}

STDMETHODIMP TimestampLoggerFilter::Load( LPCOLESTR lpwszFileName, const AM_MEDIA_TYPE *pmt )
{
	// Store the filename
	m_sFileName = StringUtil::wideToStl(lpwszFileName);
	return S_OK;
}

STDMETHODIMP TimestampLoggerFilter::GetCurFile( LPOLESTR * ppszFileName, AM_MEDIA_TYPE *pmt )
{
	if (m_sFileName != "")
	{
		WCHAR* pFileName = StringUtil::stlToWide(m_sFileName);	
		DWORD n = sizeof(WCHAR)*(1+lstrlenW(pFileName));
		*ppszFileName = (LPOLESTR) CoTaskMemAlloc( n );
		if (*ppszFileName!=NULL) {
			CopyMemory(*ppszFileName, pFileName, n);
		}
		delete[] pFileName;
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}

HRESULT TimestampLoggerFilter::Run( REFERENCE_TIME tStart )
{
	return CTransInPlaceFilter::Run(tStart);
}

HRESULT TimestampLoggerFilter::Stop( void )
{
	// Write time info to file
	m_outStream.open(m_sFileName.c_str(), std::ios_base::out);
	if (m_outStream.is_open())
	{
		for (std::vector<TimeInfo>::iterator it = m_vTimeInfo.begin(); it != m_vTimeInfo.end(); ++it)
		{
      m_outStream << (*it).StartTime << " " << (*it).StopTime <<  " " << (*it).StopTime - (*it).StartTime << " " <<  (*it).Size <<  std::endl;
		}
		m_outStream.flush();
		m_outStream.close();
	}

  // Reset vector
  m_vTimeInfo.clear();

	return CTransInPlaceFilter::Stop();
}

