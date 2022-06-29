#include <cmath>
#include <daisysp.h>
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
  ChordSeq()
  {
  }

  void sampleRateDidChange(float sampleRate) override
  {
  }

  void gateValueDidChange(IOPort *gatePort) override
  {
  }

  void process(FrameInfo frameInfo) override
  {
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
