// #pragma once

// /**
//  * @file SeedModule.hpp
//  * @author your name (you@domain.com)
//  * @brief
//  * @version 0.1
//  * @date 2022-09-14
//  *
//  * @copyright Copyright (c) 2022
//  *
//  * TODO Tests:
//  * [x] Button
//  * [x] Gate In (CV)
//  * [ ] Potentiometer
//  * [ ] Gate Out
//  * [ ] CV In
//  * [ ] CV Out
//  * [ ] Audio In
//  * [ ] Audio Out
//  */

#include "daisysp.h"
#include "daisy_seed.h"
#include "../engine/Engine.hpp"

extern phnq::engine::Engine *engineInstance;

using namespace daisy;
using namespace daisy::seed;

const DacHandle::Channel DAC_CHANNELS[] = {DacHandle::Channel::ONE, DacHandle::Channel::TWO};

struct AdcChannel
{
  uint8_t index;
  Pin pin;
};
const AdcChannel ADC_CHANNELS[] = {{0, A0}, {1, A1}, {2, A2}, {3, A3}, {4, A4}, {5, A5}, {6, A6}, {9, A9}, {10, A10}, {11, A11}};

struct GPIOChannel
{
  uint8_t index;
  Pin pin;
};
const GPIOChannel GPIO_CHANNELS[] = {{1, D1}, {2, D2}, {3, D3}, {4, D4}, {5, D5}, {6, D6}, {7, D7}, {8, D8}, {9, D9}, {10, D10}, {11, D11}, {12, D12}, {13, D13}, {14, D14}, {29, D29}, {30, D30}};

template <class T>
struct AudioMapping
{
  size_t index;
  T *port;
};

struct DACMapping
{
  DacHandle::Channel channel;
  phnq::engine::ControlOut *port;
};

template <class T>
struct ADCMapping
{
  AdcChannel channel;
  T *port;
};

template <class T>
struct GPIOMapping
{
  GPIO *gpio = new GPIO();
  GPIOChannel channel;
  T *port;
};

struct LedMapping
{
  Led *led = new Led();
  GPIOChannel channel;
  phnq::engine::Port<float> *port;
};

DaisySeed hw;
DacHandle::Config cfg;
AdcChannelConfig *adcConfig;
phnq::engine::Engine *engine = engineInstance;
phnq::engine::FrameInfo frameInfo;
std::vector<AudioMapping<phnq::engine::AudioIn>> audioInMappings;
std::vector<AudioMapping<phnq::engine::AudioOut>> audioOutMappings;
std::vector<DACMapping> dacMappings;
std::vector<ADCMapping<phnq::engine::ControlIn>> controlInMappings;
std::vector<ADCMapping<phnq::engine::Param>> paramMappings;
std::vector<GPIOMapping<phnq::engine::Button>> buttonMappings;
std::vector<GPIOMapping<phnq::engine::GateIn>> gpioInMappings;
std::vector<GPIOMapping<phnq::engine::GateOut>> gpioOutMappings;
std::vector<LedMapping> ledMappings;

void initializeHardware()
{
  hw.Configure();
  hw.Init();
  hw.SetAudioBlockSize(4);
  hw.StartLog(true);

  frameInfo.sampleRate = hw.AudioSampleRate();
  frameInfo.sampleTime = 1.f / frameInfo.sampleRate;
}

