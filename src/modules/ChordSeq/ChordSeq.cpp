#include <cmath>
#include <daisysp.h>
#include <vector>
#include "../../core/Engine.hpp"

/**
 * Polyphonic oscillator where you can program a chord progression. The
 * progression is a stepped, looping sequence; trigger advances to the next
 * step.
 *
 * Audio output plays the sustained chord:
 * - choose waveform (i.e. saw, square, triangle, sine, etc.)
 * - stereo out? Maybe some/all of:
 *    - chorus
 *    - detune (i.e. each note of poly gets 2 detuned oscs L/R)
 *    - pan the chord notes individually.
 *
 * Triggered CV out:
 * - While on a chord in the progression, trigger to cycle through the notes
 *    in the current chord like an arpeggiator. Maybe have a few different
 *    modes: up, down, random, etc.
 *
 *
 * UI Elements:
 * - Seq record section
 *    - CV/Gate in: each high gate adds the CV in value to the current chord.
 *    - button
 *        - long press: erase all chords
 *        - single press: advance to the next chord.
 *
 *
 *
 * 1. Erase chords -- press button
 * 2. Play several notes for a chord -- cv/gate in (i.e. keystep to set notes)
 * 3. Next chord -- press button, go to 2.
 * 4. Play seq, advancing chords by trigger.
 * 5.
 *
 */

using namespace phnq;
using namespace daisysp;

struct ChordSeq : phnq::Engine
{
  size_t chordIndex = 0;
  vector<vector<float>> chords;
  vector<Oscillator *> oscillators;

  IOPort *nextChordGate;
  IOPort *nextChordButton;
  IOPort *resetGate;
  IOPort *resetButton;
  IOPort *addChordGate;
  IOPort *addChordButton;
  IOPort *removeChordGate;
  IOPort *removeChordButton;

  IOPort *addNoteGate;
  IOPort *addNoteButton;
  IOPort *addNoteCV;
  IOPort *removeNoteGate;
  IOPort *removeNoteButton;

  IOPort *detuneParam;

  IOPort *audioOut1;
  IOPort *audioOut2;

  ChordSeq()
  {
    this->nextChordGate = addIOPort(IOPortType::Gate, IOPortDirection::Input, "nextChordGate");
    this->nextChordButton = addIOPort(IOPortType::Button, IOPortDirection::Input, "nextChordButton");
    this->resetGate = addIOPort(IOPortType::Gate, IOPortDirection::Input, "resetGate");
    this->resetButton = addIOPort(IOPortType::Button, IOPortDirection::Input, "resetButton");
    this->addChordGate = addIOPort(IOPortType::Gate, IOPortDirection::Input, "addChordGate");
    this->addChordButton = addIOPort(IOPortType::Button, IOPortDirection::Input, "addChordButton");
    this->removeChordGate = addIOPort(IOPortType::Gate, IOPortDirection::Input, "removeChordGate");
    this->removeChordButton = addIOPort(IOPortType::Button, IOPortDirection::Input, "removeChordButton");

    this->addNoteGate = addIOPort(IOPortType::Gate, IOPortDirection::Input, "addNoteGate")->setDelay(10);
    this->addNoteButton = addIOPort(IOPortType::Button, IOPortDirection::Input, "addNoteButton");
    this->addNoteCV = addIOPort(IOPortType::CV, IOPortDirection::Input, "addNoteCV");
    this->removeNoteGate = addIOPort(IOPortType::Gate, IOPortDirection::Input, "removeNoteGate");
    this->removeNoteButton = addIOPort(IOPortType::Button, IOPortDirection::Input, "removeNoteButton");

    this->detuneParam = addIOPort(IOPortType::Param, IOPortDirection::Input, "detune");

    this->audioOut1 = addIOPort(IOPortType::Audio, IOPortDirection::Output, "audioOut1");
    this->audioOut2 = addIOPort(IOPortType::Audio, IOPortDirection::Output, "audioOut2");

    addChord();
    addNoteToCurrentChord(0.3f);
    addNoteToCurrentChord(0.3f + (0.7f / 12.f));

    addChord();
    addNoteToCurrentChord(0.3f + (0.3f / 12.f));
    addNoteToCurrentChord(0.3f + (1.f / 12.f));
  }

