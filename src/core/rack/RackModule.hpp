#pragma once

#include <rack.hpp>
#include "../Module.hpp"
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

  template <class TEngine = phnq::Module>
  struct RackModule : rack::engine::Module
  {
  private:
    phnq::Module *engine;
    vector<PortMapping> portMappings;

  public:
    RackModule()
    {
      engine = new TEngine();
      IOConfig ioConfig = engine->getIOConfig();
      config(ioConfig.numParams, ioConfig.numAudioIns + ioConfig.numCVIns + ioConfig.numGateIns, ioConfig.numAudioOuts + ioConfig.numCVOuts + ioConfig.numGateOuts);
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
            // scale from Eurorack convention 5 to nominal amplitude of 1.
            it->port->setValue(inputs[it->id].getVoltage() / 5.f);
            break;
          case IOPortType::Param:
            // The phnq::Module instance treats params and CV ins the same. Multiply the 0-1 param range by 10.
            it->port->setValue(params[it->id].getValue() * 10.f);
            break;
          case IOPortType::CV:
            // phnq::Module uses the Eurorack convention, so nothing done here.
            it->port->setValue(inputs[it->id].getVoltage());
            break;
          case IOPortType::Gate:
            // phnq::Module uses the Eurorack convention, so nothing done here.
            it->port->setValue(inputs[it->id].getVoltage());
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
            // scale from nominal amplitude of 1 to Eurorack convention 5.
            outputs[it->id].setVoltage(it->port->getValue() * 5.f);
            break;
          case IOPortType::CV:
            // phnq::Module uses the Eurorack convention, so nothing done here.
            outputs[it->id].setVoltage(it->port->getValue());
            break;
          case IOPortType::Gate:
            // phnq::Module uses the Eurorack convention, so nothing done here.
            outputs[it->id].setVoltage(it->port->getValue());
            break;
          case IOPortType::Param:
            // N/A
            break;
          }
        }
      }
    }
  };
}
