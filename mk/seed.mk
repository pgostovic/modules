PHNQ_DIR ?= .

# Project Name
TARGET ?= ChordSeq
MODULE_DIR := $(PHNQ_DIR)/src/modules/$(TARGET)

ifeq ($(wildcard $(PHNQ_DIR)/src/modules/$(TARGET)),)
$(error No such module directory: $(MODULE_DIR))	
endif

# Sources
CPP_SOURCES := $(shell find $(PHNQ_DIR)/src/core -type f -name '*.cpp') $(shell find $(MODULE_DIR) -type f -name '*.cpp')

C_DEFS := -DPHNQ_SEED

# Library Locations
LIBDAISY_DIR = $(PHNQ_DIR)/vendor/libDaisy
DAISYSP_DIR = $(PHNQ_DIR)/vendor/DaisySP

# Uncomment to log floats -- apparently this increases the executable size by ~8kB
# LDFLAGS = -u _printf_float

# Core location, and generic makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

install: all program-dfu

print:
	@echo $(CPP_SOURCES)
