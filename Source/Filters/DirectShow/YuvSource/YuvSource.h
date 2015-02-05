/** @file

MODULE				: YUVSource

FILE NAME			: YuvSource.h

DESCRIPTION			: 

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
#include <strsafe.h>

#include <fstream>
#include <DirectShow/CStatusInterface.h>
#include <DirectShow/CSettingsInterface.h>
#include <Filters/DirectShow/FilterParameters.h>


// {DAC3AA2A-5AB3-4705-963B-FFAF9C0D08D8}
DEFINE_GUID(CLSID_VPP_YUVSource, 
  0xdac3aa2a, 0x5ab3, 0x4705, 0x96, 0x3b, 0xff, 0xaf, 0x9c, 0xd, 0x8, 0xd8);

// {B044F35E-A7BD-464a-AD9F-9A1BEFBD95ED}
DEFINE_GUID(CLSID_YUVProperties, 
  0xb044f35e, 0xa7bd, 0x464a, 0xad, 0x9f, 0x9a, 0x1b, 0xef, 0xbd, 0x95, 0xed);

// Filter name strings
#define g_wszYuvSource     L"CSIR VPP YUV Source"

/**
 * @brief YUV source filter that currently supports reading YUV420P and YUV444 interlaced packet.
 */
class YuvSourceFilter : public CSource,
  public CSettingsInterface,  /* Rtvc Settings Interface */
  public CStatusInterface,    /* Rtvc Status Interface */
  public IFileSourceFilter,   /* To facilitate loading of URL */
  public ISpecifyPropertyPages
{
  friend class YuvOutputPin;

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
    addParameter( YUV_FORMAT, &m_nYuvFormat, YUV420P);
    addParameter( SOURCE_DIMENSIONS, &m_sDimensions, "352x288");
    addParameter( SOURCE_FPS, &m_iFramesPerSecond, 30);
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
    pPages->pElems[0] = CLSID_YUVProperties;
    return S_OK;
  }

private:
  // Constructor is private because you have to use CreateInstance
  YuvSourceFilter(IUnknown *pUnk, HRESULT *phr);
  ~YuvSourceFilter();

  bool setDimensions(const std::string& sDimensions);
  bool updatePictureBuffer(int iWidth, int iHeight, double dBytesPerPixel);

  void recalculate();
  bool readFrame();
  /**
   * @brief Creates the buffer to store YUV frames in. It tries to determine the dimensions 
   * of the YUV image based on the file name. Also tries to determine the YUV format of the 
   * YUV image based on the file name. This defaults to YUV420P.
   */
  void createYuvFrameBuffer();
  /**
   * @brief 
   */
  void guessFormat();

  YuvOutputPin *m_pPin;

  int m_nYuvFormat;
  int m_iWidth;
  int m_iHeight;
  std::string m_sDimensions;
  int m_iFramesPerSecond;
  int m_iNoFrames;
  int m_iFrameSize;
  double m_dBytesPerPixel;

  std::string m_sFile;

  unsigned char* m_pYuvBuffer;
  int m_iFileSize;
  int m_iRead;
  std::ifstream m_in1;
};


