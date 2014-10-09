/** @file

MODULE				: VideoMixingFilter

FILE NAME			: PicInPicFilter.h

DESCRIPTION			: Picture in picture Filter for RGB24 and RGB32 Media
					  
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

#include <DirectShow/VideoMixingBase.h>
#include <DirectShow/CSettingsInterface.h>
#include <Filters/DirectShow/FilterParameters.h>

// {31AD3B24-52E5-4227-B1D7-687AB061C2DE}
static const GUID CLSID_PicInPicFilter = 
{ 0x31ad3b24, 0x52e5, 0x4227, { 0xb1, 0xd7, 0x68, 0x7a, 0xb0, 0x61, 0xc2, 0xde } };

// {AA41A585-1DE0-43bd-8450-F04712130B6F}
static const GUID CLSID_PicInPicProperties = 
{ 0xaa41a585, 0x1de0, 0x43bd, { 0x84, 0x50, 0xf0, 0x47, 0x12, 0x13, 0xb, 0x6f } };


#ifdef _BUILD_FOR_SHORT
typedef short yuvType;
#else
typedef signed char yuvType ;
#endif

// Forward declarations
class PicInPicBase;
class PicScalerBase;

/**
 * @brief PicInPicFilter that accepts two RGB24 inputs.
 * The filter should only be confgured after the inputs have been connected.
 */
class PicInPicFilter	:	public VideoMixingBase,
							          public CSettingsInterface,
							          public ISpecifyPropertyPages
{
public:
	DECLARE_IUNKNOWN;

	/// Constructor
	PicInPicFilter();
	/// Destructor
	~PicInPicFilter();

	/// Static object-creation method (for the class factory)
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN pUnk, HRESULT *pHr); 
	
	///  From CBasePin
	virtual HRESULT CheckOutputType(const CMediaType* pMediaType);

	/// Overridden from VideoMixingBase 
	virtual HRESULT ReceiveFirstSample(IMediaSample *pSample);
	/// Overridden from VideoMixingBase 
	virtual HRESULT ReceiveSecondSample(IMediaSample *pSample);
	/// Overridden from VideoMixingBase 
	virtual HRESULT CreateVideoMixer(const CMediaType *pMediaType, int nIndex);
	/// Overridden from VideoMixingBase 
  virtual HRESULT SetOutputDimensions(BITMAPINFOHEADER* pBmih1, BITMAPINFOHEADER* pBmih2, int& nOutputWidth, int& nOutputHeight, int& nOutputSize);

	HRESULT GenerateOutputSample(IMediaSample *pSample, int nIndex);

	virtual void initParameters();

	/// For configuration dialog
	STDMETHODIMP GetPages(CAUUID *pPages)
	{
		if (pPages == NULL) return E_POINTER;
		pPages->cElems = 1;
		pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
		if (pPages->pElems == NULL) 
		{
			return E_OUTOFMEMORY;
		}
		pPages->pElems[0] = CLSID_PicInPicProperties;
		return S_OK;
	}

	/// For configuration dialog
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv)
	{
		if (riid == IID_ISpecifyPropertyPages)
		{
			return GetInterface(static_cast<ISpecifyPropertyPages*>(this), ppv);
		}
		else if(riid == (IID_ISettingsInterface))
		{
			return GetInterface((ISettingsInterface*) this, ppv);
		}
		else
		{
			// Call the parent class.
			return VideoMixingBase::NonDelegatingQueryInterface(riid, ppv);
		}
	}

	// TEMP Overriding to prevent RGB32
	virtual void InitialiseInputTypes()
	{
		AddInputType(&MEDIATYPE_Video, &MEDIASUBTYPE_RGB24, &FORMAT_VideoInfo);
	}
	virtual void InitialiseOutputTypes()
	{
		AddOutputType(&MEDIATYPE_Video, &MEDIASUBTYPE_RGB24, &FORMAT_VideoInfo);
	}

	STDMETHODIMP SetParameter( const char* type, const char* value);
private:
	bool parameterChangeAffectsOutput( const char* szParam );
	void reconfigure();
	/// Pointers to our last received Media Sample
	BYTE* m_pSampleBuffers[2];
	/// Sizes
	int m_nSampleSizes[2];

	enum SUB_PIC_POSITION
	{
		SUB_PIC_POSITION_1		= 0,
		SUB_PIC_POSITION_2		= 1,
		SUB_PIC_POSITION_3		= 2,
		SUB_PIC_POSITION_4		= 3,
		SUB_PIC_POSITION_CUSTOM = 4
	};

	/// Position where subvideo is inserted
	//int m_nPosition;
	/// target width: if zero the width of the primary picture is used
	int m_nTargetWidth;
  /// target height: if zero the height of the primary picture is used
  int m_nTargetHeight;
	/// target width of sub pic: if zero the width of the secondary picture is used
	int m_nSubPictureWidth;
  /// target height of sub pic: if zero the width of the secondary picture is used
  int m_nSubPictureHeight;
	/// custom X offset from bottom left corner used when position = SUB_PIC_POSITION_CUSTOM
  int m_nCustomOffsetX;
  /// custom Y offset from bottom left corner used when position = SUB_PIC_POSITION_CUSTOM
  int m_nCustomOffsetY;
	/// picture in pic class
	PicInPicBase* m_pPicInPic;
	/// target picture scaler used if primary picture output dimensions != input picture dimensions
  PicScalerBase* m_pTargetPicScaler;
  /// target picture scaler used if secondary picture output dimensions != input picture dimensions
  PicScalerBase* m_pSubPicScaler;
	/// Stores bytes per needed to store pixel according to media type
	double m_nBytesPerPixel;
  /// buffer to scale secondary image into if required
  BYTE* m_pBufferForScaledSecondaryImage;
};
