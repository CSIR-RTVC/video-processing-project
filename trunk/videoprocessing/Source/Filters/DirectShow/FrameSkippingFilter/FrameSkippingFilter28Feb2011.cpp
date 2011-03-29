/** @file

MODULE                : FrameSkippingFilter

FILE NAME			        : FrameSkippingFilter.cpp

DESCRIPTION           :

LICENSE: Software License Agreement (BSD License)

Copyright (c) 2010, CSIR
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

#include "FrameSkippingFilter.h"
#include <Shared/StringUtil.h>

#include <Gdiplus.h>

#include <numeric>


const unsigned MAX_FRAMERATE_INTERVAL_SIZE = 50;

FrameSkippingFilter::FrameSkippingFilter(LPUNKNOWN pUnk, HRESULT *pHr)
  : CTransInPlaceFilter(NAME("CSIR RTVC Frame Skipping Filter"), pUnk, CLSID_RTVCFrameSkippingFilter, pHr, false),
  seenFirstFrame(false),
  previousTimestamp(0),
  in_framecount(0),
  out_framecount(0)
{;}

FrameSkippingFilter::~FrameSkippingFilter()
{;}

CUnknown * WINAPI FrameSkippingFilter::CreateInstance( LPUNKNOWN pUnk, HRESULT *pHr )
{
  FrameSkippingFilter *pFilter = new FrameSkippingFilter(pUnk, pHr);
  if (pFilter== NULL) 
  {
    *pHr = E_OUTOFMEMORY;
  }
  return pFilter;
}

STDMETHODIMP FrameSkippingFilter::NonDelegatingQueryInterface( REFIID riid, void **ppv )
{
  return CTransInPlaceFilter::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT FrameSkippingFilter::Transform(IMediaSample *pSample)
{
   using namespace Gdiplus;

    REFERENCE_TIME tStart = 0;
	  REFERENCE_TIME tStop  = 0;

    int skipparram = 2; // parameter to define how many frames to skip


    HRESULT hr = pSample->GetTime(&tStart, &tStop); //get the current time now
    REFERENCE_TIME frameduration = tStop - tStart;

    if (!seenFirstFrame)
    {
      if (SUCCEEDED(hr))
      {
        previousTimestamp = tStart;
        seenFirstFrame = true;
      }
    return S_OK;
    }
    else
    {
      if (SUCCEEDED(hr))
      {
        in_framecount++ ; // counting the incoming samples

        REFERENCE_TIME tDiff = tStart - previousTimestamp;
        // Make sure timestamps are increasing
        if (tDiff > 0)
        {
          if((in_framecount % skipparram)== 0) 
           {
             pSample = NULL;
             return S_FALSE;
           } // skip this frame if it is true


           out_framecount++; // here we count only outgoing samples

         /*  REFERENCE_TIME frameduration = tStop - tStart;*/
           /*REFERENCE_TIME frameduration = tDiff;*/ /// duration in seconds for a single frame to go through the pipeline
                               ///time you want the frame to sleep for.. time in the future

                                 /// or is the length of time the particular frame takes to pass through the pipeline?

            tStart = out_framecount*frameduration;
            tStop = tStart + frameduration;

            hr = pSample->SetTime(&tStart, &tStop); //refer to Smaple Grabber filter
            if (FAILED(hr))
            {
              return hr;
            }
            
            m_qDurations.push_back(out_framecount);
            previousTimestamp = tStart;

        // limit the number of measurements
            if (m_qDurations.size() > MAX_FRAMERATE_INTERVAL_SIZE)
            {
              m_qDurations.pop_front();
            }      
      }
    }
   }

  if (!m_qDurations.empty())
  {
    // calc avg framerate
    REFERENCE_TIME tAvg = std::accumulate(m_qDurations.begin(), m_qDurations.end(), 0) / m_qDurations.size();

     // Avoid div by zero in cases where the timestamps are invalid
    if (tAvg > 0)
    {

         // Create a string.
            std::string skippedFramerate = StringUtil::doubleToString(in_framecount) + " frames";
            wchar_t* wsFramerate = StringUtil::stlToWide(skippedFramerate); 

          // get image properties
            AM_MEDIA_TYPE mt;
            hr = InputPin()->ConnectionMediaType(&mt);
            if (FAILED(hr))
            {
              return hr;
            }

            ASSERT(mt.formattype == FORMAT_VideoInfo);
            BITMAPINFOHEADER *pbmi = HEADER(mt.pbFormat);

            // Get image buffer
            BYTE *pBuffer(NULL);
            hr = pSample->GetPointer(&pBuffer);
            if (FAILED(hr))
            {
              return hr;
            }

            BITMAPINFO bitmapInfo;
            bitmapInfo.bmiHeader = *pbmi;

            Bitmap bm(&bitmapInfo, (void*)pBuffer);
            Graphics* pGraphics = Graphics::FromImage(&bm);

            // Initialize font
            Font myFont(L"Arial", 16);
            RectF layoutRect(0.0f, 0.0f, 200.0f, 50.0f);
            //RectF layoutRect(m_uiX, m_uiY, 200.0f, 50.0f);
            StringFormat format;
            format.SetAlignment(StringAlignmentNear);
            SolidBrush blackBrush(Color(255, 0, 0, 0));
            SolidBrush greenBrush(Color(255, 0, 255, 0));

            // Draw string
            pGraphics->DrawString(
              wsFramerate,
              skippedFramerate.length(),
              &myFont,
              layoutRect,
              &format,
              &greenBrush);

            delete[] wsFramerate;

            // Do we need to delete it?
            delete pGraphics;
            }
          }

	  return S_OK;
}

HRESULT FrameSkippingFilter::CheckInputType(const CMediaType* mtIn)
{
  // Check the major type.
	if (mtIn->majortype != MEDIATYPE_Video)
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	// Adding advert media type to this method
	if ((mtIn->subtype != MEDIASUBTYPE_RGB24) && (mtIn->subtype != MEDIASUBTYPE_RGB32) )
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	if (mtIn->formattype != FORMAT_VideoInfo)
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}
  return S_OK;
}

HRESULT FrameSkippingFilter::Run( REFERENCE_TIME tStart )
{
  return CTransInPlaceFilter::Run(tStart);
}

HRESULT FrameSkippingFilter::Stop( void )
{
  return CTransInPlaceFilter::Stop();
}

