#include "stdafx.h"

// DirectShow includes
#pragma warning(push)     // disable for this header only
#pragma warning(disable:4312) 
// DirectShow
#include <Streams.h>
#pragma warning(pop)      // restore original warning level

#include <DirectShow/CustomMediaTypes.h>
#include "RtspSinkFilter.h"
#include "RtspSinkInputPin.h"

static const WCHAR g_wszName[] = L"CSIR VPP RTSP Sink Filter";   /// A name for the filter 

// Filter setup data
const AMOVIESETUP_MEDIATYPE sudOpPinTypes[] =
{
  { 
    &MEDIATYPE_Video, &MEDIASUBTYPE_VPP_H264
  },
  {
    &MEDIATYPE_Audio, &MEDIASUBTYPE_AMR
  },
  {
    &MEDIATYPE_Audio, &MEDIASUBTYPE_AAC
  },
};

const AMOVIESETUP_PIN sudRtspSinkSet = 
{
  L"Output",        // Obsolete, not used.
  TRUE,             // Is this pin rendered?
  FALSE,            // Is it an output pin?
  FALSE,            // Can the filter create zero instances?
  TRUE,             // Does the filter create multiple instances?
  &CLSID_NULL,      // Obsolete.
  NULL,             // Obsolete.
  3,                // Number of media types.
  &sudOpPinTypes[0] // Pointer to media types.
};

#if 1
const AMOVIESETUP_FILTER sudRtspSink =
{
  &CLSID_CSIR_VPP_RtspSinkFilter,  // Filter CLSID
  g_wszName,                       // String name
  MERIT_DO_NOT_USE,                // Filter merit
  0,                               // Number pins
  NULL                             // Pin details
};

#else
const AMOVIESETUP_FILTER sudRtspSink=
{
  &CLSID_CSIR_VPP_RtspSinkFilter,  // Filter CLSID
  g_wszName,                       // String name
  MERIT_DO_NOT_USE,                // Filter merit
  1,                               // Number pins
  &sudRtspSinkSet                  // Pin details
};
#endif

// List of class IDs and creator functions for the class factory. This
// provides the link between the OLE entry point in the DLL and an object
// being created. The class factory will call the static CreateInstance.
// We provide a set of filters in this one DLL.

CFactoryTemplate g_Templates[] = 
{
  { 
    g_wszName,                      // Name
    &CLSID_CSIR_VPP_RtspSinkFilter, // CLSID
    RtspSinkFilter::CreateInstance, // Method to create an instance of MyComponent
    NULL,                           // Initialization function
    &sudRtspSink                    // Set-up information (for filters)
    }
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);    



////////////////////////////////////////////////////////////////////////
//
// Exported entry points for registration and unregistration 
// (in this case they only call through to default implementations).
//
////////////////////////////////////////////////////////////////////////

STDAPI DllRegisterServer()
{
  return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
  return AMovieDllRegisterServer2( FALSE );
}

//
// DllEntryPoint
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  dwReason, 
                      LPVOID lpReserved)
{
  return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}

