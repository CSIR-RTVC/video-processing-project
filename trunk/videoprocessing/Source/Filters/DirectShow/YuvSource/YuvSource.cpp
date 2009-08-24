/** @file

MODULE				: YUVSource

FILE NAME			: YuvSource.cpp

DESCRIPTION			: 

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
#include <streams.h>

#include <fstream>
#include <sstream>

#include "YuvSource.h"

#include <Shared/StringUtil.h>

const AMOVIESETUP_MEDIATYPE sudOpPinTypes =
{
    &MEDIATYPE_Video,       // Major type
    &MEDIASUBTYPE_NULL      // Minor type
};


/**********************************************
 *
 *  YuvOutputPin Class
 *  
 *
 **********************************************/

YuvOutputPin::YuvOutputPin(HRESULT *phr, YuvSourceFilter* pFilter)
      : CSourceStream(NAME("YUV Source"), phr, pFilter, L"Out"),
		m_pYuvFilter(pFilter),
		//m_szYuvFile(NULL),
		//m_iNoFrames(0),
		//m_iFrameSize(0),
		//m_iLengthY(0),
		//m_iLengthUV(0),
		m_iCurrentFrame(0),
        m_rtFrameLength(FPS_30) // Display 5 bitmap frames per second: TODO_ move to property page
{
//    std::ifstream in1("BUS_352x288_30.yuv", std::ifstream::in | std::ifstream::binary);
//	if ( in1.is_open() )
//	{
//		in1.seekg( 0, std::ios::end );
//		int iLength = in1.tellg();
//		in1.seekg( 0 , std::ios::beg );
//
//		m_szYuvFile = new unsigned char[ iLength ];
//		in1.read( (char*)m_szYuvFile, iLength );
//
//		in1.close();
//
//// 		// Init memory pointers
//// 		m_pY = m_szYuvFile;
//
//		m_iNoFrames = 150;
//		m_iFrameSize = iLength/m_iNoFrames;
//		m_iLengthY = 352*288;
//		m_iLengthUV = m_iLengthY >> 1;
//
//// 		m_pU = m_pY + ( m_iLengthY );
//// 		m_pV = m_pU + ( m_iLengthUV );
//	}
}


YuvOutputPin::~YuvOutputPin()
{   
}


// GetMediaType: This method tells the downstream pin what types we support.

// Here is how CSourceStream deals with media types:
//
// If you support exactly one type, override GetMediaType(CMediaType*). It will then be
// called when (a) our filter proposes a media type, (b) the other filter proposes a
// type and we have to check that type.
//
// If you support > 1 type, override GetMediaType(int,CMediaType*) AND CheckMediaType.
//
// In this case we support only one type, which we obtain from the bitmap file.

HRESULT YuvOutputPin::GetMediaType(CMediaType *pMediaType)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    CheckPointer(pMediaType, E_POINTER);

    // Allocate enough room for the VIDEOINFOHEADER
    VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER*)pMediaType->AllocFormatBuffer( sizeof(VIDEOINFOHEADER) );
    if (pvi == 0) 
        return(E_OUTOFMEMORY);

    ZeroMemory(pvi, pMediaType->cbFormat);   
    pvi->AvgTimePerFrame = m_rtFrameLength;

	pvi->bmiHeader.biBitCount = 12;
	pvi->bmiHeader.biCompression = MAKEFOURCC('Y', 'V', '1', '2');;

	pvi->bmiHeader.biClrImportant = 0;
	pvi->bmiHeader.biClrUsed = 0;
	pvi->bmiHeader.biPlanes = 1;
	pvi->bmiHeader.biXPelsPerMeter = 0;
	pvi->bmiHeader.biYPelsPerMeter = 0;
	pvi->bmiHeader.biWidth = m_pYuvFilter->m_iWidth;
	pvi->bmiHeader.biHeight = m_pYuvFilter->m_iHeight;

	int iSize = m_pYuvFilter->m_iWidth * m_pYuvFilter->m_iHeight * m_pYuvFilter->m_iBitsPerPixel;
	pvi->bmiHeader.biSizeImage = iSize;
	pvi->bmiHeader.biSize = 40;

    // Clear source and target rectangles
    SetRectEmpty(&(pvi->rcSource)); // we want the whole image area rendered
    SetRectEmpty(&(pvi->rcTarget)); // no particular destination rectangle

    pMediaType->SetType(&MEDIATYPE_Video);
    pMediaType->SetFormatType(&FORMAT_VideoInfo);
    pMediaType->SetTemporalCompression(FALSE);

	pMediaType->SetSubtype(&MEDIASUBTYPE_YV12);

	pMediaType->SetSampleSize( iSize );
    return S_OK;
}


