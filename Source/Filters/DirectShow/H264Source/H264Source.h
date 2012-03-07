/** @file

MODULE				: H264

FILE NAME			: H264Source.h

DESCRIPTION			: H264 source filter header

LICENSE: Software License Agreement (BSD License)

Copyright (c) 2012, CSIR
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


//#include <strsafe.h>

#include <fstream>
#include <vector>

#include <DirectShow/CStatusInterface.h>
#include <DirectShow/CSettingsInterface.h>


// UNITS = 10 ^ 7  
// UNITS / 30 = 30 fps;
// UNITS / 20 = 20 fps, etc
const REFERENCE_TIME FPS_30 = UNITS / 30;
const REFERENCE_TIME FPS_25 = UNITS / 25;
const REFERENCE_TIME FPS_20 = UNITS / 20;
const REFERENCE_TIME FPS_10 = UNITS / 10;
const REFERENCE_TIME FPS_5  = UNITS / 5;
const REFERENCE_TIME FPS_4  = UNITS / 4;
const REFERENCE_TIME FPS_3  = UNITS / 3;
const REFERENCE_TIME FPS_2  = UNITS / 2;
const REFERENCE_TIME FPS_1  = UNITS / 1;

const REFERENCE_TIME rtDefaultFrameLength = FPS_10;

// {C2E56BB0-A497-439B-A9BA-79D73D8D1549}
static const GUID CLSID_H264Source = 
{ 0xc2e56bb0, 0xa497, 0x439b, { 0xa9, 0xba, 0x79, 0xd7, 0x3d, 0x8d, 0x15, 0x49 } };

// {E6A2AE5B-F41A-4595-84DB-D997242B4587}
static const GUID CLSID_H264Properties = 
{ 0xe6a2ae5b, 0xf41a, 0x4595, { 0x84, 0xdb, 0xd9, 0x97, 0x24, 0x2b, 0x45, 0x87 } };

// Filter name strings
#define g_wszH264Source     L"CSIR RTVC H264 Source"

#define SOURCE_DIMENSIONS	"sourcedimensions"
#define SOURCE_FPS			  "fps"
#define USE_RTVC_H264     "usertvch264"

// fwd
class H264OutputPin;

class H264SourceFilter :  public CSource,
                          public CSettingsInterface,  /* Rtvc Settings Interface */
                          public CStatusInterface,    /* Rtvc Status Interface */
                          public IFileSourceFilter,	  /* To facilitate loading of URL */
                          public ISpecifyPropertyPages,
                          public IMediaSeeking
{
  friend class H264OutputPin;

public:
  ///this needs to be declared for the extra interface (adds the COM AddRef, etc methods)
  DECLARE_IUNKNOWN;

  static CUnknown * WINAPI CreateInstance(IUnknown *pUnk, HRESULT *phr);  

  /// override this to publicize our interfaces
  STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

  /// From CSource
  STDMETHODIMP Stop();

  /// From CSettingsInterface
  virtual void initParameters()
  {
    addParameter( SOURCE_FPS, &m_iFramesPerSecond, 30);
    addParameter( USE_RTVC_H264, &m_bUseRtvcH264, false);
  }

  STDMETHODIMP SetParameter( const char* type, const char* value );

  /// From IFileSourceFilter
  STDMETHODIMP Load(LPCOLESTR lpwszFileName, const AM_MEDIA_TYPE *pmt);
  /// From IFileSourceFilter
  STDMETHODIMP GetCurFile(LPOLESTR * ppszFileName, AM_MEDIA_TYPE *pmt);

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

  // IMediaSeeking
  STDMETHODIMP CheckCapabilities( DWORD *pCapabilities );
  STDMETHODIMP ConvertTimeFormat( LONGLONG *pTarget, const GUID *pTargetFormat, LONGLONG Source, const GUID *pSourceFormat);
  STDMETHODIMP GetAvailable(LONGLONG *pEarliest,  LONGLONG *pLatest);
  STDMETHODIMP GetCapabilities(DWORD *pCapabilities);
  STDMETHODIMP GetCurrentPosition(LONGLONG *pCurrent);
  STDMETHODIMP GetDuration(LONGLONG *pDuration);
  STDMETHODIMP GetPositions( LONGLONG *pCurrent, LONGLONG *pStop);
  STDMETHODIMP GetPreroll( LONGLONG *pllPreroll );
  STDMETHODIMP GetRate( double *dRate );
  STDMETHODIMP GetStopPosition( LONGLONG *pStop);
  STDMETHODIMP GetTimeFormat( GUID *pFormat );
  STDMETHODIMP IsFormatSupported( const GUID *pFormat );
  STDMETHODIMP IsUsingTimeFormat( const GUID *pFormat );
  STDMETHODIMP QueryPreferredFormat( GUID *pFormat );
  STDMETHODIMP SetPositions(LONGLONG *pCurrent, DWORD dwCurrentFlags, LONGLONG *pStop, DWORD dwStopFlags);
  STDMETHODIMP SetRate( double dRate );
  STDMETHODIMP SetTimeFormat( const GUID *pFormat );

private:
  // Constructor is private because you have to use CreateInstance
  H264SourceFilter(IUnknown *pUnk, HRESULT *phr);
  ~H264SourceFilter();

  void skipToFrame(unsigned uiFrameNumber);

  bool isIdrFrame(unsigned char nalUnitHeader)
  {
    unsigned uiForbiddenZeroBit = nalUnitHeader & 0x80;
    //assert(uiForbiddenZeroBit == 0);
    unsigned uiNalRefIdc = nalUnitHeader & 0x60;
    unsigned char uiNalUnitType = nalUnitHeader & 0x1f;
    switch (uiNalUnitType)
    {
      // IDR nal unit types
    case 5:
      return true;
    default:
      return false;
    }
  }

  bool isSps(unsigned char nalUnitHeader)
  {
    unsigned uiForbiddenZeroBit = nalUnitHeader & 0x80;
    unsigned uiNalRefIdc = nalUnitHeader & 0x60;
    unsigned char uiNalUnitType = nalUnitHeader & 0x1f;
    switch (uiNalUnitType)
    {
      // IDR nal unit types
    case 7:
      return true;
    default:
      return false;
    }
  }

  bool isPps(unsigned char nalUnitHeader)
  {
    unsigned uiForbiddenZeroBit = nalUnitHeader & 0x80;
    unsigned uiNalRefIdc = nalUnitHeader & 0x60;
    unsigned char uiNalUnitType = nalUnitHeader & 0x1f;
    switch (uiNalUnitType)
    {
      // IDR nal unit types
    case 8:
      return true;
    default:
      return false;
    }
  }

  bool isParameterSet(unsigned char nalUnitHeader)
  {
    return isSps(nalUnitHeader) || isPps(nalUnitHeader);
  }

  void recalculate();
  void reset();
  bool readNalUnit();
  int findIndexOfNextStartCode(unsigned uiStartingPos);

  H264OutputPin *m_pPin;
  bool m_bUseRtvcH264;

  int m_iFramesPerSecond;

  int m_iWidth;
  int m_iHeight;
  std::string m_sFile;

  unsigned char* m_pSeqParamSet;
  unsigned m_uiSeqParamSetLen;
  unsigned char* m_pPicParamSet;
  unsigned m_uiPicParamSetLen;

  unsigned m_uiCurrentBufferSize;
  unsigned char* m_pBuffer;
  unsigned m_uiBytesInBuffer;
  unsigned m_uiCurrentNalUnitSize;
  unsigned m_uiCurrentNalUnitStartPos;

  int m_iFileSize;
  int m_iRead;
  std::ifstream m_in1;

  // store byte indexes of various frames for IMediaSeeking implementation
  bool m_bAnalyseOnLoad;
  std::vector<unsigned> m_vParameterSets;
  std::vector<unsigned> m_vIdrFrames;
  std::vector<unsigned> m_vFrames;
};


