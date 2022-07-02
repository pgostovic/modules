#pragma once

#include <rack.hpp>
#include "RackModule.hpp"
#include "pugixml.hpp"
#include "../Engine.hpp"
#include <fmt/core.h>
#include <string>

using namespace rack;

extern Plugin *pluginInstance;

namespace phnq
{
  template <class TEngine = phnq::Engine>
  struct RackModuleUI : rack::app::ModuleWidget
  {
    RackModuleUI(RackModule<TEngine> *module, std::string panelFile)
    {
      setModule(module);

      setPanel(createPanel(asset::plugin(pluginInstance, panelFile)));

      pugi::xml_document doc;
      pugi::xml_parse_result result = doc.load_file(asset::plugin(pluginInstance, panelFile).c_str());

      addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
      addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
      addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
      addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

      // NOTE: when this widget is used to generate the module selector preview, module will be NULL.

      if (module)
      {
        IOConfig ioConfig = module->getEngine()->getIOConfig();

        uint8_t paramIndex = 0;
        uint8_t inputIndex = 0;
        uint8_t outputIndex = 0;

        for (IOPort *ioPort : module->getEngine()->getIOPorts())
        {
          float cx = 0.f, cy = 0.f;

          pugi::xpath_node node = doc.select_node(fmt::format("//circle[@id = '{}']", ioPort->getPanelId()).c_str());
          if (node)
          {
            cx = std::stof(std::string(node.node().attribute("cx").value()));
            cy = std::stof(std::string(node.node().attribute("cy").value()));
          }
          else
          {
            PHNQ_LOG("A circle with id=\"%s\" could not be found in ", ioPort->getPanelId().c_str());
          }

          switch (ioPort->getType())
          {
          case IOPortType::Audio:
          case IOPortType::CV:
          case IOPortType::Gate:
            if (ioPort->getDirection() == IOPortDirection::Input)
            {
              addInput(createInputCentered<PJ301MPort>(mm2px(Vec(cx, cy)), module, inputIndex));
              module->addPortMapping(inputIndex, ioPort);
              inputIndex++;
            }
            else if (ioPort->getDirection() == IOPortDirection::Output)
            {
              addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(cx, cy)), module, outputIndex));
              module->addPortMapping(outputIndex, ioPort);
              outputIndex++;
            }
            break;
          case IOPortType::Param:
            if (ioPort->getDirection() == IOPortDirection::Input)
            {
              addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(cx, cy)), module, paramIndex));
              module->addPortMapping(paramIndex, ioPort);
              paramIndex++;
            }
            break;
          case IOPortType::Button:
            if (ioPort->getDirection() == IOPortDirection::Input)
            {
              addParam(createParamCentered<VCVButton>(mm2px(Vec(cx, cy)), module, paramIndex));
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
