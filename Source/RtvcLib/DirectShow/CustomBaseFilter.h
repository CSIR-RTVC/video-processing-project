/** @file

MODULE				: CustomBaseFilter

FILE NAME			: CustomBaseFilter.h

DESCRIPTION			: Base class for CSIR DirectShow filters with one input and one output pin.

LICENSE: Software License Agreement (BSD License)

Copyright (c) 2008 - 2014, CSIR
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
#include <DirectShow/CStatusInterface.h>
#include <DirectShow/CSettingsInterface.h>

typedef std::vector<GUID*> GUIDList_t;

/**
 * \defgroup DirectShowFilters DirectShow Filters
 * \defgroup DirectShowLib DirectShow Library
 * \defgroup ImageLib Image Library
 */

/**
 * \ingroup DirectShowLib
 *  Base class for CSIR 1 to 1 transform filters. This class inherits from CSettingsInterface and CStatusInterface to give all our filters basic configuration and notification functionality.
 */
class CCustomBaseFilter : public CTransformFilter, public CSettingsInterface, public CStatusInterface
{
  /////this needs to be declared for the extra interface (adds the COM AddRef, etc methods)
  //DECLARE_IUNKNOWN;

public:
  /// Constructor
  CCustomBaseFilter(TCHAR *pObjectName, LPUNKNOWN lpUnk, CLSID clsid);
  /// Destructor
  virtual ~CCustomBaseFilter(void);

  /// Overridden method which validates the input media type against the ones accepted by this filter
  virtual HRESULT CheckInputType(const CMediaType *pMediaType);

  /// Helper method which allows us to capture the input output media type properties
  /// Override this method to set other initialization code but call this base class method first
  virtual HRESULT SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt);

  /// Transform code
  /**
   * @brief This method takes the source sample and transforms it and writes the transformed sample to the destination
   * This method is called by the transform input pin's Receive method.
   * @param pSource The source sample
   * @param pDest The destination sample
   */
  HRESULT Transform(IMediaSample *pSource, IMediaSample *pDest);

  /// override this to publicize our interfaces
  STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

  /// This method needs to overridden by child classes - add all possible accepted input combinations
  /// using the AddInputType method
  virtual void InitialiseInputTypes() PURE;

protected:
  /**
   * Used for Media Type Negotiation
   * Call the AddInputType method in your overridden InitialiseInputTypes
   * method to add your acceptable media types, subtypes and formats.
   */
  void AddInputType(const GUID* pType, const GUID* pSubType, const GUID* pFormat);

  /**
   * This method must be overridden in the sub class to apply the filter specific transformation
   * It must return the size of the resulting transformed output buffer
   */
  virtual void ApplyTransform(BYTE* pBufferIn, long lInBufferSize, long lActualDataLength, BYTE* pBufferOut, long lOutBufferSize, long& lOutActualDataLength) PURE;

  /// Video header of input
  VIDEOINFOHEADER m_videoInHeader;
  /// Video header of output
  VIDEOINFOHEADER m_videoOutHeader;
  // Other video information
  /// Height of image
  int m_nInHeight;
  /// Width of image
  int m_nInWidth;
  /// Number of pixels in image
  int m_nInPixels;
  /// Height in output image
  int m_nOutHeight;
  /// Width in output image
  int m_nOutWidth;
  /// Number of pixels in output image
  int m_nOutPixels;
  /// Bit count of image
  int m_nBitCount;

private:
  // Used for storing and checking connection types
  GUIDList_t m_vInTypes;
  GUIDList_t m_vInSubTypes;
  GUIDList_t m_vInFormats;
};
