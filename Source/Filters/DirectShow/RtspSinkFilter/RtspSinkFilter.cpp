#include "stdafx.h"
#include "RtspSinkFilter.h"
#include "RtspSinkInputPin.h"
#include "StepBasedRateController.h"
#include "DirectShowNetworkCodecControlInterface.h"
#include <cassert>
#include <Shared/StringUtil.h>
#include <Media/MediaTypes.h>
#include <Media/PacketManagerMediaChannel.h>
#include <Media/SingleChannelManager.h>
#include <Media/RtspService.h>
#include <SimpleRateAdaptation/SimpleRateAdaptationFactory.h>

#define CHANNEL_ID 1

RtspSinkFilter::RtspSinkFilter( IUnknown* pUnk, HRESULT* phr )
  : CBaseFilter(NAME("CSIR VPP RTSP Sink Filter"), pUnk, &m_stateLock, CLSID_CSIR_VPP_RtspSinkFilter, phr),
  m_channelManager(CHANNEL_ID),
  m_pDsNetworkControlInterface(new DirectShowNetworkCodecControlInterface(this)),
  m_pFactory(new lme::SimpleRateAdaptationFactory()),
  m_pRateController(new StepBasedRateController(m_pDsNetworkControlInterface)),
  m_rtspService(m_channelManager, m_pFactory, m_pRateController),
  m_hLiveMediaThreadHandle(NULL),
  m_hLiveMediaStopEvent(NULL),
  m_dwThreadID(0),
  m_bHaveSeenSpsPpsIdr(false),
  m_uiRtspPort(554)
{
  VLOG(2) << "RtspSinkFilter()";
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

  // Create Live Media Event loop handle - this will be used to notify the main thread that the live Media RTSP thread has finished
  m_hLiveMediaStopEvent = CreateEvent(
    NULL,                       // default security attributes
    FALSE,                      // auto-reset event
    FALSE,                      // initial state is nonsignaled
    TEXT("LiveMediaStopEvent")  // object name
    );

  m_hLiveMediaInitEvent = CreateEvent(
    NULL,                       // default security attributes
    FALSE,                      // auto-reset event
    FALSE,                      // initial state is nonsignaled
    TEXT("LiveMediaInitEvent")  // object name
    );

  m_rtspService.setOnClientSessionPlayCallback(boost::bind(&RtspSinkFilter::onRtspClientSessionPlay, this, _1));

}

