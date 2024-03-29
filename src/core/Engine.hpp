#pragma once

#include <string>
#include <vector>
#include <queue>
#include <math.h>
#include <assert.h>
#include <daisysp.h>

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
  const float FREQ_C1 = 32.7032f;
  const float CV_CHANGE_THRESHOLD = 0.00001f;

  static float pitchToFrequency(float pitch)
  {
    return FREQ_C1 * std::pow(2.f, pitch * 10.f);
  }

  static void assertCondition(std::string text, bool condition)
  {
    if (!condition)
    {
      PHNQ_LOG("***** Failed Assertion: %s", text.c_str());
    }
    assert(condition);
  }

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
    size_t numGateIns;   // max 18 for ins + outs + buttons + leds
    size_t numGateOuts;  // max 18 for ins + outs + buttons + leds
    size_t numButtons;   // max 18 for ins + outs + buttons + leds
    size_t numLeds;      // max 18 for ins + outs + buttons + leds
  };

  const float GATE_LOW = 0.f;
  const float GATE_HIGH = 1.f;
  const float GATE_LOW_THRESH = 0.01f;
  const float GATE_HIGH_THRESH = 0.2f;

  enum IOPortType
  {
    Audio,  // nominal range of [-1, 1], clamped at [-2, 2] when setting output value
    CV,     // range of [0, 1]
    Gate,   // low is 0.f, hight is 1.f
    Param,  // range of [0, 1]
    Button, // low is 0.f, hight is 1.f
    Led,    // range of [0, 1]
  };

  enum IOPortDirection
  {
    Input,
    Output
  };

  struct IOPort;

  struct PortListener
  {
    virtual void gateValueDidChange(IOPort *gatePort, bool high) {}
    virtual void cvValueDidChange(IOPort *cvPort, float value) {}
  };

  struct IOPort
  {
  private:
    PortListener *portListener;
    IOPortType type;
    IOPortDirection dir;
    float value;
    std::string panelId;
    uint16_t delay;
    queue<float> delayBuffer;

  public:
    IOPort(PortListener *portListener, IOPortType type, IOPortDirection dir, std::string panelId)
    {
      this->portListener = portListener;
      this->type = type;
      this->dir = dir;
      this->value = 0.f;
      this->panelId = panelId;
      this->delay = 0;
    }

    IOPortType getType()
    {
      return type;
    }

    IOPortDirection getDirection()
    {
      return dir;
    }

    /**
     * @brief Get the Value of the port.
     * Audio: nominal range of [-1, 1], clamped at [-2, 2] when setting output value
     * CV: range of [0, 1]
     * Gate: low is 0.f, hight is 1.f
     * Param: range of [0, 1]
     * Button: low is 0.f, hight is 1.f
     * Led: range of [0, 1]
     *
     * @return the port value
     */
    float getValue()
    {
      return value;
    }

    float getFrequencyValue()
    {
      return pitchToFrequency(getValue());
    }

    bool getGateValue()
    {
      return value > GATE_LOW_THRESH;
    }

    void setValue(float currentValue)
    {
      float value = currentValue;

      // Queue up the value change if there is a delay set.
      if (delay > 0)
      {
        delayBuffer.push(value);
        if (delayBuffer.size() < delay)
        {
          return;
        }
        value = delayBuffer.front();
        delayBuffer.pop();
      }

      switch (type)
      {
      case IOPortType::Audio:
        this->value = dir == IOPortDirection::Input ? value : daisysp::fclamp(value, -2.f, 2.f);
        break;
      case IOPortType::Param:
      case IOPortType::CV:
      case IOPortType::Led:
        if (dir == IOPortDirection::Input)
        {
          if (abs(this->value - value) > CV_CHANGE_THRESHOLD)
          {
            this->value = value;
            portListener->cvValueDidChange(this, this->value);
          }
        }
        else
        {
          this->value = daisysp::fclamp(value, 0.f, 1.f);
        }
        break;
      case IOPortType::Button:
      case IOPortType::Gate:
        if (this->value == GATE_HIGH && value < GATE_LOW_THRESH)
        {
          this->value = GATE_LOW;
          portListener->gateValueDidChange(this, false);
        }
        else if (this->value == GATE_LOW && value > GATE_HIGH_THRESH)
        {
          this->value = GATE_HIGH;
          portListener->gateValueDidChange(this, true);
        }
        break;
      }
    }

    /**
     * @brief Set the number frames before a set value takes effect. This can
     * be useful when coordinating related input ports such as CV and Gate. The
     * CV value change may lag a bit so delaying the gate allows the CV to
     * settle before it's value is taken.
     *
     * @param delay number frames before a set value takes effect.
     * @return IOPort* for chainability.
     */
    IOPort *setDelay(uint16_t delay)
    {
      this->delay = delay;
      return this;
    }

    std::string getPanelId()
    {
      return panelId;
    }
  };

  struct Engine : PortListener
  {
  private:
    IOConfig ioConfig = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    FrameInfo frameInfo;
    vector<IOPort *> ioPorts;

    void validateIOConfig()
    {
      assertCondition("max 2 audio ins", ioConfig.numAudioIns <= 2);
      assertCondition("max 2 audio outs", ioConfig.numAudioOuts <= 2);
      assertCondition("max 10 params + CV ins", (ioConfig.numParams + ioConfig.numCVIns) <= 10);
      assertCondition("max 2 CV outs", ioConfig.numCVOuts <= 2);
      assertCondition("max 18 gate ins + gate outs + buttons", (ioConfig.numGateIns + ioConfig.numCVOuts + ioConfig.numButtons + ioConfig.numLeds) <= 18);
    }

  protected:
    void logIOCapacity()
    {
      PHNQ_LOG("IO Capacity:");
      PHNQ_LOG("         audio ins: %lu of %i", ioConfig.numAudioIns, 2);
      PHNQ_LOG("        audio outs: %lu of %i", ioConfig.numAudioOuts, 2);
      PHNQ_LOG("     params/cv ins: %lu of %i", ioConfig.numParams + ioConfig.numCVIns, 10);
      PHNQ_LOG("           cv outs: %lu of %i", ioConfig.numCVOuts, 2);
      PHNQ_LOG("gates/buttons/leds: %lu of %i", ioConfig.numGateIns + ioConfig.numCVOuts + ioConfig.numButtons + ioConfig.numLeds, 18);
    }

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
          assertCondition("IOPortType::Param can only be used for input.", false);
        }
      }
      else if (type == IOPortType::Button)
      {
        if (dir == IOPortDirection::Input)
        {
          ioConfig.numButtons++;
        }
        else
        {
          assertCondition("IOPortType::Button can only be used for input.", false);
        }
      }
      else if (type == IOPortType::Led)
      {
        if (dir == IOPortDirection::Input)
        {
          assertCondition("IOPortType::Led can only be used for output.", false);
        }
        else
        {
          ioConfig.numLeds++;
        }
      }

      validateIOConfig();

      return port;
    }

    virtual void sampleRateDidChange(float sampleRate) {}

    virtual void process(FrameInfo frameInfo) {}

    FrameInfo getFrameInfo()
    {
      return frameInfo;
    }

    bool isDaisySeed()
    {
#ifdef PHNQ_SEED
      return true;
#else
      return false;
#endif
    }

    bool isVCVRack()
    {
#ifdef PHNQ_RACK
      return true;
#else
      return false;
#endif
    }

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
