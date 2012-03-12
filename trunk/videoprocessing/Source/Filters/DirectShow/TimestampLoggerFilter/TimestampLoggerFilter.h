/** @file

MODULE                : TimestampLoggerFilter

FILE NAME             : TimestampLoggerFilter.h

DESCRIPTION           : Filter that logs media sample timestamps and durations to file specified using IFileSourceFilter interface
                     
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
#include <streams.h>

#include <fstream>
#include <vector>

// {DD467633-CFFA-4db7-8213-FE260668CD7F}
static const GUID CLSID_RTVCTimeStampLoggerFilter = 
{ 0xdd467633, 0xcffa, 0x4db7, { 0x82, 0x13, 0xfe, 0x26, 0x6, 0x68, 0xcd, 0x7f } };

// Forward declarations

/**
 * \ingroup DirectShowFilters
 * Filter that logs media sample timestamps and durations 
 * to file specified using IFileSourceFilter interface
 */
class TimestampLoggerFilter : public CTransInPlaceFilter,
							  public IFileSourceFilter
{
public:
	DECLARE_IUNKNOWN

	/// Constructor
	TimestampLoggerFilter(LPUNKNOWN pUnk, HRESULT *pHr);
	/// Destructor
	~TimestampLoggerFilter();

	/// Static object-creation method (for the class factory)
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr); 

	/// override this to publicize our interfaces
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

	/// From IFileSourceFilter
	STDMETHODIMP Load(LPCOLESTR lpwszFileName, const AM_MEDIA_TYPE *pmt);
	/// From IFileSourceFilter
	STDMETHODIMP GetCurFile(LPOLESTR * ppszFileName, AM_MEDIA_TYPE *pmt);

	//Overriding various CTransInPlace methods
	HRESULT Transform(IMediaSample *pSample);/* Overrriding the receive method. 
	This method receives a media sample, processes it, and delivers it to the downstream filter.*/

	HRESULT CheckInputType(const CMediaType* mtIn);

	STDMETHODIMP Run( REFERENCE_TIME tStart );
	STDMETHODIMP Stop(void);

private:
	std::string m_sFileName;	
	std::ofstream m_outStream;

	struct TimeInfo
	{
		TimeInfo(REFERENCE_TIME tStart, REFERENCE_TIME tStop, unsigned uiSize)
			:StartTime(tStart), StopTime(tStop), Size(uiSize)
		{;}
		REFERENCE_TIME StartTime;
		REFERENCE_TIME StopTime;
    unsigned Size;
	};

	std::vector<TimeInfo> m_vTimeInfo;
};