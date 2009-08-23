/** @file

MODULE				: YUVSource

FILE NAME			: YuvSource.h

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
#include <strsafe.h>

// UNITS = 10 ^ 7  
// UNITS / 30 = 30 fps;
// UNITS / 20 = 20 fps, etc
const REFERENCE_TIME FPS_30 = UNITS / 30;
const REFERENCE_TIME FPS_20 = UNITS / 20;
const REFERENCE_TIME FPS_10 = UNITS / 10;
const REFERENCE_TIME FPS_5  = UNITS / 5;
const REFERENCE_TIME FPS_4  = UNITS / 4;
const REFERENCE_TIME FPS_3  = UNITS / 3;
const REFERENCE_TIME FPS_2  = UNITS / 2;
const REFERENCE_TIME FPS_1  = UNITS / 1;

const REFERENCE_TIME rtDefaultFrameLength = FPS_10;

// {DAC3AA2A-5AB3-4705-963B-FFAF9C0D08D8}
DEFINE_GUID(CLSID_YUVSource, 
			0xdac3aa2a, 0x5ab3, 0x4705, 0x96, 0x3b, 0xff, 0xaf, 0x9c, 0xd, 0x8, 0xd8);

// Filter name strings
#define g_wszYuvSource     L"CSIR RTVC YUV Source"


/**********************************************
 *
 *  Class declarations
 *
 **********************************************/

class YuvOutputPin : public CSourceStream
{
protected:
	unsigned char* m_szYuvFile;

// 	BITMAPINFOHEADER m_bitmapInfo;
// 	unsigned char* m_pY;
// 	unsigned char* m_pU;
// 	unsigned char* m_pV;
	int m_iNoFrames;
	int m_iFrameSize;
	int m_iLengthY;
	int m_iLengthUV;
	int m_iCurrentFrame;

//     CRefTime m_rtSampleTime;	        // The time stamp for each sample

//     DWORD       m_cbBitmapInfo;         // Size of the bitmap header	
	// File opening variables 
// 	HANDLE m_hFile;                     // Handle returned from CreateFile
//     BYTE * m_pFile;                     // Points to beginning of file buffer
// 	BYTE * m_pImage;                    // Points to pixel bits                                      
// 
//     int m_iFrameNumber;
    const REFERENCE_TIME m_rtFrameLength;

    CCritSec m_cSharedState;            // Protects our internal state
//     CImageDisplay m_Display;            // Figures out our media type for us

public:

    YuvOutputPin(HRESULT *phr, CSource *pFilter);
    ~YuvOutputPin();

    // Override the version that offers exactly one media type
    HRESULT GetMediaType(CMediaType *pMediaType);
    HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest);
    HRESULT FillBuffer(IMediaSample *pSample);
    
    // Quality control
	// Not implemented because we aren't going in real time.
	// If the file-writing filter slows the graph down, we just do nothing, which means
	// wait until we're unblocked. No frames are ever dropped.
    STDMETHODIMP Notify(IBaseFilter *pSelf, Quality q)
    {
        return E_FAIL;
    }

};

class YuvSourceFilter : public CSource
{

private:
    // Constructor is private because you have to use CreateInstance
    YuvSourceFilter(IUnknown *pUnk, HRESULT *phr);
    ~YuvSourceFilter();

    YuvOutputPin *m_pPin;

public:
    static CUnknown * WINAPI CreateInstance(IUnknown *pUnk, HRESULT *phr);  

};


