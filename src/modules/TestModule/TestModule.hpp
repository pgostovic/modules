#pragma once

#include <cmath>
#include "../../core/Module.hpp"
#include <daisysp.h>

using namespace phnq;
using namespace daisysp;

static const float FREQ_C4 = 261.6256f;

IOConfig getTestModuleIOConfig()
{
  IOConfig ioConfig = {0, 0, 0, 0, 0, 0};
  ioConfig.numAudioOuts = 1;
  ioConfig.numCVIns = 2;
  return ioConfig;
}

struct TestModule : phnq::Module
{
  CVPort *pitchParam;
  CVPort *pitchCV;
  AudioPort *audioOut;

  Oscillator osc1;
  Oscillator osc2;
  Oscillator osc3;

  TestModule() : phnq::Module(getTestModuleIOConfig())
  {
    pitchParam = getCVIn(0)->asParam();
    pitchCV = getCVIn(1);
    audioOut = getAudioOut(0);
  }

  void onSampleRateChange(float sampleRate) override
  {
    osc1.Init(sampleRate);
    osc1.SetWaveform(Oscillator::WAVE_SAW);
    osc2.Init(sampleRate);
    osc2.SetWaveform(Oscillator::WAVE_SAW);
    osc3.Init(sampleRate);
    osc3.SetWaveform(Oscillator::WAVE_SAW);
  }

  void process(FrameInfo frameInfo) override
  {
    float pitch = pitchParam->getValue() + pitchCV->getValue();

    osc1.SetFreq(FREQ_C4 * std::pow(2.f, pitch));
    osc2.SetFreq(FREQ_C4 * std::pow(2.f, pitch + (2.f / 12.f)));
    osc3.SetFreq(FREQ_C4 * std::pow(2.f, pitch + (7.f / 12.f)));

    audioOut->setValue(osc1.Process() + osc2.Process() + osc3.Process());
  }
};
