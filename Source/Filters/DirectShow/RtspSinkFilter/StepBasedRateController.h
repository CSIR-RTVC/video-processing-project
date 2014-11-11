#pragma once
#include <vector>
#include <Media/IRateController.h>
#include "INetworkCodecControlInterface.h"

#define FBS_26KBPS_10FPS 2600
#define FBS_32KBPS_10FPS 3200
#define FBS_38KBPS_10FPS 3800
#define FBS_44KBPS_10FPS 4400
#define FBS_50KBPS_10FPS 5000
#define FBS_56KBPS_10FPS 5600
#define FBS_64KBPS_10FPS 6400
#define FBS_72KBPS_10FPS 7200
#define FBS_84KBPS_10FPS 8400
#define FBS_92KBPS_10FPS 9200
#define FBS_100KBPS_10FPS 10000

/**
 * @brief This class stores a set of ordered framebitlimits
 * which the upstream H.264 codec can be configured with.
 *
 */
class StepBasedRateController : public lme::IRateController
{
public:
  /**
   * @brief Constructor
   */
  StepBasedRateController(INetworkCodecControlInterface* pNetworkControlInterface)
    : m_uiFrameBitLimitIndex(0),
    m_pNetworkControlInterface(pNetworkControlInterface)
  {
    assert(m_pNetworkControlInterface);

    m_vFrameBitLimits.push_back(FBS_26KBPS_10FPS);
    m_vFrameBitLimits.push_back(FBS_32KBPS_10FPS);
    m_vFrameBitLimits.push_back(FBS_38KBPS_10FPS);
    m_vFrameBitLimits.push_back(FBS_44KBPS_10FPS);
    m_vFrameBitLimits.push_back(FBS_50KBPS_10FPS);
    m_vFrameBitLimits.push_back(FBS_56KBPS_10FPS);
    m_vFrameBitLimits.push_back(FBS_64KBPS_10FPS);
    m_vFrameBitLimits.push_back(FBS_72KBPS_10FPS);
    m_vFrameBitLimits.push_back(FBS_84KBPS_10FPS);
    m_vFrameBitLimits.push_back(FBS_92KBPS_10FPS);
    m_vFrameBitLimits.push_back(FBS_100KBPS_10FPS);
  }
  /**
   * @brief Setter for frame bit limits.
   * @param vFrameBitLimits A vector storing the frame bit limits to be configured.
   * The vector may not be empty and may not contain 0 values.
   * @param uiFrameBitLimitIndex The index to be started off at. This must be in the valid range.
   * @return true if the configuration is valid, and false otherwise.
   */
  bool configure(const std::vector<unsigned>& vFrameBitLimits, unsigned uiFrameBitLimitIndex = 0)
  {
    if (vFrameBitLimits.empty() || uiFrameBitLimitIndex >= vFrameBitLimits.size()) return false;
    // check for zeros
    for (size_t i = 0; i < vFrameBitLimits.size(); ++i)
    {
      if (vFrameBitLimits[i] == 0)
      {
        return false;
      }
    }
    m_vFrameBitLimits = vFrameBitLimits;
    m_uiFrameBitLimitIndex = uiFrameBitLimitIndex;
    return true;
  }
  /**
   * @brief This method should be triggered by an external rate control.
   * It is the link between the rate control advise, and the step-based
   * bit-rate model.
   */
  virtual void controlBitrate(lme::SwitchDirection eSwitch)
  {
    switch (eSwitch)
    {
    case lme::SWITCH_DOWN:
    case lme::SWITCH_DOWN_MULTIPLE:
    {
      if (m_uiFrameBitLimitIndex > 0)
      {
        --m_uiFrameBitLimitIndex;
      }
      break;
    }
    case lme::SWITCH_UP:
    case lme::SWITCH_UP_MULTIPLE:
    {
      if (m_uiFrameBitLimitIndex < m_vFrameBitLimits.size() - 1)
      {
        ++m_uiFrameBitLimitIndex;
      }
      break;
    }
    case lme::SWITCH_DOWN_MIN:
    {
      m_uiFrameBitLimitIndex = 0;
      break;
    }
    case lme::SWITCH_UP_MAX:
    {
      m_uiFrameBitLimitIndex = m_vFrameBitLimits.size() - 1;
      break;
    }
    default:
    {
      break;
    }
    }
    m_pNetworkControlInterface->setFramebitLimit(m_vFrameBitLimits[m_uiFrameBitLimitIndex]);
  }

private:
  unsigned m_uiFrameBitLimitIndex;
  std::vector<unsigned> m_vFrameBitLimits;
  INetworkCodecControlInterface* m_pNetworkControlInterface;
};

