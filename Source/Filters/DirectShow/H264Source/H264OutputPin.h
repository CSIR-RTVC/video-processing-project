#pragma once

// fwd
class H264SourceFilter;

class H264OutputPin : public CSourceStream
{
  friend class H264SourceFilter;

public:
  H264OutputPin(HRESULT *phr, H264SourceFilter *pFilter);
  ~H264OutputPin();

  // Override the version that offers exactly one media type
  HRESULT GetMediaType(CMediaType *pMediaType);
  HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest);
  HRESULT FillBuffer(IMediaSample *pSample);

  // Quality control
  // Not implemented because we aren't going in real time.
  // If the file-writing filter slows the graph down, we just do nothing, which means
  // wait until we're unblocked. No frames are ever dropped.
  STDMETHODIMP Notify(IBaseFilter *pSelf, Quality q)
  {
    return E_FAIL;
  }

protected:
  H264SourceFilter* m_pFilter;

  //int m_iWidth;
  //int m_iHeight;
  int m_iCurrentFrame;
  REFERENCE_TIME m_rtFrameLength;

  CCritSec m_cSharedState;            // Protects our internal state
};
