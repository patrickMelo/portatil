//
// Runtime/Drivers/Speaker/Speaker.SDL.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../../Drivers.h"

#include <SDL2/SDL.h>

// Speaker --------------------------------------------------------------------

static i8                soundBuffer[SoundBufferSize];
static SDL_AudioDeviceID sdlDevice;
static u64               busyTime = 0;

static void fillSDLBuffer(void* userData, u8* audioStream, int streamLength) {
    for (u16 sampleIndex = 0; sampleIndex < streamLength; sampleIndex++) {
        audioStream[sampleIndex] = soundBuffer[sampleIndex];
    }
}

// Driver ---------------------------------------------------------------------

bool DrvSpeakerInitialize(void) {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        return false;
    }

    static SDL_AudioSpec desiredSpec;

    desiredSpec.callback = &fillSDLBuffer;
    desiredSpec.channels = 1;
    desiredSpec.format   = AUDIO_S8;
    desiredSpec.freq     = SoundFrequency;
    desiredSpec.samples  = SoundBufferSize;
    desiredSpec.userdata = NULL;

    sdlDevice = SDL_OpenAudioDevice(NULL, 0, &desiredSpec, NULL, 0);
    SDL_PauseAudioDevice(sdlDevice, 0);

    busyTime = 0;
    return true;
}

void DrvSpeakerFinalize(void) {
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

u64 DrvSpeakerSync(const i8* soundData) {
    u64 startTime = DrvCpuGetTick();

    for (u16 sampleIndex = 0; sampleIndex < SoundBufferSize; sampleIndex++) {
        soundBuffer[sampleIndex] = soundData[sampleIndex];
    }

    busyTime = DrvCpuGetTick() - startTime;
    return busyTime;
}

u64 DrvSpeakerGetTime(void) {
    return busyTime;
}