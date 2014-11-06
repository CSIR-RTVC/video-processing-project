/** @file

MODULE                : Stereo2MonoFilter

FILE NAME			        : Stereo2MonoFilter.cpp

DESCRIPTION           : This filter skips a specified number of frames depending on the parameter denoted by "skipFrame"

LICENSE: Software License Agreement (BSD License)

Copyright (c) 2014, CSIR
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
#include "stdafx.h"

#include "Stereo2MonoFilter.h"
#include <Shared/StringUtil.h>

Stereo2MonoFilter::Stereo2MonoFilter(LPUNKNOWN pUnk, HRESULT *pHr)
  : CCustomBaseFilter(NAME("CSIR RTVC Stereo2Mono Filter"), pUnk, CLSID_RTVCStereo2MonoFilter),
  m_channel(0),
  m_nBitsPerSample(8),
  m_nSamplesPerSecond(0)
{
  // Init parameters
  initParameters();
}

Stereo2MonoFilter::~Stereo2MonoFilter()
{;}

CUnknown * WINAPI Stereo2MonoFilter::CreateInstance( LPUNKNOWN pUnk, HRESULT *pHr )
{
  Stereo2MonoFilter *pFilter = new Stereo2MonoFilter(pUnk, pHr);
  if (pFilter== NULL) 
  {
    *pHr = E_OUTOFMEMORY;
  }
  return pFilter;
}

STDMETHODIMP Stereo2MonoFilter::NonDelegatingQueryInterface( REFIID riid, void **ppv )
{
  if (riid == IID_ISpecifyPropertyPages)
  {
    return GetInterface(static_cast<ISpecifyPropertyPages*>(this), ppv);
  }
  else
  {
    // Call the parent class.
    return CCustomBaseFilter::NonDelegatingQueryInterface(riid, ppv);
  }
}

HRESULT Stereo2MonoFilter::ApplyTransform(BYTE* pBufferIn, long lInBufferSize, long lActualDataLength, BYTE* pBufferOut, long lOutBufferSize, long& lOutActualDataLength)
{
  switch (m_nBitsPerSample)
  {
  case 8:
    {
      BYTE* pDst = pBufferOut;
      BYTE* pSrc;
      switch (m_channel)
      {
      case 0:
        {
          pSrc = pBufferIn;
        }
        break;
      case 1:
        {
          pSrc = pBufferIn + 1;
          break;
        }
      }
      BYTE* pEnd = pBufferIn + lActualDataLength;
      while (pSrc < pEnd)
      {
        *pDst = *pSrc;
        ++pDst;
        pSrc += 2;
      }
      break;
    }
  case 16:
    {
      unsigned short* pDst = (unsigned short*)pBufferOut;
      unsigned short* pSrc;
      switch (m_channel)
      {
      case 0:
        {
          pSrc = (unsigned short*)pBufferIn;
          break;
        }
      case 1:
        {
          pSrc = (unsigned short*)pBufferIn + 1;
          break;
        }
      }
      unsigned short* pEnd = (unsigned short*)(pBufferIn + lActualDataLength);
      while (pSrc < pEnd)
      {
        *pDst = *pSrc;
        ++pDst;
        pSrc += 2;
      }
      break;
    }
  }
  lOutActualDataLength = lActualDataLength/2;
  return S_OK;
}

void Stereo2MonoFilter::InitialiseInputTypes()
{
  AddInputType(&MEDIATYPE_Audio, &MEDIASUBTYPE_PCM, &FORMAT_WaveFormatEx);
}

HRESULT Stereo2MonoFilter::CheckInputType(const CMediaType* mtIn)
{
  // Check the major type.
  if (mtIn->majortype != MEDIATYPE_Audio)
  {
    return VFW_E_TYPE_NOT_ACCEPTED;
  }

  // Adding advert media type to this method
  if (mtIn->subtype != MEDIASUBTYPE_PCM )
  {
    return VFW_E_TYPE_NOT_ACCEPTED;
  }

  if (mtIn->formattype != FORMAT_WaveFormatEx)
  {
    return VFW_E_TYPE_NOT_ACCEPTED;
  }

  // we only accept stero channels
  WAVEFORMATEX* pWf = (WAVEFORMATEX*) mtIn->Format();
  if (pWf->nChannels != 2)
  {    
    return VFW_E_TYPE_NOT_ACCEPTED;
  }

  // store bits per sample here
  m_nBitsPerSample = pWf->wBitsPerSample;
  return S_OK;
}

HRESULT Stereo2MonoFilter::SetMediaType( PIN_DIRECTION direction, const CMediaType *pmt )
{
	HRESULT hr = CCustomBaseFilter::SetMediaType(direction, pmt);
	if (direction == PINDIR_INPUT)
	{
		//Determine whether we are connected to a RGB24 or 32 source
    if (pmt->majortype == MEDIATYPE_Audio && pmt->formattype == FORMAT_WaveFormatEx)
		{
      WAVEFORMATEX* pWf = (WAVEFORMATEX*)pmt->Format();
      m_nBitsPerSample = pWf->wBitsPerSample;
      m_nSamplesPerSecond = pWf->nSamplesPerSec;
		}
	}
	return hr;
}

HRESULT Stereo2MonoFilter::GetMediaType( int iPosition, CMediaType *pMediaType )
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
		
		// Get the bitmap info header and adapt the cropped 
		//make sure that it's a video info header
		ASSERT(pMediaType->formattype == FORMAT_WaveFormatEx);
    WAVEFORMATEX *pWf = (WAVEFORMATEX*)pMediaType->pbFormat;
    // update to mono
    pWf->nChannels = 1;

    pWf->nBlockAlign = 1 * (m_nBitsPerSample >> 3);

    // From MSDN: Required average data-transfer rate, in bytes per second, for the format tag. If wFormatTag is WAVE_FORMAT_PCM, nAvgBytesPerSec should be equal to the product of nSamplesPerSec and nBlockAlign. For non-PCM formats, this member must be computed according to the manufacturer's specification of the format tag. 
    // OLD pWavHeader->nAvgBytesPerSec = m_nChannels*m_nSamplesPerSecond;
    pWf->nAvgBytesPerSec =  m_nSamplesPerSecond * pWf->nBlockAlign;
    return S_OK;
  }
  return VFW_S_NO_MORE_ITEMS;
}

HRESULT Stereo2MonoFilter::DecideBufferSize( IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequestProperties )
{
  // Get media type of the output pin
  AM_MEDIA_TYPE mt;
  HRESULT hr = m_pOutput->ConnectionMediaType(&mt);
  if (FAILED(hr))
  {
    return hr;
  }

  WAVEFORMATEX *pWavHeader = (WAVEFORMATEX*)mt.pbFormat;
  pRequestProperties->cbBuffer = pWavHeader->nSamplesPerSec * pWavHeader->nChannels * (pWavHeader->wBitsPerSample >> 3);

  if (pRequestProperties->cbAlign == 0)
  {
    pRequestProperties->cbAlign = 1;
  }
  if (pRequestProperties->cBuffers == 0)
  {
    pRequestProperties->cBuffers = 1;
  }
  // Release the format block.
  FreeMediaType(mt);

  // Set allocator properties.
  ALLOCATOR_PROPERTIES Actual;
  hr = pAlloc->SetProperties(pRequestProperties, &Actual);
  if (FAILED(hr)) 
  {
    return hr;
  }
  // Even when it succeeds, check the actual result.
  if (pRequestProperties->cbBuffer > Actual.cbBuffer) 
  {
    return E_FAIL;
  }
  return S_OK;
}

HRESULT Stereo2MonoFilter::CheckTransform( const CMediaType *mtIn, const CMediaType *mtOut )
{
  // Check the major type.
  if (mtOut->majortype != MEDIATYPE_Audio)
  {
    return VFW_E_TYPE_NOT_ACCEPTED;
  }

  if (mtOut->subtype != MEDIASUBTYPE_PCM)
  {
    return VFW_E_TYPE_NOT_ACCEPTED;
  }

  if (mtOut->formattype != FORMAT_WaveFormatEx)
  {
    return VFW_E_TYPE_NOT_ACCEPTED;
  }

  WAVEFORMATEX* pWf = (WAVEFORMATEX*)mtOut->Format();
  if (pWf->nChannels != 1)
  {
    return VFW_E_TYPE_NOT_ACCEPTED;
  }
  if (pWf->nSamplesPerSec != m_nSamplesPerSecond)
  {
    return VFW_E_TYPE_NOT_ACCEPTED;
  }
  if (pWf->wBitsPerSample != m_nBitsPerSample)
  {
    return VFW_E_TYPE_NOT_ACCEPTED;
  }

  // Everything is good.
  return S_OK;
}