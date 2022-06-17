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
    uint8_t numAudioIns;  // max 2
    uint8_t numAudioOuts; // max 2
    uint8_t numCVIns;     // max 10
    uint8_t numCVOuts;    // max 2
    uint8_t numGateIns;   // max 18 for ins + outs
    uint8_t numGateOuts;  // max 18 for ins + outs
  };

  /**
   * Nominal amplitude is 1, clamped at 2.
   */
  struct AudioPort
  {
  private:
    bool isInput;
    float value;

  public:
    AudioPort(bool isInput)
    {
      this->isInput = isInput;
    }

    float getValue()
    {
      return this->value;
    }

    void setValue(float value)
    {
      this->value = isInput ? value : clamp(value, -2.f, 2.f);
    }
  };

  struct CVPort
  {
  private:
    bool isInput;
    float value;
    bool _isParam = false;

  public:
    CVPort(bool isInput)
    {
      this->isInput = isInput;
    }

    float getValue()
    {
      return this->value;
    }

    void setValue(float value)
    {
      this->value = isInput ? value : clamp(value, -10.f, 10.f);
    }

    CVPort *asParam()
    {
      this->_isParam = true;
      return this;
    }

    bool isParam()
    {
      return this->_isParam;
    }
  };

  struct GatePort
  {
  private:
    bool isInput;
    bool value;
    float voltage;

  public:
    GatePort(bool isInput)
    {
      this->isInput = isInput;
      this->voltage = 0;
    }

    bool getValue()
    {
      return this->value;
    }

    void setValue(bool value)
    {
      this->value = value;
    }

    /**
     * This method prevents rapid toggling of state by specifying divergent
     * low and high thresholds for value flipping. A true gate value will only
     * flip to false if the incoming voltage dips below `lowThresh`. To flip
     * back to true, the voltage has to exceed `highThresh`. This prevents an
     * erratic signal from rapidly toggling the state.
     *
     * @param voltage incoming CV signal.
     * @param lowThresh threshold below which the state is toggled to false.
     * @param highThresh threshold above which the state is toggled to true.
     */
    void setValue(float voltage, float lowThresh = 0.1f, float highThresh = 2.f)
    {
      if (this->value && voltage < lowThresh)
      {
        setValue(false);
      }
      else if (!this->value && voltage > highThresh)
      {
        setValue(true);
      }
    }
  };

  struct Module
  {
  private:
    IOConfig config;
    FrameInfo frameInfo;
    vector<AudioPort *> audioIns;
    vector<AudioPort *> audioOuts;
    vector<CVPort *> cvIns;
    vector<CVPort *> cvOuts;
    vector<GatePort *> gateIns;
    vector<GatePort *> gateOuts;

  protected:
    Module(IOConfig config)
    {
      this->config = config; // Need to validate this config.

      for (uint8_t i = 0; i < config.numAudioIns; i++)
      {
        audioIns.push_back(new AudioPort(true));
      }
      for (uint8_t i = 0; i < config.numAudioOuts; i++)
      {
        audioOuts.push_back(new AudioPort(false));
      }
      for (uint8_t i = 0; i < config.numCVIns; i++)
      {
        cvIns.push_back(new CVPort(true));
      }
      for (uint8_t i = 0; i < config.numCVOuts; i++)
      {
        cvOuts.push_back(new CVPort(false));
      }
      for (uint8_t i = 0; i < config.numGateIns; i++)
      {
        gateIns.push_back(new GatePort(true));
      }
      for (uint8_t i = 0; i < config.numGateOuts; i++)
      {
        gateOuts.push_back(new GatePort(false));
      }
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
      for (vector<AudioPort *>::iterator it = audioIns.begin(); it != audioIns.end();)
      {
        delete *it;
        it = audioIns.erase(it);
      }
      for (vector<AudioPort *>::iterator it = audioOuts.begin(); it != audioOuts.end();)
      {
        delete *it;
        it = audioOuts.erase(it);
      }
      for (vector<CVPort *>::iterator it = cvIns.begin(); it != cvIns.end();)
      {
        delete *it;
        it = cvIns.erase(it);
      }
      for (vector<CVPort *>::iterator it = cvOuts.begin(); it != cvOuts.end();)
      {
        delete *it;
        it = cvOuts.erase(it);
      }
      for (vector<GatePort *>::iterator it = gateIns.begin(); it != gateIns.end();)
      {
        delete *it;
        it = gateIns.erase(it);
      }
      for (vector<GatePort *>::iterator it = gateOuts.begin(); it != gateOuts.end();)
      {
        delete *it;
        it = gateOuts.erase(it);
      }
    }

    AudioPort *getAudioIn(uint8_t index)
    {
      return index < audioIns.size() ? audioIns[index] : NULL;
    }

    AudioPort *getAudioOut(uint8_t index)
    {
      return index < audioOuts.size() ? audioOuts[index] : NULL;
    }

    CVPort *getCVIn(uint8_t index)
    {
      return index < cvIns.size() ? cvIns[index] : NULL;
    }

    CVPort *getCVOut(uint8_t index)
    {
      return index < cvOuts.size() ? cvOuts[index] : NULL;
    }

    GatePort *getGateIn(uint8_t index)
    {
      return index < gateIns.size() ? gateIns[index] : NULL;
    }

    GatePort *getGateOut(uint8_t index)
    {
      return index < gateOuts.size() ? gateOuts[index] : NULL;
    }

    IOConfig getIOConfig()
    {
      return config;
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
