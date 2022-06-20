#include <cmath>
#include "../../core/Engine.hpp"
#include <daisysp.h>

using namespace phnq;
using namespace daisysp;

static const float FREQ_C4 = 261.6256f;

struct TestModule : phnq::Engine
{
  IOPort *gateIn;
  IOPort *pitchParam;
  IOPort *pitchCV;
  IOPort *audioOut;

  Oscillator osc1;
  Oscillator osc2;
  Oscillator osc3;

  TestModule()
  {
    this->gateIn = addIOPort(IOPortType::Gate, IOPortDirection::Input);
    this->pitchParam = addIOPort(IOPortType::Param, IOPortDirection::Input);
    this->pitchCV = addIOPort(IOPortType::CV, IOPortDirection::Input);
    this->audioOut = addIOPort(IOPortType::Audio, IOPortDirection::Output);
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

  void gateChanged(IOPort *gatePort) override
  {
    if (gatePort == this->gateIn)
    {
      PHNQ_LOG("============================== GATE IN CHANGE: %f", gatePort->getValue());
    }
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

// Maybe this can be replaced by a macro?
#ifdef PHNQ_RACK
#include <rack.hpp>
#include "../../core/rack/RackModule.hpp"
#include "../../core/rack/RackModuleUI.hpp"
rack::plugin::Model *modelTestModule = rack::createModel<phnq::RackModule<TestModule>, phnq::RackModuleUI<TestModule>>("TestModule");
#endif

#ifdef PHNQ_SEED
#include "../../core/seed/SeedModule.hpp"
phnq::Engine *moduleInstance = new TestModule();
#endif
