#include "../../core2/engine/Engine.hpp"
#include <daisysp.h>

using namespace phnq::engine;

using Osc = daisysp::VariableShapeOscillator;
using Glide = daisysp::Port;

struct PolyVox : Engine, GateIn::GateChangeListener, ControlIn::ControlChangeListener, Button::ButtonChangeListener
{
  /*****************
   ***** PORTS *****
   *****************/
  GateIn *resetSeqGateIn = createGateIn("reset")->setListener(this);
  GateIn *advanceSeqGateIn = createGateIn("trigger")->setListener(this);

  AudioOut *audioOutLeft = createAudioOut("audioOutLeft");
  AudioOut *audioOutRight = createAudioOut("audioOutRight");

  GateIn *addNoteGateIn = createGateIn("addNoteGate")->setListener(this);
  ControlIn *addNoteCVIn = createControlIn("addNoteCV")->setListener(this);

  Button *addChordButton = createButton("addChord")->setListener(this);
  Light *addChordModeLED = createLight("addChordMode");
  Button *deleteChordButton = createButton("deleteChord")->setListener(this);

  Light *seqPos1LED = createLight("seqPos1");
  Light *seqPos2LED = createLight("seqPos2");
  Light *seqPos3LED = createLight("seqPos3");
  Light *seqPos4LED = createLight("seqPos4");

  Param *tuneKnob = createParam("tune")->setListener(this);
  Param *detuneKnob = createParam("detune")->setListener(this);
  Param *shapeKnob = createParam("shape")->setListener(this);
  Param *glideKnob = createParam("glide")->setListener(this);

  ControlIn *tuneCVIn = createControlIn("tuneCV")->setListener(this);
  ControlIn *detuneCVIn = createControlIn("detuneCV")->setListener(this);
  ControlIn *shapeCVIn = createControlIn("shapeCV")->setListener(this);
  ControlIn *glideCVIn = createControlIn("glideCV")->setListener(this);

  /*****************
   ***** STATE *****
   *****************/
  uint8_t seqPos = 0;
  bool isWriteMode = false;
  std::vector<std::vector<float>> chords;
  std::vector<Osc *> oscillators;
  std::vector<Glide *> glides;

  PolyVox()
  {
    updateLEDs();
  }

  /******************
   ***** EVENTS *****
   ******************/
  void gateValueDidChange(GateIn *gateIn, bool value) override
  {
    if (gateIn == addNoteGateIn && value && isWriteMode)
    {
      addNoteToChord();
    }
    else if (gateIn == resetSeqGateIn && value)
    {
      resetSequence();
    }
    else if (gateIn == advanceSeqGateIn && value)
    {
      advanceSequence();
    }
  }

  void buttonValueDidChange(Button *button, bool value) override
  {
    if (button == addChordButton && value)
    {
      setChordWriteModeEnabled(!isWriteMode);
    }
    else if (button == deleteChordButton && value)
    {
      deleteLastChord();
    }
  }

  void controlValueDidChange(ControlIn *port, float value) override
  {
    // if (port == addChordButton && addChordButton->getStepValue() == 1)
    // {
    //   addChordModeEnabled = !addChordModeEnabled;
    // }
    // addChordModeLED->setValue(addChordModeEnabled ? 1.f : 0.f);

    // PHNQ_LOG("============= CONTROL: %s %f", port->getId().c_str(), value);
  }

  /**********************
   ***** OPERATIONS *****
   **********************/
  void setChordWriteModeEnabled(bool enabled)
  {
    if (isWriteMode != enabled)
    {
      isWriteMode = enabled;

      if (isWriteMode)
      {
        chords.push_back(std::vector<float>());
        seqPos = chords.size() - 1;
      }
      else if (chords[seqPos].empty())
      {
        chords.pop_back();
        if (seqPos > 0)
        {
          seqPos--;
        }
      }
      updateLEDs();
    }
  }

  void deleteLastChord()
  {
    if (!chords.empty())
    {
      chords.pop_back();

      if (chords.empty())
      {
        seqPos = 0;
      }
      else if (seqPos >= chords.size())
      {
        seqPos = chords.size() - 1;
      }
      adjustOscillatorPool();
      updateLEDs();
      logChords();
    }
  }

  void addNoteToChord()
  {
    chords[seqPos].push_back(addNoteCVIn->getValue());
    adjustOscillatorPool();
    logChords();
  }

  void resetSequence()
  {
    setChordWriteModeEnabled(false);
    seqPos = 0;
    updateLEDs();
  }

  void advanceSequence()
  {
    setChordWriteModeEnabled(false);
    seqPos = (seqPos + 1) % chords.size();
    updateLEDs();
  }

  void updateLEDs()
  {
    addChordModeLED->setValue(isWriteMode ? 1.f : 0.f);

    // Display the current sequence position in binary on the LEDs.
    seqPos1LED->setValue((seqPos + 1) & 1 << 0 ? 1.f : 0.f);
    seqPos2LED->setValue((seqPos + 1) & 1 << 1 ? 1.f : 0.f);
    seqPos3LED->setValue((seqPos + 1) & 1 << 2 ? 1.f : 0.f);
    seqPos4LED->setValue((seqPos + 1) & 1 << 3 ? 1.f : 0.f);
  }