HRESULT YuvOutputPin::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest)
{
    HRESULT hr;
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    CheckPointer(pAlloc, E_POINTER);
    CheckPointer(pRequest, E_POINTER);

    // If the bitmap file was not loaded, just fail here.
//     if (!m_pImage)
//         return E_FAIL;

    VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER*) m_mt.Format();
    
    // Ensure a minimum number of buffers
    if (pRequest->cBuffers == 0)
    {
        pRequest->cBuffers = 1;
    }
    pRequest->cbBuffer = pvi->bmiHeader.biSizeImage * 2;

    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pRequest, &Actual);
    if (FAILED(hr)) 
    {
        return hr;
    }

    // Is this allocator unsuitable?
    if (Actual.cbBuffer < pRequest->cbBuffer) 
    {
        return E_FAIL;
    }

    return S_OK;
}


// This is where we insert the DIB bits into the video stream.
// FillBuffer is called once for every sample in the stream.
HRESULT YuvOutputPin::FillBuffer(IMediaSample *pSample)
{
    BYTE *pData;
    long cbData;

    CheckPointer(pSample, E_POINTER);

    // If the bitmap file was not loaded, just fail here.
    //if (!m_pImage)
    //    return E_FAIL;

    CAutoLock cAutoLockShared(&m_cSharedState);

	if ( m_iCurrentFrame < m_pYuvFilter->m_iNoFrames )
	{
		// Access the sample's data buffer
		pSample->GetPointer(&pData);
		cbData = pSample->GetSize();

		// Check that we're still using video
		ASSERT(m_mt.formattype == FORMAT_VideoInfo);

		VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)m_mt.pbFormat;

		// If we want to change the contents of our source buffer (m_pImage)
		// at some interval or based on some condition, this is where to do it.
		// Remember that the new data has the same format that we specified in GetMediaType.
		// For example: 
		// if(m_iFrameNumber > SomeValue)
		//    LoadNewBitsIntoBuffer(m_pImage)

		// Copy the DIB bits over into our filter's output buffer.
		// Since sample size may be larger than the image size, bound the copy size.

		
		BYTE* pSource = m_pYuvFilter->m_szYuvFile + ( m_iCurrentFrame * m_pYuvFilter->m_iFrameSize );

		memcpy( pData, pSource, m_pYuvFilter->m_iFrameSize );

		// CRASH
		//BYTE* pSourceY = m_pY + ( m_iCurrentFrame * m_iFrameSize );
		//BYTE* pSourceU = pSourceY + m_iLengthY;
		//BYTE* pSourceV = pSourceY + m_iLengthY + m_iLengthUV;

		//memcpy( pData, pSourceY, m_iLengthY );
		//memcpy( pData + m_iLengthY, pSourceU, m_iLengthUV );
		//memcpy( pData + m_iLengthY + m_iLengthUV, pSourceV, m_iLengthUV );


		// CRASH
		//memcpy( pData, m_pY, m_iLengthY );
		//memcpy( pData + m_iLengthY, m_pU, m_iLengthUV );
		//memcpy( pData + m_iLengthY + m_iLengthUV, m_pV, m_iLengthUV );

		//m_pY += m_iFrameSize;
		//m_pU += m_iFrameSize;
		//m_pV += m_iFrameSize;


		//memcpy(pData, m_pImage, min(pVih->bmiHeader.biSizeImage, (DWORD) cbData));

		// Set the timestamps that will govern playback frame rate.
		// If this file is getting written out as an AVI,
		// then you'll also need to configure the AVI Mux filter to 
		// set the Average Time Per Frame for the AVI Header.
		// The current time is the sample's start
		REFERENCE_TIME rtStart = m_iCurrentFrame * m_rtFrameLength;
		REFERENCE_TIME rtStop  = rtStart + m_rtFrameLength;

		pSample->SetTime( &rtStart, &rtStop );
// 		m_iFrameNumber++;

		// Set TRUE on every sample for uncompressed frames
		pSample->SetSyncPoint(TRUE);

		m_iCurrentFrame++;
		return S_OK;
	}
	else
	{
		// EOF
		return S_FALSE;
	}
    
}


/**********************************************
 *
 *  YuvSourceFilter Class
 *
 **********************************************/

YuvSourceFilter::YuvSourceFilter(IUnknown *pUnk, HRESULT *phr)
	: CSource(NAME("CSIR RTVC YUV Source"), pUnk, CLSID_YUVSource),
	m_iWidth(0),
	m_iHeight(0),
	m_iNoFrames(150),		//TODO: move to property page
	m_iBitsPerPixel(1.5),	//TODO: move to property page
	m_szYuvFile(NULL),
	m_iFileSize(0)
{
	// Init CSettingsInterface
	initParameters();

    m_pPin = new YuvOutputPin(phr, this);

    if (phr)
    {
        if (m_pPin == NULL)
            *phr = E_OUTOFMEMORY;
        else
            *phr = S_OK;
    }  
}

