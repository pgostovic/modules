#pragma once

#include <rack.hpp>
#include "../Module.hpp"
#include <vector>
#include <map>
#include <string>

using namespace std;

namespace phnq
{
  struct ParamMapping
  {
    unsigned int id;
    phnq::CVPort *port;
  };

  struct AudioInputMapping
  {
    unsigned int id;
    phnq::AudioPort *port;
  };

  struct AudioOutputMapping
  {
    unsigned int id;
    phnq::AudioPort *port;
  };

  struct CVInputMapping
  {
    unsigned int id;
    phnq::CVPort *port;
  };

  struct CVOutputMapping
  {
    unsigned int id;
    phnq::CVPort *port;
  };

  struct GateInputMapping
  {
    unsigned int id;
    phnq::GatePort *port;
  };

  struct GateOutputMapping
  {
    unsigned int id;
    phnq::GatePort *port;
  };

  template <class TDelegate = phnq::Module>
  struct RackModule : rack::engine::Module
  {
  private:
    phnq::Module *delegate;
    vector<ParamMapping> paramMappings;
    vector<AudioInputMapping> audioInputMappings;
    vector<AudioOutputMapping> audioOutputMappings;
    vector<CVInputMapping> cvInputMappings;
    vector<CVOutputMapping> cvOutputMappings;
    vector<GateInputMapping> gateInputMappings;
    vector<GateOutputMapping> gateOutputMappings;

  public:
    RackModule()
    {
      delegate = new TDelegate();

      IOConfig ioConfig = delegate->getIOConfig();

      uint8_t numParams = 0;
      uint8_t numIns = ioConfig.numAudioIns + ioConfig.numCVIns + ioConfig.numGateIns;
      uint8_t numOuts = ioConfig.numAudioOuts + ioConfig.numCVOuts + ioConfig.numGateOuts;

      for (uint8_t i = 0; i < ioConfig.numCVIns; i++)
      {
        if (delegate->getCVIn(i)->isParam())
        {
          numParams++;
          numIns--;
        }
      }

      config(numParams, numIns, numOuts);
    }

    ~RackModule()
    {
      delete delegate;
    }

    TDelegate *getDelegate()
    {
      return (TDelegate *)this->delegate;
    }

    void addParamMapping(unsigned int id, phnq::CVPort *port)
    {
      paramMappings.push_back({id, port});
    }

    void addAudioInputMapping(unsigned int id, phnq::AudioPort *port)
    {
      audioInputMappings.push_back({id, port});
    }

    void addAudioOutputMapping(unsigned int id, phnq::AudioPort *port)
    {
      audioOutputMappings.push_back({id, port});
    }

    void addCVInputMapping(unsigned int id, phnq::CVPort *port)
    {
      cvInputMappings.push_back({id, port});
    }

    void addCVOutputMapping(unsigned int id, phnq::CVPort *port)
    {
      cvOutputMappings.push_back({id, port});
    }

    void addGateInputMapping(unsigned int id, phnq::GatePort *port)
    {
      gateInputMappings.push_back({id, port});
    }

    void addGateOutputMapping(unsigned int id, phnq::GatePort *port)
    {
      gateOutputMappings.push_back({id, port});
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
      for (vector<AudioInputMapping>::iterator it = audioInputMappings.begin(); it != audioInputMappings.end(); it++)
      {
        // scale from Eurorack convention 5 to nominal amplitude of 1.
        it->port->setValue(inputs[it->id].getVoltage() / 5.f);
      }

      for (vector<ParamMapping>::iterator it = paramMappings.begin(); it != paramMappings.end(); it++)
      {
        // The phnq::Module instance treats params and CV ins the same. Multiply the 0-1 param range by 5.
        it->port->setValue(params[it->id].getValue() * 5.f);
      }

      for (vector<CVInputMapping>::iterator it = cvInputMappings.begin(); it != cvInputMappings.end(); it++)
      {
        // phnq::Module uses the Eurorack convention of 5v amplitude, so nothing done here.
        it->port->setValue(inputs[it->id].getVoltage());
      }

      for (vector<GateInputMapping>::iterator it = gateInputMappings.begin(); it != gateInputMappings.end(); it++)
      {
        it->port->setValue(inputs[it->id].getVoltage());
      }

      /**
       * DSP Processing is done here. The inputs set above are used and output values are set.
       */
      delegate->doProcess({args.sampleRate, args.sampleTime});

      /**
       * Take the output values that were set in `delegate->doProcess()` and send them to the module host:
       * - Audio
       * - CV
       * - Gate
       */
      for (vector<AudioOutputMapping>::iterator it = audioOutputMappings.begin(); it != audioOutputMappings.end(); it++)
      {
        outputs[it->id].setVoltage(it->port->getValue() * 5.f); // scale from nominal amplitude of 1 to Eurorack convention 5.
      }

      for (vector<CVOutputMapping>::iterator it = cvOutputMappings.begin(); it != cvOutputMappings.end(); it++)
      {
        // phnq::Module uses the Eurorack convention of 5v amplitude, so nothing done here.
        outputs[it->id].setVoltage(it->port->getValue());
      }

      for (vector<GateOutputMapping>::iterator it = gateOutputMappings.begin(); it != gateOutputMappings.end(); it++)
      {
        outputs[it->id].setVoltage(it->port->getValue() ? 10.f : 0.f);
      }
    }
  };
}
