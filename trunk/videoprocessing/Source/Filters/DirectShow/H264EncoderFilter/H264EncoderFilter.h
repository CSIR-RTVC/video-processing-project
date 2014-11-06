/** @file

MODULE				: H264

FILE NAME			: H264EncoderFilter.h

DESCRIPTION			: DirectShow H.264 encoder filter header

LICENSE	: GNU Lesser General Public License

Copyright (c) 2008 - 2012, CSIR
All rights reserved.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

===========================================================================
*/
#pragma once

// Meraka includes
#include <DirectShow/CodecControlInterface.h>
#include <DirectShow/CustomBaseFilter.h>
#include <DirectShow/CustomMediaTypes.h>
#include <DirectShow/NotifyCodes.h>
#include <Filters/DirectShow/FilterParameters.h>

// Forward
class ICodecv2;

// {28D61FDF-2646-422D-834C-EFFF45884A36}
static const GUID CLSID_RTVC_VPP_H264Encoder = 
{ 0x28d61fdf, 0x2646, 0x422d, { 0x83, 0x4c, 0xef, 0xff, 0x45, 0x88, 0x4a, 0x36 } };

// {8B7A757C-EE7D-4D7A-A9E2-B318A9EEB53E}
static const GUID CLSID_H264Properties = 
{ 0x8b7a757c, 0xee7d, 0x4d7a, { 0xa9, 0xe2, 0xb3, 0x18, 0xa9, 0xee, 0xb5, 0x3e } };

class H264EncoderFilter : public CCustomBaseFilter,
                          public ISpecifyPropertyPages,
                          public ICodecControlInterface
{
public:
  DECLARE_IUNKNOWN

  /**
   * @brief Constructor
   */
  H264EncoderFilter();
  /**
   * @brief Destructor
   */
  ~H264EncoderFilter();
  /**
   * @brief Static object-creation method (for the class factory)
   */
  static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr); 
  /**
   * @brief Overriding this so that we can set whether this is an RGB24 or an RGB32 Filter
   */
  HRESULT SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt);
  /**
   * @brief Used for Media Type Negotiation 
   * Returns an HRESULT value. Possible values include those shown in the following table.
   * <table border="0" cols="2"><tr valign="top"><td><b>Value</b></td><td><b>Description</b></td></TR><TR><TD>S_OK</TD><TD>Success</TD></TR><TR><TD>VFW_S_NO_MORE_ITEMS</TD><TD>Index out of range</TD></TR><TR><TD>E_INVALIDARG</TD><TD>Index less than zero</TD></TR></TABLE>
   * The output pin's CTransformOutputPin::GetMediaType method calls this method. The derived class must implement this method. For more information, see CBasePin::GetMediaType.
   * To use custom media types, the media type can be manipulated in this method.
   */
  HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
  /**
   * @brief The output pin's CTransformOutputPin::DecideBufferSize method calls this method. The derived class must implement this method. For more information, see CBaseOutputPin::DecideBufferSize. 
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
  /**
   * @brief Overridden from CSettingsInterface
   */
  virtual void initParameters()
  {
    addParameter(FRAME_BIT_LIMIT, &m_nFrameBitLimit, 0);
    addParameter(NOTIFYONIFRAME, &m_bNotifyOnIFrame, false);
    addParameter(IFRAME_PERIOD, &m_uiIFramePeriod, 0);
    addParameter(SPS, &m_sSeqParamSet, "", true); // read-only
    addParameter(PPS, &m_sPicParamSet, "", true); // read-only
    addParameter(H264_TYPE, &m_nH264Type, H264_VPP);
  }
  /**
   * @brief Overridden from CSettingsInterface
   */
  STDMETHODIMP GetParameter( const char* szParamName, int nBufferSize, char* szValue, int* pLength );
  /**
   * @brief Overridden from CSettingsInterface
   */
  STDMETHODIMP SetParameter( const char* type, const char* value);
  /**
   * @brief Overridden from CSettingsInterface
   */
  STDMETHODIMP GetParameterSettings( char* szResult, int nSize );
  /**
   * @brief Overridden from ICodecControlInterface
   */
  STDMETHODIMP SetFramebitLimit(unsigned uiFrameBitLimit);
  /**
   * @brief Overridden from ICodecControlInterface
   */
  STDMETHODIMP GenerateIdr();
  /**
   * @brief Property pages
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
    pPages->pElems[0] = CLSID_H264Properties;
    return S_OK;
  }
  /**
   * @brief query interface
   */
  STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv)
  {
    if (riid == IID_ISpecifyPropertyPages)
    {
      return GetInterface(static_cast<ISpecifyPropertyPages*>(this), ppv);
    }
    else if (riid == IID_ICodecControlInterface)
    {
      return GetInterface(static_cast<ICodecControlInterface*>(this), ppv);
    }
    else
    {
      // Call the parent class.
      return CCustomBaseFilter::NonDelegatingQueryInterface(riid, ppv);
    }
  }
  /**
   * @brief Overridden from CCustomBaseFilter
   */
  virtual void InitialiseInputTypes();

private:
  /**
   * @brief This method copies the h.264 sequence and picture parameter sets into the passed in buffer
   * and returns the total length including start codes
   */
  unsigned copySequenceAndPictureParameterSetsIntoBuffer(BYTE* pBuffer);
  unsigned getParameterSetLength() const;

  /**
   * @brief This method encodes the frame to H.264
   * @param[in] pBufferIn The source buffer
   * @param[in] lInBufferSize The size of the in buffer
   * @param[in] lActualDataLength The size of the actual frame inside the in buffer
   * @param[in,out] pBufferOut The destination buffer
   * @param[in] lOutBufferSize The size of the destination buffer
   * @param[in,out] lOutActualDataLength The size of the data encoded into the destination buffer
   */
  virtual HRESULT ApplyTransform(BYTE* pBufferIn, long lInBufferSize, long lActualDataLength, BYTE* pBufferOut, long lOutBufferSize, long& lOutActualDataLength);
  
  int m_nH264Type;

  ICodecv2* m_pCodec;
  /// Receive Lock
  CCritSec m_csCodec;
  int m_nFrameBitLimit;
  bool m_bNotifyOnIFrame;
  unsigned char* m_pSeqParamSet;
  unsigned m_uiSeqParamSetLen;
  unsigned char* m_pPicParamSet;
  unsigned m_uiPicParamSetLen;
  std::string m_sSeqParamSet;
  std::string m_sPicParamSet;
  /// For auto i-frame generation
  unsigned m_uiIFramePeriod;
  /// frame counter for i-frame generation
  unsigned m_uiCurrentFrame;
};
