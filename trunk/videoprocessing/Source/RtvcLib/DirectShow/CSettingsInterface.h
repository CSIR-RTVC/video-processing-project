/** @file

MODULE				: CSettingsInterface

FILE NAME			: CSettingsInterface.h

DESCRIPTION			: The implementation of the COM ISettingsInterface
					DirectShow filters inheriting this implementation should implement the initParameters method in which 
					the addParameter method should be called for each member variable that is to be managed by this class.
					  
LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008 - 2011, CSIR
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
#pragma once

#include <Windows.h>
#include "SettingsInterface.h"

// STL
#include <algorithm>
#include <map>
#include <string>
#include <vector>

/**
 * \ingroup DirectShowLib
 * ISettingsInterface implementation.
 */
class CSettingsInterface :
	public ISettingsInterface
{
public:
	CSettingsInterface(void);
	virtual ~CSettingsInterface(void);
	/// This helper method reverts a member variables value back to the initial value and should be used e.g. if any of the postconditions of 
	/// SetParameter are unfulfilled. This is usually the case if other conditions exist upon which the success of SetParameter depends.
	/// One way to use this method is to override SetParameter and call the base class method. Then check the postcondition and if that has not been met,
	/// Call revertParameter
	/// @param[in] szParamName The name of the parameter that should be reverted to its previous value
	virtual bool revertParameter(const char* szParamName);

	/// Overridden from ISettingsInterface
	/// This method returns the value of the requested parameter.
	/// @return This method returns S_OK on success and E_FAIL on failure.
	/// @param[in] szParamName The name of the parameter
	/// @param[in] nBufferSize The size of the passed in buffer szValue
	/// @param[out] szValue The buffer to store the result in
	/// @param[out] pLength The length of the result
	STDMETHODIMP GetParameter( const char* szParamName, int nBufferSize, char* szValue, int* pLength);
	/// Overridden from ISettingsInterface
	/// This method sets the value of the requested parameter provided the parameter exists
	/// @return This method returns S_OK on success and E_FAIL on failure
	/// @param[in] szParamName The name of the parameter
	/// @param[in] szValue The value the parameter should be set to
	STDMETHODIMP SetParameter( const char* szParamName, const char* szValue);
	/// Overridden from ISettingsInterface
	/// This method iterates over all parameters and constructs a string containing all parameters current settings.
	/// @return This method returns S_OK on success and E_FAIL on failure e.g. if the passed in buffer is too small. 
	/// @param[out] szResult A buffer for the result of the method
	/// @param[in] nSize The size of the passed in buffer
	STDMETHODIMP GetParameterSettings(char* szResult, int nSize);

protected:
	/// This method needs to be overridden by child classes and called in the child class constructor
	/// It takes care of mapping the parameters to member variables
	/// Call the addParameter method from this method where <br>
	/// szParamName: is the name of the parameter<br>
	/// pAddr is a pointer to the member variable which will store the value<br>
	/// bReadOnly will ensure that the param value cannot be changed<br>
	/// vAllowedValues: If there are any items in this vector, only those will be deemed as valid values
	virtual void initParameters() = 0; 

	/// The addParameter methods should be called from inside initParameters at start up
	/// @param[in] szParamName The name of the parameter that should be reverted to its previous value
	/// @param[in] pAddr A pointer to the member variable that will store the value
	/// @param[in] nDefaultValue An optional default value for the member variable
	/// @param[in] bReadOnly Readonly parameters cannot be set using the SetParameter method
	/// @param[in] vAllowedValues A vector of allowed values. If the size of this vector > 0 only the values in the vector are considered valid
	virtual void addParameter(const char* szParamName, int* pAddr, int nDefaultValue, bool bReadOnly = false, std::vector<int> vAllowedValues = std::vector<int>());
	/// The addParameter methods should be called from inside initParameters at start up
  /// @param[in] szParamName The name of the parameter that should be reverted to its previous value
  /// @param[in] pAddr A pointer to the member variable that will store the value
  /// @param[in] uiDefaultValue An optional default value for the member variable
  /// @param[in] bReadOnly Readonly parameters cannot be set using the SetParameter method
  /// @param[in] vAllowedValues A vector of allowed values. If the size of this vector > 0 only the values in the vector are considered valid
  virtual void addParameter(const char* szParamName, unsigned* pAddr, unsigned uiDefaultValue, bool bReadOnly = false, std::vector<unsigned> vAllowedValues = std::vector<unsigned>());
	/// The addParameter methods should be called from inside initParameters at start up
	/// @param[in] szParamName The name of the parameter that should be reverted to its previous value
	/// @param[in] pAddr A pointer to the member variable that will store the value
	/// @param[in] szDefaultValue An optional default value for the member variable
	/// @param[in] bReadOnly Readonly parameters cannot be set using the SetParameter method
	/// @param[in] vAllowedValues A vector of allowed values. If the size of this vector > 0 only the values in the vector are considered valid
  virtual void addParameter(const char* szParamName, std::string* pAddr, std::string szDefaultValue, bool bReadOnly = false, std::vector<std::string> vAllowedValues = std::vector<std::string>());
	/// The addParameter methods should be called from inside initParameters at start up
	/// @param[in] szParamName The name of the parameter that should be reverted to its previous value
	/// @param[in] pAddr A pointer to the member variable that will store the value
	/// @param[in] bDefaultValue An optional default value for the member variable
	/// @param[in] bReadOnly Readonly parameters cannot be set using the SetParameter method
	/// @param[in] vAllowedValues A vector of allowed values. If the size of this vector > 0 only the values in the vector are considered valid
	virtual void addParameter(const char* szParamName, bool* pAddr, bool bDefaultValue, bool bReadOnly = false, std::vector<bool> vAllowedValues = std::vector<bool>());
	/// The addParameter methods should be called from inside initParameters at start up
	/// @param[in] szParamName The name of the parameter that should be reverted to its previous value
	/// @param[in] pAddr A pointer to the member variable that will store the value
	/// @param[in] dDefaultValue An optional default value for the member variable
	/// @param[in] bReadOnly Readonly parameters cannot be set using the SetParameter method
	/// @param[in] vAllowedValues A vector of allowed values. If the size of this vector > 0 only the values in the vector are considered valid
	virtual void addParameter(const char* szParamName, double* pAddr, double dDefaultValue, bool bReadOnly = false, std::vector<double> vAllowedValues = std::vector<double>());

private:

	enum RTVC_PARAM_TYPES
	{
		RTVC_STRING,
		RTVC_INT,
    RTVC_UINT,
		RTVC_BOOL,
		RTVC_DOUBLE
	};

	typedef std::map<std::string, CSettingsInterface::RTVC_PARAM_TYPES> PARAM_TYPE_MAP;


	std::vector<std::string> m_vParameters;
	
	/// Templated helper class to store string - parameter mappings, access attributes and allowed values
	template <class T>
	class RtvcParameterValue
	{
	public:
		RtvcParameterValue()
		{;}

		~RtvcParameterValue()
		{;}

		bool contains(const std::string& sName)
		{
			return (m_mAddresses.find(sName) != m_mAddresses.end());
		}

		std::vector<std::string> getParameterNames()
		{
			std::vector<std::string> vResults;
			for (std::map<std::string, T*>::iterator it = m_mAddresses.begin(); it != m_mAddresses.end(); it++)
			{
				vResults.push_back(it->first);
			}
			return vResults;
		}

		T* getParamAddress(const std::string& sName)
		{
			std::map<std::string, T*>::iterator it = m_mAddresses.find(sName);
			if (it != m_mAddresses.end())
			{
				return it->second;
			}
			else
			{
				return NULL;
			}
		}

		bool setParameterValue(const std::string& sName, T newValue)
		{
			std::map<std::string, std::vector<T> >::iterator it = m_mAllowedValues.find(sName);
			if (it != m_mAllowedValues.end())
			{
				// If access through this interface is read only then exit
				if (m_mReadOnly[sName] == true)
				{
					return false;
				}

				// Get allowed values if they exist
				std::vector<T> vAllowedValues = it->second;
				if (vAllowedValues.size() != 0)
				{
					// Only these values are allowed
					std::vector<T>::iterator it = std::find(vAllowedValues.begin(), vAllowedValues.end(), newValue);
					if (it != vAllowedValues.end())
					{
						// Get pointer to address
						T* pValue = getParamAddress(sName);
						// Store previous value
						m_mPreviousValues[sName] = *pValue;
						*pValue = newValue;
						return true;
					}
					else
					{
						// Allowed values does not contain value
						return false;
					}
				}
				else
				{
					T* pValue = getParamAddress(sName);
					// Store previous value
					m_mPreviousValues[sName] = *pValue;
					*pValue = newValue;
					return true;
				}
			}
			else
			{
				return false;
			}		
		}

		// Reverts the parameters value back to the previous one if it exists in the map
		// Parameters get stored in the previousmap on setting
		bool revertToPreviousValue(const std::string& sName)
		{
			std::map<std::string, T>::iterator it = m_mPreviousValues.find(sName);
			if (it != m_mPreviousValues.end())
			{
				// We found the previous value
				// Get pointer to address
				T* pValue = getParamAddress(sName);
				*pValue = it->second;
				return true;
			}
			else
			{
				return false;
			}
		}

		// This method should be called by child classes to initialise the parameter
		void addParam(const std::string& sName, T* pAddr, T defaultValue, bool bReadOnly, std::vector<T>& vAllowedValues)
		{
			// Check if it exists already
			if (m_mAddresses.find(sName) == m_mAddresses.end())
			{
				m_mAddresses[sName] = pAddr;
				m_mAllowedValues[sName] = vAllowedValues; 
				m_mReadOnly[sName] = bReadOnly;
				// Set initial value
				*pAddr = defaultValue;
			}
		}

		int getSize(){ return m_vAddresses.size();}
	private:
		std::map<std::string, T*> m_mAddresses;
		std::map<std::string, T> m_mPreviousValues;
		std::map<std::string, bool> m_mReadOnly;
		std::map<std::string, std::vector<T> > m_mAllowedValues;
	};

	RtvcParameterValue<int> m_intParams;
  RtvcParameterValue<unsigned> m_uintParams;
	RtvcParameterValue<double> m_doubleParams;
	RtvcParameterValue<bool> m_boolParams;
	RtvcParameterValue<std::string> m_stringParams;

	PARAM_TYPE_MAP m_mParamTypes;
};

