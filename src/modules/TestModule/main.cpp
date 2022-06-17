/**
 * This is where implementation-specific (Rack/Seed) code is included.
 */
#include "TestModule.hpp"

#ifdef PHNQ_RACK
#include "TestModuleWidget.hpp"
#endif

#ifdef PHNQ_SEED
#include "../../core/seed/SeedModule.hpp"
phnq::Module *moduleInstance = new TestModule();
#endif
