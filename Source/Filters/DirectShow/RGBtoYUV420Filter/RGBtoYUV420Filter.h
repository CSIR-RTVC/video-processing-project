/** @file

MODULE				: RGBtoYUV420Filter

FILE NAME			: RGBtoYUV420Filter.h

DESCRIPTION			: DirectShow Filter that converts RGB24 or RGB32 media to our custom packet YUV 420 format.

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

// {BB0980EB-97F6-439a-847E-B94C2F5EFCD7}
static const GUID CLSID_RGBtoYUV420ColorConverter = 
{ 0xbb0980eb, 0x97f6, 0x439a, { 0x84, 0x7e, 0xb9, 0x4c, 0x2f, 0x5e, 0xfc, 0xd7 } };
// {353FCF3B-910A-4e47-9D2B-C410D21B1792}
static const GUID CLSID_RGBtoYUV420Properties = 
{ 0x353fcf3b, 0x910a, 0x4e47, { 0x9d, 0x2b, 0xc4, 0x10, 0xd2, 0x1b, 0x17, 0x92 } };

// Parameters
#define CHROMINANCE_OFFSET "chrominanceoffset"
#define INVERT "invert"

// Forward declarations
class RGBtoYUV420Converter;

/**
 * \ingroup DirectShowFilters
 * DirectShow Filter that converts RGB24 or RGB32 media to I420 format.
 */
class RGBtoYUV420Filter : public CCustomBaseFilter,
                          public ISpecifyPropertyPages
{
public:
  DECLARE_IUNKNOWN

  /**
   * @brief Constructor
   */
  RGBtoYUV420Filter();
  /**
   * @brief Destructor
   */
  ~RGBtoYUV420Filter();
  /**
   * @brief Static object-creation method (for the class factory)
   */
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
  /**
   * The output pin's CTransformOutputPin::DecideBufferSize method calls this method. The derived class must implement this method. For more information, see CBaseOutputPin::DecideBufferSize. 
   * @param pAlloc Pointer to the IMemAllocator interface on the output pin's allocator.
   * @param pProp Pointer to an ALLOCATOR_PROPERTIES structure that contains buffer requirements from the downstream input pin.
   * @return Value: Returns S_OK or another HRESULT value.
   */
  HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProp);
  /**
   * @brief The CheckTransform method checks whether an input media type is compatible with an output media type.
   * <table border="0" cols="2"> <tr valign="top"> <td  width="50%"><b>Value</b></td> <td width="50%"><b>Description</b></td> </tr> <tr valign="top"> <td width="50%">S_OK</td> <td width="50%">The media types are compatible.</td> </tr> <tr valign="top"> <td width="50%">VFW_E_TYPE_NOT_ACCEPTED</td> <td width="50%">The media types are not compatible.</td> </tr> </table>
   */
  HRESULT CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut);
  /**
   * @brief Overridden from CCustomBaseFilter to define allowed input types
   */
  virtual void InitialiseInputTypes();
  /**
   * @brief Overridden from CSettingsInterface to intialise filter parameters
   */
  virtual void initParameters()
  {
    addParameter(CHROMINANCE_OFFSET, &m_nChrominanceOffset, 128);
#ifdef RTVC_INVERT_OPTIONAL
    // YUV is always top-down: give user option to not flip image
    // http://msdn.microsoft.com/en-us/library/windows/desktop/dd407212(v=vs.85).aspx
    addParameter(INVERT, &m_bInvert, true);
#endif
  }
  /**
   * @brief Sets the filter parameters
   */
  STDMETHODIMP SetParameter( const char* type, const char* value);
  /**
   * @brief required for property page
   */
  STDMETHODIMP GetPages(CAUUID *pPages)
  {
    if (pPages == NULL) return E_POINTER;
    pPages->cElems = 1;
    pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL) 
    {
      return E_OUTOFMEMORY;
    }
    pPages->pElems[0] = CLSID_RGBtoYUV420Properties;
    return S_OK;
  }
  /**
   * @brief required for COM
   */
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
  virtual HRESULT ApplyTransform(BYTE* pBufferIn, long lInBufferSize, long lActualDataLength, BYTE* pBufferOut, long lOutBufferSize, long& lOutActualDataLength);

  /// Pointer to our RGB24 | 32 to YUV420 converter
  RGBtoYUV420Converter* m_pConverter;
  /// Size of YUV image
  int m_nSizeYUV;
  /// Chrominance offset
  int m_nChrominanceOffset;
  /// invert image
  bool m_bInvert;
};
