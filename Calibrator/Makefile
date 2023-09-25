# Project Name
TARGET = Calibrator

# Sources
CPP_SOURCES = main.cpp

# DEBUG = 1

LIBS += -ldsp
LIBDIR += -L ../externals/libdsp/daisy/src

C_INCLUDES += -I../externals/libdsp/include

# Library Locations
LIBDAISY_DIR = ../externals/libDaisy/
DAISYSP_DIR = ../externals/DaisySP/

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
