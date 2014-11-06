/** @file

MODULE				: H264

FILE NAME			: H264DecoderFilter.cpp

DESCRIPTION			: H.264 decoder filter implementation

LICENSE	: GNU Lesser General Public License

Copyright (c) 2008 - 2014, CSIR
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
#include "H264DecoderFilter.h"
#include "wmcodecdsp.h"
#include "Dvdmedia.h"

// RTVC
#include <Codecs/H264v2/H264v2.h>
#include <Codecs/CodecUtils/ICodecv2.h>
#include <DirectShow/CommonDefs.h>
#include <Shared/CommonDefs.h>
#include <Shared/Conversion.h>

// HACK for backwards compatibility with pre-CMake projects
#ifndef VPP_CMAKE_BUILD
// Define used to switch between the actual switching version of the codec and the non switching
#ifdef _DEBUG
#pragma comment(lib, "H264v2d.lib")
#else
#pragma comment(lib, "H264v2.lib")
#endif
#endif

const DWORD MAX_SUPPORTED_PROFILE = 66;
const DWORD MAX_SUPPORTED_LEVEL = 20;

H264DecoderFilter::H264DecoderFilter()
  : CCustomBaseFilter(NAME("CSIR VPP H264 Decoder"), 0, CLSID_RTVC_VPP_H264Decoder),
  m_pCodec(NULL),
  m_pSeqParamSet(NULL),
  m_uiSeqParamSetLen(0),
  m_pPicParamSet(NULL),
  m_uiPicParamSetLen(0),
  m_uiAvc1PrefixLength(0),
  m_pEncodedPictureBuffer(NULL),
  m_uiEncodedPictureBufferLength(0)
{
  //Call the initialise input method to load all acceptable input types for this filter
  InitialiseInputTypes();
  // Init parameters
  initParameters();
  H264v2Factory factory;
  m_pCodec = factory.GetCodecInstance();
  // Set default codec properties 
  if (m_pCodec)
  {
    //16 = YUV
    //m_pCodec->SetParameter(IN_COLOUR, "16");
    //0 = RGB only RGB is supported as an output
    m_pCodec->SetParameter("outcolour", "0");
  }
  else
  {
    SetLastError("Unable to create H264 Encoder from Factory.", true);
  }
}

H264DecoderFilter::~H264DecoderFilter()
{
  SafeDeleteArray(m_pSeqParamSet);
  SafeDeleteArray(m_pPicParamSet);
  if (m_uiAvc1PrefixLength > 0 && m_uiAvc1PrefixLength != 4)
    SafeDeleteArray(m_pEncodedPictureBuffer);

  if (m_pCodec)
  {
    m_pCodec->Close();
    H264v2Factory factory;
    factory.ReleaseCodecInstance(m_pCodec);
  }
}

CUnknown * WINAPI H264DecoderFilter::CreateInstance( LPUNKNOWN pUnk, HRESULT *pHr )
{
  H264DecoderFilter *pFilter = new H264DecoderFilter();
  if (pFilter== NULL) 
  {
    *pHr = E_OUTOFMEMORY;
  }
  return pFilter;
}

void H264DecoderFilter::InitialiseInputTypes()
{
  AddInputType(&MEDIATYPE_Video, &MEDIASUBTYPE_VPP_H264,		&FORMAT_VideoInfo);
  AddInputType(&MEDIATYPE_Video, &MEDIASUBTYPE_AVC1,		    &FORMAT_MPEG2Video);
  //RG Test 18012008
  AddInputType(&MEDIATYPE_Stream, &MEDIASUBTYPE_VPP_H264,		&FORMAT_VideoInfo);
}

// TODO: refactor from H264Source
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

HRESULT H264DecoderFilter::SetMediaType( PIN_DIRECTION direction, const CMediaType *pmt )
{
  HRESULT hr = CCustomBaseFilter::SetMediaType(direction, pmt);
  if (direction == PINDIR_INPUT)
  {
    // Configure codec
    m_pCodec->SetParameter(IMAGE_WIDTH, toString(m_nInWidth).c_str());
    m_pCodec->SetParameter(IMAGE_HEIGHT, toString(m_nInHeight).c_str());
    // always flip the image on windows to get it into the desired YUV format
    m_pCodec->SetParameter("flip",            "1");
    // NB: New codec settings for auto-iframe detection: These settings need to correlate to the settings of the DECODER
    m_pCodec->SetParameter("unrestrictedmotion",	"1");
    m_pCodec->SetParameter("extendedpicturetype",	"1");
    m_pCodec->SetParameter("unrestrictedmotion",	"1");
    m_pCodec->SetParameter("advancedintracoding",	"1");
    m_pCodec->SetParameter("alternativeintervlc",	"1");
    m_pCodec->SetParameter("modifiedquant",			"1");
    m_pCodec->SetParameter("autoipicture",			"1");
    m_pCodec->SetParameter("ipicturefraction",		"0");

    if (pmt->subtype == MEDIASUBTYPE_VPP_H264)
    {
      // Check for private codec data
      // Store SPS and PPS in media format header
      int nSize = pmt->cbFormat;
      BYTE* pFormat = pmt->Format();
      unsigned uiAdditionalSize(0);
      memcpy(&uiAdditionalSize, pFormat + nSize - sizeof(int), sizeof(int));

      if (uiAdditionalSize > 0)
      {
        unsigned char startCode[4] = { 0, 0, 0, 1 };  
        char* pBuffer = new char[uiAdditionalSize];
        memcpy(pBuffer, pFormat + (nSize - uiAdditionalSize), uiAdditionalSize);
        // check for first start code
        if (memcmp(startCode, pBuffer, 4) == 0)
        {
          int index = 0;
          // adjust to compensate for start code
          for (size_t i = 4; i < uiAdditionalSize - 8; ++i) // skip 1st start code and since start code is 4 bytes long don't cmp past the end
          {
            // search for second start code
            if (memcmp(startCode, &pBuffer[i], 4) == 0)
            {
              index = i;
              break;
            }
          }

          // if we found the second start code
          if (index != 0)
          {
            SafeDeleteArray(m_pSeqParamSet);
            SafeDeleteArray(m_pPicParamSet);
            m_uiSeqParamSetLen = index /*- 4*/; // subtract
            m_pSeqParamSet = new unsigned char[m_uiSeqParamSetLen];
            memcpy(m_pSeqParamSet, pBuffer /*+ 4*/, m_uiSeqParamSetLen);
            m_uiPicParamSetLen = uiAdditionalSize /*- 8 */- m_uiSeqParamSetLen - sizeof(int);
            m_pPicParamSet = new unsigned char[m_uiPicParamSetLen];
            memcpy(m_pPicParamSet, pBuffer + index /*+ 4*/, m_uiPicParamSetLen);

            return initialiseH264Decoder();
          }
        }
        delete[] pBuffer;
      }
    }
    else if (pmt->subtype == MEDIASUBTYPE_AVC1)
    {
      if (pmt->formattype != FORMAT_MPEG2Video)
      {
        return E_FAIL;
      }
      else
      {
        // see http://msdn.microsoft.com/en-us/library/windows/desktop/dd757808(v=vs.85).aspx

        MPEG2VIDEOINFO* pMpeg2VideoInfo = (MPEG2VIDEOINFO*) pmt->Format();
        if (pMpeg2VideoInfo->dwProfile > MAX_SUPPORTED_PROFILE)
          return E_FAIL;

        if (pMpeg2VideoInfo->dwLevel > MAX_SUPPORTED_LEVEL)
          return E_FAIL;
        // length of sequence header storing parameter sets
        DWORD sequenceHeaderLen = pMpeg2VideoInfo->cbSequenceHeader;
        // get length of length field
        m_uiAvc1PrefixLength = pMpeg2VideoInfo->dwFlags;
        ASSERT(m_uiAvc1PrefixLength == 1 || m_uiAvc1PrefixLength == 2 || m_uiAvc1PrefixLength == 4);
        // temporary solution for now: we won't handle prefixes of length 2 or 1
        if (m_uiAvc1PrefixLength != 4)
        {
          return E_FAIL;
        }

        // copy sequence header: The SPS and PPS in the sequence header have a 2 byte length prefix
        unsigned char* sequenceHeader = (unsigned char*)pMpeg2VideoInfo->dwSequenceHeader;

        // NOTE: this code only currently supports *one* SPS and one PPS
        // if there are multiple, only the last one wil be remembered
        DWORD processed = 0;
        while (processed < sequenceHeaderLen)
        {
          unsigned psLen = (sequenceHeader[processed] << 8) | sequenceHeader[processed + 1];
          unsigned char* pParamSet = new unsigned char[psLen + 4];
          // add start code for our decoder
          pParamSet[0] = 0; pParamSet[1] = 0; pParamSet[2] = 0; pParamSet[3] = 1;
          memcpy(pParamSet + 4, &sequenceHeader[processed + 2], psLen);
          processed += (2 + psLen);
          if (isSps(pParamSet[4]))
          {
            SafeDeleteArray(m_pSeqParamSet);
            m_pSeqParamSet = pParamSet;
            m_uiSeqParamSetLen = psLen + 4;
          }
          else if (isPps(pParamSet[4]))
          {
            SafeDeleteArray(m_pPicParamSet);
            m_pPicParamSet = pParamSet;
            m_uiPicParamSetLen = psLen + 4;
          }
          else
          {
            // free 
            delete[] pParamSet;
          }
        }

        return initialiseH264Decoder();
      }
    }
  }
  return hr;
}

