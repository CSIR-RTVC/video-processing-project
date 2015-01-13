#pragma once
//This file needs to be included - else we get a linker error for the GUID
#include <INITGUID.H>

// {15377191-6942-4E6E-8D5C-A958D39FFD95}
DEFINE_GUID(IID_IFilterInfoSourceInterface,
  0x15377191, 0x6942, 0x4e6e, 0x8d, 0x5c, 0xa9, 0x58, 0xd3, 0x9f, 0xfd, 0x95);

#undef  INTERFACE
#define INTERFACE IFilterInfoSourceInterface
DECLARE_INTERFACE_(IFilterInfoSourceInterface, IUnknown)
{
  /// @brief Method to retrieve last set value
  /// @param value[out] Buffer for value to be stored in
  /// @param[in/out] buffersize Size of output buffer. Also stores length of string in buffer after output
  STDMETHOD(GetMeasurement)(char* value, int* buffersize) = 0;
};
