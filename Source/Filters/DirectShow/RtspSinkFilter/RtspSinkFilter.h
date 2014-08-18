#pragma once
#include <map>
#include <memory>
#include <DirectShow/CSettingsInterface.h>
#include <DirectShow/CStatusInterface.h>

#include "RtspSinkGuids.h"

// Forward
class RtspSinkInputPin;

/**
 * @brief This RtspSinkFilter class supports at most two input pins, one for video and one for audio.
 * The RtspSinkFilter will periodically send RTSP REGISTER commands to the specified proxy once the graph is played.
 * It will send these commands while no RTSP session is active. Once a session has been setup, it will cease sending
 * REGISTER requests to the specified client/proxy.
 * If the REGISTER succeeds it will await the RTSP session setup from the RTSP client or proxy
 */
class RtspSinkFilter : public CBaseFilter,				  /* Source Filter */
                       public CSettingsInterface,	  /* Rtvc Settings Interface */
                       public CStatusInterface	    /* Rtvc Status Interface */
{
	///this needs to be declared for the extra interface (adds the COM AddRef, etc methods)
	DECLARE_IUNKNOWN;

	/// Give the pin friend access
	friend class RtspSinkInputPin;

public:
	/// DLL factory method
	static CUnknown *WINAPI CreateInstance(IUnknown* pUnk, HRESULT* phr);

	/// override this to publicize our interfaces
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);
	
	/// From CSettingsInterface
	virtual void initParameters()
	{
    // reuse connection of RTSP register
    // addParameter("reuse_connection", &m_bReuseConnection, false);
  }

	/// CBase Filter methods - Overridden since we don't just have one input and output pin as the standard transform filter does
	virtual int GetPinCount();
	/// Override this method: the pins get created in this method of the transform filter
	virtual CBasePin * GetPin(int n);
	/// Method needed to connect pins based on their names
	virtual STDMETHODIMP FindPin(LPCWSTR Id, IPin **ppPin);

  /// Connection management
  /// Overridden to create more input pins on connection
  virtual void OnConnect(int nIndex);
  /// Overridden to remove unused input pins on disconnect
  virtual void OnDisconnect(int nIndex);

  STDMETHODIMP Run(REFERENCE_TIME tStart);
  STDMETHODIMP Pause();
  STDMETHODIMP Stop();
	
  HRESULT sendMediaSample(unsigned uiId, IMediaSample* pSample);
private:

	///Private Constructor
	RtspSinkFilter(IUnknown* pUnk, HRESULT* phr);
	virtual ~RtspSinkFilter(void);

  //void createInputPin(HRESULT* pHr, uint32_t uiPayloadType);
  /// Creates the input pins based on the SDP
  //STDMETHODIMP createInputPins();
  void createInputPin(HRESULT* pHr);

	/// vector of output pins
	std::vector<RtspSinkInputPin*> m_vInputPins;

	CCritSec m_stateLock;

  // map for incoming media samples mapping pin index to type
  std::map<unsigned, RTSP_MediaSubtype> m_mMediaSubtype;
};