HRESULT H264DecoderFilter::initialiseH264Decoder()
{
  BYTE buffer[256];
  // now configure the codec
  int nResult = m_pCodec->Decode(m_pSeqParamSet, m_uiSeqParamSetLen * 8, buffer);
  if (nResult)
  {
    //Encoding was successful
    nResult = m_pCodec->Decode(m_pPicParamSet, m_uiPicParamSetLen * 8, buffer);
    if (!nResult)
    {
      //An error has occurred
      DbgLog((LOG_TRACE, 0, TEXT("H264 Decode Error: %s"), m_pCodec->GetErrorStr()));
      std::string sError = "H264 decode error has occurred: " + std::string(m_pCodec->GetErrorStr());
      SetLastError(sError.c_str(), true);
      return E_FAIL;
    }
    else
    {
      m_pCodec->SetParameter((char *)("generate param set on open"),  "0");
      if (!m_pCodec->Open())
      {
        //Houston: we have a failure
        char* szErrorStr = m_pCodec->GetErrorStr();
        printf("%s\n", szErrorStr);
        // report the error to the main application
        SetLastError(szErrorStr, true);
        return E_FAIL;
      }
      else
      {
        return S_OK;
      }
    }
  }
  else
  {
    //An error has occurred
    DbgLog((LOG_TRACE, 0, TEXT("H264 Decode Error: %s"), m_pCodec->GetErrorStr()));
    std::string sError = "H264 decode error has occurred: " + std::string(m_pCodec->GetErrorStr());
    SetLastError(sError.c_str(), true);
    return E_FAIL;
  }
}

