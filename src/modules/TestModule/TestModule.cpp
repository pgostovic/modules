#include <cmath>
#include "../../core/Engine.hpp"
#include <daisysp.h>
#include "../../core/dsp/Trigger.hpp"

using namespace phnq;
using namespace daisysp;

static const float FREQ_C4 = 261.6256f;

struct TestModule : phnq::Engine
{
  IOPort *gateIn;
  IOPort *pitchParam;
  IOPort *pitchCV;
  IOPort *audioOut;
  IOPort *gateOut;

  Oscillator osc1;
  Oscillator osc2;
  Oscillator osc3;
  Trigger trigger;

  TestModule()
  {
    this->gateIn = addIOPort(IOPortType::Gate, IOPortDirection::Input, "gateIn");
    this->pitchParam = addIOPort(IOPortType::Param, IOPortDirection::Input, "pitchParam");
    this->pitchCV = addIOPort(IOPortType::CV, IOPortDirection::Input, "pitchCV");
    this->audioOut = addIOPort(IOPortType::Audio, IOPortDirection::Output, "audioOut");
    this->gateOut = addIOPort(IOPortType::Gate, IOPortDirection::Output, "gateOut");
  }

  void onSampleRateChange(float sampleRate) override
  {
    osc1.Init(sampleRate);
    osc1.SetWaveform(Oscillator::WAVE_SAW);
    osc2.Init(sampleRate);
    osc2.SetWaveform(Oscillator::WAVE_SAW);
    osc3.Init(sampleRate);
    osc3.SetWaveform(Oscillator::WAVE_SAW);

    trigger.init(sampleRate);
  }

  void gateChanged(IOPort *gatePort) override
  {
    if (gatePort == gateIn && gateIn->getValue() == 1.f)
    {
      PHNQ_LOG("============================== GATE IN CHANGE: %f", gatePort->getValue());
      trigger.activate();
    }
  }

  void process(FrameInfo frameInfo) override
  {
    float pitch = pitchParam->getValue() + pitchCV->getValue();

    osc1.SetFreq(FREQ_C4 * std::pow(2.f, pitch));
    osc2.SetFreq(FREQ_C4 * std::pow(2.f, pitch + (2.f / 12.f)));
    osc3.SetFreq(FREQ_C4 * std::pow(2.f, pitch + (7.f / 12.f)));

    gateOut->setValue(trigger.process() ? 1.f : 0.f);

    audioOut->setValue(osc1.Process() + osc2.Process() + osc3.Process());
  }
};

#ifdef PHNQ_RACK
#include "../../core/rack/RackModuleUI.hpp"
rack::plugin::Model *modelTestModule = rack::createModel<phnq::RackModule<TestModule>, phnq::RackModuleUI<TestModule>>("TestModule");
#endif

#ifdef PHNQ_SEED
#include "../../core/seed/SeedModule.hpp"
phnq::Engine *moduleInstance = new TestModule();
#endif
