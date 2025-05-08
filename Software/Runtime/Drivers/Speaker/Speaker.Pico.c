//
// Runtime/Drivers/Speaker/Speaker.Pico.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../../Drivers.h"

#include <hardware/clocks.h>
#include <hardware/pwm.h>
#include <pico/stdlib.h>

// Speaker --------------------------------------------------------------------

static u16 bufferIndex = 0;
static u8  soundBuffer[SoundBufferSize];

// PWM ------------------------------------------------------------------------

#define outputPin       6
#define pwmClockDivider 22.2311f    // (CPU Frequency / Sound Frequency) / 255

static struct {
        int slice;
        int channel;
} pwm = {
    .slice   = 0,
    .channel = 0,
};

void pwmWrap(void) {
    pwm_clear_irq(pwm.slice);
    pwm_set_chan_level(pwm.slice, pwm.channel, soundBuffer[bufferIndex++]);

    if (bufferIndex >= SoundBufferSize) {
        bufferIndex = 0;
    }
}

// Driver ---------------------------------------------------------------------

bool DrvSpeakerInitialize(void) {
    gpio_init(outputPin);
    gpio_set_function(outputPin, GPIO_FUNC_PWM);

    pwm.slice   = pwm_gpio_to_slice_num(outputPin);
    pwm.channel = pwm_gpio_to_channel(outputPin);

    pwm_set_phase_correct(pwm.slice, false);
    pwm_set_clkdiv(pwm.slice, pwmClockDivider);

    pwm_set_wrap(pwm.slice, 255);
    pwm_set_enabled(pwm.slice, true);
    pwm_set_chan_level(pwm.slice, pwm.channel, 0);

    for (u16 sampleIndex = 0; sampleIndex < SoundBufferSize; sampleIndex++) {
        soundBuffer[sampleIndex] = 0;
    }

    pwm_clear_irq(pwm.slice);
    pwm_set_irq_enabled(pwm.slice, true);

    irq_set_exclusive_handler(PWM_DEFAULT_IRQ_NUM(), pwmWrap);
    irq_set_enabled(PWM_DEFAULT_IRQ_NUM(), true);

    return true;
}

void DrvSpeakerFinalize(void) {
    gpio_deinit(outputPin);
}

void DrvSpeakerSync(const i8* soundData) {
    for (u16 sampleIndex = 0; sampleIndex < SoundBufferSize; sampleIndex++) {
        soundBuffer[sampleIndex] = soundData[sampleIndex];
    }

    bufferIndex = 0;
}