HRESULT H264DecoderFilter::GetMediaType( int iPosition, CMediaType *pMediaType )
{
  if (iPosition < 0)
  {
    return E_INVALIDARG;
  }
  else if (iPosition == 0)
  {
    // Get the input pin's media type and return this as the output media type - we want to retain
    // all the information about the image
    HRESULT hr = m_pInput->ConnectionMediaType(pMediaType);
    if (FAILED(hr))
    {
      return hr;
    }
    ASSERT(pMediaType->formattype == FORMAT_VideoInfo || pMediaType->formattype == FORMAT_MPEG2Video);
    if (pMediaType->formattype == FORMAT_MPEG2Video)
    {
      // FORMAT_MPEG2Video
      // get copy of BMI
      MPEG2VIDEOINFO* pMpeg2 = (MPEG2VIDEOINFO*)pMediaType->Format();
      BITMAPINFOHEADER bmiHeader = pMpeg2->hdr.bmiHeader;
      VIDEOINFOHEADER* pvi = (VIDEOINFOHEADER*)pMediaType->ReallocFormatBuffer(sizeof(VIDEOINFOHEADER));
      ASSERT(pvi != NULL);
      // copy existing BMI
      pvi->bmiHeader = bmiHeader;
      pMediaType->formattype = FORMAT_VideoInfo;
      int nSampleSize = m_nInHeight * m_nInWidth * 3;
      pvi->bmiHeader.biSizeImage = nSampleSize;
      pMediaType->SetSampleSize(nSampleSize);
    }
    else
    {
      // we're dealing with FORMAT_VideoInfo and our custom extension data
      // get rid of decoder specific data
      // Store SPS and PPS in media format header
      int nSize = pMediaType->cbFormat;
      BYTE* pFormat = pMediaType->Format();
      unsigned uiAdditionalSize(0);
      memcpy(&uiAdditionalSize, pFormat + nSize - sizeof(int), sizeof(int));
      pMediaType->ReallocFormatBuffer(nSize - uiAdditionalSize);
    }

    // update BMI info
    VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pMediaType->pbFormat;
    //Now we need to calculate the size of the output image
    BITMAPINFOHEADER* pBi = &(pVih->bmiHeader); 
    // We only support outputing 24bit RGB (This solves issues were RGB32 is encoded and decoded
    if (pBi->biBitCount != 24)
      pBi->biBitCount = 24;
    // in the case of YUV I420 input to the H264 encoder, we need to change this back to RGB
    pBi->biCompression = BI_RGB;
    // Set subtype of stream
    pMediaType->subtype = MEDIASUBTYPE_RGB24;
    return S_OK;
  }
  return VFW_S_NO_MORE_ITEMS;
}

