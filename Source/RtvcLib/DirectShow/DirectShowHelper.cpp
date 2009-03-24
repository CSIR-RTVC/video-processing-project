/** @file

MODULE				: DirectShowHelper

FILE NAME			: DirectShowHelper.cpp

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
#include "stdafx.h"
#include "DirectShowHelper.h"
#include <Shared/StringUtil.h>


CDirectShowHelper::CDirectShowHelper()
{;}

HRESULT CDirectShowHelper::GetUnconnectedPin( IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin)
{
	*ppPin = 0;
	IEnumPins *pEnum = 0;
	IPin *pPin = 0;
	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (FAILED(hr))
	{
		return hr;
	}
	while (pEnum->Next(1, &pPin, NULL) == S_OK)
	{
		PIN_DIRECTION ThisPinDir;
		pPin->QueryDirection(&ThisPinDir);
		if (ThisPinDir == PinDir)
		{
			IPin *pTmp = 0;
			hr = pPin->ConnectedTo(&pTmp);
			if (SUCCEEDED(hr))  // Already connected, not the pin we want.
			{
				pTmp->Release();
			}
			else  // Unconnected, this is the pin we want.
			{
				pEnum->Release();
				*ppPin = pPin;
				return S_OK;
			}
		}
		pPin->Release();
	}
	pEnum->Release();
	// Did not find a matching pin.
	return E_FAIL;
}

HRESULT CDirectShowHelper::ConnectFilters( IGraphBuilder *pGraph, IPin *pOut, IBaseFilter *pDest )
{
	if ((pGraph == NULL) || (pOut == NULL) || (pDest == NULL))
	{
		return E_POINTER;
	}

	// Find an input pin on the downstream filter.
	IPin *pIn = 0;
	HRESULT hr = CDirectShowHelper::GetUnconnectedPin(pDest, PINDIR_INPUT, &pIn);
	if (FAILED(hr))
	{
		return hr;
	}
	// Try to connect them.
	hr = pGraph->Connect(pOut, pIn);
	pIn->Release();
	return hr;
}

HRESULT CDirectShowHelper::ConnectFilters( IGraphBuilder *pGraph, IBaseFilter *pSrc, IBaseFilter *pDest )
{
	if ((pGraph == NULL) || (pSrc == NULL) || (pDest == NULL))
	{
		return E_POINTER;
	}

	// Find an output pin on the first filter.
	IPin *pOut = 0;
	HRESULT hr = CDirectShowHelper::GetUnconnectedPin(pSrc, PINDIR_OUTPUT, &pOut);
	if (FAILED(hr)) 
	{
		return hr;
	}
	hr = ConnectFilters(pGraph, pOut, pDest);
	pOut->Release();
	return hr;
}

//HRESULT CDirectShowHelper::GetFilterByCategory( IBaseFilter** gottaFilter, wchar_t* matchName, GUID Category )
HRESULT CDirectShowHelper::GetFilterByCategory( IBaseFilter** gottaFilter, LPCWSTR matchName, GUID Category )
{
	BOOL done = false;

	// Create the System Device Enumerator.
	ICreateDevEnum *pSysDevEnum = NULL;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
	{
		return hr;
	}

	// Obtain a class enumerator for the audio input category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(Category, &pEnumCat, 0);

	if (hr == S_OK) 
	{
		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		while ((pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK) && (!done))
		{
			// Bind the first moniker to an object
			IPropertyBag *pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, 
				(void **)&pPropBag);
			if (SUCCEEDED(hr))
			{
				// To retrieve the filter's friendly name, do the following:
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if (SUCCEEDED(hr))
				{
					//wprintf(L"Testing Device: %s\n", varName.bstrVal);

					// Do a comparison, find out if it's the right one
					if (wcsncmp(varName.bstrVal, matchName, 
						wcslen(matchName)) == 0) {

							// We found it, so send it back to the caller
							hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**) gottaFilter);
							done = true;
					}
				}
				VariantClear(&varName);	
				pPropBag->Release();
			}
			pMoniker->Release();
		}
		pEnumCat->Release();
	}
	pSysDevEnum->Release();
	if (done) {
		return hr;	// found it, return native error
	} else {
		return VFW_E_NOT_FOUND;	// didn't find it error
	}
}

HRESULT CDirectShowHelper::GetFilterByCategory( IBaseFilter** gottaFilter, std::string sMatchName, GUID Category )
{
	WCHAR* wszName = StringUtil::stlToWide(sMatchName);
	HRESULT hr = CDirectShowHelper::GetFilterByCategory(gottaFilter, wszName, Category);
	delete wszName;
	return hr;
}

HRESULT CDirectShowHelper::SaveGraphFile( IGraphBuilder *pGraph, WCHAR *wszPath )
{
	const WCHAR wszStreamName[] = L"ActiveMovieGraph"; 
	HRESULT hr;
	IStorage *pStorage = NULL;

	// First, create a document file which will hold the GRF file
	hr = StgCreateDocfile(
		wszPath,
		STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
		0, &pStorage);
	if(FAILED(hr)) 
	{
		return hr;
	}

	// Next, create a stream to store.
	IStream *pStream;
	hr = pStorage->CreateStream(
		wszStreamName,
		STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
		0, 0, &pStream);
	if (FAILED(hr)) 
	{
		pStorage->Release();    
		return hr;
	}

	// The IPersistStream converts a stream into a persistent object.
	IPersistStream *pPersist = NULL;
	pGraph->QueryInterface(IID_IPersistStream, reinterpret_cast<void**>(&pPersist));
	hr = pPersist->Save(pStream, TRUE);
	pStream->Release();
	pPersist->Release();
	if (SUCCEEDED(hr)) 
	{
		hr = pStorage->Commit(STGC_DEFAULT);
	}
	pStorage->Release();
	return hr;
}

HRESULT CDirectShowHelper::AddFilterByCLSID( IGraphBuilder *pGraph, const GUID& clsid, LPCWSTR wszName, IBaseFilter **ppF )
{
	if (!pGraph || ! ppF) return E_POINTER;
	*ppF = 0;
	IBaseFilter *pF = 0;
	HRESULT hr = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, reinterpret_cast<void**>(&pF));
	if (SUCCEEDED(hr))
	{
		hr = pGraph->AddFilter(pF, wszName);
		if (SUCCEEDED(hr))
			*ppF = pF;
		else
			pF->Release();
	}
	return hr;
}

HRESULT CDirectShowHelper::ShowFilterPropertyPages( IBaseFilter *pFilter )
{
	/* Obtain the filter's IBaseFilter interface. (Not shown) */
	ISpecifyPropertyPages *pProp;
	HRESULT hr = pFilter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pProp);
	if (SUCCEEDED(hr)) 
	{
		// Get the filter's name and IUnknown pointer.
		FILTER_INFO FilterInfo;
		hr = pFilter->QueryFilterInfo(&FilterInfo); 
		IUnknown *pFilterUnk;
		pFilter->QueryInterface(IID_IUnknown, (void **)&pFilterUnk);

		// Show the page. 
		CAUUID caGUID;
		pProp->GetPages(&caGUID);
		pProp->Release();
		OleCreatePropertyFrame(
			NULL,                   // Parent window
			0, 0,                   // Reserved
			FilterInfo.achName,     // Caption for the dialog box
			1,                      // Number of objects (just the filter)
			&pFilterUnk,            // Array of object pointers. 
			caGUID.cElems,          // Number of property pages
			caGUID.pElems,          // Array of property page CLSIDs
			0,                      // Locale identifier
			0, NULL                 // Reserved
			);

		// Clean up.
		pFilterUnk->Release();
		FilterInfo.pGraph->Release(); 
		CoTaskMemFree(caGUID.pElems);
	}
	return hr;
}

