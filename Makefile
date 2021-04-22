# Project Name
TARGET = leslie

# Sources
CPP_SOURCES = leslie.cpp

# Library Locations
LIBDAISY_DIR = ../libDaisy
DAISYSP_DIR = ../DaisySP

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

# Include terrarium.h
C_INCLUDES += -I../Terrarium

