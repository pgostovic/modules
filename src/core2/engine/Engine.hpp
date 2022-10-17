#pragma once

#include "ports/Port.hpp"
#include "ports/AudioIn.hpp"
#include "ports/AudioOut.hpp"
#include "ports/GateIn.hpp"
#include "ports/GateOut.hpp"
#include "ports/ControlIn.hpp"
#include "ports/ControlOut.hpp"
#include "ports/Param.hpp"
#include "ports/Button.hpp"
#include "ports/Light.hpp"

#ifdef PHNQ_RACK
#include <rack.hpp>
#define PHNQ_LOG INFO
#else
#ifdef PHNQ_SEED
#include "daisy_seed.h"
#define PHNQ_LOG daisy::DaisySeed::PrintLine
#else
#define PHNQ_LOG(format, ...) printf(format, ##__VA_ARGS__)
#endif
#endif

/**
 * GLOSSARY
 * ========
 * Ports:
 * - AudioIn: audio input jack, nominally +/-1.0, but can be higher.
 * - AudioOut: audio output jack, nominally +/-1.0, but can be higher, clamped to +/-2.0.
 * - ControlIn: CV input jack, +/-1.0 (clamped).
 * - ControlOut: CV output jack, +/-1.0 (clamped).
 * - GateIn: trigger/gate input jack, boolean.
 * - GateOut: trigger/gate output jack, boolean.
 * - Param: knob/button/encoder (UI elements), 0.0 to 1.0 (clamped).
 * - Light: typically an LED, 0.0 to 1.0 (clamped).
 */

namespace phnq
{
  namespace engine
  {
    const float FREQ_C1 = 32.7032f;

    static float pitchToFrequency(float pitch)
    {
      return FREQ_C1 * std::pow(2.f, pitch * 10.f);
    }

    struct FrameInfo
    {
      float sampleRate;
      float sampleTime;
    };

    struct Engine
    {
    private:
      FrameInfo frameInfo;
      std::vector<AudioIn *> audioIns;
      std::vector<AudioOut *> audioOuts;
      std::vector<ControlIn *> controlIns;
      std::vector<ControlOut *> controlOuts;
      std::vector<GateIn *> gateIns;
      std::vector<GateOut *> gateOuts;
      std::vector<Param *> params;
      std::vector<Light *> lights;

    public:
      Engine()
      {
      }

      const std::vector<AudioIn *> getAudioIns()
      {
        return this->audioIns;
      }

      const std::vector<AudioOut *> getAudioOuts()
      {
        return this->audioOuts;
      }

      const std::vector<ControlIn *> getControlIns()
      {
        return this->controlIns;
      }

      const std::vector<ControlOut *> getControlOuts()
      {
        return this->controlOuts;
      }

      const std::vector<GateIn *> getGateIns()
      {
        return this->gateIns;
      }

      const std::vector<GateOut *> getGateOuts()
      {
        return this->gateOuts;
      }

      const std::vector<Param *> getParams()
      {
        return this->params;
      }

      const std::vector<Light *> getLights()
      {
        return this->lights;
      }

      void doProcess(FrameInfo frameInfo)
      {
        if (frameInfo.sampleRate != this->frameInfo.sampleRate)
        {
          this->frameInfo = frameInfo;
          sampleRateDidChange(frameInfo.sampleRate);
        }
        this->process(frameInfo);
      }

    protected:
      virtual void sampleRateDidChange(float sampleRate) {}

      AudioIn *createAudioIn(std::string id)
      {
        AudioIn *audioIn = new AudioIn();
        this->audioIns.push_back(audioIn);
        return static_cast<AudioIn *>(audioIn->setId(id));
      }

      AudioOut *createAudioOut(std::string id)
      {
        AudioOut *audioOut = new AudioOut();
        this->audioOuts.push_back(audioOut);
        return static_cast<AudioOut *>(audioOut->setId(id));
      }

      ControlIn *createControlIn(std::string id)
      {
        ControlIn *controlIn = new ControlIn();
        this->controlIns.push_back(controlIn);
        return static_cast<ControlIn *>(controlIn->setId(id));
      }

      ControlOut *createControlOut(std::string id)
      {
        ControlOut *controlOut = new ControlOut();
        this->controlOuts.push_back(controlOut);
        return static_cast<ControlOut *>(controlOut->setId(id));
      }

      GateIn *createGateIn(std::string id)
      {
        GateIn *gateIn = new GateIn();
        this->gateIns.push_back(gateIn);
        return static_cast<GateIn *>(gateIn->setId(id));
      }

      GateOut *createGateOut(std::string id)
      {
        GateOut *gateOut = new GateOut();
        this->gateOuts.push_back(gateOut);
        return static_cast<GateOut *>(gateOut->setId(id));
      }

      Param *createParam(std::string id)
      {
        Param *param = new Param();
        this->params.push_back(param);
        return static_cast<Param *>(param->setId(id));
      }

      Button *createButton(std::string id)
      {
        Button *button = new Button();
        this->params.push_back(button);
        return static_cast<Button *>(button->setId(id));
      }

      Light *createLight(std::string id)
      {
        Light *light = new Light();
        this->lights.push_back(light);
        return static_cast<Light *>(light->setId(id));
      }

      FrameInfo getFrameInfo()
      {
        return this->frameInfo;
      }

      virtual void process(FrameInfo frameInfo)
      {
      }
    };
  }
}
