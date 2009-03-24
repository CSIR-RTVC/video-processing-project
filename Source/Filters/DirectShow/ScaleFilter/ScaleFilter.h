/** @file

MODULE				: ScaleFilter

FILE NAME			: ScaleFilter.h

DESCRIPTION			: Scale Filter for RGB24 and YUV420P Media
					Scale parameters are settable via the ISettingsInterface COM interface.
					  
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
#pragma once

#include <DirectShow/CustomBaseFilter.h>

// {60E0821D-9E6C-48f4-8E1C-1B25E67488F8}
static const GUID CLSID_ScaleFilter = 
{ 0x60e0821d, 0x9e6c, 0x48f4, { 0x8e, 0x1c, 0x1b, 0x25, 0xe6, 0x74, 0x88, 0xf8 } };

// {1DE541CE-7FF7-47be-9CE7-055E6C2BD53F}
static const GUID CLSID_ScaleProperties = 
{ 0x1de541ce, 0x7ff7, 0x47be, { 0x9c, 0xe7, 0x5, 0x5e, 0x6c, 0x2b, 0xd5, 0x3f } };


#ifdef _BUILD_FOR_SHORT
typedef short yuvType;
#else
typedef signed char yuvType ;
#endif

// Forward declarations
class PicScalerBase;

class CScaleFilter : public CCustomBaseFilter,
					public ISpecifyPropertyPages
{
public:
	DECLARE_IUNKNOWN;

	/// Constructor
	CScaleFilter();
	/// Destructor
	~CScaleFilter();

	/// Static object-creation method (for the class factory)
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr); 

	/**
	* Overriding this so that we can set whether this is an RGB24 or an RGB32 Filter
	*/
	HRESULT SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt);

	/**
	* Used for Media Type Negotiation 
	* Returns an HRESULT value. Possible values include those shown in the following table.
	* <table border="0" cols="2"><tr valign="top"><td><b>Value</b></td><td><b>Description</b></td></TR><TR><TD>S_OK</TD><TD>Success</TD></TR><TR><TD>VFW_S_NO_MORE_ITEMS</TD><TD>Index out of range</TD></TR><TR><TD>E_INVALIDARG</TD><TD>Index less than zero</TD></TR></TABLE>
	* The output pin's CTransformOutputPin::GetMediaType method calls this method. The derived class must implement this method. For more information, see CBasePin::GetMediaType.
	* To use custom media types, the media type can be manipulated in this method.
	*/
	HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);

	/// Buffer Allocation
	/**
	* The output pin's CTransformOutputPin::DecideBufferSize method calls this method. The derived class must implement this method. For more information, see CBaseOutputPin::DecideBufferSize. 
	* @param pAlloc Pointer to the IMemAllocator interface on the output pin's allocator.
	* @param pProp Pointer to an ALLOCATOR_PROPERTIES structure that contains buffer requirements from the downstream input pin.
	* @return Value: Returns S_OK or another HRESULT value.
	*/
	HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProp);

	/**
	* The CheckTransform method checks whether an input media type is compatible with an output media type.
	* <table border="0" cols="2"> <tr valign="top"> <td  width="50%"><b>Value</b></td> <td width="50%"><b>Description</b></td> </tr> <tr valign="top"> <td width="50%">S_OK</td> <td width="50%">The media types are compatible.</td> </tr> <tr valign="top"> <td width="50%">VFW_E_TYPE_NOT_ACCEPTED</td> <td width="50%">The media types are not compatible.</td> </tr> </table>
	*/
	HRESULT CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut);

	/// Overridden from CCustomBaseFilter
	virtual void InitialiseInputTypes();

	///Overridden from CSettingsInterface
	virtual void initParameters()
	{
		addParameter(TARGET_WIDTH, &m_nOutWidth, 0);
		addParameter(TARGET_HEIGHT, &m_nOutHeight, 0);
	}
	STDMETHODIMP SetParameter( const char* type, const char* value);

	STDMETHODIMP GetPages(CAUUID *pPages)
	{
		if (pPages == NULL) return E_POINTER;
		pPages->cElems = 1;
		pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
		if (pPages->pElems == NULL) 
		{
			return E_OUTOFMEMORY;
		}
		pPages->pElems[0] = CLSID_ScaleProperties;
		return S_OK;
	}

	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv)
	{
		if (riid == IID_ISpecifyPropertyPages)
		{
			return GetInterface(static_cast<ISpecifyPropertyPages*>(this), ppv);
		}
		else
		{
			// Call the parent class.
			return CCustomBaseFilter::NonDelegatingQueryInterface(riid, ppv);
		}
	}
private:
	/**
	* This method converts the input buffer from RGB24 | 32 to YUV420P
	* @param pSource The source buffer
	* @param pDest The destination buffer
	*/
	DWORD ApplyTransform(BYTE* pBufferIn, BYTE* pBufferOut);
	
	HRESULT RecalculateFilterParameters();

	/// Pointer to our picture scaler
	PicScalerBase* m_pScaler;
	/// Stores bytes per needed to store pixel according to media type
	double m_nBytesPerPixel;
};
