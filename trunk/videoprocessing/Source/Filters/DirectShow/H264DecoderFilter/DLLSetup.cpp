// H264EncoderFilter.cpp : Defines the entry point for the DLL application.
//

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