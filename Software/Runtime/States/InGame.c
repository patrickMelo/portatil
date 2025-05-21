//
// Runtime/States/InGame.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../States.h"

static f16         speedMultiplier     = 0;
static u64         currentFrameTime    = 0;
static bool        showStats           = true;
static BitmapFont* defaultFont         = NULL;
static Rectangle2D backgroundRectangle = {.X = 1, .Y = 1};
static Rectangle2D shadowRectangle     = {.X = 2, .Y = 2};
static u8          backgroundColor     = 0;
static u8          shadowColor         = 0;

static inline void updateGameSpeed(void) {
    speedMultiplier = F16Div(F16(currentFrameTime / 1000), F16(TargetFrameTimeMs));

    if (speedMultiplier == 0) {
        speedMultiplier = F16Div(F16F((float) currentFrameTime / 1000.0f), F16F((float) TargetFrameTime / 1000.0f));
    }

    if (speedMultiplier == 0) {
        speedMultiplier = 1;
    }
}

static inline void drawPerformanceStats() {
    SaveDrawState();
    ResetDrawState();

    i16 yPos = -(defaultFont->CharHeight - 2);

    SetTransparentColor(0);

    DrawRectangle(&shadowRectangle, shadowColor);
    DrawRectangle(&backgroundRectangle, backgroundColor);

    DrawFormattedText(defaultFont, 2, yPos += defaultFont->CharHeight, "BFT:%6lld", GetBusyFrameTime());
    DrawFormattedText(defaultFont, 2, yPos += defaultFont->CharHeight, "FPS:%6lld", (u64) (1000000 / currentFrameTime));

    yPos += defaultFont->CharHeight;

    u64 displayTime = GetDisplayTime();
    u64 gpuTime     = GetGpuTime();
    u64 speakerTime = GetSpeakerTime();
    u64 spuTime     = GetSpuTime();
    u64 storageTime = GetStorageTime();

    DrawFormattedText(defaultFont, 2, yPos += defaultFont->CharHeight, "DSP:%6lld", displayTime);
    DrawFormattedText(defaultFont, 2, yPos += defaultFont->CharHeight, "GPU:%6lld", gpuTime);
    DrawFormattedText(defaultFont, 2, yPos += defaultFont->CharHeight, "SPK:%6lld", speakerTime);
    DrawFormattedText(defaultFont, 2, yPos += defaultFont->CharHeight, "SPU:%6lld", spuTime);
    DrawFormattedText(defaultFont, 2, yPos += defaultFont->CharHeight, "STR:%6lld", storageTime);

    yPos += defaultFont->CharHeight;

    u64 engineTime = GetEngineTime();
    u64 vmTime     = GetVirtualMachineTime();

    DrawFormattedText(defaultFont, 2, yPos += defaultFont->CharHeight, "ENG:%6lld", engineTime);
    DrawFormattedText(defaultFont, 2, yPos += defaultFont->CharHeight, "VM: %6lld", vmTime - engineTime);

    RestoreDrawState();
}

void InitializeInGame(void) {
    defaultFont = GetDefaultFont();

    backgroundColor = GetNearestColorIndex(220, 0, 0);
    shadowColor     = GetNearestColorIndex(48, 48, 48);

    backgroundRectangle.Width  = (defaultFont->CharWidth * 10) + 2;
    backgroundRectangle.Height = (defaultFont->CharHeight * 11) + 2;
    shadowRectangle.Width      = backgroundRectangle.Width;
    shadowRectangle.Height     = backgroundRectangle.Height;
}

void InGameState(const u64 frameTime) {
    currentFrameTime = frameTime;

    updateGameSpeed();

    if (IsButtonPressed(ButtonUp)) {
        if (IsButtonJustPressed(ButtonB) && IsButtonJustPressed(ButtonY)) {
            showStats = !showStats;
        }

        if (IsButtonJustPressed(ButtonA) && IsButtonJustPressed(ButtonX)) {
            PauseAllSound(true);
            ChangeState(PauseMenuState);
            return;
        }
    }

    ResetVirtualMachineTime();

    if (!SyncVirtualMachine(speedMultiplier)) {
        StopAllSound();

        string vmError = GetVirtualMachineError();
        ShowError(ShellState, vmError ? vmError : "unknown vm error");
        return;
    }

    if (showStats) {
        drawPerformanceStats();
    }
}
