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
 * which the H.264 codec can be configured with.
 *
 * @TODO: make bitrates dynamic (configurable via constructor or setter)
 */
class StepBasedRateController : public lme::IRateController
{
public:

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

    // TODO: can we set the starting frame bit limit here?
    // When is this code called in terms of the DS thread of execution?
    // No: this is in the constructor of the RtspSinkFilter
    // and the pins are not connected yet.
    // m_pNetworkControlInterface->setFramebitLimit(m_vFrameBitLimits[m_uiFrameBitLimitIndex]);
  }

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
