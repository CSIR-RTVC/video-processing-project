/** @file

MODULE				: H264

FILE NAME			: DLLSetup.cpp

DESCRIPTION			: H.264 decoder filter registration

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
#include "H264DecoderFilter.h"
#include "wmcodecdsp.h"

//////////////////////////////////////////////////////////////////////////
//###############################  Standard Filter DLL Code ###############################
static const WCHAR g_wszName[] = L"CSIR VPP H264 Decoder";   /// A name for the filter 

const AMOVIESETUP_MEDIATYPE sudMediaTypes[] = 
{
  { 
    &MEDIATYPE_Video, &MEDIASUBTYPE_VPP_H264
  },
  {
    &MEDIATYPE_Video, &MEDIASUBTYPE_AVC1
  },
  { 
    &MEDIATYPE_Video, &MEDIASUBTYPE_RGB24
  },
};

const AMOVIESETUP_PIN sudPin[] = 
{
  {
    L"",                // Obsolete, not used.
      FALSE,            // Is this pin rendered?
      FALSE,            // Is it an output pin?
      FALSE,            // Can the filter create zero instances?
      FALSE,            // Does the filter create multiple instances?
      &GUID_NULL,       // Obsolete.
      NULL,             // Obsolete.
      2,                // Number of media types.
      &sudMediaTypes[0] // Pointer to media types.
  },
  {
    L"",                // Obsolete, not used.
      FALSE,            // Is this pin rendered?
      TRUE,             // Is it an output pin?
      FALSE,            // Can the filter create zero instances?
      FALSE,            // Does the filter create multiple instances?
      &GUID_NULL,       // Obsolete.
      NULL,             // Obsolete.
      1,                // Number of media types.
      &sudMediaTypes[1] // Pointer to media types.
  }
};

// The next bunch of structures define information for the class factory.
AMOVIESETUP_FILTER FilterInfo =
{
  &CLSID_RTVC_VPP_H264Decoder,  // CLSID
  g_wszName,                    // Name
  MERIT_NORMAL,                 // Merit
  2,                            // Number of AMOVIESETUP_PIN structs
  sudPin                        // Pin registration information.
};

CFactoryTemplate g_Templates[1] = 
{
  { 
    g_wszName,                            // Name
      &CLSID_RTVC_VPP_H264Decoder,        // CLSID
      H264DecoderFilter::CreateInstance,  // Method to create an instance of MyComponent
      NULL,                               // Initialization function
      &FilterInfo                         // Set-up information (for filters)
  }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);   

// Code to reg in Video Compressor category
REGFILTER2 rf2FilterReg = {
  1,              // Version 1 (no pin mediums or pin category).
  MERIT_NORMAL,   // Merit.
  1,              // Number of pins.
  sudPin          // Pointer to pin information.
};

STDAPI DllRegisterServer(void)
{
  return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
  return AMovieDllRegisterServer2(FALSE);
}

//DLL Entry point
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);
BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
  return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}
