#pragma once
#include <string>
#include "RtspSinkGuids.h"

class RtspSinkFilter;

class RtspSinkInputPin : public  CRenderedInputPin
{
	friend class RtspSinkFilter;
public:

	/// Constructor
	RtspSinkInputPin(HRESULT* pHr, RtspSinkFilter* pFilter, unsigned uiId);
	/// Destructor
	virtual ~RtspSinkInputPin(void);

  /**
   * @brief getMLine retrieves the m-line for the SDP description
   */
  std::string getMLine() const { return m_sMLine;  }

  /**
  * @brief getRtspMediaType retrieves the media type
  */
  RTSP_MediaType getRtspMediaType() const { return m_eType; }

  /**
  * @brief getRtspMediaSubtype retrieves the media subtype
  */
  RTSP_MediaSubtype getRtspMediaSubtype() const { return m_eSubtype; }

  // lifetime management for pins is normally delegated to the filter, but
  // we need to be able to create and delete them independently, so keep 
  // a separate refcount.
  STDMETHODIMP_(ULONG) NonDelegatingRelease()
  {
    return CUnknown::NonDelegatingRelease();
  }
  STDMETHODIMP_(ULONG) NonDelegatingAddRef()
  {
    return CUnknown::NonDelegatingAddRef();
  }
protected:

  /// IMemInputPin
  STDMETHODIMP Receive( IMediaSample *pSample );
  /// CBasePin
  virtual HRESULT CheckMediaType( const CMediaType *pmt);
  /// CBasePin
  virtual HRESULT GetMediaType( int iPosition, CMediaType *pMediaType );
  // connection management -- used to maintain one free pin
  HRESULT BreakConnect();
  HRESULT CompleteConnect(IPin *pReceivePin);

private:

	/// Sink filter
	RtspSinkFilter* m_pFilter;
	/// Protects our internal state
  CCritSec m_stateLock;    // Sample critical section
  CCritSec m_receiveLock;    // Sample critical section

  // To identify which input pin media samples came from
  unsigned m_uiId;

  RTSP_MediaType m_eType;
  RTSP_MediaSubtype m_eSubtype;

  std::string m_sMLine;
	//// Audio parameters
	//int m_nBitsPerSample;
	//int m_nChannels;
	//int m_nBytesPerSecond;
	//int m_nSamplesPerSecond;
 // unsigned m_uiSamplingRate;

 // // Video Parameters
 // int m_nWidth;
 // int m_nHeight;
 // int m_nSize;
 // // H264
 // std::string m_sSps;
 // std::string m_sPps;
};
