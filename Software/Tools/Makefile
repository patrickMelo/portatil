#
# Tools/Makefile
#
# This file is part of Portatil source code.
# Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
#

CURRENT_DIRECTORY	= $(shell pwd)
SOURCE_DIRECTORY	= $(CURRENT_DIRECTORY)/
BINARY_DIRECTORY	= $(CURRENT_DIRECTORY)/../
BINARY_PATH			= $(BINARY_DIRECTORY)/Portatil.Tools
C					= gcc
C_FLAGS				= -std=c23 -Wall -Werror -Wpedantic -g3
INCLUDES			= -I$(SOURCE_DIRECTORY)/
LIBS				= -lm -lpthread -lpng
ARCH				= x64

OBJECTS	= 	$(SOURCE_DIRECTORY)/Debug.o \
			$(SOURCE_DIRECTORY)/FileIO.o \
			$(SOURCE_DIRECTORY)/Tools.o \
			$(SOURCE_DIRECTORY)/Linker/Linker.o \
			$(SOURCE_DIRECTORY)/Linker/Programs.o \
			$(SOURCE_DIRECTORY)/Linker/Programs.ELF.o \
			$(SOURCE_DIRECTORY)/Packer/Assets.o \
			$(SOURCE_DIRECTORY)/Packer/Images.o \
			$(SOURCE_DIRECTORY)/Packer/Images.PNG.o \
			$(SOURCE_DIRECTORY)/Packer/Packer.o
			

# Targets

%.o: %.c
	$(C) $(C_FLAGS) $(INCLUDES) -c $< -o $@

all: $(OBJECTS)
	mkdir -p $(BINARY_DIRECTORY)
	$(C) $(C_FLAGS) $(INCLUDES) $(OBJECTS) $(LIBS) -o $(BINARY_PATH).$(ARCH)

clean:
	rm -fv $(OBJECTS)
