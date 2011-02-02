// FrameGrabber.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <algorithm>
#include <dshow.h>
// Needed for sample grabber
#include <qedit.h>

#include "ComTypeDefs.h"
#include "CustomSampleGrabberCB.h"
#include <DirectShow/DirectShowHelper.h>
#include <Shared/CommonDefs.h>
#include <Shared/StringUtil.h>
#include <Shared/TimerUtil.h>

/// Command line arguments
StringMap_t g_mArguments;
StringList_t g_vArguments;

/// Helper method to parse cmd line args
/// Stores single arguments in g_vArguments
/// Stores name value pairs in g_mArguments
void parseCmdLineArgs(int argc, char** argv)
{
  for (size_t i = 2; i < (size_t)argc; ++i)
  {
    std::string sArg(argv[i]);
    size_t pos = sArg.find("=");
    if (pos != std::string::npos && pos > 0)
    {
      // NVP
      g_mArguments[sArg.substr(0, pos)] = sArg.substr(pos + 1);
    }
    else
    {
      g_vArguments.push_back(sArg);
    }
  }
}

int main(int argc, char** argv)
{
  if (argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " <<Filename>> [-PSNR] [mode=<<mode>> ]" << std::endl;
    std::cout << "<<mode>> 0: Original" << std::endl;
    std::cout << "<<mode>> 1: Lookup table" << std::endl;
    std::cout << "<<mode>> 2: Fixed-point" << std::endl;
    std::cout << "<<mode>> 3: SIMD" << std::endl;
    std::cout << "<<mode>> 4: CUDA" << std::endl;
    return -1;
  }

  std::string sFilename(argv[1]);
  // Check file name
  wchar_t* pwszFileName = new wchar_t[sFilename.length()];
  pwszFileName = StringUtil::stlToWide(sFilename);
  // TODO: check if file exists

  parseCmdLineArgs(argc, argv);

  Mode eMode = ORIGINAL;
  if (g_mArguments.find("mode") != g_mArguments.end())
  {
    int iMode = atoi(g_mArguments["mode"].c_str());
    eMode = static_cast<Mode>(iMode);
    if (eMode > GPU) eMode = ORIGINAL;
  }

  IGraphBuilderPtr pGraph = NULL;
  IMediaControlPtr pControl = NULL;
  IMediaEventPtr pEvent = NULL;
  IBaseFilterPtr pBaseFilter = NULL;
  ISampleGrabberPtr pSampleGrabber = NULL;
  // Create and configure sample grabber
  CustomSampleGrabberCB* pSampleGrabberCB = new CustomSampleGrabberCB(eMode);
  
  StringList_t::iterator it = std::find(g_vArguments.begin(), g_vArguments.end(), "-PSNR");
  if (it != g_vArguments.end())
  {
    pSampleGrabberCB->measurePSNR(true);
  }

  //CustomSampleGrabberCB sampleGrabberCB;
  BITMAPINFOHEADER bitmapInfoHeader;

  // Initialize the COM library.
  HRESULT hr = CoInitialize(NULL);
  if (FAILED(hr))
  {
    printf("ERROR - Could not initialize COM library");
    return -1;
  }

  // Create the filter graph manager and query for interfaces.
  hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
    IID_IGraphBuilder, (void **)&pGraph);
  if (FAILED(hr))
  {
    printf("ERROR - Could not create the Filter Graph Manager.");
    return -1;
  }

  hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
  hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);

  // load sample grabber

  const GUID* pClsID = &CLSID_SampleGrabber;

  //Add Filter using Class ID
  hr = CDirectShowHelper::AddFilterByCLSID(pGraph, *pClsID, L"SampleGrabber", &pBaseFilter);
  if (SUCCEEDED(hr))
  {
    pSampleGrabber = pBaseFilter;

    if (pSampleGrabber)
    {
      AM_MEDIA_TYPE acceptedMediaType;
      ZeroMemory(&acceptedMediaType, sizeof(acceptedMediaType));
      acceptedMediaType.majortype = GUID_NULL;
      acceptedMediaType.subtype = GUID_NULL;
      acceptedMediaType.formattype = GUID_NULL;
      acceptedMediaType.majortype = MEDIATYPE_Video;
      acceptedMediaType.subtype = MEDIASUBTYPE_RGB24;
      acceptedMediaType.formattype = FORMAT_VideoInfo;

      pSampleGrabber->SetMediaType(&acceptedMediaType);
      pSampleGrabber->SetOneShot(false);
      pSampleGrabber->SetCallback(pSampleGrabberCB, 0);  
    }
    else
    {
      printf("ERROR - Invalid sample grabber interface");
      return -1;
    }
  }
  else
  {
    printf("ERROR - Failed to load sample grabber filter");
    return -1;
  }

  // Build the graph. IMPORTANT: Change this string to a file on your system.
  hr = pGraph->RenderFile(pwszFileName, NULL);
  delete[] pwszFileName;

  // Get media type of sample grabber
  IPinPtr pPin;
  // Get Input Pin
  hr = CDirectShowHelper::GetPin(pBaseFilter, PINDIR_INPUT, &pPin);
  if (SUCCEEDED(hr))
  {
    AM_MEDIA_TYPE mediaType;
    hr = pPin->ConnectionMediaType(&mediaType);
    // Release pin
    pPin = NULL;
    if(SUCCEEDED(hr))
    {
      assert(mediaType.formattype == FORMAT_VideoInfo);
      // make a copy of the bitmap info header
      VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*) mediaType.pbFormat;
      BITMAPINFOHEADER* pBmi = &pVih->bmiHeader;
      CopyMemory(&bitmapInfoHeader, pBmi, sizeof(BITMAPINFOHEADER));
      // Free the media type
      FreeMediaType(mediaType);

      // update sample grabber callback with bitmap info header
      pSampleGrabberCB->setBitmapInfoHeader(&bitmapInfoHeader);
    }
    else
    {
      printf("ERROR - Failed to get pin media type");
      return -1;
    }
  }
  else
  {
    printf("ERROR - Failed to get pin of sample grabber");
    return -1;
  }

  // Add graph to running object table
  DWORD ROT_ID = 0;
  CDirectShowHelper::AddGraphToRot(pGraph, &ROT_ID);

  TimerUtil timer;
  timer.start();

  if (SUCCEEDED(hr))
  {
    // Run the graph.
    hr = pControl->Run();
    if (SUCCEEDED(hr))
    {
      // Wait for completion.
      long evCode;
      pEvent->WaitForCompletion(INFINITE, &evCode);
      // Note: Do not use INFINITE in a real application, because it
      // can block indefinitely.
    }
  }

  double dMilliseconds = timer.stop();

  std::cout << "Total time: " << dMilliseconds << " ms"
            << " Average PSNR: " << pSampleGrabberCB->getAveragePSNR() 
            << " Average Time per frame: " << pSampleGrabberCB->getAverageConversionTime() << " ms"
            << " Measurements: " << pSampleGrabberCB->getNumberOfMeasurements() 
            << std::endl;

  // Remove graph from running object table
  CDirectShowHelper::RemoveGraphFromRot(ROT_ID);

  // reset sample grabber callback before deleting memory
  pSampleGrabber->SetCallback(NULL, 0);
  // reclaim memory
  if (pSampleGrabberCB)
  {
    delete pSampleGrabberCB;
    pSampleGrabberCB = NULL;
  }
  // reset filters
  pSampleGrabber = NULL;
  pBaseFilter = NULL;
  // reset DS 
  pControl  = NULL;
  pEvent    = NULL;
  pGraph    = NULL;

  CoUninitialize();
  return 0;
}
