PHNQ_DIR ?= .

usage:
	@echo "Usage: make [seed|rack] targets...\ne.g. make rack clean plugins"

$(PHNQ_DIR)/vendor/Rack-SDK:
	curl -s https://vcvrack.com/downloads/Rack-SDK-2.1.1-mac.zip > $(PHNQ_DIR)/vendor/Rack-SDK.zip
	unzip -q -d $(PHNQ_DIR)/vendor $(PHNQ_DIR)/vendor/Rack-SDK.zip
	rm $(PHNQ_DIR)/vendor/Rack-SDK.zip

$(PHNQ_DIR)/vendor/fmt/CMakeLists.txt:
	git submodule update --init

$(PHNQ_DIR)/vendor/pugixml/CMakeLists.txt:
	git submodule update --init

$(PHNQ_DIR)/vendor/DaisySP/Makefile:
	git submodule update --init

$(PHNQ_DIR)/vendor/DaisySP/build/libdaisysp.a: $(PHNQ_DIR)/vendor/DaisySP/Makefile
	make -C $(PHNQ_DIR)/vendor/DaisySP

$(PHNQ_DIR)/vendor/libDaisy/Makefile:
	git submodule update --init

$(PHNQ_DIR)/vendor/libDaisy/build/libdaisy.a: $(PHNQ_DIR)/vendor/libDaisy/Makefile
	make -C $(PHNQ_DIR)/vendor/libDaisy

seed: $(PHNQ_DIR)/vendor/libDaisy/build/libdaisy.a $(PHNQ_DIR)/vendor/DaisySP/build/libdaisysp.a
	@make -f mk/seed.mk $(patsubst seed,,$(MAKECMDGOALS))

rack: $(PHNQ_DIR)/vendor/Rack-SDK $(PHNQ_DIR)/vendor/DaisySP/Makefile $(PHNQ_DIR)/vendor/fmt/CMakeLists.txt $(PHNQ_DIR)/vendor/pugixml/CMakeLists.txt
	@arch -x86_64 make -f mk/rack.mk $(patsubst rack,,$(MAKECMDGOALS))

.DEFAULT:
	@echo $@

