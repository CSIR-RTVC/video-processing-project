#pragma once

// STL
#include <cassert>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#pragma warning(push)     // disable for this header only
#pragma warning(disable:4312)
// DirectShow
#include <streams.h>
#pragma warning(pop)      // restore original warning level
// Needed for sample grabber
#include <qedit.h>

// Color conversion helper classes
#include <Image/ImageHandlerV2.h>
#include <Image/RealRGB24toYUV420Converter.h>
#ifdef USE_MULTI_THREADED
  #include <Image/MtRGB24toYUV420Converter.h>
#endif
#include <Image/RealYUV420toRGB24Converter.h>
#include <Image/FastLookupTableRGB24toYUV420Converter.h>
#include <Image/FastFixedPointRGB24toYUV420Converter.h>
#include <Image/FastSimdRGB24toYUV420Converter.h>
#include <Image/FastGpuRGB24toYUV420Converter.h>

#include <Shared/TimerUtil.h>

enum Mode
{
  ORIGINAL,
  LOOKUP_TABLE,
  FIXED_POINT,
  SIMD, 
  GPU
};

class CustomSampleGrabberCB : public ISampleGrabberCB
{
public:

  CustomSampleGrabberCB(Mode eMode)
    : m_eMode(eMode),
    m_pBbitmapInfoHeader(NULL),
    m_pRgbToYuvConverter(NULL),
    m_pYuvToRgbConverter(NULL),
    m_pYuvDataBuffer(NULL),
    m_pRgbDataBuffer(NULL),
    m_uiPixels(0),
    m_uiYuvSize(0),
    m_bMeasurePSNR(false)
  {
    // Need to adapt this for other videos
    m_vPsnr.reserve(10000);
    m_vConversionTimes.reserve(10000);
  }

  ~CustomSampleGrabberCB()
  {
    SafeDelete(m_pRgbToYuvConverter);
    SafeDelete(m_pYuvToRgbConverter);
    SafeDeleteArray(m_pYuvDataBuffer);
    SafeDeleteArray(m_pRgbDataBuffer);
  }

  void initColorConverters()
  {
    //////////////////////////////////////////////////////////////////////////
    // FAST ALGORITHMS 2
    // instantiate your color converter
    // Don't forget to undefine the standard color converter

    switch (m_eMode)
    {
    case ORIGINAL:
      {
        printf("Creating standard color converter\r\n");
        m_pRgbToYuvConverter = new RealRGB24toYUV420Converter();
        break;
      }
    case LOOKUP_TABLE:
      {
        printf("Creating lookup table-based color converter\r\n");
        m_pRgbToYuvConverter = new FastLookupTableRGB24toYUV420Converter();
        break;
      }
    case FIXED_POINT:
      {
        printf("Creating fixed-point color converter\r\n");
        m_pRgbToYuvConverter = new FastFixedPointRGB24toYUV420Converter();
        break;
      }
    case SIMD:
      {
        printf("Creating SIMD color converter\r\n");
        m_pRgbToYuvConverter = new FastSimdRGB24toYUV420Converter();
        break;
      }
    case GPU:
      {
        printf("Creating GPU-based color converter\r\n");
        m_pRgbToYuvConverter = new FastGpuRGB24toYUV420Converter();
        break;
      }
    default:
      {
        m_pRgbToYuvConverter = new RealRGB24toYUV420Converter();
      }
    }

#ifdef USE_MULTI_THREADED
    // Use Multi-threaded converter
    m_pRgbToYuvConverter = new MtRGB24toYUV420Converter();
#endif

    m_pYuvToRgbConverter = new RealYUV420toRGB24Converter();
  }

  void setBitmapInfoHeader(BITMAPINFOHEADER* pBitmapInfoHeader)
  {
    assert(pBitmapInfoHeader);

    initColorConverters();

    m_pBbitmapInfoHeader = pBitmapInfoHeader;

    m_uiPixels = m_pBbitmapInfoHeader->biWidth * m_pBbitmapInfoHeader->biHeight;
    m_uiYuvSize = m_uiPixels >> 2;

    // configure converters
    m_pRgbToYuvConverter->SetDimensions(m_pBbitmapInfoHeader->biWidth, m_pBbitmapInfoHeader->biHeight);
    m_pRgbToYuvConverter->setChrominanceOffset(128);
    m_pYuvToRgbConverter->SetDimensions(m_pBbitmapInfoHeader->biWidth, m_pBbitmapInfoHeader->biHeight);
    m_pYuvToRgbConverter->SetRotate(0);

    // allocate memory for YUV (1.5 suffices but BUILD_FOR_SHORT requires 2 x 1.5)
    SafeDeleteArray(m_pYuvDataBuffer);
    unsigned uiBufferSize = m_pBbitmapInfoHeader->biWidth * m_pBbitmapInfoHeader->biHeight * 3;
    m_pYuvDataBuffer = new BYTE[uiBufferSize];
    memset(m_pYuvDataBuffer, 0, uiBufferSize);

    // allocate memory for RGB output
    SafeDeleteArray(m_pRgbDataBuffer);
    m_pRgbDataBuffer = new BYTE[uiBufferSize];
    memset(m_pRgbDataBuffer, 0, uiBufferSize);
  }

  void measurePSNR(bool bValue)
  {
    m_bMeasurePSNR = bValue;
  }

