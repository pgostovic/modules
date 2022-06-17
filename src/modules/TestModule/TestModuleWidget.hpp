#pragma once

#include <rack.hpp>
#include "../../core/rack/RackModule.hpp"
#include "TestModule.hpp"

using namespace rack;

extern Plugin *pluginInstance;

struct TestModuleWidget : rack::app::ModuleWidget
{
  TestModuleWidget(phnq::RackModule<TestModule> *module)
  {
    setModule(module);

    setPanel(createPanel(asset::plugin(pluginInstance, "res/TestModule.svg")));

    // addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    // addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    // addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    // addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    // NOTE: when this widget is used to generate the module selector previes, module will be NULL.
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(5.161, 15.6)), module, 0));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.161, 26.239)), module, 0));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.161, 36.817)), module, 0));

    if (module)
    {
      module->addCVInputMapping(0, module->getDelegate()->pitchCV);
      module->addParamMapping(0, module->getDelegate()->pitchParam);
      module->addAudioOutputMapping(0, module->getDelegate()->audioOut);
    }

    // addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(5.161, 8.741)), module, 0));
  }
};

rack::plugin::Model *modelTestModule = createModel<phnq::RackModule<TestModule>, TestModuleWidget>("TestModule");
