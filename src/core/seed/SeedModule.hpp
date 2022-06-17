#include "../Module.hpp"
#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisy;
using namespace daisysp;

extern phnq::Module *moduleInstance;

DaisySeed hw;

phnq::FrameInfo frameInfo;

/**
 * Seed Hardware Data Ranges
 * =========================
 * 1. Audio
 *      - in/out [-1, 1]
 * 2. ADC (CV in)
 *      - hw.adc.GetFloat(x) [0, 1]
 * 3. DAC (CV out)
 *      - hw.dac.WriteValue(chan, val) [0, 4095] and [0, 255] for 12-bit, 8-bit respectively.
 * 4. GPIO (gate in/out)
 *      - gpio.Read() returns a boolean.
 *      - gpio.Write(state) takes a boolean arg.
 */

vector<GPIO *> gpioGateIns;
vector<GPIO *> gpioGateOuts;

static void AudioCallback(AudioHandle::InterleavingInputBuffer in, AudioHandle::InterleavingOutputBuffer out, size_t size)
{
  phnq::IOConfig ioConfig = moduleInstance->getIOConfig();

  /**
   * Read values from the ADC pins and set the corresponding input port values.
   * Note: this is done outside of the sample buffer loop.
   */
  for (uint8_t i = 0; i < ioConfig.numCVIns; i++)
  {
    moduleInstance->getCVIn(i)->setValue(hw.adc.GetFloat(i));
  }

  /**
   * Read states from the GPIO pins (that are configured for input) and set the
   * corresponding port values.
   */
  for (uint8_t i = 0; i < ioConfig.numGateIns; i++)
  {
    moduleInstance->getGateIn(i)->setValue(gpioGateIns[i]->Read());
  }

  for (size_t i = 0; i < size; i += 2)
  {
    // Read the audio input values from the hardware.
    for (uint8_t j = 0; j < ioConfig.numAudioIns; j++)
    {
      moduleInstance->getAudioIn(j)->setValue(in[i + j]);
    }

    // Call module's process method. This is called once per sample.
    moduleInstance->doProcess(frameInfo);

    // Set the hardware's audio output values.
    for (uint8_t j = 0; j < ioConfig.numAudioOuts; j++)
    {
      out[i + j] = moduleInstance->getAudioOut(j)->getValue();
    }

    // Set the hardware's DAC output values.
    for (uint8_t j = 0; j < ioConfig.numCVOuts; j++)
    {
      float cvOutVal = moduleInstance->getCVOut(j)->getValue();
      DacHandle::Channel channel = j == 0 ? DacHandle::Channel::ONE : DacHandle::Channel::TWO;
      hw.dac.WriteValue(channel, (uint16_t)roundf(cvOutVal * 4095.f));
    }

    // Set the hardware's GPIO states.
    for (uint8_t j = 0; j < ioConfig.numGateOuts; j++)
    {
      gpioGateOuts[j]->Write(moduleInstance->getGateOut(j)->getValue());
    }
  }
}

const uint8_t ADC_PINS[] = {15, 16, 17, 18, 19, 20, 21, 24, 25, 28};
const Pin GPIO_PINS[] = {seed::D1, seed::D2, seed::D3, seed::D4, seed::D5, seed::D6, seed::D7, seed::D8,
                         seed::D9, seed::D10, seed::D11, seed::D12, seed::D13, seed::D14, seed::D29, seed::D30};

int main(void)
{
  phnq::IOConfig ioConfig = moduleInstance->getIOConfig();

  hw.Configure();
  hw.Init();
  hw.SetAudioBlockSize(4);

  hw.StartLog(true);
  hw.PrintLine("Hello");

  // Configure DAC -- CV outs
  DacHandle::Config cfg;
  cfg.bitdepth = DacHandle::BitDepth::BITS_12;
  cfg.buff_state = DacHandle::BufferState::ENABLED;
  cfg.mode = DacHandle::Mode::POLLING;
  cfg.chn = DacHandle::Channel::BOTH;
  hw.dac.Init(cfg);

  // Configure ADC -- CV ins
  AdcChannelConfig adcConfig[ioConfig.numCVIns];
  for (uint8_t i = 0; i < ioConfig.numCVIns; i++)
  {
    adcConfig[i].InitSingle(hw.GetPin(ADC_PINS[i]));
  }
  hw.adc.Init(adcConfig, ioConfig.numCVIns);
  hw.adc.Start();

  // Configure GPIOs -- Gates
  uint8_t nextPinIndex = 0;
  for (uint8_t i = 0; i < ioConfig.numGateIns; i++)
  {
    GPIO *gpio = new GPIO();
    gpio->Init(GPIO_PINS[nextPinIndex++], GPIO::Mode::INPUT);
    gpioGateIns.push_back(gpio);
  }
  for (uint8_t i = 0; i < ioConfig.numGateOuts; i++)
  {
    GPIO *gpio = new GPIO();
    gpio->Init(GPIO_PINS[nextPinIndex++], GPIO::Mode::OUTPUT);
    gpioGateOuts.push_back(gpio);
  }

  frameInfo.sampleRate = hw.AudioSampleRate();
  frameInfo.sampleTime = 1.f / frameInfo.sampleRate;

  hw.StartAudio(AudioCallback);

  // Loop forever
  for (;;)
  {
  }
}
