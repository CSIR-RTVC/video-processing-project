#include "stdafx.h"
#include "RtspSinkFilter.h"
#include <cassert>
#include "RtspSinkInputPin.h"
#include <Shared/StringUtil.h>

RtspSinkFilter::RtspSinkFilter( IUnknown* pUnk, HRESULT* phr )
  : CBaseFilter(NAME("CSIR VPP RTSP Sink Filter"), pUnk, &m_stateLock, CLSID_CSIR_VPP_RtspSinkFilter, phr)
{
#if 0
  DbgSetModuleLevel(LOG_MEMORY, 5);
#endif

	// Init CSettingsInterface
	initParameters();

  if (SUCCEEDED(*phr))
  {
    // create one input pin
    createInputPin(phr);
  }
}

RtspSinkFilter::~RtspSinkFilter(void)
{
  // Release all the COM interfaces
  for (size_t i = 0; i < m_vInputPins.size(); i++)
  {
    m_vInputPins[i]->Release();
  }
}

CUnknown *WINAPI RtspSinkFilter::CreateInstance( IUnknown* pUnk, HRESULT* phr )
{
	RtspSinkFilter* pRtspSource = new RtspSinkFilter(pUnk, phr);
	if (!pRtspSource)
	{
		*phr = E_OUTOFMEMORY;
	}
	return pRtspSource;
}

STDMETHODIMP RtspSinkFilter::NonDelegatingQueryInterface( REFIID riid, void **ppv )
{
  if(riid == (IID_ISettingsInterface))
	{
		return GetInterface((ISettingsInterface*) this, ppv);
	}
	else if (riid == IID_IStatusInterface)
	{
		return GetInterface((IStatusInterface*) this, ppv);
	}
	else if (riid == IID_IFileSinkFilter)
	{
		return GetInterface((IFileSinkFilter*) this, ppv);
	}
	else
	{
		return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
	}
}

void RtspSinkFilter::OnConnect(int nIndex)
{
  // create at most two input pins
  if (m_vInputPins.size() == 1)
  {
    HRESULT hr(S_OK);
    createInputPin(&hr);
    assert(SUCCEEDED(hr));
  }
}

void RtspSinkFilter::OnDisconnect(int nIndex)
{
  // if both pins are now not connected
  // remove one of them
  if (m_vInputPins.size() == 2)
  {
    if ((nIndex == 0 && !m_vInputPins[1]->IsConnected()) ||
      (nIndex == 1 && !m_vInputPins[0]->IsConnected()))
    {
      m_vInputPins[1]->Release();
      m_vInputPins.pop_back();
      // Refresh enumerator
      IncrementPinVersion();
    }
  }
}

void RtspSinkFilter::createInputPin(HRESULT* pHr)
{
  // Create the output pin using the size of the output pin vector as an ID
  RtspSinkInputPin* pPin = new RtspSinkInputPin(pHr, this, m_vInputPins.size());
  pPin->AddRef();
  // Add to list
  m_vInputPins.push_back(pPin);
  // Refresh enumerator
  IncrementPinVersion();
}

int RtspSinkFilter::GetPinCount()
{
	return (int)(m_vInputPins.size());
}

CBasePin * RtspSinkFilter::GetPin( int n )
{
	if ((n >= GetPinCount()) || (n < 0))
		return NULL;

	return m_vInputPins[n];
}

STDMETHODIMP RtspSinkFilter::FindPin( LPCWSTR Id, IPin **ppPin )
{
	//Find pin according to it's name
	CheckPointer(ppPin,E_POINTER);
	ValidateReadWritePtr(ppPin,sizeof(IPin *));

	const char* szInput = StringUtil::wideToStl(Id).c_str();

	char szTemp[10];
	ZeroMemory(szTemp, 10);
	memcpy(szTemp, szInput, 6);
	szTemp[6] = '\0';
	if (0 == strcmp(szTemp, "Input")) 
	{
		const char* szID = szInput + 7;
		int nId = atoi(szID);
		*ppPin = GetPin(nId);
	} 
	else 
	{
		*ppPin = NULL;
		return VFW_E_NOT_FOUND;
	}

	HRESULT hr = NOERROR;
	//  AddRef() returned pointer - but GetPin could fail if memory is low.
	if (*ppPin) 
	{
		(*ppPin)->AddRef();
	} 
	else 
	{
		hr = E_OUTOFMEMORY;  // probably.  There's no pin anyway.
	}
	return hr;
}

