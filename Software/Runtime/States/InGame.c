//
// Runtime/States/InGame.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../States.h"

static f16         speedMultiplier     = 0;
static u64         currentFrameTime    = 0;
static bool        showStats           = false;
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

    DrawFormattedText(defaultFont, 2, yPos += defaultFont->CharHeight, "FT: %6lld", currentFrameTime);
    DrawFormattedText(defaultFont, 2, yPos += defaultFont->CharHeight, "BFT:%6lld", GetBusyFrameTime());
    DrawFormattedText(defaultFont, 2, yPos += defaultFont->CharHeight, "FPS:%6lld", (u64) (1000000 / currentFrameTime));
    DrawFormattedText(defaultFont, 2, yPos += defaultFont->CharHeight, "SPM: %1.3f", F16ToFloat(speedMultiplier));
    DrawFormattedText(defaultFont, 2, yPos += defaultFont->CharHeight, "ENT:%6d", GetNumberOfEntities(0) + GetNumberOfEntities(1) + GetNumberOfEntities(2) + GetNumberOfEntities(3));

    RestoreDrawState();
}

void InitializeInGame(void) {
    defaultFont = GetDefaultFont();

    backgroundColor = GetNearestColorIndex(220, 0, 0);
    shadowColor     = GetNearestColorIndex(48, 48, 48);

    backgroundRectangle.Width  = (defaultFont->CharWidth * 10) + 2;
    backgroundRectangle.Height = (defaultFont->CharHeight * 5) + 2;
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
