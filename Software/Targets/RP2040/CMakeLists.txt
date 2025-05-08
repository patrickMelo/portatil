#
# Targets/RP2040/CMakeLists.txt
#
# This file is part of Portatil source code.
# Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
#

cmake_minimum_required(VERSION 3.10)

include(./SDK/pico_sdk_init.cmake)

project(portatil C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(portatil 
    ../../Runtime/Assets.c
    ../../Runtime/Engine.c
    ../../Runtime/Kernel.c
    ../../Runtime/Main.c
    ../../Runtime/VM.c
    ../../Runtime/States/Error.c
    ../../Runtime/States/InGame.c
    ../../Runtime/States/PauseMenu.c
    ../../Runtime/States/Shell.c
    ../../Runtime/Drivers/CPU/CPU.Pico.c
    ../../Runtime/Drivers/Display/Display.ILI9341.c
    ../../Runtime/Drivers/GPIO/GPIO.Pico.c
    ../../Runtime/Drivers/GPU/GPU.Generic.c
    ../../Runtime/Drivers/Input/Input.Pico.c
    ../../Runtime/Drivers/Power/Power.Pico.c
    ../../Runtime/Drivers/Serial/Serial.Pico.c
    ../../Runtime/Drivers/Speaker/Speaker.Pico.c
    ../../Runtime/Drivers/SPU/SPU.Generic.c
    ../../Runtime/Drivers/Storage/Storage.FAT32.c
    ../../Runtime/Drivers/Storage/Storage.FAT32.SDCard.c
)

pico_enable_stdio_usb(portatil 1)
pico_enable_stdio_uart(portatil 0)

target_link_libraries(portatil hardware_adc hardware_clocks hardware_dma hardware_pwm hardware_spi pico_multicore pico_stdlib pico_bootsel_via_double_reset)
set_property(TARGET portatil APPEND_STRING PROPERTY LINK_FLAGS "-Wl,--print-memory-usage")

pico_add_extra_outputs(portatil)
