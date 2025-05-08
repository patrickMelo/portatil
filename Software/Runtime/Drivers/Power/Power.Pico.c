//
// Runtime/Drivers/Power/Power.Pico.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../../Drivers.h"
#include "../../Kernel.h"

#include <hardware/adc.h>
#include <pico/stdlib.h>

// Power ----------------------------------------------------------------------

#define voltagePin      29
#define numberOfSamples 50
#define minValue        2410
#define maxValue        2470

static u16 lastRead = 0;

// Driver ---------------------------------------------------------------------

bool DrvPowerInitialize(void) {
    return DrvGpioConfigure(voltagePin, GpioAnalog, GpioInput);
}

void DrvPowerFinalize(void) {
    // Empty
}

u8 DrvPowerSync(void) {
    u32 valueAccumulator = 0;

    for (u8 readIndex = 0; readIndex < numberOfSamples; readIndex++) {
        valueAccumulator += DrvGpioAnalogRead(voltagePin);
    }

    u32 averageRead = valueAccumulator / numberOfSamples;

    lastRead = averageRead;

    if (lastRead <= minValue) {
        return 0;
    }

    if (lastRead >= maxValue) {
        return 100;
    }

    return ((lastRead - minValue) * 100) / (maxValue - minValue);
}
