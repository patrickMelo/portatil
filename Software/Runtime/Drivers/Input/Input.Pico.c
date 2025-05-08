//
// Runtime/Drivers/Input/Input.Pico.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../../Drivers.h"

#include <pico/stdlib.h>

// Input ----------------------------------------------------------------------

static const u8 buttonPins[NumberOfButtons] = {28, 27, 26, 15, 14, 13, 12, 7};

// Driver ---------------------------------------------------------------------

bool DrvInputInitialize(void) {
    for (u8 buttonIndex = 0; buttonIndex < NumberOfButtons; buttonIndex++) {
        DrvGpioConfigure(buttonPins[buttonIndex], GpioDigital, GpioInput);
    }

    return true;
}

void DrvInputFinalize(void) {
    // Empty
}

u8 DrvInputSync(void) {
    u8 state = 0;

    for (u8 buttonIndex = 0; buttonIndex < NumberOfButtons; buttonIndex++) {
        state |= DrvGpioDigitalRead(buttonPins[buttonIndex]) << buttonIndex;
    }

    return state;
}
