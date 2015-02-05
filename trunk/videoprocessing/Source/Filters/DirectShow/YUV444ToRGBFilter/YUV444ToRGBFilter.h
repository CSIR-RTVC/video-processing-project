/** @file

MODULE				: YUV444ToRGBFilter

FILE NAME			: YUV444ToRGBFilter.h

DESCRIPTION			: This Filter converts the YUV444 Interleaved media back into RGB24 media.

LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008 - 2014, CSIR
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

// Parameters
#define CHROMINANCE_OFFSET "chrominanceoffset"
#define INVERT "invert"

// GUID

// {5F513F58-4E58-4955-B987-2591D441590E}
static const GUID CLSID_VPP_YUV444toRGBColorConverter = 
{ 0x5f513f58, 0x4e58, 0x4955, { 0xb9, 0x87, 0x25, 0x91, 0xd4, 0x41, 0x59, 0xe } };

// {084B9704-01AF-48F2-A4A4-8EA6FB8B1519}
static const GUID CLSID_YUVtoRGB444Properties = 
{ 0x84b9704, 0x1af, 0x48f2, { 0xa4, 0xa4, 0x8e, 0xa6, 0xfb, 0x8b, 0x15, 0x19 } };

// Forward declarations
class YUV444toRGBConverter;

/**
* \ingroup DirectShowFilters
* This Filter converts the YUV444 Interleaved media back into RGB24 media.
*/
class YUV444toRGBFilter :   public CCustomBaseFilter,
  public ISpecifyPropertyPages
{
public:
  DECLARE_IUNKNOWN

  /// Constructor
  YUV444toRGBFilter();
  /// Destructor
  ~YUV444toRGBFilter();

  /// Static object-creation method (for the class factory)
  static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr); 

  /**
  * Overriding this so that we can set whether this is an RGB24 or an RGB32 Filter
  */
  HRESULT SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt);

  /**
  * Used for Media Type Negotiation.
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

  /// Overridden from CSettingsInterface
  STDMETHODIMP SetParameter( const char* type, const char* value);

  /// Overridden from CSettingsInterface
  virtual void initParameters()
  {
    addParameter(CHROMINANCE_OFFSET, &m_nChrominanceOffset, 128);
    addParameter(INVERT, &m_bInvert, true);
  }

  STDMETHODIMP GetPages(CAUUID *pPages)
  {
    if (pPages == NULL) return E_POINTER;
    pPages->cElems = 1;
    pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL) 
    {
      return E_OUTOFMEMORY;
    }
    pPages->pElems[0] = CLSID_YUVtoRGB444Properties;
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
protected:


private:
  /**
  * This method converts the input buffer from YUV444I to RGB32
  * @param pSource The source buffer
  * @param pDest The destination buffer
  */
  virtual HRESULT ApplyTransform(BYTE* pBufferIn, long lInBufferSize, long lActualDataLength, BYTE* pBufferOut, long lOutBufferSize, long& lOutActualDataLength);

  /// Pointer to our YUV444 to RGB24 converter
  YUV444toRGBConverter* m_pConverter;
  // Size of YUV image
  int m_nSizeUV;
  // Chrominance offset
  int m_nChrominanceOffset;
  // YUV is always top down: the invert flag determines whether the image
  // is flipped during conversion, or whether the sign is simply set
  bool m_bInvert;
};
