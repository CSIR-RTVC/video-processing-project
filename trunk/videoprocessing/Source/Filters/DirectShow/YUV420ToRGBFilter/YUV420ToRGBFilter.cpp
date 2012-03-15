/** @file

MODULE				: YUV420ToRGBFilter

FILE NAME			: YUV420ToRGBFilter.cpp

DESCRIPTION			: 

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
#include "YUV420ToRGBFilter.h"

#include <DirectShow/CommonDefs.h>
#include <DirectShow/CustomMediaTypes.h>
#include <Image/RealYUV420toRGB24Converter.h>

DEFINE_GUID(MEDIASUBTYPE_I420, 0x30323449, 0x0000, 0x0010, 0x80, 0x00,
  0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71); 

YUV420toRGBFilter::YUV420toRGBFilter()
  : CCustomBaseFilter(NAME("CSIR VPP YUV420P 2 RGB Converter"), 0, CLSID_YUV420toRGBColorConverter),
  m_pConverter(NULL),
  m_nSizeUV(0),
  m_bInvert(true)
{
  //Call the initialise input method to load all acceptable input types for this filter
  InitialiseInputTypes();
  initParameters();
 
  // init converter
  m_pConverter = new RealYUV420toRGB24Converter(m_nInWidth, m_nInHeight);
  m_pConverter->SetFlip(m_bInvert);
  m_pConverter->SetChrominanceOffset(m_nChrominanceOffset);
}

YUV420toRGBFilter::~YUV420toRGBFilter()
{
  if (m_pConverter)
  {
    delete m_pConverter;
    m_pConverter = NULL;
  }
}

CUnknown * WINAPI YUV420toRGBFilter::CreateInstance( LPUNKNOWN pUnk, HRESULT *pHr )
{
  YUV420toRGBFilter *pFilter = new YUV420toRGBFilter();
  if (pFilter== NULL) 
  {
    *pHr = E_OUTOFMEMORY;
  }
  return pFilter;
}


void YUV420toRGBFilter::InitialiseInputTypes()
{
  AddInputType(&MEDIATYPE_Video, &MEDIASUBTYPE_YUV420P, &FORMAT_VideoInfo);
  AddInputType(&MEDIATYPE_Video, &MEDIASUBTYPE_I420,	  &FORMAT_VideoInfo);
}


HRESULT YUV420toRGBFilter::SetMediaType( PIN_DIRECTION direction, const CMediaType *pmt )
{
  HRESULT hr = CCustomBaseFilter::SetMediaType(direction, pmt);
  if (direction == PINDIR_INPUT)
  {
    m_nSizeUV = m_nInPixels >> 2;
    m_pConverter->SetDimensions(m_nInWidth, m_nInHeight);
  }
  return hr;
}

HRESULT YUV420toRGBFilter::GetMediaType( int iPosition, CMediaType *pMediaType )
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
     
    VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER*)pMediaType->Format();
    //Adjust the bitmapinfo header
    BITMAPINFOHEADER *pbmi = HEADER(pMediaType->pbFormat);

    // The hackery below should not be ncessary: we simply convert to RGB24
#if 0
    if (pbmi->biBitCount == 32)
    {
      // Set the bit count to 24 since the RBGtoYUV filter also accepts 32 bit but this filter only
      // outputs 24 bit RGB.
      pbmi->biBitCount = 24;
      pbmi->biSizeImage = m_nInPixels * BYTES_PER_PIXEL_RGB24;
    }
    else if (pbmi->biBitCount == 24)
    {
      // We still need to set the size of the out image: the original image might have been scaled
      // So we need to recalculate the size in RGB 24
      pbmi->biSizeImage = m_nInPixels * BYTES_PER_PIXEL_RGB24;
    }
    // It's a 12bit YUV source
    else if (pbmi->biBitCount == 12)
    {
      pbmi->biBitCount = 24;
      // We still need to set the size of the out image: the original image might have been scaled
      // So we need to recalculate the size in RGB 24
      pbmi->biSizeImage = m_nInPixels * BYTES_PER_PIXEL_RGB24;
    }
#else

    pbmi->biBitCount = 24;
    pbmi->biSizeImage = m_nInPixels * BYTES_PER_PIXEL_RGB24;

#endif

    // negative height of YUV DIBS should be ignored according to 
    // http://msdn.microsoft.com/en-us/library/windows/desktop/dd407212(v=vs.85).aspx
    // we will set our output height negative according to m_bInvert
    if (m_bInvert)
    {
      if (pvi->bmiHeader.biHeight < 0) // make sure height is not already negative
        pvi->bmiHeader.biHeight = -1 * pvi->bmiHeader.biHeight;
    }
    else
    {
      // we will flip the image: make sure height is positive
      if (pvi->bmiHeader.biHeight > 0) // make sure height is not already negative
        pvi->bmiHeader.biHeight = -1 * pvi->bmiHeader.biHeight;
    }
#if 0
    if (pvi->bmiHeader.biHeight < 0)
    {
      if (m_bInvert)
        pvi->bmiHeader.biHeight = -1 * pvi->bmiHeader.biHeight;
    }
    else
    {
      if (m_bInvert)
        pvi->bmiHeader.biHeight = -1 * pvi->bmiHeader.biHeight;
    }
#endif

    pMediaType->SetSampleSize( pbmi->biSizeImage );
    // adapt compression in case necessary
    pbmi->biCompression = BI_RGB;
    //Reset the media subtype
    // Change the output format from MEDIASUBTYPE_YUV420P to RGB24
    pMediaType->SetSubtype(&MEDIASUBTYPE_RGB24);
    return S_OK;
  }
  return VFW_S_NO_MORE_ITEMS;
}

HRESULT YUV420toRGBFilter::DecideBufferSize( IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProp )
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

void YUV420toRGBFilter::ApplyTransform(BYTE* pBufferIn, long lInBufferSize, long lActualDataLength, BYTE* pBufferOut, long lOutBufferSize, long& lOutActualDataLength)
{
  int nRet = 0;

  if (m_pConverter)
  {
    yuvType* pYUV = NULL;
    pYUV = (yuvType*)pBufferIn;
    if (pYUV)
    {
      int iYuvSize = sizeof(yuvType);
      //set the Y, U and V pointers to point to the correct positions within the byte array
      yuvType* pY = pYUV;
      yuvType* pU = pYUV + m_nInPixels;
      yuvType* pV = pYUV + m_nInPixels + m_nSizeUV;
      //Convert
      m_pConverter->SetRotate(0);
      m_pConverter->Convert((void*)pY, (void*)pU, (void*)pV, (void*)pBufferOut);
      DbgLog((LOG_TRACE, 0, TEXT("Converted from YUV420 to RGB Directly")));
      //RGB24 stores 3 Bytes per pixel
      nRet = m_nInPixels * BYTES_PER_PIXEL_RGB24;
    }
  }
  lOutActualDataLength = nRet;
}

HRESULT YUV420toRGBFilter::CheckTransform( const CMediaType *mtIn, const CMediaType *mtOut )
{
  //Does this also occur when format changes occur such as when changing the renderer size???
  long lSize = mtOut->GetSampleSize();

  // Check the major type.
  if (mtOut->majortype != MEDIATYPE_Video)
  {
    return VFW_E_TYPE_NOT_ACCEPTED;
  }
  if (mtOut->subtype != MEDIASUBTYPE_RGB24)
  {
    return VFW_E_TYPE_NOT_ACCEPTED;
  }
  else
  {
    // TEST
    VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)mtOut->pbFormat;
    if (pVih)
    {
#if 0
      //Now we need to calculate the size of the output image
      BITMAPINFOHEADER* pBi = &(pVih->bmiHeader);
      int iNewStride = pBi->biWidth;
      // TEMP HACK TO TEST IF IMAGE RENDERS CORRECTLY
      /*if (iNewStride != m_nOutWidth)
      {
      return VFW_E_TYPE_NOT_ACCEPTED;
      }*/
      int iNewWidth = pVih->rcSource.right - pVih->rcSource.left;
      int iNewHeight = pBi->biHeight;
      if (iNewHeight < 0)
      {
        // REJECT INVERTED PICTURES
        return VFW_E_TYPE_NOT_ACCEPTED;
      }
