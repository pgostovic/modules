PHNQ_DIR ?= .
ARCH := $(shell arch)
BUILD := build
DIST := $(BUILD)/dist
RACK_INSTALL_DIR := ~/Documents/Rack2/plugins
CXX := clang

SOURCES := $(shell find $(PHNQ_DIR)/src -type f -name '*.cpp') $(shell find $(PHNQ_DIR)/vendor/DaisySP/Source -type f -name '*.cpp')
SOURCES += $(PHNQ_DIR)/vendor/pugixml/src/pugixml.cpp $(PHNQ_DIR)/vendor/fmt/src/format.cc # $(shell find $(PHNQ_DIR)/vendor/fmt/src -type f -name '*.cc')
OBJECTS := $(patsubst $(PHNQ_DIR)/%.cpp, $(BUILD)/%.o, $(SOURCES))
OBJECTS := $(patsubst $(PHNQ_DIR)/%.cc, $(BUILD)/%.o, $(OBJECTS))

# MODULE_NAMES := $(subst $(PHNQ_DIR)/src/modules/,, $(wildcard $(PHNQ_DIR)/src/modules/*))



PLUGIN_NAMES := $(subst $(PHNQ_DIR)/src/plugins/,, $(wildcard $(PHNQ_DIR)/src/plugins/*))
PLUGIN_DIR := $(patsubst %, $(DIST)/plugins/%, $(PLUGIN_NAMES))
PLUGIN_LIB := $(patsubst %, $(DIST)/plugins/%/plugin.dylib, $(PLUGIN_NAMES))
PLUGIN_MAN := $(patsubst %, $(DIST)/plugins/%/plugin.json, $(PLUGIN_NAMES))
PLUGIN_RES := $(patsubst %, $(DIST)/plugins/%/res/.latest, $(PLUGIN_NAMES))
PLUGIN_INSTALL := $(patsubst %, $(RACK_INSTALL_DIR)/%, $(PLUGIN_NAMES))


CXXFLAGS += -I$(PHNQ_DIR)/vendor/DaisySP/Source -I$(PHNQ_DIR)/vendor/DaisySP/Source/Utility -MD

ifneq ($(ARCH),i386)
$(error VCV Rack requires i386-based artifacts. Use arch -x86_64 make.)	
endif
CXXFLAGS += -std=c++11 -stdlib=libc++
CXXFLAGS += -DPHNQ_RACK
CXXFLAGS += -I$(PHNQ_DIR)/vendor/Rack-SDK/include -I$(PHNQ_DIR)/vendor/Rack-SDK/dep/include -I$(PHNQ_DIR)/vendor/pugixml/src -I$(PHNQ_DIR)/vendor/fmt/include
LDFLAGS += -stdlib=libc++ -L $(PHNQ_DIR)/vendor/Rack-SDK -lRack -undefined dynamic_lookup -fPIC -shared

all: plugins

print:
	@echo $(PLUGIN_INSTALL)

clean:
	rm -rf $(BUILD)

install: $(PLUGIN_INSTALL)

$(RACK_INSTALL_DIR)/%: vendor $(PLUGIN_LIB) $(PLUGIN_MAN) $(PLUGIN_RES)
	cp -r $(DIST)/plugins/$* $(RACK_INSTALL_DIR)

plugins: vendor $(PLUGIN_LIB) $(PLUGIN_MAN) $(PLUGIN_RES)

$(DIST)/plugins/%/plugin.dylib: $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(DIST)/plugins/%/plugin.json: $(shell find $(PHNQ_DIR)/src/plugins -type f -name plugin.json)
	@echo Updating resource files: $*
	@mkdir -p $(@D)
	@cp $(PHNQ_DIR)/src/plugins/$*/plugin.json $@

# This will copy all resources when anything changes in any module's res directory -- fine!
$(DIST)/plugins/%/res/.latest: $(shell find $(PHNQ_DIR)/src/modules/*/res -type f)
	@echo Updating resource files: $*
	@mkdir -p $(@D)
	@touch $@
	@for module in $(shell jq -r '.modules[] | .slug' $(PHNQ_DIR)/src/plugins/$*/plugin.json); do \
		cp $(PHNQ_DIR)/src/modules/$${module}/res/* $(@D); \
	done

$(BUILD)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD)/%.o: %.cc
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

-include $(OBJECTS:.o=.d)

