//
// Runtime/Drivers/SPU/SPU.Generic.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../../Drivers.h"

// SPU ------------------------------------------------------------------------

#define sampleMin F16(-127)
#define sampleMax F16(127)

static i8  soundBuffer[SoundBufferSize];
static u64 busyTime = 0;

typedef struct ChannelState {
        WaveType waveType;
        f16      noteFrequency;
        i64      timeLeft;
        f16      sampleStep;
        f16      internalSample;
        f16      sampleValue;
        f16      volumeMultiplier;
        bool     isPaused;
        bool     playForever;
        u64      lastSyncTick;
} ChannelState;

static ChannelState channels[NumberOfSoundChannels];

static inline void updateChannel(const SoundChannel channelIndex) {
    switch (channels[channelIndex].waveType) {
        case SawtoothWave: {
            channels[channelIndex].sampleValue = channels[channelIndex].sampleValue + channels[channelIndex].sampleStep;

            if (channels[channelIndex].sampleValue >= sampleMax) {
                channels[channelIndex].sampleValue = sampleMin;
            }

            break;
        }

        case SquareWave: {
            channels[channelIndex].internalSample = channels[channelIndex].internalSample + channels[channelIndex].sampleStep;

            if (channels[channelIndex].internalSample >= sampleMax) {
                channels[channelIndex].internalSample = sampleMin;
                channels[channelIndex].sampleValue *= -1;
            }

            break;
        }

        case TriangleWave: {
            channels[channelIndex].sampleValue = channels[channelIndex].sampleValue + channels[channelIndex].sampleStep;

            if (channels[channelIndex].sampleValue >= sampleMax) {
                channels[channelIndex].sampleValue = sampleMax;
                channels[channelIndex].sampleStep *= -1;
            } else if (channels[channelIndex].sampleValue <= sampleMin) {
                channels[channelIndex].sampleValue = sampleMin;
                channels[channelIndex].sampleStep *= -1;
            }

            break;
        }

        default: {
            break;
        }
    }
}

static void fillBuffer(void) {
    static i64 sampleAccumulator;
    static u8  channelCounter;

    // TODO: figure out how to balance volume between channels.

    for (u16 sampleIndex = 0; sampleIndex < SoundBufferSize; sampleIndex++) {
        sampleAccumulator = 0;
        channelCounter    = 0;

        for (SoundChannel channelIndex = SoundChannel1; channelIndex < NumberOfSoundChannels; ++channelIndex) {
            if (channels[channelIndex].noteFrequency <= 0 || channels[channelIndex].isPaused) {
                continue;
            }

            updateChannel(channelIndex);

            sampleAccumulator += F16ToInt(F16Mult(channels[channelIndex].sampleValue, channels[channelIndex].volumeMultiplier));
            channelCounter++;
        }

        if (channelCounter == 0) {
            soundBuffer[sampleIndex] = 0;
            continue;
        }

        soundBuffer[sampleIndex] = F16ToInt(F16Div(F16(sampleAccumulator), F16(channelCounter)));
    }
}

// Driver ---------------------------------------------------------------------

bool DrvSpuInitialize(void) {
    for (SoundChannel channelIndex = SoundChannel1; channelIndex < NumberOfSoundChannels; ++channelIndex) {
        channels[channelIndex].noteFrequency    = 0;
        channels[channelIndex].timeLeft         = 0;
        channels[channelIndex].volumeMultiplier = F16One;
        channels[channelIndex].isPaused         = false;
    }

    busyTime = 0;
    return true;
}

void DrvSpuFinalize(void) {
    // Empty
}

u64 DrvSpuSync(void) {
    u64 syncTick = DrvCpuGetTick();

    for (SoundChannel channelIndex = SoundChannel1; channelIndex < NumberOfSoundChannels; ++channelIndex) {
        if ((channels[channelIndex].noteFrequency <= 0) || (channels[channelIndex].timeLeft <= 0) || (channels[channelIndex].playForever)) {
            continue;
        }

        if (channels[channelIndex].isPaused) {
            channels[channelIndex].lastSyncTick = syncTick;
            continue;
        }

        channels[channelIndex].timeLeft -= (syncTick - channels[channelIndex].lastSyncTick);
        channels[channelIndex].lastSyncTick = syncTick;

        if (channels[channelIndex].timeLeft <= 0) {
            channels[channelIndex].noteFrequency = 0;
        }
    }

    fillBuffer();

    busyTime = DrvCpuGetTick() - syncTick;

    DrvSpeakerSync(soundBuffer);

    return busyTime;
}

u64 DrvSpuGetTime(void) {
    return busyTime;
}

void DrvSpuSetChannelVolume(const SoundChannel channelIndex, const u8 volumePercent) {
    if (channelIndex >= NumberOfSoundChannels) {
        return;
    }

    channels[channelIndex].volumeMultiplier = F16Div(F16(volumePercent), F16(100));

    if (channels[channelIndex].volumeMultiplier > F16One) {
        channels[channelIndex].volumeMultiplier = F16One;
    }
}

void DrvSpuPlayTone(const SoundChannel channelIndex, const WaveType waveType, const u16 noteFrequency, const u32 durationMs) {
    if ((channelIndex >= NumberOfSoundChannels) || (waveType >= NumberOfWaveTypes)) {
        return;
    }

    channels[channelIndex].isPaused     = false;
    channels[channelIndex].lastSyncTick = DrvCpuGetTick();
    channels[channelIndex].timeLeft     = durationMs * 1000;
    channels[channelIndex].playForever  = durationMs == PlayForever;

    channels[channelIndex].waveType      = waveType;
    channels[channelIndex].noteFrequency = F16(noteFrequency);

    f16 waveSamples = F16Div(F16(SoundFrequency), channels[channelIndex].noteFrequency);

    channels[channelIndex].sampleValue    = sampleMin;
    channels[channelIndex].internalSample = sampleMin;
    channels[channelIndex].sampleStep     = F16Div(F16(waveType == SawtoothWave ? 256 : 512), waveSamples);
}

void DrvSpuPauseChannel(const SoundChannel channelIndex, const bool doPause) {
    if (channelIndex >= NumberOfSoundChannels) {
        return;
    }

    channels[channelIndex].isPaused = doPause;
}

void DrvSpuStopChannel(const SoundChannel channelIndex) {
    if (channelIndex >= NumberOfSoundChannels) {
        return;
    }

    channels[channelIndex].timeLeft      = 0;
    channels[channelIndex].noteFrequency = 0;
}