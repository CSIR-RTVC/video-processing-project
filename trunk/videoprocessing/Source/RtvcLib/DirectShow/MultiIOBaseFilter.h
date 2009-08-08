/** @file

MODULE				: MultiIOBaseFilter

FILE NAME			: MultiIOBaseFilter.h

DESCRIPTION			: This base class takes care of doing most of the menial work when writing a transform filter that 
					has multiple input and output pins.

					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008, CSIR
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

//STL
#include <vector>
#include <string>
#include <map>

#include "MultiIOInputPin.h"
#include "MultiIOOutputPin.h"

//Forward declarations
class CMultiIOInputPin;
class CMultiIOOutputPin;

using namespace std;

//Typedefs
typedef vector<CMultiIOInputPin*> INPUT_PIN_LIST;
typedef vector<CMultiIOOutputPin*> OUTPUT_PIN_LIST;
typedef map<int, CMediaType> MEDIA_TYPE_LIST;

enum RTVC_DIRECTION
{
	RTVC_IN,
	RTVC_OUT
};

/**
 * This base class takes care of doing most of the menial work when writing a transform filter that 
 * has multiple input and output pins.
 */
class CMultiIOBaseFilter : public CBaseFilter
{
	friend class CMultiIOInputPin;
	friend class CMultiIOOutputPin;
		
public:
	CMultiIOBaseFilter(TCHAR *pObjectName, LPUNKNOWN lpUnk, CLSID clsid);
	virtual ~CMultiIOBaseFilter(void);
	
	void Initialise();
	//////////////////////////////////////////////////////////////////////////
	/// CBase Filter methods - Overridden since we don't just have one input and output pin as the standard transform filter does
	virtual int GetPinCount();
	/// Override this method: the pins get created in this method of the transform filter
	virtual CBasePin * GetPin(int n);
	/// Method needed to connect pins based on their names
	virtual STDMETHODIMP FindPin(LPCWSTR Id, IPin **ppPin);

	//////////////////////////////////////////////////////////////////////////
	// Input pin methods
	virtual HRESULT CheckInputType(const CMediaType *mtIn);
	/// Overridden to create more input pins on connection
	virtual void OnConnect(int nIndex, RTVC_DIRECTION eDirection );
	/// Overridden to remove unused input pins on disconnect
	virtual void OnDisconnect(int nIndex);
	/// chance to customize the transform process
	virtual HRESULT Receive(IMediaSample *pSample, int nIndex ) PURE;
	/// Standard setup for output sample
	virtual HRESULT InitializeOutputSample(IMediaSample *pSample, IMediaSample **ppOutSample, int nInputIndex, int nOutputIndex);

	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	/// Input input pin creation management methods
	virtual int InitialNumberOfInputPins();
	/// Input output pin creation management methods
	virtual int InitialNumberOfOutputPins();
	/// Determines whether the filter creates more inputs once all inputs have been used up
	virtual bool OnFullCreateMoreInputs();
	/// Determines whether the filter creates more outputs once all inputs have been used up
	virtual bool OnFullCreateMoreOutputs();
	/// The allowed media types for the input must be added in the overridden method in the subclass
	virtual void InitialiseInputTypes() = 0;
	/// The allowed media types for the output must be added in the overridden method in the subclass
	virtual void InitialiseOutputTypes() = 0;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Output pin methods
	///  From CBaseOutputPin
	virtual HRESULT DecideBufferSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* pRequestProperties, int m_nIndex)PURE;
	///  From CBasePin
	virtual HRESULT CheckOutputType(const CMediaType* pMediaType);
	/// To Check media type
	virtual HRESULT GetMediaType(int iPosition, CMediaType* pmt, int nOutputPinIndex) PURE;
	//////////////////////////////////////////////////////////////////////////
	
	/// override this to know when the media type is actually set
	/// This method has been extended to allow the subclass to specify which media type is allowed
	/// on a per pin basis. If the index - 1 is specified, the media type is allowable for all pins
	virtual HRESULT SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt, int nIndex);

	// Streaming calls delegated from input pins
	virtual STDMETHODIMP EndOfStream(int nIndex);
	virtual STDMETHODIMP BeginFlush(int nIndex);
	virtual STDMETHODIMP EndFlush(int nIndex);
	virtual STDMETHODIMP NewSegment( REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate, int nIndex);
protected:
	/**
	* Used for Media Type Negotiation 
	* Call the AddInputType method in your overridden InitialiseInputTypes 
	* method to add your acceptable media types, subtypes and formats.
	*/	

	/// Add input types, the default value for pin = -1 which indicates that the specified
	/// input type can be used for all pins
	void AddInputType(const GUID* pType, const GUID* pSubType, const GUID* pFormat, int nPin = -1);
	/// Add output types, the default value for pin = -1 which indicates that the specified
	/// input type can be used for all pins
	void AddOutputType(const GUID* pType, const GUID* pSubType, const GUID* pFormat, int nPin = -1);

	/// Helper method to create an input pin with our naming convention
	void CreateInputPin();
	/// Helper method to create an output pin with our naming convention
	void CreateOutputPin();

	/// List of input pins of the filter
	INPUT_PIN_LIST m_vInputPins;
	/// List of output pins of the filter
	OUTPUT_PIN_LIST m_vOutputPins;
	/// Media types of the connected input pins
	MEDIA_TYPE_LIST m_mMediaTypes;

	/// Filter Lock
	CCritSec m_csFilter;
	/// Receive Lock
	CCritSec m_csReceive;

private:
	bool m_bInitialised;

	//#######################################################################################
	//Nested helper class and methods for media type storage and search
	class CIOMediaType
	{
	public:
		CIOMediaType(const GUID* pInputType, const GUID* pInputSubtype, const GUID* pFormat)
			:MediaType(pInputType),
			MediaSubtype(pInputSubtype),
			Format(pFormat)
		{}
		const GUID* MediaType;
		const GUID* MediaSubtype;
		const GUID* Format;
	};

	// typedefs
	typedef vector<CIOMediaType*> MEDIA_TYPE_LIST;
	typedef map<int, MEDIA_TYPE_LIST*> MEDIA_TYPE_MAP;
	typedef map<int, MEDIA_TYPE_LIST*>::iterator MEDIA_TYPE_MAP_ITERATOR;

	// Method to do cleanup
	void CleanupTypeMap(MEDIA_TYPE_MAP mTypeMap);

	/// Helper method to add types to list
	void AddMediaType(CIOMediaType* pMediaType, MEDIA_TYPE_LIST& typeList, MEDIA_TYPE_MAP& typeMap, int nPin = -1);
	/// Helper method to search input type list
	HRESULT FindMediaType(const CMediaType* pmt, MEDIA_TYPE_LIST* pInputTypeList);
	/// Helper method to see if the output media type is correct
	HRESULT FindMediaTypeInLists(const CMediaType* pMediaType, MEDIA_TYPE_LIST* typeList, MEDIA_TYPE_MAP& typeMap);

	/// List for generic input types which apply to all pins
	MEDIA_TYPE_LIST m_vInputTypes;
	MEDIA_TYPE_LIST m_vOutputTypes;
	/// Map to link pin indexes to input type
	MEDIA_TYPE_MAP m_mInputTypeMap;
	MEDIA_TYPE_MAP m_mOutputTypeMap;
	//#######################################################################################
};
