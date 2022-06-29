#pragma once

#include <string>
#include <vector>
#include <math.h>
#include <assert.h>

#ifdef PHNQ_RACK
#include <rack.hpp>
#define PHNQ_LOG INFO
#else
#ifdef PHNQ_SEED
#include "daisy_seed.h"
extern daisy::DaisySeed *seedHw;
#define PHNQ_LOG seedHw->PrintLine
#else
#define PHNQ_LOG(format, ...) printf(format, ##__VA_ARGS__)
#endif
#endif

/**
 * This is the abstraction class that the generic module extends.
 * Implementation-specific code will hold an instance of a sublass of this
 * class, using it as a delegate for a variety of tasks such as:
 * - DSP
 * - managing control (i.e. CV) parameters
 */

/**
 * Stuff to get/set
 * - Audio in
 * - Audio out
 * - CV in
 *    - input jack
 *    - knob
 *    - button
 * - CV out
 *
 */

using namespace std;

namespace phnq
{
  struct FrameInfo
  {
    float sampleRate;
    float sampleTime;
  };

  struct IOConfig
  {
    size_t numAudioIns;  // max 2
    size_t numAudioOuts; // max 2
    size_t numParams;    // max 10 for params + CV ins
    size_t numCVIns;     // max 10 for params + CV ins
    size_t numCVOuts;    // max 2
    size_t numGateIns;   // max 18 for ins + outs
    size_t numGateOuts;  // max 18 for ins + outs
  };

  const float GATE_LOW = 0.f;
  const float GATE_HIGH = 1.f;
  const float GATE_LOW_THRESH = 0.01f;
  const float GATE_HIGH_THRESH = 0.2f;

  enum IOPortType
  {
    Audio, // nominal range of [-1, 1], clamped at [-2, 2] when setting output value
    CV,    // range of [0, 1]
    Gate,  // low is 0.f, hight is 1.f
    Param, // same as CV
  };

  enum IOPortDirection
  {
    Input,
    Output
  };

  struct IOPort;

  struct GateListener
  {
    virtual void gateValueDidChange(IOPort *gatePort) {}
  };

  struct IOPort
  {
  private:
    GateListener *gateListener;
    IOPortType type;
    IOPortDirection dir;
    float value = 0.f;
    std::string panelId;

  public:
    IOPort(GateListener *gateListener, IOPortType type, IOPortDirection dir, std::string panelId)
    {
      this->gateListener = gateListener;
      this->type = type;
      this->dir = dir;
      this->value = 0.f;
      this->panelId = panelId;
    }

    IOPortType getType()
    {
      return type;
    }

    IOPortDirection getDirection()
    {
      return dir;
    }

    float getValue()
    {
      return value;
    }

    void setValue(float value)
    {
      switch (type)
      {
      case IOPortType::Audio:
        this->value = dir == IOPortDirection::Input ? value : clamp(value, -2.f, 2.f);
        break;
      case IOPortType::Param:
      case IOPortType::CV:
        this->value = dir == IOPortDirection::Input ? value : clamp(value, 0.f, 1.f);
        break;
      case IOPortType::Gate:
        if (this->value == GATE_HIGH && value < GATE_LOW_THRESH)
        {
          this->value = GATE_LOW;
          gateListener->gateValueDidChange(this);
        }
        else if (this->value == GATE_LOW && value > GATE_HIGH_THRESH)
        {
          this->value = GATE_HIGH;
          gateListener->gateValueDidChange(this);
        }
        break;
      }
    }

    std::string getPanelId()
    {
      return panelId;
    }

    float clamp(float val, float min, float max)
    {
      return val < min ? min : val > max ? max
                                         : val;
    }
  };

  struct Engine : GateListener
  {
  private:
    IOConfig ioConfig = {0, 0, 0, 0, 0, 0, 0};
    FrameInfo frameInfo;
    vector<IOPort *> ioPorts;

    void assertCondition(std::string text, bool condition)
    {
      if (!condition)
      {
        PHNQ_LOG("***** Failed Assertion: %s", text.c_str());
      }
      assert(condition);
    }

    void validateIOConfig()
    {
      assertCondition("max 2 audio ins", ioConfig.numAudioIns <= 2);
      assertCondition("max 2 audio outs", ioConfig.numAudioOuts <= 2);
      assertCondition("max 10 params + CV ins", (ioConfig.numParams + ioConfig.numCVIns) <= 10);
      assertCondition("max 2 CV outs", ioConfig.numCVOuts <= 2);
      assertCondition("max 18 gate ins + gate outs", (ioConfig.numGateIns + ioConfig.numCVOuts) <= 18);
    }

  protected:
    IOPort *addIOPort(IOPortType type, IOPortDirection dir, std::string panelId)
    {
      IOPort *port = new IOPort(this, type, dir, panelId);
      ioPorts.push_back(port);
      if (type == IOPortType::Audio)
      {
        if (dir == IOPortDirection::Input)
        {
          ioConfig.numAudioIns++;
        }
        else
        {
          ioConfig.numAudioOuts++;
        }
      }
      else if (type == IOPortType::CV)
      {
        if (dir == IOPortDirection::Input)
        {
          ioConfig.numCVIns++;
        }
        else
        {
          ioConfig.numCVOuts++;
        }
      }
      else if (type == IOPortType::Gate)
      {
        if (dir == IOPortDirection::Input)
        {
          ioConfig.numGateIns++;
        }
        else
        {
          ioConfig.numGateOuts++;
        }
      }
      else if (type == IOPortType::Param)
      {
        if (dir == IOPortDirection::Input)
        {
          ioConfig.numParams++;
        }
        else
        {
          // Not a thing...
        }
      }

      validateIOConfig();

      return port;
    }

    virtual void sampleRateDidChange(float sampleRate) {}

    virtual void process(FrameInfo frameInfo) {}

  public:
    ~Engine()
    {
      for (vector<IOPort *>::iterator it = ioPorts.begin(); it != ioPorts.end();)
      {
        delete *it;
        it = ioPorts.erase(it);
      }
    }

    IOConfig getIOConfig()
    {
      return ioConfig;
    }

    vector<IOPort *> getIOPorts()
    {
      vector<IOPort *> ioPortsCopy = ioPorts;
      return ioPortsCopy;
    }

    void doProcess(FrameInfo frameInfo)
    {
      if (frameInfo.sampleRate != this->frameInfo.sampleRate)
      {
        this->frameInfo = frameInfo;
        sampleRateDidChange(frameInfo.sampleRate);
      }
      this->process(this->frameInfo);
    }
  };
}
