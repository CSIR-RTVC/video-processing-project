#pragma once
#include "INetworkCodecControlInterface.h"

// DirectShow includes
#pragma warning(push)     // disable for this header only
#pragma warning(disable:4312) 
// DirectShow
#include <Streams.h>
#pragma warning(pop)      // restore original warning level

#include <DirectShow/ComTypeDefs.h>
#include <DirectShow/DirectShowHelper.h>

/**
 * @brief DirectShow-based implementation of INetworkCodecControlInterface.\
 * The interface implements how the upstream codec can sets the current frame bit limit
 * or trigger the generation of an IDR frame.
 */
class DirectShowNetworkCodecControlInterface : public INetworkCodecControlInterface
{
public:
  /**
   * @brief Constructor
   */
  DirectShowNetworkCodecControlInterface(IBaseFilter* pThisFilter)
    :m_pThisFilter(pThisFilter),
    m_pCodecInterface(NULL),
    m_hrInterfaceAquired(S_OK)
  {

  }
  /**
   * @brief Destructor
   */
  virtual ~DirectShowNetworkCodecControlInterface()
  {
    m_pThisFilter = NULL;
    m_pCodecInterface = NULL;
  }
  /**
   * @brief This method sets the current frame bit limit of the upstream codec.
   *
   * This should only be called once the graph has been constructed as it has to 
   * traverse upstream pins and filters until it discovers the IID_ICodecControlInterface 
   * interface.
   */
  virtual void setFramebitLimit(unsigned uiFrameBitLimit)
  {
    VLOG(5) << "DirectShowNetworkCodecControlInterface::setFramebitLimit called: " << uiFrameBitLimit;
    // only try once to get the interface
    if (FAILED(m_hrInterfaceAquired))
    {
      LOG(WARNING) << "Previously failed to find CodecControlinterface";
      return;
    }
    if (!m_pCodecInterface)
    {
      m_hrInterfaceAquired = CDirectShowHelper::FindFirstInterface(m_pThisFilter, PINDIR_INPUT, IID_ICodecControlInterface, (void **)&m_pCodecInterface);
      if (FAILED(m_hrInterfaceAquired))
      {
        LOG(WARNING) << "Failed to find CodecControlinterface ";
        return;
      }
    }
    HRESULT hr = m_pCodecInterface->SetFramebitLimit(uiFrameBitLimit);
    if (FAILED(hr))
    {
      LOG(WARNING) << "Failed to set frame bit limit to " << uiFrameBitLimit;
    }
    else
    {
      VLOG(5) << "Set frame bit limit to " << uiFrameBitLimit;
    }
  }
  /**
   * @brief This method triggers the generation of an IDR.
   *
   * This should only be called once the graph has been constructed as it has to 
   * traverse upstream pins and filters until it discovers the IID_ICodecControlInterface 
   * interface.
   */
  virtual void generateIdr()
  {
    VLOG(5) << "DirectShowNetworkCodecControlInterface::generateIdr called";
    if (FAILED(m_hrInterfaceAquired)) return;
    if (!m_pCodecInterface)
    {
      m_hrInterfaceAquired = CDirectShowHelper::FindFirstInterface(m_pThisFilter, PINDIR_INPUT, IID_ICodecControlInterface, (void **)&m_pCodecInterface);
      if (FAILED(m_hrInterfaceAquired))
      {
        LOG(WARNING) << "Failed to find CodecControlinterface ";
        return;
      }
    }
    HRESULT hr = m_pCodecInterface->GenerateIdr();
    if (FAILED(hr))
    {
      LOG(WARNING) << "Failed to set generate IDR";
    }
  }

private:
  IBaseFilter* m_pThisFilter;
  ICodecControlInterfacePtr m_pCodecInterface;
  HRESULT m_hrInterfaceAquired;
};
