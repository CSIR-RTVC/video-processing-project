/** @file

MODULE				: H264

FILE NAME			: H264DecoderFilter.cpp

DESCRIPTION			: H.264 encoder filter implementation

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
#include "stdafx.h"
#include "H264EncoderFilter.h"

//Codec classes
#include <Codecs/H264v2/H264v2.h>
#include <Codecs/CodecUtils/ICodecv2.h>

#include <Shared/Conversion.h>
#include <Shared/CommonDefs.h>

#include <dvdmedia.h>
#include <wmcodecdsp.h>

// Define used to switch between the actual switching version of the codec and the non switching
#ifdef _DEBUG
#pragma comment(lib, "H264v2d.lib")
#else
#pragma comment(lib, "H264v2.lib")
#endif

H264EncoderFilter::H264EncoderFilter()
  :CCustomBaseFilter(NAME("CSIR VPP H264 Encoder"), 0, CLSID_RTVC_VPP_H264Encoder),
  m_pCodec(NULL),
  m_nFrameBitLimit(0),
  m_bNotifyOnIFrame(false),
  m_pSeqParamSet(0),
  m_uiSeqParamSetLen(0),
  m_pPicParamSet(0),
  m_uiPicParamSetLen(0),
  m_uiIFramePeriod(0),
  m_uiCurrentFrame(0)
{
  //Call the initialise input method to load all acceptable input types for this filter
  InitialiseInputTypes();
  initParameters();
  H264v2Factory factory;
  m_pCodec = factory.GetCodecInstance();
  // Set default codec properties 
  if (m_pCodec)
  {
    //Set default quality to 16
    m_pCodec->SetParameter( QUALITY, "1");
    //16 = YUV
    m_pCodec->SetParameter(IN_COLOUR, "16");
    //16 = YUV
    m_pCodec->SetParameter(OUT_COLOUR, "16");
    // This multiplies the bit size of every i-frame by 2
    m_pCodec->SetParameter(I_PICTURE_MULTIPLIER, "2");
  }
  else
  {
    SetLastError("Unable to create H264 Encoder from Factory.", true);
  }
}

H264EncoderFilter::~H264EncoderFilter()
{
  if (m_pCodec)
  {
    m_pCodec->Close();
    H264v2Factory factory;
    factory.ReleaseCodecInstance(m_pCodec);
  }

  SafeDeleteArray(m_pSeqParamSet);
  SafeDeleteArray(m_pPicParamSet);
}

CUnknown * WINAPI H264EncoderFilter::CreateInstance( LPUNKNOWN pUnk, HRESULT *pHr )
{
  H264EncoderFilter *pFilter = new H264EncoderFilter();
  if (pFilter== NULL) 
  {
    *pHr = E_OUTOFMEMORY;
  }
  return pFilter;
}


void H264EncoderFilter::InitialiseInputTypes()
{
  AddInputType(&MEDIATYPE_Video, &MEDIASUBTYPE_RGB24,		&FORMAT_VideoInfo);
  AddInputType(&MEDIATYPE_Video, &MEDIASUBTYPE_YUV420P,	&FORMAT_VideoInfo);
  AddInputType(&MEDIATYPE_Video, &MEDIASUBTYPE_I420,	&FORMAT_VideoInfo);
}


HRESULT H264EncoderFilter::SetMediaType( PIN_DIRECTION direction, const CMediaType *pmt )
{
  HRESULT hr = CCustomBaseFilter::SetMediaType(direction, pmt);
  if (direction == PINDIR_INPUT)
  {
    // try close just in case
    m_pCodec->Close();

    if (pmt->subtype == MEDIASUBTYPE_RGB24)
    {
      m_pCodec->SetParameter(IN_COLOUR, IN_COLOUR_RGB24);
    }
    else if (pmt->subtype == MEDIASUBTYPE_YUV420P)
    {
      // NOTE: RG: _BUILD_FOR_SHORT has been elminated outside of the H.264 codec
      // We will only use 8 bit YUV outside of the codec!
      //m_pCodec->SetParameter(IN_COLOUR, IN_COLOUR_YUV420P);
      m_pCodec->SetParameter(IN_COLOUR, IN_COLOUR_YUV420P8);
    }
    else if (pmt->subtype == MEDIASUBTYPE_I420)
    {
      m_pCodec->SetParameter(IN_COLOUR, IN_COLOUR_YUV420P8);
    }

    m_pCodec->SetParameter(IMAGE_WIDTH, toString(m_nInWidth).c_str());
    m_pCodec->SetParameter(IMAGE_HEIGHT, toString(m_nInHeight).c_str());

    // NB: New codec settings for auto-iframe detection: These settings need to correlate to the settings of the DECODER
    m_pCodec->SetParameter("unrestrictedmotion",	"1");
    m_pCodec->SetParameter("extendedpicturetype",	"1");
    m_pCodec->SetParameter("unrestrictedmotion",	"1");
    m_pCodec->SetParameter("advancedintracoding",	"1");
    m_pCodec->SetParameter("alternativeintervlc",	"1");
    m_pCodec->SetParameter("modifiedquant",			  "1");
    m_pCodec->SetParameter("autoipicture",			  "1");
    m_pCodec->SetParameter("ipicturefraction",		"0");

    // On Windows we will always flip the color-converted image by default
    m_pCodec->SetParameter("flip",		            "1");

    // generate sequence and picture parameter sets
    SafeDeleteArray(m_pSeqParamSet);
    SafeDeleteArray(m_pPicParamSet);

    // set the selected parameter set numbers 
    m_pCodec->SetParameter((char *)"seq param set",  "0");
    m_pCodec->SetParameter((char *)"pic param set",  "0");

    m_pCodec->SetParameter((char *)("generate param set on open"),  "0");
    m_pCodec->SetParameter((char *)("picture coding type"),         "2");	///< Seq param set = H264V2_SEQ_PARAM.
    m_pSeqParamSet = new unsigned char[100];
    if(!m_pCodec->Code(NULL, m_pSeqParamSet, 100 * 8))
    {
      SafeDeleteArray(m_pSeqParamSet);
      SetLastError(m_pCodec->GetErrorStr(), true);
      return hr;
    }
    else
    {
      m_uiSeqParamSetLen = m_pCodec->GetCompressedByteLength();
      m_pSeqParamSet[m_uiSeqParamSetLen] = 0;
      m_sSeqParamSet = std::string((const char*)m_pSeqParamSet, m_uiSeqParamSetLen);
      int nCheck = strlen((char*)m_pSeqParamSet);
    }

    m_pCodec->SetParameter((char *)("picture coding type"),	  "3");	///< Pic param set = H264V2_PIC_PARAM.
    m_pPicParamSet = new unsigned char[100];
    if(!m_pCodec->Code(NULL, m_pPicParamSet, 100 * 8))
    {
      SafeDeleteArray(m_pPicParamSet);
      SafeDeleteArray(m_pSeqParamSet);
      SetLastError(m_pCodec->GetErrorStr(), true);
      return hr;
    }
    else
    {
      m_uiPicParamSetLen = m_pCodec->GetCompressedByteLength();
      m_pPicParamSet[m_uiPicParamSetLen] = 0;
      m_sPicParamSet = std::string((const char*)m_pPicParamSet, m_uiPicParamSetLen);
      int nCheck = strlen((char*)m_pPicParamSet);
    }

    // RG: copied from codec anayser, do we need this?
    // reset codec for standard operation
    m_pCodec->SetParameter((char *)("picture coding type"),	  "0");	///< I-frame = H264V2_INTRA.
    m_pCodec->SetParameter((char *)("generate param set on open"),  "1");

    if (!m_pCodec->Open())
    {
      //Houston: we have a failure
      char* szErrorStr = m_pCodec->GetErrorStr();
      printf("%s\n", szErrorStr);
      SetLastError(szErrorStr, true);
      return hr;
    }
  }
  return hr;
}

HRESULT H264EncoderFilter::GetMediaType( int iPosition, CMediaType *pMediaType )
{
  if (iPosition < 0)
  {
    return E_INVALIDARG;
  }
  else if (iPosition == 0)
  {
    if (!m_bUseMsH264)
    {
      // Get the input pin's media type and return this as the output media type - we want to retain
      // all the information about the image
      HRESULT hr = m_pInput->ConnectionMediaType(pMediaType);
      if (FAILED(hr))
      {
        return hr;
      }

      pMediaType->subtype = MEDIASUBTYPE_VPP_H264;

      ASSERT(pMediaType->formattype == FORMAT_VideoInfo);
      VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pMediaType->pbFormat;
      BITMAPINFOHEADER* pBi = &(pVih->bmiHeader);

      // For compressed formats, this value is the implied bit 
      // depth of the uncompressed image, after the image has been decoded.
      if (pBi->biBitCount != 24)
        pBi->biBitCount = 24;

      pBi->biSizeImage = DIBSIZE(pVih->bmiHeader);
      pBi->biSizeImage = DIBSIZE(pVih->bmiHeader);

      // COMMENTING TO REMOVE
      // in the case of YUV I420 input to the H264 encoder, we need to change this back to RGB
      //pBi->biCompression = BI_RGB;
      pBi->biCompression = DWORD('1cva');

      // Store SPS and PPS in media format header
      int nCurrentFormatBlockSize = pMediaType->cbFormat;

      if (m_uiSeqParamSetLen + m_uiPicParamSetLen > 0)
      {
        // old size + one int to store size of SPS/PPS + SPS/PPS/prepended by start codes
        int iAdditionalLength = sizeof(int) + m_uiSeqParamSetLen + m_uiPicParamSetLen;
        int nNewSize = nCurrentFormatBlockSize + iAdditionalLength;
        pMediaType->ReallocFormatBuffer(nNewSize);

        BYTE* pFormat = pMediaType->Format();
        BYTE* pStartPos = &(pFormat[nCurrentFormatBlockSize]);
        // copy SPS
        memcpy(pStartPos, m_pSeqParamSet, m_uiSeqParamSetLen);
        pStartPos += m_uiSeqParamSetLen;
        // copy PPS
        memcpy(pStartPos, m_pPicParamSet, m_uiPicParamSetLen);
        pStartPos += m_uiPicParamSetLen;
        // Copy additional header size
        memcpy(pStartPos, &iAdditionalLength, sizeof(int));
      }
      else
      {
        // not configured: just copy in size of 0
        pMediaType->ReallocFormatBuffer(nCurrentFormatBlockSize + sizeof(int));
        BYTE* pFormat = pMediaType->Format();
        memset(pFormat + nCurrentFormatBlockSize, 0, sizeof(int));
      }
    }
    else
    {
      pMediaType->InitMediaType();    
      pMediaType->SetType(&MEDIATYPE_Video);
      pMediaType->SetSubtype(&MEDIASUBTYPE_H264);
      //MEDIASUBTYPE_h264
      //MEDIASUBTYPE_X264
      //MEDIASUBTYPE_x264
      //MEDIASUBTYPE_AVC1
      //FORMAT_MPEG2Video

      pMediaType->SetFormatType(&FORMAT_VideoInfo2);
      VIDEOINFOHEADER2* pvi2 = (VIDEOINFOHEADER2*)pMediaType->AllocFormatBuffer(sizeof(VIDEOINFOHEADER2));
      ZeroMemory(pvi2, sizeof(VIDEOINFOHEADER2));
      pvi2->bmiHeader.biBitCount = 24;
      pvi2->bmiHeader.biSize = 40;
      pvi2->bmiHeader.biPlanes = 1;
      pvi2->bmiHeader.biWidth = m_nInWidth;
      pvi2->bmiHeader.biHeight = m_nInHeight;
      pvi2->bmiHeader.biSize = m_nInWidth * m_nInHeight * 3;
      pvi2->bmiHeader.biSizeImage = DIBSIZE(pvi2->bmiHeader);
      pvi2->bmiHeader.biCompression = DWORD('1cva');
      //pvi2->AvgTimePerFrame = m_tFrame;
      //pvi2->AvgTimePerFrame = 1000000;
      const REFERENCE_TIME FPS_25 = UNITS / 25;
      pvi2->AvgTimePerFrame = FPS_25;
      //SetRect(&pvi2->rcSource, 0, 0, m_cx, m_cy);
      SetRect(&pvi2->rcSource, 0, 0, m_nInWidth, m_nInHeight);
      pvi2->rcTarget = pvi2->rcSource;

      pvi2->dwPictAspectRatioX = m_nInWidth;
      pvi2->dwPictAspectRatioY = m_nInHeight;
    }
    return S_OK;
  }
  return VFW_S_NO_MORE_ITEMS;
}

HRESULT H264EncoderFilter::DecideBufferSize( IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProp )
{
  AM_MEDIA_TYPE mt;
  HRESULT hr = m_pOutput->ConnectionMediaType(&mt);
  if (FAILED(hr))
  {
    return hr;
  }

  if (m_bUseMsH264)
  {
    //Make sure that the format type is our custom format
    ASSERT(mt.formattype == FORMAT_VideoInfo2);
    VIDEOINFOHEADER2* pVih = (VIDEOINFOHEADER2*)mt.pbFormat;
    BITMAPINFOHEADER *pbmi = &pVih->bmiHeader;
    //TOREVISE: Should actually change mode and see what the maximum size is per frame?
    pProp->cbBuffer = DIBSIZE(*pbmi);
  }
  else
  {
    ASSERT(mt.formattype == FORMAT_VideoInfo);
    BITMAPINFOHEADER *pbmi = HEADER(mt.pbFormat);
    //TOREVISE: Should actually change mode and see what the maximum size is per frame?
    pProp->cbBuffer = DIBSIZE(*pbmi);
  }

  if (pProp->cbAlign == 0)
  {
    pProp->cbAlign = 1;
  }
  if (pProp->cBuffers == 0)
  {
    pProp->cBuffers = 1;
  }
  // Release the format block.
  FreeMediaType(mt);

  // Set allocator properties.
  ALLOCATOR_PROPERTIES Actual;
  hr = pAlloc->SetProperties(pProp, &Actual);
  if (FAILED(hr)) 
  {
    return hr;
  }
  // Even when it succeeds, check the actual result.
  if (pProp->cbBuffer > Actual.cbBuffer) 
  {
    return E_FAIL;
  }
  return S_OK;
}

inline unsigned H264EncoderFilter::getParameterSetLength() const
{
  return m_uiSeqParamSetLen + m_uiPicParamSetLen;
}

inline unsigned H264EncoderFilter::copySequenceAndPictureParameterSetsIntoBuffer( BYTE* pBuffer )
{
  // The SPS and PPS contain start code
  ASSERT(m_uiSeqParamSetLen > 0 && m_uiPicParamSetLen > 0);
  memcpy(pBuffer, (void*)m_pSeqParamSet, m_uiSeqParamSetLen);
  pBuffer += m_uiSeqParamSetLen;
  memcpy(pBuffer, m_pPicParamSet, m_uiPicParamSetLen);
  pBuffer += m_uiPicParamSetLen;
  return getParameterSetLength();
}

void H264EncoderFilter::ApplyTransform( BYTE* pBufferIn, long lInBufferSize, long lActualDataLength, BYTE* pBufferOut, long lOutBufferSize, long& lOutActualDataLength )
{
  lOutActualDataLength = 0;
  //make sure we were able to initialise our Codec
  if (m_pCodec)
  {
    if (m_pCodec->Ready())
    {
      if (m_uiIFramePeriod)
      {
        ++m_uiCurrentFrame;
        if (m_uiCurrentFrame%m_uiIFramePeriod == 0)
        {
          m_pCodec->Restart();
        }
      }

      int nFrameBitLimit = 0;
      if (m_nFrameBitLimit == 0)
        // An encoded frame can never be bigger than an RGB format frame
        nFrameBitLimit = m_nInWidth * m_nInHeight * 3 /* RGB */ * 8 /*8 bits*/ ;
      else
        nFrameBitLimit = m_nFrameBitLimit;

      BYTE* pOutBufferPos = pBufferOut;

      DbgLog((LOG_TRACE, 0, 
        TEXT("H264 Codec Byte Limit: %d"), nFrameBitLimit));
      int nResult = m_pCodec->Code(pBufferIn, pOutBufferPos, nFrameBitLimit);
      if (nResult)
      {
        //Encoding was successful
        lOutActualDataLength += m_pCodec->GetCompressedByteLength();

        // check if an i-frame was encoded
        int nLen = 0;
        char szBuffer[20];
        m_pCodec->GetParameter("last pic coding type",&nLen, szBuffer );
        if (strcmp(szBuffer, "0") == 0 /*H264V2_INTRA*/)
        {
          if (m_bNotifyOnIFrame)
          {
            SetNotificationMessage("I-Frame");
          }
        }

        DbgLog((LOG_TRACE, 0, TEXT("H264 Codec Success: Bit Length: %d Byte Length: %d"), m_pCodec->GetCompressedBitLength(), m_pCodec->GetCompressedByteLength()));
      }
      else
      {
        //An error has occurred
        DbgLog((LOG_TRACE, 0, TEXT("H264 Codec Error: %s"), m_pCodec->GetErrorStr()));
        std::string sError = m_pCodec->GetErrorStr();
        sError += ". Requested frame bit limit=" + toString(nFrameBitLimit) + ".";
        SetLastError(sError.c_str(), true);
      }
    }
  }
}

