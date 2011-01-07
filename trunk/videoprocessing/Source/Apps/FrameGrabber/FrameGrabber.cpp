// FrameGrabber.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <dshow.h>
// Needed for sample grabber
#include <qedit.h>

#include "ComTypeDefs.h"
#include "CustomSampleGrabberCB.h"
#include <DirectShow/DirectShowHelper.h>

int main()
{
  IGraphBuilderPtr pGraph = NULL;
  IMediaControlPtr pControl = NULL;
  IMediaEventPtr pEvent = NULL;
  IBaseFilterPtr pBaseFilter = NULL;
  ISampleGrabberPtr pSampleGrabber = NULL;
  CustomSampleGrabberCB* pSampleGrabberCB = new CustomSampleGrabberCB();
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

  //WCHAR* wszName = StringUtil::stlToWide(pFilterInfo->getName());
  hr = CDirectShowHelper::AddFilterByCLSID(pGraph, *pClsID, L"SampleGrabber", &pBaseFilter);
  //delete[] wszName;
#if 1
  if (SUCCEEDED(hr))
  {
    pSampleGrabber = pBaseFilter;

    if (pSampleGrabber)
    {
#if 1
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
      //pSampleGrabber->SetCallback(&sampleGrabberCB, 0);  
#endif
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
#endif

  // Build the graph. IMPORTANT: Change this string to a file on your system.
  hr = pGraph->RenderFile(L"D:\\Foreman_352x288_30fps.avi", NULL);

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

  //pControl->Release();
  //pEvent->Release();
  //pGraph->Release();
  CoUninitialize();
  return 0;
}
