/** @file

MODULE				: H264

FILE NAME			: DLLSetup.cpp

DESCRIPTION			: H.264 decoder filter registration
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2012, CSIR
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

#include "H264DecoderFilter.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

//////////////////////////////////////////////////////////////////////////
//###############################  Standard Filter DLL Code ###############################
static const WCHAR g_wszName[] = L"CSIR VPP H264 Decoder";   /// A name for the filter 

const AMOVIESETUP_MEDIATYPE sudMediaTypes[] = 
{
	{ 
		&MEDIATYPE_Video, &MEDIASUBTYPE_VPP_H264
	},
	{ 
		&MEDIATYPE_Video, &MEDIASUBTYPE_RGB24 
	},
};

const AMOVIESETUP_PIN sudPin[] = 
{
	//RG 01/04/trying different input type again
	//{
	//	L"",            // Obsolete, not used.
	//		FALSE,          // Is this pin rendered?
	//		FALSE,           // Is it an output pin?
	//		FALSE,          // Can the filter create zero instances?
	//		FALSE,          // Does the filter create multiple instances?
	//		&GUID_NULL,     // Obsolete.
	//		NULL,           // Obsolete.
	//		1,              // Number of media types.
	//		&sudMediaTypes[1]   // Pointer to media types.
	//},
	// RG Before testing with media type stream
	{
		L"",                // Obsolete, not used.
			FALSE,            // Is this pin rendered?
			FALSE,            // Is it an output pin?
			FALSE,            // Can the filter create zero instances?
			FALSE,            // Does the filter create multiple instances?
			&GUID_NULL,       // Obsolete.
			NULL,             // Obsolete.
			1,                // Number of media types.
			&sudMediaTypes[0] // Pointer to media types.
	},
	{
		L"",                // Obsolete, not used.
			FALSE,            // Is this pin rendered?
			TRUE,             // Is it an output pin?
			FALSE,            // Can the filter create zero instances?
			FALSE,            // Does the filter create multiple instances?
			&GUID_NULL,       // Obsolete.
			NULL,             // Obsolete.
			1,                // Number of media types.
			&sudMediaTypes[1] // Pointer to media types.
	}
};

// The next bunch of structures define information for the class factory.
AMOVIESETUP_FILTER FilterInfo =
{
	&CLSID_RTVC_VPP_H264Decoder,  // CLSID
	g_wszName,							  // Name
	MERIT_NORMAL,					    // Merit
	2,									      // Number of AMOVIESETUP_PIN structs
	sudPin								    // Pin registration information.
};


CFactoryTemplate g_Templates[1] = 
{
	{ 
		g_wszName,							            // Name
		&CLSID_RTVC_VPP_H264Decoder,        // CLSID
		H264DecoderFilter::CreateInstance,	// Method to create an instance of MyComponent
		NULL,								                // Initialization function
		&FilterInfo							            // Set-up information (for filters)
	}
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);   

// Code to reg in Video Compressor category
REGFILTER2 rf2FilterReg = {
	1,              // Version 1 (no pin mediums or pin category).
	MERIT_NORMAL,   // Merit.
	1,              // Number of pins.
	sudPin          // Pointer to pin information.
};

STDAPI DllRegisterServer(void)
{
	return AMovieDllRegisterServer2(TRUE);

#if 0
	// trying to register in "Video Compressor" category
	HRESULT hr;
	IFilterMapper2 *pFM2 = NULL;

	hr = AMovieDllRegisterServer2(TRUE);
	if (FAILED(hr))
		return hr;

	hr = CoCreateInstance(CLSID_FilterMapper2, NULL, CLSCTX_INPROC_SERVER,
		IID_IFilterMapper2, (void **)&pFM2);

	if (FAILED(hr))
		return hr;

	hr = pFM2->RegisterFilter(
		CLSID_RTVC_H264Decoder,          // Filter CLSID. 
		g_wszName,                       // Filter name.
		NULL,                            // Device moniker. 
		&CLSID_VideoCompressorCategory,  // Video compressor category.
		g_wszName,                       // Instance data.
		&rf2FilterReg                    // Pointer to filter information.
		);
	pFM2->Release();
	return hr;
#endif
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2(FALSE);
}

//DLL Entry point
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);
BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}

#ifdef _MANAGED
#pragma managed(pop)
#endif