  double getAverageConversionTime() const
  {
    if (m_vConversionTimes.empty()) return 0.0;
    return std::accumulate(m_vConversionTimes.begin(), m_vConversionTimes.end(), 0.0)/m_vConversionTimes.size();
  }

  double getAveragePSNR() const
  {
    if (m_vPsnr.empty()) return 0.0;
    return std::accumulate(m_vPsnr.begin(), m_vPsnr.end(), 0.0)/m_vPsnr.size();
  }

  size_t getNumberOfMeasurements() const
  {
    return m_vPsnr.size();
  }

  // Fake out any COM ref counting
  STDMETHODIMP_(ULONG) AddRef() { return 2; }
  STDMETHODIMP_(ULONG) Release() { return 1; }

  // Fake out any COM QI'ing
  STDMETHODIMP QueryInterface(REFIID riid, void ** ppv)
  {
    CheckPointer(ppv,E_POINTER);
    if( riid == IID_ISampleGrabberCB || riid == IID_IUnknown ) 
    {
      *ppv = (void *) static_cast<ISampleGrabberCB*> ( this );
      return NOERROR;
    }    
    return E_NOINTERFACE;
  }

  /// Callback that is triggered per media sample
  /// Note this all happens in the DirectShow streaming thread!!!!!!!!!!!!
  STDMETHODIMP SampleCB( double dSampleTime, IMediaSample * pSample )
  {
    // set time to NULL to allow for fast rendering since the 
    // the video renderer controls the rendering rate according
    // to the timestamps
    pSample->SetTime(NULL, NULL);
    
    // Get byte pointer
    BYTE* pbData(NULL);
    HRESULT hr = pSample->GetPointer(&pbData);
    if (FAILED(hr)) 
    {
      return hr;
    }
    // Get length
    long sampleLength = pSample->GetActualDataLength();

    // Do image processing
    if (m_pBbitmapInfoHeader)
    {
      unsigned uiTotalSize(0);

      //////////////////////////////////////////////////////////////////////////

      // convert RGB to YUV
      //Map everything into yuvType pointers
      yuvType* pYUV = NULL;
      pYUV = (yuvType*)m_pYuvDataBuffer;
      assert (pYUV);

      uiTotalSize = (m_uiPixels + (2*m_uiYuvSize))*sizeof(yuvType);
      //Set pointer offsets into YUV array
      yuvType* pY = pYUV;
      yuvType* pU = pYUV + m_uiPixels;
      yuvType* pV = pU + m_uiYuvSize;
      //Convert to YUV
      m_timer.start();

      m_pRgbToYuvConverter->Convert((void*)pbData, (void*)pY, (void*)pU, (void*)pV);

      double dTotalTime = m_timer.stop();
      m_vConversionTimes.push_back(dTotalTime);

      //////////////////////////////////////////////////////////////////////////
      // convert back to RGB
      pYUV = (yuvType*)m_pYuvDataBuffer;
      assert (pYUV);
      int iYuvSize = sizeof(yuvType);
      //set the Y, U and V pointers to point to the correct positions within the byte array
      pY = pYUV;
      pU = pYUV + m_uiPixels;
      pV = pYUV + m_uiPixels + m_uiYuvSize;
      //Convert
      m_pYuvToRgbConverter->Convert((void*)pY, (void*)pU, (void*)pV, (void*)m_pRgbDataBuffer);

      if (m_bMeasurePSNR)
      {
        // Time PSNR
        m_timer.start();

        // Do PSNR calculations
        ImageHandlerV2 imageHandler;
        imageHandler.CreateImage(m_pBbitmapInfoHeader);
        // point at the original data
        memcpy(imageHandler._bmptr, pbData, sampleLength);
        imageHandler.ConvertToYUV(ImageHandlerV2::YUV444);

        ImageHandlerV2 imageHandler2;
        imageHandler2.CreateImage(m_pBbitmapInfoHeader);
        // point at converted data
        memcpy(imageHandler2._bmptr, m_pRgbDataBuffer, sampleLength);
        imageHandler2.ConvertToYUV(ImageHandlerV2::YUV444);

        double dPsnr = imageHandler.PSNR(imageHandler2);
        m_vPsnr.push_back(dPsnr);

        double dPsnrTotalTime = m_timer.stop();
#if 0
        std::cout << "PSNR time: " << dPsnrTotalTime << std::endl;
        std::cout << "PSNR: " << dPsnr << std::endl;
#endif
      }
      
      // Copy converted sample to media sample
      CopyMemory(pbData, m_pRgbDataBuffer, uiTotalSize);
    }

    return S_OK;
  }

  STDMETHODIMP BufferCB( double SampleTime, BYTE * pBuffer, long BufferSize )
  {
    return 0;
  }

private:

  Mode m_eMode;
  BITMAPINFOHEADER* m_pBbitmapInfoHeader;

  RGBtoYUV420Converter* m_pRgbToYuvConverter;
  YUV420toRGBConverter* m_pYuvToRgbConverter;

  BYTE* m_pYuvDataBuffer;
  BYTE* m_pRgbDataBuffer;
  unsigned m_uiPixels;
  unsigned m_uiYuvSize;
  bool m_bMeasurePSNR;

  TimerUtil m_timer;
  std::vector<double> m_vConversionTimes;
  std::vector<double> m_vPsnr;
};
