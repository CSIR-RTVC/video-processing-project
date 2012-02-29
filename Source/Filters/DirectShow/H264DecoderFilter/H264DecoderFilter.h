#pragma once

// Meraka includes
#include <DirectShow/CustomBaseFilter.h>
#include <DirectShow/CustomMediaTypes.h>
#include <DirectShow/NotifyCodes.h>

// {4F722923-7E4D-4263-926D-2A1F405B2619}
static const GUID CLSID_RTVC_VPP_H264Decoder = 
{ 0x4f722923, 0x7e4d, 0x4263, { 0x92, 0x6d, 0x2a, 0x1f, 0x40, 0x5b, 0x26, 0x19 } };

// Forward
class ICodecv2;

class H264DecoderFilter : public CCustomBaseFilter
{
public:
	/// Constructor
	H264DecoderFilter();
	/// Destructor
	~H264DecoderFilter();

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

	///Overridden from CSettingsInterface
	virtual void initParameters(){;}
	STDMETHODIMP GetParameter( const char* type, int* length, void* value );
	STDMETHODIMP SetParameter( const char* type, const char* value);


	/// Overridden from RtvcBaseFilter
	virtual void InitialiseInputTypes();
	
private:
	/**
	* This method converts the input buffer from RGB24 | 32 to YUV420P
	* @param pSource The source buffer
	* @param pDest The destination buffer
	*/
	virtual void ApplyTransform(BYTE* pBufferIn, long lInBufferSize, long lActualDataLength, BYTE* pBufferOut, long lOutBufferSize, long& lOutActualDataLength);

	ICodecv2* m_pCodec;

  unsigned char* m_pSeqParamSet;
  unsigned m_uiSeqParamSetLen;
  unsigned char* m_pPicParamSet;
  unsigned m_uiPicParamSetLen;
};