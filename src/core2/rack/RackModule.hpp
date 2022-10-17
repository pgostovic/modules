#pragma once

#include <rack.hpp>
#include "../engine/Engine.hpp"

namespace phnq
{
  namespace vcv
  {
    std::map<engine::BasePort *, u_int8_t> getPortIndexes(engine::Engine *engine);

    template <class TEngine>
    struct RackModule : rack::engine::Module
    {
    private:
      engine::Engine *engine = new TEngine();
      std::map<engine::BasePort *, u_int8_t> portIndexes;

    public:
      RackModule()
      {
        portIndexes = getPortIndexes(engine);
        config(engine->getParams().size(),
               engine->getAudioIns().size() + engine->getCVIns().size() + engine->getGateIns().size(),
               engine->getAudioOuts().size() + engine->getCVOuts().size() + engine->getGateOuts().size(),
               engine->getLights().size());
      }

      TEngine *getEngine()
      {
        return static_cast<TEngine *>(this->engine);
      }

      void process(const ProcessArgs &args) override
      {
        for (engine::Param *param : engine->getParams())
        {
          param->setValue(params[portIndexes[param]].getValue());
        }

        for (engine::AudioIn *audioIn : engine->getAudioIns())
        {
          audioIn->setValue(inputs[portIndexes[audioIn]].getVoltage() / 5.f);
        }

        for (engine::CVIn *cvIn : engine->getCVIns())
        {
          cvIn->setValue(inputs[portIndexes[cvIn]].getVoltage() / 10.f);
        }

        for (engine::GateIn *gateIn : engine->getGateIns())
        {
          /**
           * @brief Avoid rapid gate flipping as per:
           *    https://vcvrack.com/manual/VoltageStandards#Triggers-and-Gates
           */
          float voltage = inputs[portIndexes[gateIn]].getVoltage();
          if (gateIn->getValue() && voltage < 2.f)
          {
            gateIn->setValue(false);
          }
          else if (!gateIn->getValue() && voltage > 0.1f)
          {
            gateIn->setValue(true);
          }
        }

        engine->doProcess({args.sampleRate, args.sampleTime});

        for (engine::AudioOut *audioOut : engine->getAudioOuts())
        {
          outputs[portIndexes[audioOut]].setVoltage(audioOut->getValue() * 5.f);
        }

        for (engine::CVOut *cvOut : engine->getCVOuts())
        {
          outputs[portIndexes[cvOut]].setVoltage(cvOut->getValue() * 10.f);
        }

        for (engine::GateOut *gateOut : engine->getGateOuts())
        {
          outputs[portIndexes[gateOut]].setVoltage(gateOut->getValue() ? 10.f : 0.f);
        }

        for (engine::Light *light : engine->getLights())
        {
          lights[portIndexes[light]].setBrightness(light->getValue());
        }
      }
    };

    std::map<engine::BasePort *, u_int8_t> getPortIndexes(engine::Engine *engine)
    {
      std::map<engine::BasePort *, u_int8_t> portIndexes;

      u_int8_t index = 0;
      for (engine::BasePort *port : engine->getAudioIns())
      {
        portIndexes[port] = index++;
      }
      for (engine::BasePort *port : engine->getCVIns())
      {
        portIndexes[port] = index++;
      }
      for (engine::BasePort *port : engine->getGateIns())
      {
        portIndexes[port] = index++;
      }

      index = 0;
      for (engine::BasePort *port : engine->getAudioOuts())
      {
        portIndexes[port] = index++;
      }
      for (engine::BasePort *port : engine->getCVOuts())
      {
        portIndexes[port] = index++;
      }
      for (engine::BasePort *port : engine->getGateOuts())
      {
        portIndexes[port] = index++;
      }

      index = 0;
      for (engine::BasePort *port : engine->getParams())
      {
        portIndexes[port] = index++;
      }

      index = 0;
      for (engine::BasePort *port : engine->getLights())
      {
        portIndexes[port] = index++;
      }

      return portIndexes;
    }
  }
}