HRESULT H264DecoderFilter::DecideBufferSize( IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProp )
{
  AM_MEDIA_TYPE mt;
  HRESULT hr = m_pOutput->ConnectionMediaType(&mt);
  if (FAILED(hr))
  {
    return hr;
  }
  //Make sure that the format type is our custom format
  ASSERT(mt.formattype == FORMAT_VideoInfo);
  BITMAPINFOHEADER *pbmi = HEADER(mt.pbFormat);
  pProp->cbBuffer = DIBSIZE(*pbmi);

  if (pProp->cbAlign == 0)
  {
    pProp->cbAlign = 1;
  }
  if (pProp->cBuffers == 0)
  {
    pProp->cBuffers = 30;
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

HRESULT H264DecoderFilter::ApplyTransform(BYTE* pBufferIn, long lInBufferSize, long lActualDataLength, BYTE* pBufferOut, long lOutBufferSize, long& lOutActualDataLength)
{
  //make sure we were able to initialise our converter
  if (m_pCodec)
  {
    if (m_pCodec->Ready())
    {
      long lDataLength = lActualDataLength;
      // For MEDIASUBTYPE_AVC1 we have to replace the length prefix with a start code
      // since the RTVC H264 decoder expects start codes in the stream
      // this will only be set for MEDIASUBTYPE_AVC1!
      if (m_uiAvc1PrefixLength > 0)
      {
        // now we unfortunately have to do a buffer copy if m_uiAvc1PrefixLength != 4
        switch (m_uiAvc1PrefixLength)
        {
        case 4:
          {
            // there can be multiple NALUs per media sample: replace all lengths with start codes
            int iCurrentPos = 0;
            while (iCurrentPos < lDataLength)
            {
              int nNaluLength = (pBufferIn[iCurrentPos] << 24) | (pBufferIn[iCurrentPos + 1] << 16) 
                | (pBufferIn[iCurrentPos + 2] << 8) | pBufferIn[iCurrentPos + 3];
              // just replace length with start code
              pBufferIn[iCurrentPos] = 0;
              pBufferIn[iCurrentPos + 1] = 0;
              pBufferIn[iCurrentPos + 2] = 0;
              pBufferIn[iCurrentPos + 3] = 1;
              iCurrentPos += nNaluLength + m_uiAvc1PrefixLength;
            }
            // update pointer without copy
            m_pEncodedPictureBuffer = pBufferIn;
            break;
          }
          /*
        case 2:
          {
            lDataLength = lActualDataLength + 2;
            resizeEncodedPictureBufferIfNecessary(lDataLength);
            // copy data
            memcpy(m_pEncodedPictureBuffer + 4, pBufferIn + 2, lActualDataLength - 2);
            break;
          }
        case 1:
          {
            lDataLength = lActualDataLength + 3;
            resizeEncodedPictureBufferIfNecessary(lDataLength);
            // copy data
            memcpy(m_pEncodedPictureBuffer + 4, pBufferIn + 1, lActualDataLength - 1);
            break;
          }
          */
        default:
          {
            // unsupported
            DbgLog((LOG_TRACE, 0, TEXT("H264 Decode Error: unsupported prefix length %d"), m_uiAvc1PrefixLength));
          }
        }
      }
      else
      {
        m_pEncodedPictureBuffer = pBufferIn;
        // here the length is already correct
      }

      // Mult by 8 to get number of bits
      int nBitLength = lDataLength * 8;

      int nResult = m_pCodec->Decode(m_pEncodedPictureBuffer, nBitLength, pBufferOut);

      if (nResult)
      {
        //Encoding was successful
        lOutActualDataLength = m_nInWidth * m_nInHeight * BYTES_PER_PIXEL_RGB24;
      }
      else
      {
        //An error has occurred
        DbgLog((LOG_TRACE, 0, TEXT("H264 Decode Error: %s"), m_pCodec->GetErrorStr()));
        std::string sError = "H264 decode error has occurred: " + std::string(m_pCodec->GetErrorStr());
        SetLastError(sError.c_str(), true);
      }
    }
  }
  return S_OK;
}

void H264DecoderFilter::resizeEncodedPictureBufferIfNecessary(long lActualDataLength)
{
  if (lActualDataLength > m_uiEncodedPictureBufferLength)
  {
    // resize buffer 
    SafeDeleteArray(m_pEncodedPictureBuffer);
    m_pEncodedPictureBuffer = new BYTE[2 * lActualDataLength];
    m_uiEncodedPictureBufferLength = 2 * lActualDataLength;
    // insert start code
    m_pEncodedPictureBuffer[0] = 0;
    m_pEncodedPictureBuffer[1] = 0;
    m_pEncodedPictureBuffer[2] = 0;
    m_pEncodedPictureBuffer[3] = 1;
  }
}

STDMETHODIMP H264DecoderFilter::GetParameter( const char* type, int* length, void* value )
{
  if (m_pCodec)
  {
    if (m_pCodec->GetParameter(type, length, value))
    {
      return S_OK;
    }
  }
  return E_FAIL;
}

STDMETHODIMP H264DecoderFilter::SetParameter( const char* type, const char* value )
{
  if (m_pCodec)
  {
    if (m_pCodec->SetParameter(type, value))
    {
      return S_OK;
    }
  }
  return E_FAIL;
}


HRESULT H264DecoderFilter::CheckTransform( const CMediaType *mtIn, const CMediaType *mtOut )
{
  // Check the major type.
  if (mtOut->majortype != MEDIATYPE_Video)
  {
    return VFW_E_TYPE_NOT_ACCEPTED;
  }

  // Check subtypes
  if (mtIn->subtype == MEDIASUBTYPE_VPP_H264 || mtIn->subtype == MEDIASUBTYPE_AVC1)
  {
    if (mtOut->subtype != MEDIASUBTYPE_RGB24)
    {
      return VFW_E_TYPE_NOT_ACCEPTED;
    }
  }
  else
  {
    return VFW_E_TYPE_NOT_ACCEPTED;
  }

  if (mtOut->formattype != FORMAT_VideoInfo)
  {
    return VFW_E_TYPE_NOT_ACCEPTED;
  }
  return S_OK;
}