void setupPinMappings()
{
  PHNQ_LOG("Pin:Port mappings:");
  for (auto *audioIn : engine->getAudioIns())
  {
    AudioMapping<phnq::engine::AudioIn> mapping = {audioInMappings.size(), audioIn};
    audioInMappings.push_back(mapping);
    PHNQ_LOG("  [Audio In %d] \"%s\"", mapping.index + 1, mapping.port->getId().c_str());
  }

  for (auto *audioOut : engine->getAudioOuts())
  {
    AudioMapping<phnq::engine::AudioOut> mapping = {audioOutMappings.size(), audioOut};
    audioOutMappings.push_back(mapping);
    PHNQ_LOG("  [Audio Out %d] \"%s\"", mapping.index + 1, mapping.port->getId().c_str());
  }

  for (auto *controlIn : engine->getControlIns())
  {
    ADCMapping<phnq::engine::ControlIn> mapping = {ADC_CHANNELS[controlInMappings.size() + paramMappings.size()], controlIn};
    controlInMappings.push_back(mapping);
    PHNQ_LOG("  [ADC %d] \"%s\"", mapping.channel.index, mapping.port->getId().c_str());
  }

  for (auto *param : engine->getParams())
  {
    if (param->getType() != phnq::engine::Param::BUTTON)
    {
      ADCMapping<phnq::engine::Param> mapping = {ADC_CHANNELS[controlInMappings.size() + paramMappings.size()], param};
      paramMappings.push_back(mapping);
      PHNQ_LOG("  [ADC %d] \"%s\"", mapping.channel.index, mapping.port->getId().c_str());
    }
  }

  for (auto *controlOut : engine->getControlOuts())
  {
    DACMapping mapping = {DAC_CHANNELS[dacMappings.size()], controlOut};
    dacMappings.push_back(mapping);
    PHNQ_LOG("  [DAC OUT %d] \"%s\"", mapping.channel == DacHandle::Channel::ONE ? 1 : 2, mapping.port->getId().c_str());
  }

  uint16_t gpioIndex = 0;

  for (auto *param : engine->getParams())
  {
    if (param->getType() == phnq::engine::Param::BUTTON)
    {
      GPIOMapping<phnq::engine::Button> mapping;
      mapping.channel = GPIO_CHANNELS[gpioIndex++];
      mapping.port = (phnq::engine::Button *)param;
      buttonMappings.push_back(mapping);
      PHNQ_LOG("  [D%d] \"%s\"", mapping.channel.index, mapping.port->getId().c_str());
    }
  }

  for (auto *gateIn : engine->getGateIns())
  {
    GPIOMapping<phnq::engine::GateIn> mapping;
    mapping.channel = GPIO_CHANNELS[gpioIndex++];
    mapping.port = gateIn;
    gpioInMappings.push_back(mapping);
    PHNQ_LOG("  [D%d] \"%s\"", mapping.channel.index, mapping.port->getId().c_str());
  }

  for (auto *gateOut : engine->getGateOuts())
  {
    GPIOMapping<phnq::engine::GateOut> mapping;
    mapping.channel = GPIO_CHANNELS[gpioIndex++];
    mapping.port = gateOut;
    gpioOutMappings.push_back(mapping);
    PHNQ_LOG("  [D%d] \"%s\"", mapping.channel.index, mapping.port->getId().c_str());
  }

  for (auto *light : engine->getLights())
  {
    LedMapping mapping;
    mapping.channel = GPIO_CHANNELS[gpioIndex++];
    mapping.port = light;
    ledMappings.push_back(mapping);
    PHNQ_LOG("  [D%d] \"%s\"", mapping.channel.index, mapping.port->getId().c_str());
  }
}

