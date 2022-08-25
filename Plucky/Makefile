# Project Name
TARGET = Plucky

# Sources
CPP_SOURCES = main.cpp

DEBUG = 1

LIBS += -lmarguerite
LIBDIR += -L ../dsp/build

C_INCLUDES += -I../dsp

# Library Locations
LIBDAISY_DIR = ../externals/libDaisy/
DAISYSP_DIR = ../externals/DaisySP/

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