HRESULT H264EncoderFilter::CheckTransform( const CMediaType *mtIn, const CMediaType *mtOut )
{
  // Check the major type.
  if (mtOut->majortype != MEDIATYPE_Video)
  {
    return VFW_E_TYPE_NOT_ACCEPTED;
  }

  if (m_bUseMsH264)
  {
    if (mtOut->subtype != MEDIASUBTYPE_H264)
    {
      return VFW_E_TYPE_NOT_ACCEPTED;
    }

    if (mtOut->formattype != FORMAT_VideoInfo2)
    {
      return VFW_E_TYPE_NOT_ACCEPTED;
    }
  }
  else
  {
    if (mtOut->subtype != MEDIASUBTYPE_VPP_H264) 
    {
      return VFW_E_TYPE_NOT_ACCEPTED;
    }

    if (mtOut->formattype != FORMAT_VideoInfo)
    {
      return VFW_E_TYPE_NOT_ACCEPTED;
    }
  }

  // Everything is good.
  return S_OK;
}

STDMETHODIMP H264EncoderFilter::GetParameter( const char* szParamName, int nBufferSize, char* szValue, int* pLength )
{
  if (SUCCEEDED(CCustomBaseFilter::GetParameter(szParamName, nBufferSize, szValue, pLength)))
  {
    return S_OK;
  }
  else
  {
    // Check if its a codec parameter
    if (m_pCodec && m_pCodec->GetParameter(szParamName, pLength, szValue))
    {
      return S_OK;
    }
    return E_FAIL;
  }
}

