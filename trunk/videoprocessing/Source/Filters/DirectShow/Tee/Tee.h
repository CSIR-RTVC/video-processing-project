/** @file

MODULE				: Tee

FILE NAME			: Tee.h

DESCRIPTION			: Basic Tee filter implementation demonstrating how the usage of the CMultiIOBaseFilter class
					NOTE: This filter will not handle ALL media types and has been created for demonstration purposes.
					Use the standard DirectShow tee in applications.
					  
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

// {2BA06815-01F7-43fd-B1F3-55354400E849}
static const GUID CLSID_RtvcTee = 
{ 0x2ba06815, 0x1f7, 0x43fd, { 0xb1, 0xf3, 0x55, 0x35, 0x44, 0x0, 0xe8, 0x49 } };

class CRtvcTee : public CMultiIOBaseFilter
{
public:
	/// Constructor
	CRtvcTee();
	/// Destructor
	~CRtvcTee();

	/// Static object-creation method (for the class factory)
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr); 

	virtual HRESULT Receive(IMediaSample *pSample, int nIndex );
	
	virtual int InitialNumberOfInputPins() {return 1;}
	virtual int InitialNumberOfOutputPins() { return 1;}
	virtual bool OnFullCreateMoreInputs() { return false;}
	virtual bool OnFullCreateMoreOutputs() { return true; }
	virtual void InitialiseInputTypes();
	virtual void InitialiseOutputTypes();

	///  From CBasePin
	virtual HRESULT CheckOutputType(const CMediaType* pMediaType);


	/// From CBaseOutputPin
	virtual HRESULT DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pRequestProperties, int m_nIndex);

	/// To Check media type
	virtual HRESULT GetMediaType(int iPosition, CMediaType* pmt, int nOutputPinIndex) ;

protected:

};
