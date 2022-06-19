#pragma once

#include <rack.hpp>
#include "RackModule.hpp"
#include "../Module.hpp"

using namespace rack;

extern Plugin *pluginInstance;

namespace phnq
{
  template <class TEngine = phnq::Module>
  struct RackModuleUI : rack::app::ModuleWidget
  {
    RackModuleUI(RackModule<TEngine> *module)
    {
      setModule(module);

      setPanel(createPanel(asset::plugin(pluginInstance, "res/TestModule.svg")));

      addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
      addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
      addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
      addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

      // NOTE: when this widget is used to generate the module selector preview, module will be NULL.

      float y = 0.f;

      if (module)
      {
        IOConfig ioConfig = module->getEngine()->getIOConfig();

        uint8_t paramIndex = 0;
        uint8_t inputIndex = 0;
        uint8_t outputIndex = 0;

        for (IOPort *ioPort : module->getEngine()->getIOPorts())
        {
          y += 20.f;
          switch (ioPort->getType())
          {
          case IOPortType::Audio:
          case IOPortType::CV:
          case IOPortType::Gate:
            if (ioPort->getDirection() == IOPortDirection::Input)
            {
              addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, y)), module, inputIndex));
              module->addPortMapping(inputIndex, ioPort);
              inputIndex++;
            }
            else if (ioPort->getDirection() == IOPortDirection::Output)
            {
              addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24, y)), module, outputIndex));
              module->addPortMapping(outputIndex, ioPort);
              outputIndex++;
            }
            break;
          case IOPortType::Param:
            if (ioPort->getDirection() == IOPortDirection::Input)
            {
              addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, y)), module, paramIndex));
              module->addPortMapping(paramIndex, ioPort);
              paramIndex++;
            }
            break;
          }
        }
      }
    }
  };
}
