#include "../Engine.hpp"
#include "daisysp.h"
#include "daisy_seed.h"

using namespace daisy;
using namespace daisysp;

extern phnq::Engine *moduleInstance;

DaisySeed hw;

DaisySeed *seedHw = &hw;

phnq::FrameInfo frameInfo;

const DacHandle::Channel DAC_CHANNELS[] = {DacHandle::Channel::ONE, DacHandle::Channel::TWO};
const uint8_t ADC_PINS[] = {15, 16, 17, 18, 19, 20, 21, 24, 25, 28};
const Pin GPIO_PINS[] = {seed::D1, seed::D2, seed::D3, seed::D4, seed::D5, seed::D6, seed::D7, seed::D8,
                         seed::D9, seed::D10, seed::D11, seed::D12, seed::D13, seed::D14, seed::D29, seed::D30};

/**
 * Seed Hardware Data Ranges
 * =========================
 * 1. Audio
 *      - in/out [-1, 1]
 *      - yields 0dBFs @ 1Vrms
 * 2. ADC (CV in)
 *      - hw.adc.GetFloat(x) [0, 1]
 *      - 0 to 3V3
 * 3. DAC (CV out)
 *      - hw.dac.WriteValue(chan, val) [0, 4095] and [0, 255] for 12-bit, 8-bit respectively.
 *      - 0 to 3V3
 * 4. GPIO (gate in/out)
 *      - gpio.Read() returns a boolean.
 *      - gpio.Write(state) takes a boolean arg.
 *      - 0 to 3V3
 *
 * Daisy Seed Data Sheet:
 * https://static1.squarespace.com/static/58d03fdc1b10e3bf442567b8/t/6227e6236f02fb68d1577146/1646781988478/Daisy_Seed_datasheet_v1.0.3.pdf
 */

struct AudioMapping
{
  uint8_t index;
  phnq::IOPort *ioPort;
};
vector<AudioMapping> audioInMappings;
vector<AudioMapping> audioOutMappings;

struct DACMapping
{
  DacHandle::Channel channel;
  phnq::IOPort *ioPort;
};
vector<DACMapping> dacMappings;

struct ADCMapping
{
  uint8_t index;
  phnq::IOPort *ioPort;
};
vector<ADCMapping> adcMappings;

struct GPIOMapping
{
  GPIO *gpio;
  phnq::IOPort *ioPort;
};
vector<GPIOMapping> gpioInMappings;
vector<GPIOMapping> gpioOutMappings;

static void AudioCallback(AudioHandle::InterleavingInputBuffer in, AudioHandle::InterleavingOutputBuffer out, size_t size)
{
  for (ADCMapping adcMapping : adcMappings)
  {
    // [0, 1] -> [0, 1] -- nothing to be done.
    adcMapping.ioPort->setValue(hw.adc.GetFloat(adcMapping.index));
  }

  for (GPIOMapping gpioInMapping : gpioInMappings)
  {
    // [false, true] -> [0, 1].
    gpioInMapping.ioPort->setValue(gpioInMapping.gpio->Read() ? 1.f : 0.f);
  }

  // Iterate through audio buffer sample frames...
  for (size_t i = 0; i < size; i += 2)
  {
    for (AudioMapping audioInMapping : audioInMappings)
    {
      // [-1, 1] -> [-1, 1] -- nothing to be done.
      audioInMapping.ioPort->setValue(in[i + audioInMapping.index]);
    }

    // Call module's process method. This is called once per sample.
    moduleInstance->doProcess(frameInfo);

    for (AudioMapping audioOutMapping : audioOutMappings)
    {
      // [-1, 1] -> [-1, 1] -- nothing to be done.
      out[i + audioOutMapping.index] = audioOutMapping.ioPort->getValue();
    }

    for (DACMapping dacMapping : dacMappings)
    {
      // [0, 1] -> [0, 1] -- nothing to be done.
      float cvOutVal = dacMapping.ioPort->getValue();
      hw.dac.WriteValue(dacMapping.channel, (uint16_t)roundf(cvOutVal * 4095.f));
    }

    for (GPIOMapping gpioOutMapping : gpioOutMappings)
    {
      // [0, 1] -> [false, true].
      gpioOutMapping.gpio->Write(roundf(gpioOutMapping.ioPort->getValue()) == 1.f);
    }
  }
}

int main(void)
{
  phnq::IOConfig ioConfig = moduleInstance->getIOConfig();

  hw.Configure();
  hw.Init();
  hw.SetAudioBlockSize(4);

  // hw.StartLog(true);
  // hw.PrintLine("Hello");

  // Configure DAC -- CV outs
  DacHandle::Config cfg;
  cfg.bitdepth = DacHandle::BitDepth::BITS_12;
  cfg.buff_state = DacHandle::BufferState::ENABLED;
  cfg.mode = DacHandle::Mode::POLLING;
  cfg.chn = DacHandle::Channel::BOTH;
  hw.dac.Init(cfg);

  // For ADC config
  AdcChannelConfig adcConfig[ioConfig.numCVIns + ioConfig.numParams];

  // Add mappings for IOPorts and corresponsing hardware interfaces...
  uint8_t cvInIndex = 0, gpioPinIndex = 0, dacIndex = 0, audioInIndex = 0, audioOutIndex = 0;
  for (phnq::IOPort *ioPort : moduleInstance->getIOPorts())
  {
    if (ioPort->getDirection() == phnq::IOPortDirection::Input)
    {
      switch (ioPort->getType())
      {
      case phnq::IOPortType::CV:
      case phnq::IOPortType::Param:
      {
        adcConfig[cvInIndex].InitSingle(hw.GetPin(ADC_PINS[cvInIndex]));
        adcMappings.push_back({cvInIndex, ioPort});
        cvInIndex++;
        break;
      }
      case phnq::IOPortType::Gate:
      {
        GPIO *gpio = new GPIO();
        gpio->Init(GPIO_PINS[gpioPinIndex++], GPIO::Mode::INPUT);
        gpioInMappings.push_back({gpio, ioPort});
        break;
      }
      case phnq::IOPortType::Audio:
      {
        audioInMappings.push_back({audioInIndex++, ioPort});
        break;
      }
      }
    }
    else if (ioPort->getDirection() == phnq::IOPortDirection::Output)
    {
      switch (ioPort->getType())
      {
      case phnq::IOPortType::CV:
      case phnq::IOPortType::Param:
      {
        dacMappings.push_back({DAC_CHANNELS[dacIndex++], ioPort});
        break;
      }
      case phnq::IOPortType::Gate:
      {
        GPIO *gpio = new GPIO();
        gpio->Init(GPIO_PINS[gpioPinIndex++], GPIO::Mode::OUTPUT);
        gpioOutMappings.push_back({gpio, ioPort});
        break;
      }
      case phnq::IOPortType::Audio:
      {
        audioOutMappings.push_back({audioOutIndex++, ioPort});
        break;
      }
      }
    }
  }

  // Configure ADC -- CV ins
  hw.adc.Init(adcConfig, ioConfig.numCVIns);
  hw.adc.Start();

  frameInfo.sampleRate = hw.AudioSampleRate();
  frameInfo.sampleTime = 1.f / frameInfo.sampleRate;

  hw.StartAudio(AudioCallback);

  // Loop forever
  for (;;)
  {
  }
}
