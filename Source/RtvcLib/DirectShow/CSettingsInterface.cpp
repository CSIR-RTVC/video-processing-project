/** @file

MODULE				: CSettingsInterface

FILE NAME			: CSettingsInterface.cpp

DESCRIPTION			: 
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008 - 2012, CSIR
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
#include "CSettingsInterface.h"

#include <Shared/StringUtil.h>
#include <Shared/Conversion.h>

CSettingsInterface::CSettingsInterface(void)
{;}

CSettingsInterface::~CSettingsInterface(void)
{;}

void CSettingsInterface::addParameter( const char* szParamName, int* pAddr, int nDefaultValue, bool bReadOnly, std::vector<int> vAllowedValues)
{
	// Add the param to the map
	std::string sParamName(szParamName);
	m_mParamTypes[sParamName] = RTVC_INT;
	m_intParams.addParam(std::string(szParamName), pAddr, nDefaultValue, bReadOnly, vAllowedValues);
}

void CSettingsInterface::addParameter( const char* szParamName, unsigned* pAddr, unsigned uiDefaultValue, bool bReadOnly, std::vector<unsigned> vAllowedValues)
{
  // Add the param to the map
  std::string sParamName(szParamName);
  m_mParamTypes[sParamName] = RTVC_UINT;
  m_uintParams.addParam(std::string(szParamName), pAddr, uiDefaultValue, bReadOnly, vAllowedValues);
}

void CSettingsInterface::addParameter( const char* szParamName, std::string* pAddr, std::string sDefaultValue, bool bReadOnly, std::vector<std::string> vAllowedValues )
{
	// Add the param to the map
	std::string sParamName(szParamName);
	m_mParamTypes[sParamName] = RTVC_STRING;
	m_stringParams.addParam(std::string(szParamName), pAddr, sDefaultValue, bReadOnly, vAllowedValues);
}

void CSettingsInterface::addParameter( const char* szParamName, bool* pAddr, bool bDefaultValue, bool bReadOnly, std::vector<bool> vAllowedValues )
{
	// Add the param to the map
	std::string sParamName(szParamName);
	m_mParamTypes[sParamName] = RTVC_BOOL;
	m_boolParams.addParam(std::string(szParamName), pAddr, bDefaultValue, bReadOnly, vAllowedValues);
}

void CSettingsInterface::addParameter( const char* szParamName, double* pAddr, double dDefaultValue, bool bReadOnly, std::vector<double> vAllowedValues )
{
	// Add the param to the map
	std::string sParamName(szParamName);
	m_mParamTypes[sParamName] = RTVC_DOUBLE;
	m_doubleParams.addParam(std::string(szParamName), pAddr, dDefaultValue, bReadOnly, vAllowedValues);
}

STDMETHODIMP CSettingsInterface::GetParameter( const char* type, int nBufferSize, char* value, int* length )
{
	// Look up if the parameter exists

	PARAM_TYPE_MAP::iterator it = m_mParamTypes.find(std::string(type));
	if ( it == m_mParamTypes.end())
	{
		return E_FAIL;
	}
	else
	{
		std::string sParamName = std::string(type);
		RTVC_PARAM_TYPES eType = it->second;
		switch (eType)
		{
		case RTVC_STRING:
			{
				std::string* pValue = m_stringParams.getParamAddress(sParamName);
				if (pValue)
				{
					if (pValue->length() < nBufferSize)
					{
						memcpy(value, (void*)pValue->c_str(), pValue->length());
						*length = pValue->length();
						value[*length] = 0;
						return S_OK;
					}
					else
					{
						return E_OUTOFMEMORY;
					}
				}
				return E_FAIL;
				break;
			}
		case RTVC_INT:
			{
				// Check the int map
				int* pValue = m_intParams.getParamAddress(sParamName);
				if (pValue)
				{
					std::string sValue =  toString(*pValue);
					if (sValue.length() < nBufferSize)
					{
						memcpy(value, (void*)sValue.c_str(), sValue.length());
						*length = sValue.length();
						value[*length] = 0;
						return S_OK;
					}
					else
					{
						return E_OUTOFMEMORY;
					}
				}
				return E_FAIL;
				break;
			}
    case RTVC_UINT:
      {
        // Check the int map
        unsigned* pValue = m_uintParams.getParamAddress(sParamName);
        if (pValue)
        {
          std::string sValue =  toString(*pValue);
          if (sValue.length() < (unsigned)nBufferSize)
          {
            memcpy(value, (void*)sValue.c_str(), sValue.length());
            *length = sValue.length();
            value[*length] = 0;
            return S_OK;
          }
          else
          {
            return E_OUTOFMEMORY;
          }
        }
        return E_FAIL;
        break;
      }
		case RTVC_BOOL:
			{
				// Check the int map
				bool* pValue = m_boolParams.getParamAddress(sParamName);
				if (pValue)
				{
					std::string sValue =  StringUtil::boolToString(*pValue);
					if (sValue.length() < nBufferSize)
					{
						memcpy(value, (void*)sValue.c_str(), sValue.length());
						*length = sValue.length();
						value[*length] = 0;
						return S_OK;
					}
					else
					{
						return E_OUTOFMEMORY;
					}
				}
				return E_FAIL;
				break;
			}
		case RTVC_DOUBLE:
			{
				// Check the int map
				double* pValue = m_doubleParams.getParamAddress(sParamName);
				if (pValue)
				{
					std::string sValue =  StringUtil::doubleToString(*pValue);
					if (sValue.length() < nBufferSize)
					{
						memcpy(value, (void*)sValue.c_str(), sValue.length());
						*length = sValue.length();
						value[*length] = 0;
						return S_OK;
					}
					else
					{
						return E_OUTOFMEMORY;
					}
				}
				return E_FAIL;
				break;
			}
		}
	}

	// if so, look up the type:
	return S_OK;
}

STDMETHODIMP CSettingsInterface::SetParameter( const char* type, const char* value )
{
	// Find out which group it's in
	PARAM_TYPE_MAP::iterator it = m_mParamTypes.find(std::string(type));
	if ( it == m_mParamTypes.end())
	{
		return E_FAIL;
	}
	else
	{
		RTVC_PARAM_TYPES eType = it->second;
		std::string sParamName = type;
		
		switch (eType)
		{
		case RTVC_STRING:
			{
				// Check if the parameter exists
				if (m_stringParams.contains(sParamName))
				{
					std::string sValue(value);
					if (m_stringParams.setParameterValue(sParamName, sValue))
					{
						return S_OK;
					}
				}
				return E_FAIL;
				break;
			}
		case RTVC_INT:
			{
				// Check if the parameter exists
				if (m_intParams.contains(sParamName))
				{
					// Get int value
					int nNewValue = atoi(value);
					if (m_intParams.setParameterValue(sParamName, nNewValue))
					{
						return S_OK;
					}
				}
				return E_FAIL;
				break;
			}
    case RTVC_UINT:
      {
        // Check if the parameter exists
        if (m_uintParams.contains(sParamName))
        {
          // Get int value
          unsigned uiNewValue = convert<unsigned>(value);
          if (m_uintParams.setParameterValue(sParamName, uiNewValue))
          {
            return S_OK;
          }
        }
        return E_FAIL;
        break;
      }
		case RTVC_BOOL:
			{
				// Allow 0, 1, TRUE, true, FALSE and false
				// Check if the parameter exists
				if (m_boolParams.contains(sParamName))
				{
					bool bValue = StringUtil::stringToBool(std::string(value));
					if (m_boolParams.setParameterValue(sParamName, bValue))
					{
						return S_OK;
					}
				}
				return E_FAIL;
				break;
			}
		case RTVC_DOUBLE:
			{
				// Allow 0, 1, TRUE, true, FALSE and false
				// Check if the parameter exists
				if (m_doubleParams.contains(sParamName))
				{
					double dValue = StringUtil::stringToDouble(std::string(value));
					if (m_doubleParams.setParameterValue(sParamName, dValue))
					{
						return S_OK;
					}
				}
				return E_FAIL;
				break;
			}
		}
	}
	return S_OK;
}

STDMETHODIMP CSettingsInterface::GetParameterSettings( char* szResult, int nSize )
{
	// Iterate over all the different params and display their current values
	std::string sResult;

	// Iterate over ints
	std::vector<std::string> vParams = m_intParams.getParameterNames();
	for (size_t i = 0; i < vParams.size(); i++)
	{
		int nValue = *(m_intParams.getParamAddress(vParams[i]));
		sResult += "Parameter name: " + vParams[i] + " Value: " + toString(nValue) + "\r\n";
	}

  // Iterate over uints
  vParams = m_uintParams.getParameterNames();
  for (size_t i = 0; i < vParams.size(); i++)
  {
    unsigned uiValue = *(m_uintParams.getParamAddress(vParams[i]));
    sResult += "Parameter name: " + vParams[i] + " Value: " + toString(uiValue) + "\r\n";
  }

	// Iterate over chars
	vParams = m_stringParams.getParameterNames();
	for (size_t i = 0; i < vParams.size(); i++)
	{
		std::string sValue = *(m_stringParams.getParamAddress(vParams[i]));
		sResult += "Parameter name: " + vParams[i] + " Value: " + sValue + "\r\n";
	}

	// Iterate over bool
	vParams = m_boolParams.getParameterNames();
	for (size_t i = 0; i < vParams.size(); i++)
	{
		bool bValue = *(m_boolParams.getParamAddress(vParams[i]));
		std::string sValue = (bValue)?"true":"false";
		sResult += "Parameter name: " + vParams[i] + " Value: " + sValue + "\r\n";
	}

	// Iterate over double
	vParams = m_doubleParams.getParameterNames();
	for (size_t i = 0; i < vParams.size(); i++)
	{
		double dValue = *(m_doubleParams.getParamAddress(vParams[i]));
		sResult += "Parameter name: " + vParams[i] + " Value: " + StringUtil::doubleToString(dValue) + "\r\n";
	}

	// TODO: bools and doubles

	if (sResult.length()< nSize)
	{
		memcpy(szResult, sResult.c_str(), sResult.length());
		szResult[sResult.length()] = 0;
		return S_OK;
	}
	// Buffer is too small
	return E_FAIL;
}

bool CSettingsInterface::revertParameter( const char* type )
{
	// Find out which group it's in
	PARAM_TYPE_MAP::iterator it = m_mParamTypes.find(std::string(type));
	if ( it != m_mParamTypes.end())
	{
		RTVC_PARAM_TYPES eType = it->second;
		std::string sParamName = type;

		switch (eType)
		{
		case RTVC_STRING:
			{
				// Check if the parameter exists
				if (m_stringParams.contains(sParamName))
				{
					return m_stringParams.revertToPreviousValue(sParamName);
				}
				return false;
			}
		case RTVC_INT:
			{
				// Check if the parameter exists
				if (m_intParams.contains(sParamName))
				{
					return m_intParams.revertToPreviousValue(sParamName);
				}
				return false;
			}
    case RTVC_UINT:
      {
        // Check if the parameter exists
        if (m_uintParams.contains(sParamName))
        {
          return m_uintParams.revertToPreviousValue(sParamName);
        }
        return false;
      }
		case RTVC_BOOL:
			{
				if (m_boolParams.contains(sParamName))
				{
					return m_boolParams.revertToPreviousValue(sParamName);
				}
				return false;
			}
		case RTVC_DOUBLE:
			{
				if (m_doubleParams.contains(sParamName))
				{
					return m_doubleParams.revertToPreviousValue(sParamName);
				}
				return false;
			}
		}
	}
	return false;
}
