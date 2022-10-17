#pragma once

#include <rack.hpp>
#include <fmt/core.h>
#include "pugixml.hpp"
#include "RackModule.hpp"

using namespace rack;

extern Plugin *pluginInstance;

namespace phnq
{
  namespace vcv
  {
    template <class TEngine>
    struct RackModuleUI : app::ModuleWidget
    {
      TEngine *engine;
      std::map<engine::BasePort *, u_int8_t> portIndexes;
      pugi::xml_document panelSvgDoc;

      RackModuleUI(RackModule<TEngine> *module, std::string panelFile)
      {
        setModule(module);

        std::string panelPath = asset::plugin(pluginInstance, panelFile);
        setPanel(createPanel(panelPath));
        panelSvgDoc.load_file(panelPath.c_str());

        // For VCV Rack module preview, `module` will be NULL, so instantiate an engine to get port counts.
        engine = module ? module->getEngine() : new TEngine();

        portIndexes = getPortIndexes(engine);
      }

      ~RackModuleUI()
      {
        // If this is VCV Rack preview mode, then dispose of the engine instance that was created above.
        if (!module)
        {
          delete engine;
        }
      }

      Vec getLocationForId(std::string id)
      {
        float cx = 0, cy = 0;
        pugi::xpath_node node = panelSvgDoc.select_node(fmt::format("//*[@id = '{}']", id).c_str());
        if (node)
        {
          cx = std::stof(std::string(node.node().attribute("cx").value()));
          cy = std::stof(std::string(node.node().attribute("cy").value()));
        }
        else
        {
          PHNQ_LOG("A circle with id=\"%s\" could not be found in ", id.c_str());
        }
        return Vec(cx, cy);
      }

      template <class TParamWidget>
      void addParamControl(phnq::engine::Param *param)
      {
        addParam(createParamCentered<TParamWidget>(mm2px(getLocationForId(param->getId())), module, portIndexes[param]));
      }

      template <class TPortWidget>
      void addInputPort(phnq::engine::AudioIn *inputPort)
      {
        addInput(createInputCentered<TPortWidget>(mm2px(getLocationForId(inputPort->getId())), module, portIndexes[inputPort]));
      }

      template <class TPortWidget>
      void addInputPort(phnq::engine::CVIn *inputPort)
      {
        addInput(createInputCentered<TPortWidget>(mm2px(getLocationForId(inputPort->getId())), module, portIndexes[inputPort]));
      }

      template <class TPortWidget>
      void addInputPort(phnq::engine::GateIn *inputPort)
      {
        addInput(createInputCentered<TPortWidget>(mm2px(getLocationForId(inputPort->getId())), module, portIndexes[inputPort]));
      }

      template <class TPortWidget>
      void addOutputPort(phnq::engine::AudioOut *outputPort)
      {
        addOutput(createOutputCentered<TPortWidget>(mm2px(getLocationForId(outputPort->getId())), module, portIndexes[outputPort]));
      }

      template <class TPortWidget>
      void addOutputPort(phnq::engine::CVOut *outputPort)
      {
        addOutput(createOutputCentered<TPortWidget>(mm2px(getLocationForId(outputPort->getId())), module, portIndexes[outputPort]));
      }

      template <class TPortWidget>
      void addOutputPort(phnq::engine::GateOut *outputPort)
      {
        addOutput(createOutputCentered<TPortWidget>(mm2px(getLocationForId(outputPort->getId())), module, portIndexes[outputPort]));
      }

      template <class TModuleLightWidget>
      void addLight(phnq::engine::Light *light)
      {
        addChild(createLightCentered<TModuleLightWidget>(mm2px(getLocationForId(light->getId())), module, portIndexes[light]));
      }
    };
  }
}