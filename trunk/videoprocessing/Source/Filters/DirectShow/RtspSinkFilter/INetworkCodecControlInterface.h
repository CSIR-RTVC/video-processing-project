#pragma once

class INetworkCodecControlInterface
{
public:
  virtual ~INetworkCodecControlInterface()
  {

  }

  virtual void setFramebitLimit(unsigned uiFrameBitLimit) = 0;

  virtual void generateIdr() = 0;

};