STDMETHODIMP H264EncoderFilter::SetParameter( const char* type, const char* value )
{
  if (SUCCEEDED(CCustomBaseFilter::SetParameter(type, value)))
  {
    return S_OK;
  }
  else
  {
    // Check if it's a codec parameter
    if (m_pCodec && m_pCodec->SetParameter(type, value))
    {
      // re-open codec if certain parameters have changed
      if ( 
        (_strnicmp(type, MODE_OF_OPERATION_H264, strlen(type)) == 0 ) ||
        (_strnicmp(type, QUALITY, strlen(type)) == 0 )
        )
      {
        if (!m_pCodec->Open())
        {
          //Houston: we have a failure
          char* szErrorStr = m_pCodec->GetErrorStr();
          printf("%s\n", szErrorStr);
          SetLastError(szErrorStr, true);
          return E_FAIL;
        }
      }
      return S_OK;
    }
    return E_FAIL;
  }
}

STDMETHODIMP H264EncoderFilter::GetParameterSettings( char* szResult, int nSize )
{
  if (SUCCEEDED(CCustomBaseFilter::GetParameterSettings(szResult, nSize)))
  {
    // Now add the codec parameters to the output:
    int nLen = strlen(szResult);
    char szValue[10];
    int nParamLength = 0;
    std::string sCodecParams("Codec Parameters:\r\n");
    if( m_pCodec->GetParameter("parameters", &nParamLength, szValue))
    {
      int nParamCount = atoi(szValue);
      char szParamValue[256];
      memset(szParamValue, 0, 256);

      int nLenName = 0, nLenValue = 0;
      for (int i = 0; i < nParamCount; i++)
      {
        // Get parameter name
        const char* szParamName;
        m_pCodec->GetParameterName(i, &szParamName, &nLenName);
        // Now get the value
        m_pCodec->GetParameter(szParamName, &nLenValue, szParamValue);
        sCodecParams += "Parameter: " + std::string(szParamName) + " : Value:" + std::string(szParamValue) + "\r\n";
      }
      // now check if the buffer is big enough:
      int nTotalSize = sCodecParams.length() + nLen;
      if (nTotalSize < nSize)
      {
        memcpy(szResult + nLen, sCodecParams.c_str(), sCodecParams.length());
        // Set null terminator
        szResult[nTotalSize] = 0;
        return S_OK;
      }
      else
      {
        return E_FAIL;
      }
    }
    else
    {
      return E_FAIL;
    }
  }
  else
  {
    return E_FAIL;
  }
}

