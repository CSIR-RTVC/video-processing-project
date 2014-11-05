/** @file

MODULE				: OpusDecoderFilter

FILE NAME			: OpusDecoderFilter.h

DESCRIPTION			:

LICENSE: Software License Agreement (BSD License)

Copyright (c) 2014, CSIR
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
#include <DirectShow/CustomBaseFilter.h>
#include <DirectShow/CustomMediaTypes.h>
#include <DirectShow/NotifyCodes.h>
#include <Filters/DirectShow/FilterParameters.h>

// Forward
class ICodecv2;

// {D235A2D7-39FA-41E1-BB36-09D62EB394DD}
static const GUID CLSID_VPP_OpusDecoder =
{ 0xd235a2d7, 0x39fa, 0x41e1, { 0xbb, 0x36, 0x9, 0xd6, 0x2e, 0xb3, 0x94, 0xdd } };

// {01B34370-1AB1-44FA-B1EF-4BB4B4EB8BE6}
static const GUID CLSID_OpusProperties =
{ 0x1b34370, 0x1ab1, 0x44fa, { 0xb1, 0xef, 0x4b, 0xb4, 0xb4, 0xeb, 0x8b, 0xe6 } };


class OpusDecoderFilter : public CCustomBaseFilter
{
public:
	/**
   * @brief Constructor
   */
	OpusDecoderFilter();
	/**
   * @brief Destructor
   */
	~OpusDecoderFilter();
	/**
   * @brief Static object-creation method (for the class factory)
   */
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr); 
#if 0
  /**
   * @brief Overridden from CCustomBaseFilter to split an incoming
   * media sample into multiple outgoing ones.
   */
  virtual HRESULT Receive(IMediaSample *pSample);
#endif
	/**
 	 * Overriding this to set outgoing media type
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
 	 * The CheckTransform method checks whether an input media type is compatible with an output media type.
	 * <table border="0" cols="2"> <tr valign="top"> <td  width="50%"><b>Value</b></td> <td width="50%"><b>Description</b></td> </tr> <tr valign="top"> <td width="50%">S_OK</td> <td width="50%">The media types are compatible.</td> </tr> <tr valign="top"> <td width="50%">VFW_E_TYPE_NOT_ACCEPTED</td> <td width="50%">The media types are not compatible.</td> </tr> </table>
	 */
	HRESULT CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut);

	/// Interface methods
	///Overridden from CSettingsInterface
	virtual void initParameters()
	{
	}

	/// Overridden from SettingsInterface
	STDMETHODIMP GetParameter( const char* szParamName, int nBufferSize, char* szValue, int* pLength );
	STDMETHODIMP SetParameter( const char* type, const char* value);
	STDMETHODIMP GetParameterSettings( char* szResult, int nSize );

	/// Overridden from CCustomBaseFilter
	virtual void InitialiseInputTypes();

private:
	/**
   * This method has to be overridden from CCustomBaseFilter.
   * It is not used in this class!
	 */
	virtual HRESULT ApplyTransform(BYTE* pBufferIn, long lInBufferSize, long lActualDataLength, BYTE* pBufferOut, long lOutBufferSize, long& lOutActualDataLength);

	ICodecv2* m_pCodec;
};
