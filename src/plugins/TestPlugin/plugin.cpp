#include "plugin.h"

Plugin *pluginInstance;

void init(Plugin *p)
{
  pluginInstance = p;

  // Add modules here
  p->addModel(modelTestModule);
  p->addModel(modelChordSeq);

  INFO("HELLOx");
  // Any other plugin initialization may go here.
  // As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
