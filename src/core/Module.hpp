#pragma once

#include <string>
#include <vector>
#include <math.h>

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
  float clamp(float val, float min, float max)
  {
    return val < min ? min : val > max ? max
                                       : val;
  }

  struct FrameInfo
  {
    float sampleRate;
    float sampleTime;
  };

  /**
   * Seed Limitations
   * ================
   * CV ins:
   * - Seed ADC pins are used. 10 of the possible 12 are allocated to input; the
   *   two bi-purpose (ADC/DAC) pins are allocated for CV out use (DAC).
   * - It is possible to increase the number of ADCs (up to 80?) in mux mode, but
   *   this requires an extra chip.
   * CV outs:
   * - The two DAC pins are used.
   * - More CV outs are possible, in theory, if some of the Seed's pins are used
   *   to output PWM. A separate DAC chip would be needed for this.
   * Gate ins/outs:
   * - The 18 GPIO pins are used for gates. They can be set to read or write.
   * Audio ins/outs
   * - The Sees has 2 ins and 2 outs of high quality audio.
   *
   * See:
   *  https://static1.squarespace.com/static/58d03fdc1b10e3bf442567b8/t/61b102adf8965f3e1cd096fd/1638990509655/Daisy_Seed_pinout.pdf
   *
   */
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
  const float GATE_HIGH = 10.f;
  const float GATE_LOW_THRESH = 0.1f;
  const float GATE_HIGH_THRESH = 2.f;

  enum IOPortType
  {
    Audio, // nominal range of [-1, 1], clamped at [-2, 2] when setting output value
    CV,    // nominal range of [0, 10]
    Gate,  // low is 0.f, hight is 10.f
    Param, // same as CV
  };

  enum IOPortDirection
  {
    Input,
    Output
  };

  struct Module;

  struct IOPort
  {
  private:
    Module *module;
    IOPortType type;
    IOPortDirection dir;
    float value;

  public:
    IOPort(Module *module, IOPortType type, IOPortDirection dir)
    {
      this->module = module;
      this->type = type;
      this->dir = dir;
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
        this->value = dir == IOPortDirection::Input ? value : clamp(value, 0.f, 10.f);
        break;
      case IOPortType::Gate:
        if (this->value == GATE_HIGH && value < GATE_LOW_THRESH)
        {
          setValue(GATE_LOW);
        }
        else if (this->value == GATE_LOW && value > GATE_HIGH_THRESH)
        {
          setValue(GATE_HIGH);
        }
        break;
      }
    }
  };

  struct Module
  {
  private:
    IOConfig ioConfig = {0, 0, 0, 0, 0, 0, 0};
    FrameInfo frameInfo;
    vector<IOPort *> ioPorts;

  protected:
    IOPort *addIOPort(IOPortType type, IOPortDirection dir)
    {
      IOPort *port = new IOPort(this, type, dir);
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
      return port;
    }

    virtual void onSampleRateChange(float sampleRate)
    {
    }

    virtual void process(FrameInfo frameInfo)
    {
    }

  public:
    ~Module()
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
        onSampleRateChange(frameInfo.sampleRate);
      }
      this->process(this->frameInfo);
    }
  };
}