#endif

      // check whether input and output height have the same sign
      // to prevent renderer from changing format
      VIDEOINFOHEADER *pVih1 = (VIDEOINFOHEADER*)mtIn->pbFormat;
      VIDEOINFOHEADER *pVih2 = (VIDEOINFOHEADER*)mtOut->pbFormat;

      if ( pVih1 && pVih2 )
      {
        BITMAPINFOHEADER* pBi1 = &(pVih1->bmiHeader);
        BITMAPINFOHEADER* pBi2 = &(pVih2->bmiHeader);

        if (m_bInvert)
        {
          // make sure height is positive
          if (pBi2->biHeight < 0)
            return VFW_E_TYPE_NOT_ACCEPTED;
        }
        else
        {
          // make sure height is negative
          if (pBi2->biHeight > 0)
            return VFW_E_TYPE_NOT_ACCEPTED;
        }
      }
    }
  }
  if (mtOut->formattype != FORMAT_VideoInfo)
  {
    return VFW_E_TYPE_NOT_ACCEPTED;
  }
  // Everything is good.
  return S_OK;
}

STDMETHODIMP YUV420toRGBFilter::SetParameter( const char* type, const char* value )
{
  if (SUCCEEDED(CSettingsInterface::SetParameter(type, value)))
  {
    if (m_pConverter)
    {
      m_pConverter->SetChrominanceOffset(m_nChrominanceOffset);
      m_pConverter->SetFlip(m_bInvert);
    }
    return S_OK;
  }
  else
  {
    return E_FAIL;
  }
}