  void adjustOscillatorPool()
  {
    size_t maxChordSize = 0;
    for (std::vector<float> chord : chords)
    {
      maxChordSize = std::max(maxChordSize, chord.size());
    }

    while (glides.size() < maxChordSize)
    {
      Glide *glide = new Glide();
      glide->Init(getFrameInfo().sampleRate, glideKnob->getValue() + glideCVIn->getValue());
      glides.push_back(glide);
    }
    while (glides.size() > maxChordSize)
    {
      Glide *glide = glides.back();
      glides.pop_back();
      delete glide;
    }

    size_t oscillatorsNeeded = maxChordSize * 2;
    while (oscillators.size() < oscillatorsNeeded)
    {
      Osc *osc = new Osc();
      osc->Init(getFrameInfo().sampleRate);
      oscillators.push_back(osc);
    }
    while (oscillators.size() > oscillatorsNeeded)
    {
      Osc *osc = oscillators.back();
      oscillators.pop_back();
      delete osc;
    }
  }

  void logChords()
  {
    // PHNQ_LOG("num chords: %lu", chords.size());
    // for (vector<float> chord : chords)
    // {
    //   PHNQ_LOG("chord");
    //   for (float note : chord)
    //   {
    //     PHNQ_LOG("- %f", pitchToFrequency(note));
    //   }
    // }
  }

  void sampleRateDidChange(float sampleRate) override
  {
    for (Osc *osc : oscillators)
    {
      osc->Init(sampleRate);
    }

    for (Glide *glide : glides)
    {
      glide->Init(sampleRate, glideKnob->getValue() + glideCVIn->getValue());
    }
  }

  void process(FrameInfo frameInfo) override
  {
    float amp1 = 0.f, amp2 = 0.f;

    if (!chords.empty())
    {
      float tune = (this->tuneKnob->getValue() - 0.5f + this->tuneCVIn->getValue()) / 2.5f;
      float detune = (this->detuneKnob->getValue() + this->detuneCVIn->getValue()) / 100.f;
      float shape = this->shapeKnob->getValue() + this->shapeCVIn->getValue();
      float glideTime = isWriteMode ? 0 : this->glideKnob->getValue() + this->glideCVIn->getValue();

      std::vector<float> chord = chords[seqPos];
      size_t chordSize = chord.size();
      for (size_t i = 0; i < chordSize; i++)
      {
        Glide *glide = glides[i];
        glide->SetHtime(glideTime);

        float pitch = glide->Process(chord[i] + tune);

        float freq1 = pitchToFrequency(pitch - detune);
        float freq2 = pitchToFrequency(pitch + detune);

        Osc *osc1 = oscillators[2 * i];
        osc1->SetSyncFreq(freq1);
        osc1->SetWaveshape(shape);
        osc1->SetPW(0.5f);
        amp1 += osc1->Process();

        Osc *osc2 = oscillators[2 * i + 1];
        osc2->SetSyncFreq(freq2);
        osc2->SetWaveshape(shape);
        osc2->SetPW(0.5f);
        amp2 += osc2->Process();
      }
    }

    audioOutLeft->setValue(amp1 * 0.5f);
    audioOutRight->setValue(amp2 * 0.5f);
  }
};

#ifdef PHNQ_RACK
#include "../../core2/rack/RackModuleUI.hpp"
struct PolyVoxUI : phnq::vcv::RackModuleUI<PolyVox>
{
  PolyVoxUI(phnq::vcv::RackModule<PolyVox> *module) : phnq::vcv::RackModuleUI<PolyVox>(module, "res/PolyVox.svg")
  {
    addInputPort<PJ301MPort>(engine->resetSeqGateIn);
    addInputPort<PJ301MPort>(engine->advanceSeqGateIn);
    addOutputPort<PJ301MPort>(engine->audioOutLeft);
    addOutputPort<PJ301MPort>(engine->audioOutRight);
    addInputPort<PJ301MPort>(engine->addNoteGateIn);
    addInputPort<PJ301MPort>(engine->addNoteCVIn);
    addParamControl<VCVButton>(engine->addChordButton);
    addLight<MediumLight<RedLight>>(engine->addChordModeLED);
    addParamControl<VCVButton>(engine->deleteChordButton);
    addLight<MediumLight<RedLight>>(engine->seqPos1LED);
    addLight<MediumLight<RedLight>>(engine->seqPos2LED);
    addLight<MediumLight<RedLight>>(engine->seqPos3LED);
    addLight<MediumLight<RedLight>>(engine->seqPos4LED);
    addParamControl<Rogan2PSWhite>(engine->tuneKnob);
    addParamControl<Rogan2PSWhite>(engine->detuneKnob);
    addParamControl<Rogan2PSWhite>(engine->shapeKnob);
    addParamControl<Rogan2PSWhite>(engine->glideKnob);
    addInputPort<PJ301MPort>(engine->tuneCVIn);
    addInputPort<PJ301MPort>(engine->detuneCVIn);
    addInputPort<PJ301MPort>(engine->shapeCVIn);
    addInputPort<PJ301MPort>(engine->glideCVIn);
  }
};
rack::plugin::Model *modelPolyVox = rack::createModel<phnq::vcv::RackModule<PolyVox>, PolyVoxUI>("PolyVox");
#endif

#ifdef PHNQ_SEED
Engine *engineInstance = new PolyVox();
#include "../../core2/seed/SeedModule.hpp"
#endif
