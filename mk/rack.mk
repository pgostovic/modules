PHNQ_DIR ?= .
ARCH := $(shell arch)
BUILD := build
DIST := $(BUILD)/dist
CXX := clang

SOURCES := $(shell find $(PHNQ_DIR)/src -type f -name '*.cpp') $(shell find $(PHNQ_DIR)/vendor/DaisySP/Source -type f -name '*.cpp')
OBJECTS := $(patsubst $(PHNQ_DIR)/%.cpp, $(BUILD)/%.o, $(SOURCES))

PLUGIN_NAMES := $(subst $(PHNQ_DIR)/src/plugins/,, $(wildcard $(PHNQ_DIR)/src/plugins/*))
PLUGIN_LIB := $(patsubst %, $(DIST)/plugins/%/plugin.dylib, $(PLUGIN_NAMES))
PLUGIN_MAN :=$(patsubst %, $(DIST)/plugins/%/plugin.json, $(PLUGIN_NAMES))
PLUGIN_RES :=$(patsubst %, $(DIST)/plugins/%/res, $(PLUGIN_NAMES))
CXXFLAGS += -I$(PHNQ_DIR)/vendor/DaisySP/Source -I$(PHNQ_DIR)/vendor/DaisySP/Source/Utility -MD


# Rack
ifneq ($(ARCH),i386)
$(error VCV Rack requires i386-based artifacts. Use rmake.)	
endif
CXXFLAGS += -std=c++11 -stdlib=libc++
CXXFLAGS += -DPHNQ_RACK
CXXFLAGS += -I$(PHNQ_DIR)/vendor/Rack-SDK/include -I$(PHNQ_DIR)/vendor/Rack-SDK/dep/include
LDFLAGS += -stdlib=libc++ -L $(PHNQ_DIR)/vendor/Rack-SDK -lRack -undefined dynamic_lookup -fPIC -shared




vendor: $(PHNQ_DIR)/vendor/Rack-SDK $(PHNQ_DIR)/vendor/libDaisy/Makefile $(PHNQ_DIR)/vendor/DaisySP/Makefile

clean-vendor:
	rm -rf $(PHNQ_DIR)/vendor/Rack-SDK

$(PHNQ_DIR)/vendor/Rack-SDK:
	curl -s https://vcvrack.com/downloads/Rack-SDK-2.1.1-mac.zip > $(PHNQ_DIR)/vendor/Rack-SDK.zip
	unzip -q -d $(PHNQ_DIR)/vendor $(PHNQ_DIR)/vendor/Rack-SDK.zip
	rm $(PHNQ_DIR)/vendor/Rack-SDK.zip

$(PHNQ_DIR)/vendor/libDaisy/Makefile:
	git submodule update --init

$(PHNQ_DIR)/vendor/DaisySP/Makefile:
	git submodule update --init

print:
# echo $(SOURCES)
	echo $(MAKECMDGOALS)


clean:
	rm -rf $(BUILD)

install: plugins
	cp -r $(DIST)/plugins/* ~/Documents/Rack2/plugins/

plugins: vendor $(PLUGIN_LIB) $(PLUGIN_MAN) $(PLUGIN_RES)

$(DIST)/plugins/%/plugin.dylib: $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(DIST)/plugins/%/plugin.json:
	@mkdir -p $(@D)
	cp $(PHNQ_DIR)/src/plugins/$*/plugin.json $@

$(DIST)/plugins/%/res:
	cp -r $(PHNQ_DIR)/src/plugins/$*/res $@

$(BUILD)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

-include $(OBJECTS:.o=.d)