RtspSinkFilter::~RtspSinkFilter(void)
{
  VLOG(2) << "~RtspSinkFilter()";
  
  if (m_hLiveMediaThreadHandle)
  {
    stopLive555EventLoop();

    CloseHandle(m_hLiveMediaThreadHandle);
    m_hLiveMediaThreadHandle = NULL;
  }

  CloseHandle(m_hLiveMediaStopEvent);
  m_hLiveMediaStopEvent = NULL;

  CloseHandle(m_hLiveMediaInitEvent);
  m_hLiveMediaInitEvent = NULL;

  // Release all the COM interfaces
  for (size_t i = 0; i < m_vInputPins.size(); ++i)
  {
    m_vInputPins[i]->Release();
  }

  if (m_pRateController) delete m_pRateController;
  if (m_pFactory) delete m_pFactory;
  if (m_pDsNetworkControlInterface) delete m_pDsNetworkControlInterface;
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
  if (riid == IID_ISpecifyPropertyPages)
  {
    return GetInterface(static_cast<ISpecifyPropertyPages*>(this), ppv);
  }
  else if(riid == (IID_ISettingsInterface))
	{
		return GetInterface((ISettingsInterface*) this, ppv);
	}
	else if (riid == IID_IStatusInterface)
	{
		return GetInterface((IStatusInterface*) this, ppv);
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

void RtspSinkFilter::startLive555EventLoop()
{
  VLOG(2) << "startLive555EventLoop";
  ResetEvent(m_hLiveMediaInitEvent);
  ResetEvent(m_hLiveMediaStopEvent);
  // Blocks until end of liveMedia eventloop
  m_ecInit = m_rtspService.init(static_cast<uint16_t>(m_uiRtspPort));
  // The Stop method waits for this event
  VLOG(2) << "Setting m_hLiveMediaInitEvent";
  SetEvent(m_hLiveMediaInitEvent);
  if (!m_ecInit)
  {
    m_rtspService.start();
    VLOG(2) << "Setting m_hLiveMediaStopEvent";
    SetEvent(m_hLiveMediaStopEvent);
  }
}

static DWORD WINAPI LiveMediaThread(LPVOID pvParam)
{
  VLOG(2) << "LiveMediaThread";
  RtspSinkFilter* pSourceFilter = (RtspSinkFilter*)pvParam;
  pSourceFilter->startLive555EventLoop();
  return S_OK;
}

STDMETHODIMP RtspSinkFilter::Run( REFERENCE_TIME tStart )
{
  VLOG(2) << "Run";
  CAutoLock cAutoLock(&m_stateLock);
  // start live555 thread
  if (m_hLiveMediaThreadHandle == NULL)
  {
    if (m_vInputPins[0]->IsConnected())
    {
      m_mMediaSubtype[0] = m_vInputPins[0]->getRtspMediaSubtype();
    }
    if (m_vInputPins.size() == 2 && m_vInputPins[1]->IsConnected())
    {
      m_mMediaSubtype[1] = m_vInputPins[1]->getRtspMediaSubtype();
    }

    // try and setup RTSP service
    int32_t iVideo = -1;
    int32_t iAudio = -1;

    for (SubtypeMap_t::iterator it = m_mMediaSubtype.begin(); it != m_mMediaSubtype.end(); ++it)
    {
      if (it->second == MST_H264)
      {
        iVideo = it->first;
        m_channelManager.setVideoSourceId(iVideo);
      }
      else if (it->second == MST_AMR)
      {
        iAudio = it->first;
        m_channelManager.setAudioSourceId(iAudio);
      }
      else if (it->second == MST_AAC)
      {
        iAudio = it->first;
        m_channelManager.setAudioSourceId(iAudio);
      }
    }

    // Create a new thread for the RTSP liveMedia event loop
    if (iVideo != -1 && iAudio != -1)
    {
      RtspSinkInputPin* pPin = m_vInputPins[iVideo];
      lme::VideoChannelDescriptor& videoDescriptor = pPin->m_videoDescriptor;
      assert(videoDescriptor.Codec == lme::H264);
      assert(videoDescriptor.Width > 0);
      assert(videoDescriptor.Height > 0);
   
      RtspSinkInputPin* pPin2 = m_vInputPins[iAudio];
      lme::AudioChannelDescriptor& audioDescriptor = pPin2->m_audioDescriptor;
      assert(audioDescriptor.Codec == lme::AMR || audioDescriptor.Codec == lme::AAC);

      // AAC can have zero bits per sample?
      // assert(audioDescriptor.BitsPerSample > 0);
      assert(audioDescriptor.SamplingFrequency > 0);
      assert(audioDescriptor.Channels > 0);
      boost::system::error_code ec = m_rtspService.createChannel(CHANNEL_ID, "live", videoDescriptor, audioDescriptor);
      if (ec)
      {
        LOG(WARNING) << "Error creating RTSP H264/" << audioDescriptor.Codec << " service channel: " << ec.message();
        // Don't fail here, this just means that the channels could have been created previously i.e. in a previous play of the graph
        //return E_FAIL;
      }
    }
    else if (iVideo != -1)
    {
      RtspSinkInputPin* pPin = m_vInputPins[iVideo];
      lme::VideoChannelDescriptor& videoDescriptor = pPin->m_videoDescriptor;
      assert(videoDescriptor.Codec == lme::H264);
      assert(videoDescriptor.Width > 0);
      assert(videoDescriptor.Height > 0);
      boost::system::error_code ec = m_rtspService.createChannel(CHANNEL_ID, "live", videoDescriptor);
      if (ec)
      {
        LOG(WARNING) << "Error creating RTSP H264 service channel: " << ec.message();
        // Don't fail here, this just means that the channels could have been created previously i.e. in a previous play of the graph
        //return E_FAIL;
      }
    }
    else if (iAudio != -1)
    {
      RtspSinkInputPin* pPin = m_vInputPins[iAudio];
      lme::AudioChannelDescriptor& audioDescriptor = pPin->m_audioDescriptor;
      assert(audioDescriptor.Codec == lme::AMR || audioDescriptor.Codec == lme::AAC);
      // AAC can have zero bits per sample?
      // assert(audioDescriptor.BitsPerSample > 0);
      assert(audioDescriptor.SamplingFrequency > 0);
      assert(audioDescriptor.Channels > 0);
      boost::system::error_code ec = m_rtspService.createChannel(CHANNEL_ID, "live", audioDescriptor);
      if (ec)
      {
        LOG(WARNING) << "Error creating RTSP " << audioDescriptor.Codec << " service channel: " << ec.message();
        // Don't fail here, this just means that the channels could have been created previously i.e. in a previous play of the graph
        //return E_FAIL;
      }
    }
    m_hLiveMediaThreadHandle = CreateThread(0, 0, LiveMediaThread, (void*)this, 0, &m_dwThreadID);

    // Wait for the liveMedia to initialise
    VLOG(2) << "Run waiting for m_hLiveMediaInitEvent";
    DWORD result = WaitForSingleObject(m_hLiveMediaInitEvent, INFINITE);
    // check result of init
    if (!m_ecInit)
    {
      return CBaseFilter::Run(tStart);
    }
    else
    {
      LOG(WARNING) << "Failed to init RTSP service: " << m_ecInit.message();
      return E_FAIL;
    }
  }
  // TODO: when is run called when the thread is not running??
  VLOG(2) << "Run m_hLiveMediaThreadHandle != NULL";
  return CBaseFilter::Run(tStart);
}

void RtspSinkFilter::stopLive555EventLoop()
{
  VLOG(2) << "stopLive555EventLoop";
  if (!m_ecInit)
  {
    boost::system::error_code ec = m_rtspService.stop();
    assert(!ec);
    // Wait for the liveMedia eventloop to finish
    VLOG(2) << "stopLive555EventLoop: waiting for m_hLiveMediaStopEvent";
    DWORD result = WaitForSingleObject(m_hLiveMediaStopEvent, INFINITE);
    CloseHandle(m_hLiveMediaThreadHandle);
    m_hLiveMediaThreadHandle = NULL;

    // Don't need to do this, handling this in RTSP service
    //// remove channels from service
    //ec = m_rtspService.removeChannel(CHANNEL_ID);
    //if (ec)
    //{
    //  LOG(WARNING) << "Error removing channel from RTSP service" << ec.message();
    //  // Don't fail here, this just means that the channels could have been created previously i.e. in a previous play of the graph
    //  //return E_FAIL;
    //}

  }
}

STDMETHODIMP RtspSinkFilter::Stop()
{
  VLOG(2) << "Stop";
  CAutoLock cAutoLock(&m_stateLock);
  // stop live555 thread
  stopLive555EventLoop();

	return CBaseFilter::Stop();
}

STDMETHODIMP RtspSinkFilter::Pause()
{
  VLOG(2) << "Pause";
  CAutoLock cAutoLock(&m_stateLock);
	return CBaseFilter::Pause();
}

bool isIdrFrame(unsigned char nalUnitHeader)
{
  unsigned uiForbiddenZeroBit = nalUnitHeader & 0x80;
  //assert(uiForbiddenZeroBit == 0);
  unsigned uiNalRefIdc = nalUnitHeader & 0x60;
  unsigned char uiNalUnitType = nalUnitHeader & 0x1f;
  switch (uiNalUnitType)
  {
    // IDR nal unit types
  case 5:
    return true;
  default:
    return false;
  }
}
bool isSps(unsigned char nalUnitHeader)
{
  unsigned uiForbiddenZeroBit = nalUnitHeader & 0x80;
  unsigned uiNalRefIdc = nalUnitHeader & 0x60;
  unsigned char uiNalUnitType = nalUnitHeader & 0x1f;
  switch (uiNalUnitType)
  {
    // IDR nal unit types
  case 7:
    return true;
  default:
    return false;
  }
}
bool isPps(unsigned char nalUnitHeader)
{
  unsigned uiForbiddenZeroBit = nalUnitHeader & 0x80;
  unsigned uiNalRefIdc = nalUnitHeader & 0x60;
  unsigned char uiNalUnitType = nalUnitHeader & 0x1f;
  switch (uiNalUnitType)
  {
    // IDR nal unit types
  case 8:
    return true;
  default:
    return false;
  }
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

//  VLOG(2) << "RtspSinkFilter::sendMediaSample: " << uiId;
  lme::PacketManagerMediaChannel& packetManager = m_channelManager.getPacketManager();

  double dStartTime = (double)tStart / (10000000.0);

//  VLOG(2) << "RtspSinkFilter::sendMediaSample: " << dStartTime;
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
#else
//    VLOG(2) << "RtspSinkFilter::sendMediaSample: H264";
    if (m_bHaveSeenSpsPpsIdr)
    {
      std::vector<lme::MediaSample> mediaSamples;
      lme::MediaSample mediaSample;
      mediaSample.setPresentationTime(dStartTime);
      uint32_t uiLen = pSample->GetActualDataLength();
      BYTE* pData = new BYTE[uiLen];
      memcpy(pData, pbData, uiLen);
      mediaSample.setData(pData, uiLen);
      mediaSamples.push_back(mediaSample);
      packetManager.addVideoMediaSamples(mediaSamples);
    }
    else
    {
      VLOG(2) << "Waiting for SPS/PPS/IDR";
      // simple check for now: check for SPS
      // TODO: improve! Proper parsing and check for IDR
      if (isSps(pbData[4]))
      {
        VLOG(2) << "SPS/PPS/IDR found";
        m_bHaveSeenSpsPpsIdr = true;
        std::vector<lme::MediaSample> mediaSamples;
        lme::MediaSample mediaSample;
        mediaSample.setPresentationTime(dStartTime);
        uint32_t uiLen = pSample->GetActualDataLength();
        BYTE* pData = new BYTE[uiLen];
        memcpy(pData, pbData, uiLen);
        mediaSample.setData(pData, uiLen);
        mediaSamples.push_back(mediaSample);
        packetManager.addVideoMediaSamples(mediaSamples);
      }
    }
#endif
    break;
  }
  case MST_AMR:
  case MST_AAC:
  {
//    VLOG(2) << "RtspSinkFilter::sendMediaSample: AMR";
    std::vector<lme::MediaSample> mediaSamples;
    lme::MediaSample mediaSample;
    mediaSample.setPresentationTime(dStartTime);
    uint32_t uiLen = pSample->GetActualDataLength();
    BYTE* pData = new BYTE[uiLen];
    memcpy(pData, pbData, uiLen);
    mediaSample.setData(pData, pSample->GetActualDataLength());
    mediaSamples.push_back(mediaSample);
//    VLOG(2) << "RtspSinkFilter::sendMediaSample: Adding to packet manager";
    packetManager.addAudioMediaSamples(mediaSamples);
//    VLOG(2) << "RtspSinkFilter::sendMediaSample: Added to packet manager";
    break;
  }
  default:
  {
    assert(false);
  }
  }
  return S_OK;
}

void RtspSinkFilter::onRtspClientSessionPlay(unsigned uiClientSessionId)
{
  // Since a new client has just played the stream, trigger generation of an IDR.
  m_pDsNetworkControlInterface->generateIdr();
}
