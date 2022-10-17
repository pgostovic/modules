#pragma once

#include <rack.hpp>
#include "../Engine.hpp"
#include <vector>
#include <map>
#include <string>

using namespace std;

namespace phnq
{
  struct PortMapping
  {
    unsigned int id;
    IOPort *port;
  };

  template <class TEngine = phnq::Engine>
  struct RackModule : rack::engine::Module
  {
  private:
    phnq::Engine *engine;
    vector<PortMapping> portMappings;

  public:
    RackModule()
    {
      engine = new TEngine();
      IOConfig ioConfig = engine->getIOConfig();
      config(ioConfig.numParams + ioConfig.numButtons, ioConfig.numAudioIns + ioConfig.numCVIns + ioConfig.numGateIns, ioConfig.numAudioOuts + ioConfig.numCVOuts + ioConfig.numGateOuts, ioConfig.numLeds);
    }

    ~RackModule()
    {
      delete engine;
    }

    TEngine *getEngine()
    {
      return (TEngine *)this->engine;
    }

    void addPortMapping(unsigned int id, IOPort *port)
    {
      portMappings.push_back({id, port});
    }

    void process(const ProcessArgs &args) override
    {
      /**
       * Perpare the input values for the module engine by querying the module host:
       * - Audio input port values are set.
       * - Param values are set on corresponding ports.
       * - CV input port values are set.
       * - Gate input port values are set.
       */
      for (vector<PortMapping>::iterator it = portMappings.begin(); it != portMappings.end(); it++)
      {
        if (it->port->getDirection() == IOPortDirection::Input)
        {
          switch (it->port->getType())
          {
          case IOPortType::Audio:
            // [-5, 5] -> [-1, 1] -- divide by 5.
            it->port->setValue(inputs[it->id].getVoltage() / 5.f);
            break;
          case IOPortType::Param:
          case IOPortType::Button:
            // [0, 1] -> [0, 1] -- nothing to be done.
            it->port->setValue(params[it->id].getValue());
            break;
          case IOPortType::CV:
          case IOPortType::Gate:
            // [0, 10] -> [0, 1] -- divide by 10.
            it->port->setValue(inputs[it->id].getVoltage() / 10.f);
            break;
          case IOPortType::Led:
            // N/A
            break;
          }
        }
      }

      /**
       * DSP Processing is done here. The inputs set above are used and output values are set.
       */
      engine->doProcess({args.sampleRate, args.sampleTime});

      /**
       * Take the output values that were set in `engine->doProcess()` and send them to the module host:
       * - Audio
       * - CV
       * - Gate
       */
      for (vector<PortMapping>::iterator it = portMappings.begin(); it != portMappings.end(); it++)
      {
        if (it->port->getDirection() == IOPortDirection::Output)
        {
          switch (it->port->getType())
          {
          case IOPortType::Audio:
            // [-1, 1] -> [-5, 5] -- multiply by 5.
            outputs[it->id].setVoltage(it->port->getValue() * 5.f);
            break;
          case IOPortType::CV:
          case IOPortType::Gate:
            // [0, 1] -> [0, 10] -- multiply by 10.
            outputs[it->id].setVoltage(it->port->getValue() * 10.f);
            break;
          case IOPortType::Led:
            lights[it->id].setBrightness(it->port->getValue());
            break;

          case IOPortType::Param:
          case IOPortType::Button:
            // N/A
            break;
          }
        }
      }
    }
  };
}