HRESULT CDirectShowHelper::GetFilterNamesByCategory( GUID Category, FILTER_NAME_LIST& vFilters )
{
	BOOL done = false;
	// Create the System Device Enumerator.
	ICreateDevEnum *pSysDevEnum = NULL;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
	{
		return hr;
	}

	// Obtain a class enumerator for the audio input category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(Category, &pEnumCat, 0);

	if (hr == S_OK) 
	{
		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		while ((pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK) && (!done))
		{
			// Bind the first moniker to an object
			IPropertyBag *pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, 
				(void **)&pPropBag);
			if (SUCCEEDED(hr))
			{
				// To retrieve the filter's friendly name, do the following:
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if (SUCCEEDED(hr))
				{
					LPCWSTR mychar; // narrow string
					mychar = (LPCWSTR) (varName.bstrVal);
					//TODO
					vFilters.push_back(StringUtil::wideToStl(mychar));
					//USES_CONVERSION;
					//char* szTempBuffer = W2A(mychar);
					//szTempBuffer  = W2A(mychar);
					//vFilters.push_back(std::string(szTempBuffer));
				}
				VariantClear(&varName);	
				pPropBag->Release();
			}
			pMoniker->Release();
		}
		pEnumCat->Release();
	}
	pSysDevEnum->Release();
	return S_OK;
}


HRESULT CDirectShowHelper::GetVideoInputs(FILTER_NAME_LIST& vList )
{
	return GetFilterNamesByCategory(CLSID_VideoInputDeviceCategory, vList);
}

HRESULT CDirectShowHelper::GetAudioInputs(FILTER_NAME_LIST& vList)
{
	return GetFilterNamesByCategory(CLSID_AudioInputDeviceCategory, vList);
}

HRESULT CDirectShowHelper::AddFilterByCategoryAndName( IGraphBuilder *pGraph, const GUID& category, LPCWSTR wszName, IBaseFilter **ppF )
{
	HRESULT hr = CDirectShowHelper::GetFilterByCategory(ppF, wszName, category);
	if (SUCCEEDED(hr)) 
	{
		hr = pGraph->AddFilter(*ppF, wszName);
	}
	else
	{
		return hr;
	}
}

HRESULT CDirectShowHelper::AddFilterByCategoryAndName( IGraphBuilder *pGraph, const GUID& category, std::string sName, IBaseFilter **ppF )
{
	WCHAR* wszName = StringUtil::stlToWide(sName);
	HRESULT hr = CDirectShowHelper::AddFilterByCategoryAndName(pGraph, category, wszName, ppF);
	delete[] wszName;
	return hr;	
}

HRESULT CDirectShowHelper::GetPin( IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin )
{
	IEnumPins  *pEnum = NULL;
	IPin       *pPin = NULL;
	HRESULT    hr;

	if (ppPin == NULL)
	{
		return E_POINTER;
	}

	hr = pFilter->EnumPins(&pEnum);
	if (FAILED(hr))
	{
		return hr;
	}
	while(pEnum->Next(1, &pPin, 0) == S_OK)
	{
		PIN_DIRECTION PinDirThis;
		hr = pPin->QueryDirection(&PinDirThis);
		if (FAILED(hr))
		{
			pPin->Release();
			pEnum->Release();
			return hr;
		}
		if (PinDir == PinDirThis)
		{
			// Found a match. Return the IPin pointer to the caller.
			*ppPin = pPin;
			pEnum->Release();
			return S_OK;
		}
		// Release the pin for the next time through the loop.
		pPin->Release();
	}
	// No more pins. We did not find a match.
	pEnum->Release();
	return E_FAIL;
}