void configureIO()
{
  // Configure DAC -- CV outs
  PHNQ_LOG("Configure DAC (CV outs)");
  cfg.bitdepth = DacHandle::BitDepth::BITS_12;
  cfg.buff_state = DacHandle::BufferState::ENABLED;
  cfg.mode = DacHandle::Mode::POLLING;
  cfg.chn = DacHandle::Channel::BOTH;
  hw.dac.Init(cfg);

  // Configure ADC -- CV ins, Params
  PHNQ_LOG("Configure ADC (CV ins, params)");
  size_t numADCs = controlInMappings.size() + paramMappings.size();
  adcConfig = static_cast<AdcChannelConfig *>(calloc(numADCs, sizeof(AdcChannelConfig)));
  for (ADCMapping<phnq::engine::ControlIn> mapping : controlInMappings)
  {
    adcConfig[mapping.channel.index].InitSingle(mapping.channel.pin);
  }
  for (ADCMapping<phnq::engine::Param> mapping : paramMappings)
  {
    adcConfig[mapping.channel.index].InitSingle(mapping.channel.pin);
  }
  hw.adc.Init(adcConfig, numADCs);

  // Configure GPIO -- Gate ins and outs, lights
  PHNQ_LOG("Configure GPIO (gate ins/outs, lights, butttons)");
  for (GPIOMapping<phnq::engine::Button> mapping : buttonMappings)
  {
    mapping.gpio->Init(mapping.channel.pin, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  }
  for (GPIOMapping<phnq::engine::GateIn> mapping : gpioInMappings)
  {
    mapping.gpio->Init(mapping.channel.pin,
                       GPIO::Mode::INPUT,
                       mapping.port->getType() == phnq::engine::GateIn::Type::CV ? GPIO::Pull::PULLDOWN : GPIO::Pull::PULLUP);
  }
  for (GPIOMapping<phnq::engine::GateOut> mapping : gpioOutMappings)
  {
    mapping.gpio->Init(mapping.channel.pin, GPIO::Mode::OUTPUT);
  }
  for (LedMapping mapping : ledMappings)
  {
    mapping.led->Init(mapping.channel.pin, false);
  }
}

uint32_t numAudios = 0;

/**
 * @brief This callback does the following:
 * 1. Fills the engine's audio input port values with audio input data from the Seed.
 * 2. Calls `doProcess()` where audio processing is done.
 * 3. Copies the resulting audio output data into the Seed's output buffer.
 *
 * @param in
 * @param out
 * @param size
 */
static void AudioCallback(AudioHandle::InterleavingInputBuffer in, AudioHandle::InterleavingOutputBuffer out, size_t size)
{
  numAudios += 1;
  // PHNQ_LOG("AudioCallback: %d", engineInstance->doStuff());

  // Iterate through audio buffer sample frames...
  for (size_t i = 0; i < size; i += 2)
  {
    for (AudioMapping<phnq::engine::AudioIn> audioInMapping : audioInMappings)
    {
      audioInMapping.port->setValue(in[i + audioInMapping.index]);
    }

    engineInstance->doProcess(frameInfo);

    for (AudioMapping<phnq::engine::AudioOut> audioOutMapping : audioOutMappings)
    {
      out[i + audioOutMapping.index] = audioOutMapping.port->getValue();
    }
  }
}

void start()
{
  PHNQ_LOG("Start ADC");
  hw.adc.Start();

  PHNQ_LOG("Start audio");
  hw.StartAudio(AudioCallback);

  PHNQ_LOG("Start IO loop");
  while (true)
  {
    // ADC -- Control Ins
    for (ADCMapping<phnq::engine::ControlIn> mapping : controlInMappings)
    {
      mapping.port->setValue(hw.adc.GetFloat(mapping.channel.index) * 2.f - 1.f);
    }

    // ADC -- Params
    for (ADCMapping<phnq::engine::Param> mapping : paramMappings)
    {
      mapping.port->setValue(hw.adc.GetFloat(mapping.channel.index));
    }

    // GPIO -- button ins
    for (GPIOMapping<phnq::engine::Button> buttonMapping : buttonMappings)
    {
      buttonMapping.port->setBoolValue(!buttonMapping.gpio->Read());
    }

    // GPIO -- gate ins
    for (GPIOMapping<phnq::engine::GateIn> gpioInMapping : gpioInMappings)
    {
      gpioInMapping.port->setValue(!gpioInMapping.gpio->Read());
    }

    // // DAC -- control outs
    for (DACMapping dacMapping : dacMappings)
    {
      float cvOutVal = (dacMapping.port->getValue() + 1.f) / 2.f;
      hw.dac.WriteValue(dacMapping.channel, (uint16_t)roundf(cvOutVal * 4095.f));
    }

    // // GPIO -- gate outs
    for (GPIOMapping<phnq::engine::GateOut> gpioOutMapping : gpioOutMappings)
    {
      gpioOutMapping.gpio->Write(gpioOutMapping.port->getValue());
    }

    // // LEDs
    for (LedMapping ledMapping : ledMappings)
    {
      ledMapping.led->Set(ledMapping.port->getValue());
      ledMapping.led->Update();
    }

    // System::Delay(1);
  }
}

int main(void)
{
  initializeHardware();
  setupPinMappings();
  configureIO();
  start();
  return 0;
}
