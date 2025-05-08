#
# Apps/DemoGame/Makefile
#
# This file is part of Portatil source code.
# Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
#


CURRENT_DIRECTORY	= $(shell pwd)
SOURCE_DIRECTORY	= $(CURRENT_DIRECTORY)
SDK_DIRECTORY		= $(CURRENT_DIRECTORY)/../../SDK
BINARY_DIRECTORY	= $(SOURCE_DIRECTORY)/..
BUILD_DIRECTORY		= $(CURRENT_DIRECTORY)/Build
EXE_NAME 			= DemoGame

TOOL_PREFIX	= riscv64-elf

C		= $(TOOL_PREFIX)-gcc
AS		= $(TOOL_PREFIX)-as
LD		= $(TOOL_PREFIX)-ld
COPY	= $(TOOL_PREFIX)-objcopy
DUMP	= $(TOOL_PREFIX)-objdump
STRIP	= $(TOOL_PREFIX)-strip

C_FLAGS		= 	-nostdinc -nostdlib -ffreestanding \
				-march=rv32im -mabi=ilp32 \
				-fpic -O2 \
				-std=c23 \
				-ffunction-sections \
				-Wall -Werror -Wpedantic \
				-I${SDK_DIRECTORY}
LD_FLAGS	= -nostdlib -no-relax -m elf32lriscv
AS_FLAGS	= -march=rv32im -mabi=ilp32

APP_OBJECTS	=	$(patsubst %.c,%.o,$(wildcard $(SOURCE_DIRECTORY)/*.c))
				
SDK_OBJECTS = 	$(patsubst %.c,%.o,$(wildcard $(SDK_DIRECTORY)/*.c)) \
				$(patsubst %.s,%.ao,$(wildcard $(SDK_DIRECTORY)/*.s))

%.o: %.c
	$(C) $(C_FLAGS) -c $< -o $@

%.ao: %.s
	$(AS) $(AS_FLAGS) $< -o $@

all: $(APP_OBJECTS) $(SDK_OBJECTS)
	mkdir -p $(BUILD_DIRECTORY)
	$(LD) $(LD_FLAGS) -T $(SDK_DIRECTORY)/Portatil.ld $(APP_OBJECTS) $(SDK_OBJECTS) -o $(BUILD_DIRECTORY)/$(EXE_NAME).elf
#	$(DUMP) -d -S $(BUILD_DIRECTORY)/$(EXE_NAME).elf > $(BUILD_DIRECTORY)/$(EXE_NAME).s
#	$(STRIP) -s $(BUILD_DIRECTORY)/$(EXE_NAME).elf -o $(BINARY_DIRECTORY)/$(EXE_NAME).elf

clean:
	rm -fv $(APP_OBJECTS) $(SDK_OBJECTS)
