/** StringUtil.h
===========================================================================
MODULE          : StringUtil

DESCRIPTIONS    : Helper class for unicode to ascii and other string conversions

REVISION HISTORY: Version 1.0

COPYRIGHT       : (c)CSIR, Meraka Institute 2010 all rights reserved

RESTRICTIONS	: The information/data/code contained within this file is 
				  the property of CSIR, Meraka Institute and has been 
				  classified as PUBLIC.
===========================================================================
*/
#pragma once

#include <stdio.h>
#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>

#ifdef _WIN32

#ifdef _CONVERT_WITH_ATL
#include <atlconv.h>
#include <strsafe.h>

#ifdef _DEBUG
# pragma comment(lib, "atls.lib")
#else
# pragma comment(lib, "atls.lib")
#endif
#endif
#endif

class StringUtil
{
public:
#ifdef _WIN32

#ifdef _CONVERT_WITH_ATL
  static std::string wideToStl(const WCHAR* wszString)
  {
    //return "";
    USES_CONVERSION;
    char *szTemp = W2A(wszString);
    return std::string(szTemp);
  }

  static WCHAR* stlToWide(std::string sString)
  {
    return(asciiToWide(sString.c_str()));
  }

  /// The caller must free the memory for the result
  static WCHAR* asciiToWide(const char* szString)
  {
    size_t nLen = strlen(szString);
    WCHAR* wszBuffer = new WCHAR[nLen + 1];
    USES_CONVERSION;
    WCHAR* wszValue = A2W(szString);
    StringCchCopyW(wszBuffer, nLen + 1, wszValue);
    return wszBuffer;
  }
#else
  static std::string wideToStl(const wchar_t* wszString)
  {
    const unsigned max_len = 255;
    char szBuffer[max_len];
    size_t uiConverted(0);
    errno_t err = wcstombs_s(&uiConverted, szBuffer, max_len, wszString, _TRUNCATE);
    return (err == 0) ? std::string(szBuffer, uiConverted - 1) : ""; // subtract 1 for null terminator
  }

  static wchar_t* stlToWide(std::string sString)
  {
    return(asciiToWide(sString.c_str()));
  }

  /// The caller must free the memory for the result
  static wchar_t* asciiToWide(const char* szString)
  {
    size_t uiLen = strlen(szString);
    wchar_t* wszBuffer = new wchar_t[uiLen + 1];
    size_t uiRes = mbstowcs(wszBuffer, szString, uiLen);
    if (uiRes != -1)
    {
      wszBuffer[uiRes] = L'\0';
      return wszBuffer;
    }
    else
    {
      delete[] wszBuffer;
      return NULL;
    }
  }
#endif
#endif

  //// TOREMOVE ONCE REDUNDANT
  //static std::string itos(int nNumber)
  //{
  //  // Buffer size ten should be sufficient for integers
  //  char szBuffer[10];
  //  sprintf( szBuffer, "%d", nNumber );
  //  return std::string(szBuffer);
  //}

  // Converts "1", "true", "TRUE" to true and "0", "FALSE" and "false" to false
  static bool stringToBool(const std::string& sValue)
  {
    if (sValue == "1" || sValue == "TRUE" || sValue == "true")
    {
      return true;
    }
    else if (sValue == "0" || sValue == "FALSE" || sValue == "false")
    {
      return false;
    }
    else
    {
      // Default
      return true;
    }
  }

  static std::string boolToString(bool bValue)
  {
    std::string sResult = bValue?"1":"0";
    return sResult;
  }

  static std::string doubleToString(double d, int nPrecision = 2)
  {
    std::ostringstream oss;

    oss << std::setprecision(nPrecision)
      << std::fixed
      << d;

    return oss.str();
  }

  static double stringToDouble(std::string sDouble)
  {
    std::istringstream i(sDouble);
    double x;
    if (!(i >> x))
    {
      return 0.0;
    }
    return x;
  }

  static std::string GetTimeString(std::string sFormat = "")
  {
    const unsigned uiBufferSize = 256;
    //Get current date and time and format
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[uiBufferSize];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    if (sFormat == "")
    {
      sFormat = "%A, %d %B %Y %X";
    }
    strftime(buffer, uiBufferSize, sFormat.c_str(), timeinfo);
    return std::string(buffer);
  }
};
