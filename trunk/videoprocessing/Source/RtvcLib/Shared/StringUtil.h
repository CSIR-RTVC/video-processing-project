/** StringUtil.h
===========================================================================
MODULE          : StringUtil

DESCRIPTIONS    : Helper class for unicode to ascii and other string conversions

REVISION HISTORY: Version 1.0

COPYRIGHT       : (c)CSIR, Meraka Institute 2008 all rights reserved

RESTRICTIONS	: The information/data/code contained within this file is 
				  the property of CSIR, Meraka Institute and has been 
				  classified as PUBLIC.
===========================================================================
*/
#pragma once

#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>

#ifdef _WIN32

#include <atlconv.h>
#include <strsafe.h>

	#ifdef _DEBUG
		# pragma comment(lib, "atls.lib")
	#else
		# pragma comment(lib, "atls.lib")
	#endif
#endif

class StringUtil
{
public:
#ifdef _WIN32
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

	static WCHAR* asciiToWide(const char* szString)
	{
		//return L"";
		size_t nLen = strlen(szString);
		WCHAR* wszBuffer = new WCHAR[nLen + 1];
		USES_CONVERSION;
		WCHAR* wszValue = A2W(szString);
		StringCchCopyW(wszBuffer, nLen + 1, wszValue);
		return wszBuffer;
	}
#endif

	static std::string itos(int nNumber)
	{
		// Buffer size ten should be sufficient for integers
		char szBuffer[10];
		sprintf( szBuffer, "%d", nNumber );      
		return std::string(szBuffer);
	}

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
};
