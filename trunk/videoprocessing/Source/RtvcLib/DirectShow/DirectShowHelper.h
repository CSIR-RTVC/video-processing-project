/** @file

MODULE				: DirectShowHelper

FILE NAME			: DirectShowHelper.h

DESCRIPTION			: DirectShow helper methods based on SDK examples
					  
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
#pragma once

#pragma warning(push)     // disable for this header only
#pragma warning(disable:4312)
// DirectShow
#include <streams.h>
#pragma warning(pop)      // restore original warning level

#include <vector>
#include <string>

using namespace std;

/*! \var typedef unsigned int UINT32
\brief A type definition for a .
Details.
*/
typedef std::vector<std::string> FILTER_NAME_LIST;

/**
 * \ingroup DirectShowLib
 * Helper class that provides static DirectShow related helper methods for generic DirectShow functionality
 */
class CDirectShowHelper
{
public:
	CDirectShowHelper();

	/// Find an unconnected pin on a filter.
	/// This too is stolen from the DX9 SDK.
	static HRESULT GetUnconnectedPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin);

	/// Connect two filters together with the filter graph manager.
	/// Stolen from the DX9 SDK.
	/// This is the base version.
	static HRESULT ConnectFilters(IGraphBuilder *pGraph, IPin *pOut, IBaseFilter *pDest);

	/// Connect two filters together with the filter graph manager.
	/// Again, stolen from the DX9 SDK.
	/// This is an overloaded version.
	static HRESULT ConnectFilters(IGraphBuilder *pGraph, IBaseFilter *pSrc, IBaseFilter *pDest);

	/// Helper function to retrieve filters of a certain type using wide string
	static HRESULT GetFilterByCategory(IBaseFilter** gottaFilter, LPCWSTR matchName, GUID Category);
	
	/// Helper function to retrieve filters of a certain type using STL string.
	static HRESULT GetFilterByCategory(IBaseFilter** gottaFilter, std::string sMatchName, GUID Category);

	/// This code was also brazenly stolen from the DX9 SDK.
	/// Pass it a file name in wszPath, and it will save the filter graph to that file.
	static HRESULT SaveGraphFile(IGraphBuilder *pGraph, WCHAR *wszPath);

	/// Helper method to add filter to graph using GUID.
	/// Stolen from the DX9 SDK.
	static HRESULT AddFilterByCLSID(IGraphBuilder *pGraph, const GUID& clsid, LPCWSTR wszName, IBaseFilter **ppF);
	//static HRESULT AddFilterByCLSID(IGraphBuilder *pGraph, const GUID& clsid, std::string sName, IBaseFilter **ppF);

	/// Helper method to add filter to graph using category GUID and name.
	static HRESULT AddFilterByCategoryAndName(IGraphBuilder *pGraph, const GUID& category, LPCWSTR wszName, IBaseFilter **ppF);
	/// Helper method to add filter to graph using category GUID and name.
	static HRESULT AddFilterByCategoryAndName(IGraphBuilder *pGraph, const GUID& category, std::string sName, IBaseFilter **ppF);

	/// Show the property pages for a filter.
	/// This is stolen from the DX9 SDK.
	HRESULT ShowFilterPropertyPages(IBaseFilter *pFilter);

	/// Retrieves filter names in a vector filtered by category.
	static HRESULT GetFilterNamesByCategory(GUID Category, FILTER_NAME_LIST& vList);
	/// Retrieves video device filter names in a vector.
	static HRESULT GetVideoInputs(FILTER_NAME_LIST& vList);
	/// Retrieves audio device filter names in a vector.
	static HRESULT GetAudioInputs(FILTER_NAME_LIST& vList);

	/// Helper method to retrieve pin from filter.
	static HRESULT GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir, IPin **ppPin);

	static HRESULT AddGraphToRot(IUnknown *pUnkGraph, DWORD *pdwRegister);

	static void RemoveGraphFromRot(DWORD pdwRegister);

	static HRESULT Render(IGraphBuilder *pGraph, IBaseFilter *pFilter);
};