STDMETHODIMP RtspSinkFilter::Run( REFERENCE_TIME tStart )
{
  CAutoLock cAutoLock(&m_stateLock);
  // start live555 thread
  
  return CBaseFilter::Run(tStart);
}

STDMETHODIMP RtspSinkFilter::Stop()
{
	CAutoLock cAutoLock(&m_stateLock);

  // stop live555 thread
	return CBaseFilter::Stop();
}

STDMETHODIMP RtspSinkFilter::Pause()
{
	CAutoLock cAutoLock(&m_stateLock);
	return CBaseFilter::Pause();
}

HRESULT RtspSinkFilter::sendMediaSample( unsigned uiId, IMediaSample* pSample )
{
  REFERENCE_TIME tStart, tStop;
  pSample->GetTime(&tStart, &tStop);

  DbgLog((LOG_TRACE, 1, TEXT("tStart(%s), tStop(%s), Diff(%d ms), Bytes(%d)"),
    (LPCTSTR) CDisp(tStart),
    (LPCTSTR) CDisp(tStop),
    pSample->GetActualDataLength()));

  // get Data
  PBYTE pbData;
  HRESULT hr = pSample->GetPointer(&pbData);
  if (FAILED(hr)) {
    return hr;
  }

  switch (m_mMediaSubtype[uiId])
  {
  case MST_H264:
  {
#if 0
    // remove start codes
    uint8_t startCode3[3] = { 0, 0, 1 };
    uint8_t startCode4[4] = { 0, 0, 0, 1 };
    if (memcmp(startCode4, pbData, 4) != 0)
    {
      LOG(WARNING) << "Invalid sample: no start code detected";
    }

    double dStartTime = (double)tStart / (10000000.0);
    std::vector<rtp_plus_plus::media::MediaSample> mediaSamples;
    uint32_t uiPrevIndex = 0;
    uint32_t uiPrevLen = 4;
    for (size_t i = 4; i < (uint32_t)pSample->GetActualDataLength() - 3; ++i)
    {
      if (memcmp(startCode3, &pbData[i], 3) == 0)
      {
        // now use rtp++ to send sample
        rtp_plus_plus::media::MediaSample mediaSample;
        mediaSample.setStartTime(dStartTime);

        uint32_t uiLen = i - uiPrevIndex - uiPrevLen; // -1 for the 4th byte
        // 3 or 4 byte in *current* start code?
        if (pbData[i - 1] == 0)
        {
          --uiLen;
        }
        BYTE* pData = new BYTE[uiLen];
        memcpy(pData, &pbData[uiPrevIndex + uiPrevLen], uiLen);
        mediaSample.setData(pData, uiLen);
        mediaSamples.push_back(mediaSample);

        // update current info
        uiPrevIndex = (pbData[i - 1] == 0) ? i - 1 : i;
        uiPrevLen = (pbData[i - 1] == 0) ? 4 : 3;
      }
    }
    // push_back final sample
    rtp_plus_plus::media::MediaSample mediaSample;
    mediaSample.setStartTime(dStartTime);
    uint32_t uiLen = pSample->GetActualDataLength() - uiPrevLen;
    BYTE* pData = new BYTE[uiLen];
    memcpy(pData, &pbData[uiPrevIndex + uiPrevLen], uiLen);
    mediaSample.setData(pData, uiLen);
    mediaSamples.push_back(mediaSample);

    m_pRtspServer->sendVideo(mediaSamples);
#endif
    break;
  }
  case MT_AUDIO:
  {
#if 0
    // send the sample using our RTP library
    // make a copy of the data, maybe we can avoid this?
    uint32_t uiLen = pSample->GetActualDataLength();
    BYTE* pData = new BYTE[uiLen];
    memcpy(pData, pbData, uiLen);

    // now use rtp++ to send sample
    double dStartTime = (double)tStart / (10000000.0);
    rtp_plus_plus::media::MediaSample mediaSample;
    mediaSample.setStartTime(dStartTime);
    mediaSample.setData(pData, uiLen);

    m_pRtspServer->sendAudio(mediaSample);
#endif
    break;
  }
  default:
  {
    assert(false);
  }
  }
  return S_OK;
}