YuvSourceFilter::~YuvSourceFilter()
{
	if (m_szYuvFile)
		delete[] m_szYuvFile;

	delete m_pPin;
}

CUnknown * WINAPI YuvSourceFilter::CreateInstance(IUnknown *pUnk, HRESULT *phr)
{
    YuvSourceFilter *pNewFilter = new YuvSourceFilter(pUnk, phr );

    if (phr)
    {
        if (pNewFilter == NULL) 
            *phr = E_OUTOFMEMORY;
        else
            *phr = S_OK;
    }

    return pNewFilter;
}

STDMETHODIMP YuvSourceFilter::Load( LPCOLESTR lpwszFileName, const AM_MEDIA_TYPE *pmt )
{
	// Store the URL
	m_sFile = StringUtil::wideToStl(lpwszFileName);

	std::ifstream in1(m_sFile.c_str()/*"BUS_352x288_30.yuv"*/, std::ifstream::in | std::ifstream::binary);
	if ( in1.is_open() )
	{
		in1.seekg( 0, std::ios::end );
		m_iFileSize = in1.tellg();
		in1.seekg( 0 , std::ios::beg );

		m_szYuvFile = new unsigned char[ m_iFileSize ];
		in1.read( (char*)m_szYuvFile, m_iFileSize );

		in1.close();

		// TODO: move to property page
		m_iFrameSize = m_iFileSize/m_iNoFrames;
		//m_iNoFrames = 150;
		//m_iFrameSize = iLength/m_iNoFrames;
		//m_iLengthY = 352*288;
		//m_iLengthUV = m_iLengthY >> 1;

		return S_OK;
	}
	else
	{
		SetLastError("Failed to open file: " + m_sFile, true);
		return E_FAIL;
	}
}

STDMETHODIMP YuvSourceFilter::GetCurFile( LPOLESTR * ppszFileName, AM_MEDIA_TYPE *pmt )
{
	//TODO: do we need to return the media type?

	// Copied from Async filter sample
	*ppszFileName = NULL;

	if (m_sFile.length()!=0) 
	{
		WCHAR* pFileName = (StringUtil::stlToWide(m_sFile));	

		DWORD n = sizeof(WCHAR)*(1+lstrlenW(pFileName));

		*ppszFileName = (LPOLESTR) CoTaskMemAlloc( n );
		if (*ppszFileName!=NULL) {
			CopyMemory(*ppszFileName, pFileName, n);
		}
		delete[] pFileName;
	}
	return NOERROR;
}

STDMETHODIMP YuvSourceFilter::NonDelegatingQueryInterface( REFIID riid, void **ppv )
{
	if(riid == (IID_ISettingsInterface))
	{
		return GetInterface((ISettingsInterface*) this, ppv);
	}
	else if (riid == IID_IStatusInterface)
	{
		return GetInterface((IStatusInterface*) this, ppv);
	}
	else if (riid == IID_IFileSourceFilter)
	{
		return GetInterface((IFileSourceFilter*) this, ppv);
	}
	else if (riid == IID_ISpecifyPropertyPages)
	{
		return GetInterface(static_cast<ISpecifyPropertyPages*>(this), ppv);
	}
	else
	{
		return CSource::NonDelegatingQueryInterface(riid, ppv);
	}
}

STDMETHODIMP YuvSourceFilter::SetParameter( const char* type, const char* value )
{
	if (strcmp(type, SOURCE_DIMENSIONS) == 0)
	{
		if ( parseDimensions(value) )
		{
			return CSettingsInterface::SetParameter(type, value);
		}
		else
		{
			return E_FAIL;
		}
	}
	else return CSettingsInterface::SetParameter(type, value);
}

bool YuvSourceFilter::parseDimensions( const std::string& sDimensions )
{
	size_t pos = sDimensions.find( "x" );
	if (pos == std::string::npos ) return false;
	else
	{
		int iWidth = 0, iHeight = 0;
		std::istringstream istr( sDimensions.substr( 0, pos ) );
		istr >> iWidth;
		std::istringstream istr2( sDimensions.substr( pos + 1) );
		istr2 >> iHeight;
		if (iWidth > 0 && iHeight > 0)
		{
			m_iWidth = iWidth;
			m_iHeight = iHeight;
			return true;
		}
		else
		{
			return false;
		}
	}
}

STDMETHODIMP YuvSourceFilter::Stop()
{
	m_pPin->m_iCurrentFrame = 0;
	return CSource::Stop();
}