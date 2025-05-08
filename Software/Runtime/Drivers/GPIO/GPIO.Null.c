//
// Runtime/Drivers/GPIO/GPIO.Null.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../../Drivers.h"

// Driver ---------------------------------------------------------------------

bool DrvGpioInitialize(void) {
    return true;
}

void DrvGpioFinalize(void) {
    // Empty
}

bool DrvGpioConfigure(const u8 pinNumber, const GpioMode mode, const GpioDirection direction) {
    return false;
}

bool DrvGpioDigitalRead(const u8 pinNumber) {
    return false;
}

void DrvGpioDigitalWrite(const u8 pinNumber, const bool value) {
    // Empty
}

u16 DrvGpioAnalogRead(const u8 pinNumber) {
    return 0;
}