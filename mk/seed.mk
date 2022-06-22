PHNQ_DIR ?= .

# Project Name
TARGET = Seedy

# Sources
CPP_SOURCES := $(shell find $(PHNQ_DIR)/src/core -type f -name '*.cpp') $(shell find $(PHNQ_DIR)/src/modules -type f -name '*.cpp')

C_DEFS := -DPHNQ_SEED

# Library Locations
LIBDAISY_DIR = $(PHNQ_DIR)/vendor/libDaisy
DAISYSP_DIR = $(PHNQ_DIR)/vendor/DaisySP

# Uncomment to log floats -- apparently this increases the executable size by ~8kB
# LDFLAGS = -u _printf_float

# Core location, and generic makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

vendor: $(PHNQ_DIR)/vendor/libDaisy/build/libdaisy.a $(PHNQ_DIR)/vendor/DaisySP/build/libdaisysp.a

clean-vendor:
	make -C $(PHNQ_DIR)/vendor/libDaisy clean
	make -C $(PHNQ_DIR)/vendor/DaisySP clean


$(PHNQ_DIR)/vendor/libDaisy/build/libdaisy.a:
	git submodule update --init
	make -C $(PHNQ_DIR)/vendor/libDaisy
	

$(PHNQ_DIR)/vendor/DaisySP/build/libdaisysp.a:
	git submodule update --init
	make -C $(PHNQ_DIR)/vendor/DaisySP

install: vendor all program-dfu
