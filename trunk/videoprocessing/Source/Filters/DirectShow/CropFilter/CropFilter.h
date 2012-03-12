/** @file

MODULE				: CropFilter

FILE NAME			: CropFilter.h

DESCRIPTION			: Cropfilter for RGB24 and RGB32 media. 
					Crop parameters can be set using the COM SettingsInterface SetParameter method.
					  
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

// CSIR includes
#include <DirectShow/CustomBaseFilter.h>

// {F5E69199-7AB2-40f0-BFC0-6938430F4F19}
static const GUID CLSID_CropFilter = 
{ 0xf5e69199, 0x7ab2, 0x40f0, { 0xbf, 0xc0, 0x69, 0x38, 0x43, 0xf, 0x4f, 0x19 } };

// {2E164ADE-2F13-4ee9-BA95-28CF9AE23511}
static const GUID CLSID_CropProperties = 
{ 0x2e164ade, 0x2f13, 0x4ee9, { 0xba, 0x95, 0x28, 0xcf, 0x9a, 0xe2, 0x35, 0x11 } };

// Forward declarations
class PicCropperBase;

/**
*  Crop filter for RGB24 and RGB32 media. Crop parameters can be set using the COM SettingsInterface SetParameter method.
*/
class CCropFilter : public CCustomBaseFilter,
					public ISpecifyPropertyPages
{
public:
	DECLARE_IUNKNOWN

	/// Constructor
	CCropFilter();
	/// Destructor
	~CCropFilter();

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

	/// Overridden from CSettingsInterface
	virtual void initParameters()
	{
		addParameter(TARGET_WIDTH, &m_nOutWidth, -1, true);
		addParameter(TARGET_HEIGHT, &m_nOutHeight, -1, true);
		addParameter(LEFT_CROP, &m_nLeftCrop, 0);
		addParameter(RIGHT_CROP, &m_nRightCrop, 0);
		addParameter(TOP_CROP, &m_nTopCrop, 0);
		addParameter(BOTTOM_CROP, &m_nBottomCrop, 0);
		RecalculateFilterParameters();
	}

	STDMETHODIMP SetParameter( const char* type, const char* value);

	/// Overridden from CCustomBaseFilter
	virtual void InitialiseInputTypes();

	STDMETHODIMP GetPages(CAUUID *pPages)
	{
		if (pPages == NULL) return E_POINTER;
		pPages->cElems = 1;
		pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
		if (pPages->pElems == NULL) 
		{
			return E_OUTOFMEMORY;
		}
		pPages->pElems[0] = CLSID_CropProperties;
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
  virtual void ApplyTransform(BYTE* pBufferIn, long lInBufferSize, long lActualDataLength, BYTE* pBufferOut, long lOutBufferSize, long& lOutActualDataLength);

	/// Sets crop if values are valid
	HRESULT SetCropIfValid(int nTotalDimensionImage, int nNewCrop, int& nOldCrop, int nOppositeCrop  );
	/// This method calculates how much should be cropped from the picture based on the crop ratios
	void RecalculateFilterParameters();

	/// Pointer to our picture cropper
	PicCropperBase* m_pCropper;

	int m_nBytesPerPixel;
	//Member variables which affect the transform
	int m_nLeftCrop;
	int m_nRightCrop;
	int m_nTopCrop;
	int m_nBottomCrop;

	int m_nStride;
	int m_nPadding;
};