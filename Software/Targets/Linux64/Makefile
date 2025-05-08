#
# Targets/Linux64/Makefile
#
# This file is part of Portatil source code.
# Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
#

CURRENT_DIRECTORY	= $(shell pwd)
SOURCE_DIRECTORY	= $(CURRENT_DIRECTORY)/../../Runtime
BINARY_DIRECTORY	= $(CURRENT_DIRECTORY)/Build
BINARY_PATH			= $(BINARY_DIRECTORY)/PortatilDesktop
C					= gcc
C_FLAGS				= -std=c23 -Wall -Werror -Wpedantic -g3
INCLUDES			= -I$(SOURCE_DIRECTORY)/
LIBS				= -lm -lpthread -lSDL2
ARCH				= x64

OBJECTS	=	$(SOURCE_DIRECTORY)/Assets.o \
			$(SOURCE_DIRECTORY)/Kernel.o \
			$(SOURCE_DIRECTORY)/Engine.o \
			$(SOURCE_DIRECTORY)/Main.o \
			$(SOURCE_DIRECTORY)/States/Error.o \
			$(SOURCE_DIRECTORY)/States/InGame.o \
			$(SOURCE_DIRECTORY)/States/PauseMenu.o \
			$(SOURCE_DIRECTORY)/States/Shell.o \
			$(SOURCE_DIRECTORY)/Drivers/CPU/CPU.Linux.o \
			$(SOURCE_DIRECTORY)/Drivers/Display/Display.SDL.o \
			$(SOURCE_DIRECTORY)/Drivers/GPIO/GPIO.Null.o \
			$(SOURCE_DIRECTORY)/Drivers/GPU/GPU.Generic.o \
			$(SOURCE_DIRECTORY)/Drivers/Input/Input.SDL.o \
			$(SOURCE_DIRECTORY)/Drivers/Power/Power.Linux.o \
			$(SOURCE_DIRECTORY)/Drivers/Serial/Serial.Null.o \
			$(SOURCE_DIRECTORY)/Drivers/Speaker/Speaker.SDL.o \
			$(SOURCE_DIRECTORY)/Drivers/SPU/SPU.Generic.o \
			$(SOURCE_DIRECTORY)/Drivers/Storage/Storage.Linux.o \
			$(SOURCE_DIRECTORY)/VM.o
			
# Targets

%.o: %.c
	$(C) $(C_FLAGS) $(INCLUDES) -c $< -o $@

all: $(OBJECTS)
	mkdir -p $(BINARY_DIRECTORY)
	$(C) $(C_FLAGS) $(INCLUDES) $(OBJECTS) $(LIBS) -o $(BINARY_PATH).$(ARCH)

clean:
	rm -fv $(OBJECTS)