  void nextChord()
  {
    chordIndex = ++chordIndex % chords.size();
  }

  void reset()
  {
    chordIndex = 0;
  }

  void addChord()
  {
    chordIndex++;
    if (chordIndex > chords.size())
    {
      chordIndex = chords.size();
    }
    chords.insert(chords.begin() + chordIndex, vector<float>());
  }

  void removeChord()
  {
    chords.erase(chords.begin() + chordIndex);
  }

  void addNoteToCurrentChord(float pitch)
  {
    while (chordIndex >= chords.size())
    {
      addChord();
    }
    chords[chordIndex].push_back(pitch);

    adjustOscillatorPool();
  }

  void removeLastNoteFromCurrentChord()
  {
    chords[chordIndex].pop_back();
    adjustOscillatorPool();
  }

  void adjustOscillatorPool()
  {
    size_t oscillatorsNeeded = 0;
    for (vector<float> chord : chords)
    {
      oscillatorsNeeded = std::max(oscillatorsNeeded, chord.size() * 2);
    }
    while (oscillators.size() < oscillatorsNeeded)
    {
      Oscillator *osc = new Oscillator();
      osc->Init(getFrameInfo().sampleRate);
      osc->SetWaveform(Oscillator::WAVE_SQUARE);
      oscillators.push_back(osc);
    }
    while (oscillators.size() > oscillatorsNeeded)
    {
      Oscillator *osc = oscillators.back();
      oscillators.pop_back();
      delete osc;
    }
  }

  void sampleRateDidChange(float sampleRate) override
  {
    for (Oscillator *osc : oscillators)
    {
      osc->Init(sampleRate);
      osc->SetWaveform(Oscillator::WAVE_SQUARE);
    }
  }

  void gateValueDidChange(IOPort *gatePort, bool high) override
  {
    if (high)
    {
      if (gatePort == nextChordGate || gatePort == nextChordButton)
      {
        nextChord();
      }
      else if (gatePort == resetGate || gatePort == resetButton)
      {
        reset();
      }
      else if (gatePort == addChordGate || gatePort == addChordButton)
      {
        addChord();
      }
      else if (gatePort == removeChordGate || gatePort == removeChordButton)
      {
        removeChord();
      }
      else if (gatePort == addNoteGate || gatePort == addNoteButton)
      {
        addNoteToCurrentChord(addNoteCV->getValue());
      }
      else if (gatePort == removeNoteGate || gatePort == removeNoteButton)
      {
        removeLastNoteFromCurrentChord();
      }
    }
  }

  void process(FrameInfo frameInfo) override
  {
    vector<float> chord = chords[chordIndex];
    float amp1 = 0.f, amp2 = 0.f;
    size_t i = chord.size();
    for (size_t i = 0; i < chord.size(); i++)
    {
      float pitch = chord[i];

      float detune = this->detuneParam->getValue() / 100.f;

      float freq1 = pitchToFrequency(pitch);
      float freq2 = pitchToFrequency(pitch + detune);

      Oscillator *osc1 = oscillators[2 * i];
      osc1->SetFreq(freq1);
      amp1 += osc1->Process();

      Oscillator *osc2 = oscillators[2 * i + 1];
      osc2->SetFreq(freq2);
      amp2 += osc2->Process();
    }

    // PHNQ_LOG("=======AMP %f", amp);

    audioOut1->setValue(amp1);
    audioOut2->setValue(amp2);
  }
};

#ifdef PHNQ_RACK
#include "../../core/rack/RackModuleUI.hpp"
struct ChordSeqUI : public RackModuleUI<ChordSeq>
{
  ChordSeqUI(RackModule<ChordSeq> *module) : RackModuleUI<ChordSeq>(module, "res/ChordSeq.svg") {}
};
rack::plugin::Model *modelChordSeq = rack::createModel<RackModule<ChordSeq>, ChordSeqUI>("ChordSeq");
#endif

#ifdef PHNQ_SEED
#include "../../core/seed/SeedModule.hpp"
Engine *moduleInstance = new ChordSeq();
#endif
