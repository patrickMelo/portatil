//
// Runtime/Drivers/GPIO/GPIO.Pico.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../../Drivers.h"

#include <hardware/adc.h>
#include <pico/stdlib.h>

// Driver ---------------------------------------------------------------------

bool DrvGpioInitialize(void) {
    adc_init();
    adc_set_temp_sensor_enabled(false);
    return true;
}

void DrvGpioFinalize(void) {
    // Empty
}

bool DrvGpioConfigure(const u8 pinNumber, const GpioMode mode, const GpioDirection direction) {
    switch (mode) {
        case GpioAnalog: {
            if ((pinNumber < 26) || (pinNumber > 29)) {
                return false;
            }

            adc_gpio_init(pinNumber);
            break;
        }

        case GpioDigital: {
            if (pinNumber > 29) {
                return false;
            }

            gpio_init(pinNumber);

            switch (direction) {
                case GpioInput: {
                    gpio_set_dir(pinNumber, GPIO_IN);
                    gpio_pull_down(pinNumber);
                    break;
                }

                case GpioOutput: {
                    gpio_set_dir(pinNumber, GPIO_OUT);
                    gpio_put(pinNumber, 0);
                    break;
                }

                default: {
                    return false;
                }
            }

            break;
        }

        default: {
            return false;
        }
    }

    return true;
}

bool DrvGpioDigitalRead(const u8 pinNumber) {
    if (pinNumber > 29) {
        return false;
    }

    return gpio_get(pinNumber);
}

void DrvGpioDigitalWrite(const u8 pinNumber, const bool value) {
    if (pinNumber > 29) {
        return;
    }

    gpio_put(pinNumber, value);
}

u16 DrvGpioAnalogRead(const u8 pinNumber) {
    if ((pinNumber < 26) || (pinNumber > 29)) {
        return 0;
    }

    adc_select_input(pinNumber - 26);
    return adc_read();
}