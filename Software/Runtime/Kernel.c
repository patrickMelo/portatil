//
// Runtime/Kernel.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "Kernel.h"

#include "Assets.h"
#include "Drivers.h"

// General --------------------------------------------------------------------

#define powerSyncInterval 10000000    // 10s
#define textBufferSize    2048

#define lowBatteryIndicatorInterval 500000    // 0.5s
#define lowBatteryWarningPercentage 10

static bool           isRunning         = false;
static bool           shutdownRequested = false;
static char           textBuffer[textBufferSize + 1];
static u8             previousInputState         = 0;
static u8             currentInputState          = 0;
static u32            lastFrameTime              = 0;
static u32            lastBusyFrameTime          = 0;
static u8             batteryPercentageLeft      = 0;
static bool           isStorageAvailable         = 0;
static KernelFunction currentState               = NULL;
static u32            lowBatteryIndicatorCounter = 0;
static bool           showLowBatteryIndicator    = false;

static bool initializeDevices(void) {
    bool requiredOk = DrvCpuInitialize() &&
                      DrvGpioInitialize() &&
                      DrvSerialInitialize() &&
                      DrvGpuInitialize() &&
                      DrvDisplayInitialize() &&
                      DrvInputInitialize() &&
                      DrvSpuInitialize() &&
                      DrvSpeakerInitialize() &&
                      DrvPowerInitialize();

    if (requiredOk) {
        isStorageAvailable = DrvStorageInitialize();
    }

    return requiredOk;
}

static void finalizeDevices(void) {
    if (isStorageAvailable) {
        DrvStorageFinalize();
    }

    DrvPowerFinalize();
    DrvSpeakerFinalize();
    DrvSpuFinalize();
    DrvInputFinalize();
    DrvDisplayFinalize();
    DrvGpuFinalize();
    DrvSerialFinalize();
    DrvGpioFinalize();
    DrvCpuFinalize();
}

static void updateLowBatteryIndicator(void) {
    if (batteryPercentageLeft > lowBatteryWarningPercentage) {
        return;
    }

    lowBatteryIndicatorCounter += lastFrameTime;

    if (lowBatteryIndicatorCounter > lowBatteryIndicatorInterval) {
        showLowBatteryIndicator    = !showLowBatteryIndicator;
        lowBatteryIndicatorCounter = 0;
    }

    static Rectangle2D clipRect;

    if (showLowBatteryIndicator) {
        clipRect.Width  = BatteryImage.Width / 3;
        clipRect.Height = BatteryImage.Height;
        clipRect.Y      = 0;
        clipRect.X      = clipRect.Width * 2;

        SetDrawAnchor(AnchorTop | AnchorRight);
        DrawImage(&BatteryImage, ScreenWidth - 1, 1, &clipRect);
        SetDrawAnchor(AnchorDefault);
    }
}

bool Boot(KernelFunction bootFunction) {
    if (isRunning || !bootFunction) {
        return false;
    }

    isRunning = true;

    if (!initializeDevices()) {
        isRunning = false;
        return false;
    }

    currentInputState     = DrvInputSync();
    batteryPercentageLeft = DrvPowerSync();

    u64 syncTick      = 0;
    u64 lastSyncTick  = DrvCpuGetTick();
    u64 lastGPUSync   = 0;
    u64 lastPowerSync = 0;

    shutdownRequested = false;
    currentState      = bootFunction;

    while (!shutdownRequested) {
        DrvStorageResetTime();

        syncTick = DrvCpuSync();

        lastFrameTime      = syncTick - lastSyncTick;
        previousInputState = currentInputState;
        currentInputState  = DrvInputSync();

        currentState(lastFrameTime);
        updateLowBatteryIndicator();

        DrvSpuSync();

        if (syncTick - lastGPUSync >= TargetFrameTime) {
            lastGPUSync = syncTick;
            DrvGpuSync();
        }

        if (syncTick - lastPowerSync >= powerSyncInterval) {
            lastPowerSync         = syncTick;
            batteryPercentageLeft = DrvPowerSync();
        }

        lastBusyFrameTime = DrvCpuGetTick() - syncTick;

        if (lastBusyFrameTime < TargetFrameTime) {
            DrvCpuWait(TargetFrameTime - lastBusyFrameTime);
        }

        lastSyncTick = syncTick;
    }

    finalizeDevices();
    isRunning = false;

    return true;
}

void ChangeState(KernelFunction stateFunction) {
    if (!stateFunction) {
        return;
    }

    currentState = stateFunction;
}

void Shutdown(void) {
    shutdownRequested = true;
}

u64 GetTick(void) {
    return DrvCpuGetTick();
}

u64 GetFrameTime(void) {
    return lastFrameTime;
}

u64 GetBusyFrameTime(void) {
    return lastBusyFrameTime;
}

void Sleep(const u64 waitTime) {
    DrvCpuWait(waitTime);
}

// Timers ---------------------------------------------------------------------

u64 GetGpuTime(void) {
    return DrvGpuGetTime();
}

u64 GetDisplayTime(void) {
    return DrvDisplayGetTime();
}

u64 GetSpuTime(void) {
    return DrvSpuGetTime();
}

u64 GetSpeakerTime(void) {
    return DrvSpeakerGetTime();
}

u64 GetStorageTime(void) {
    return DrvStorageGetTime();
}

// Power ----------------------------------------------------------------------

u8 GetBatteryPercentageLeft(void) {
    return batteryPercentageLeft;
}

void DrawBatteryIndicator(void) {
    static Rectangle2D clipRect;

    if (batteryPercentageLeft <= lowBatteryWarningPercentage) {
        return;
    }

    u32 iconWidth = BatteryImage.Width / 3;

    clipRect.X      = 0;
    clipRect.Y      = 0;
    clipRect.Width  = iconWidth;
    clipRect.Height = BatteryImage.Height;

    SetDrawAnchor(AnchorDefault);
    DrawImage(&BatteryImage, ScreenWidth - iconWidth - 1, 1, &clipRect);

    clipRect.X     = iconWidth;
    clipRect.Width = F16ToInt(F16Div(F16Mult(F16(iconWidth), F16(batteryPercentageLeft)), F16(100)));
    DrawImage(&BatteryImage, ScreenWidth - iconWidth - 1, 1, &clipRect);
}

// Input ----------------------------------------------------------------------

u8 GetInputState(void) {
    return currentInputState;
}

i8 GetInputAxis(const Button negativeButton, const Button positiveButton) {
    return (int) (currentInputState & positiveButton ? 1 : 0) - (int) (currentInputState & negativeButton ? 1 : 0);
}

bool IsButtonPressed(const Button buttonIndex) {
    return (bool) (currentInputState & buttonIndex);
}

bool IsButtonJustPressed(const Button buttonIndex) {
    return (currentInputState & buttonIndex) && !(previousInputState & buttonIndex);
}

bool IsButtonJustReleased(const Button buttonIndex) {
    return !(currentInputState & buttonIndex) && (previousInputState & buttonIndex);
}

// Graphics -------------------------------------------------------------------

typedef struct graphicsState {
        u8           drawAnchor;
        FixedPoint2D drawScale;
        u16          transparentColor;
        u16          backgroundColor;
        u16          foregroundColor;
} graphicsState;

static graphicsState currentGraphicsState = {
    .drawAnchor       = AnchorDefault,
    .drawScale        = {.X = F16One, .Y = F16One},
    .transparentColor = ColorNone,
    .backgroundColor  = ColorNone,
    .foregroundColor  = ColorNone,
};

static graphicsState savedGraphicsState = {
    .drawAnchor       = AnchorDefault,
    .drawScale        = {.X = F16One, .Y = F16One},
    .transparentColor = ColorNone,
    .backgroundColor  = ColorNone,
    .foregroundColor  = ColorNone,
};

static const BitmapFont defaultFont = {
    .Image      = (Image*) &DefaultFontImage,
    .CharWidth  = 6,
    .CharHeight = 8,
};

void SetTransparentColor(const u16 colorIndex) {
    DrvGpuSetTransparentColor(colorIndex);
}

void SetBackgroundColor(const u16 colorIndex) {
    DrvGpuSetBackgroundColor(colorIndex);
}

void SetForegroundColor(const u16 colorIndex) {
    DrvGpuSetForegroundColor(colorIndex);
}

u8 GetNearestColorIndex(const u8 redValue, const u8 greenValue, const u8 blueValue) {
    return DrvGpuGetNearestColorIndex(redValue, greenValue, blueValue);
}

BitmapFont* GetDefaultFont(void) {
    return (BitmapFont*) &defaultFont;
}

void ClearScreen(const u8 colorIndex) {
    DrvGpuClear(colorIndex);
}

void ResetDrawState(void) {
    currentGraphicsState.drawAnchor       = AnchorDefault;
    currentGraphicsState.drawScale.X      = F16One;
    currentGraphicsState.drawScale.Y      = F16One;
    currentGraphicsState.transparentColor = ColorNone;
    currentGraphicsState.backgroundColor  = ColorNone;
    currentGraphicsState.foregroundColor  = ColorNone;

    DrvGpuSetTransparentColor(currentGraphicsState.transparentColor);
    DrvGpuSetBackgroundColor(currentGraphicsState.backgroundColor);
    DrvGpuSetForegroundColor(currentGraphicsState.foregroundColor);
}

void SaveDrawState(void) {
    memcpy(&savedGraphicsState, &currentGraphicsState, sizeof(graphicsState));
}

void RestoreDrawState(void) {
    memcpy(&currentGraphicsState, &savedGraphicsState, sizeof(graphicsState));

    DrvGpuSetTransparentColor(currentGraphicsState.transparentColor);
    DrvGpuSetBackgroundColor(currentGraphicsState.backgroundColor);
    DrvGpuSetForegroundColor(currentGraphicsState.foregroundColor);
}

static inline void anchorPosition(Point2D* position, const Rectangle2D* rect) {
    uint verticalAnchor   = currentGraphicsState.drawAnchor & 0b0011;
    uint horizontalAnchor = currentGraphicsState.drawAnchor & 0b1100;

    switch (verticalAnchor) {
        case AnchorBottom: {
            position->Y -= rect->Height;
            break;
        }

        case AnchorMiddle: {
            position->Y -= rect->Height / 2;
            break;
        }

        default: {
            break;
        }
    }

    switch (horizontalAnchor) {
        case AnchorRight: {
            position->X -= rect->Width;
            break;
        }

        case AnchorCenter: {
            position->X -= rect->Width / 2;
            break;
        }

        default: {
            break;
        }
    }
}

void DrawRectangle(const Rectangle2D* rectangle, const u8 colorIndex) {
    Rectangle2D transformedRectangle = *rectangle;

    if ((currentGraphicsState.drawScale.X != F16One || currentGraphicsState.drawScale.Y != F16One)) {
        transformedRectangle.Width  = F16ToInt(F16Mult(F16(transformedRectangle.Width), currentGraphicsState.drawScale.X));
        transformedRectangle.Height = F16ToInt(F16Mult(F16(transformedRectangle.Height), currentGraphicsState.drawScale.Y));
    }

    if (currentGraphicsState.drawAnchor != AnchorDefault) {
        Point2D anchoredPosition = {.X = transformedRectangle.X, .Y = transformedRectangle.Y};

        anchorPosition(&anchoredPosition, &transformedRectangle);

        transformedRectangle.X = anchoredPosition.X;
        transformedRectangle.Y = anchoredPosition.Y;
    }

    DrvGpuDrawRectangle(&transformedRectangle, colorIndex);
}

void DrawImage(const Image* image, const int xPosition, const int yPosition, const Rectangle2D* clipRect) {
    if (xPosition >= ScreenWidth || yPosition >= ScreenHeight) {
        return;
    }

    if ((currentGraphicsState.drawScale.X == F16One && currentGraphicsState.drawScale.Y == F16One)) {
        Point2D anchoredPosition = {.X = xPosition, .Y = yPosition};

        if (currentGraphicsState.drawAnchor != AnchorDefault) {
            anchorPosition(&anchoredPosition, clipRect);
        }

        DrvGpuDraw(image, (const Point2D*) &anchoredPosition, clipRect);
    } else {
        Rectangle2D transformedRectangle = {
            .X      = xPosition,
            .Y      = yPosition,
            .Width  = clipRect->Width,
            .Height = clipRect->Height,
        };

        transformedRectangle.Width  = F16ToInt(F16Mult(F16(transformedRectangle.Width), currentGraphicsState.drawScale.X));
        transformedRectangle.Height = F16ToInt(F16Mult(F16(transformedRectangle.Height), currentGraphicsState.drawScale.Y));

        if (currentGraphicsState.drawAnchor != AnchorDefault) {
            Point2D anchoredPosition = {.X = transformedRectangle.X, .Y = transformedRectangle.Y};

            anchorPosition(&anchoredPosition, &transformedRectangle);

            transformedRectangle.X = anchoredPosition.X;
            transformedRectangle.Y = anchoredPosition.Y;
        }

        DrvGpuDrawScaled(image, clipRect, &transformedRectangle);
    }
}

void DrawText(const BitmapFont* font, const int xPosition, const int yPosition, const string text) {
    uint    textLength   = strnlen(text, textBufferSize);
    Point2D drawPosition = {.X = xPosition, .Y = yPosition};

    Rectangle2D transformedRectangle = {
        .X      = xPosition,
        .Y      = yPosition,
        .Width  = font->CharWidth * textLength,
        .Height = font->CharHeight,
    };

    bool drawScaled = (currentGraphicsState.drawScale.X != F16One || currentGraphicsState.drawScale.Y != F16One);

    if (drawScaled) {
        transformedRectangle.Width  = F16ToInt(F16Mult(F16(transformedRectangle.Width), currentGraphicsState.drawScale.X));
        transformedRectangle.Height = F16ToInt(F16Mult(F16(transformedRectangle.Height), currentGraphicsState.drawScale.Y));
    }

    if (currentGraphicsState.drawAnchor != AnchorDefault) {
        anchorPosition(&drawPosition, &transformedRectangle);
    }

    if (drawPosition.X + transformedRectangle.Width < 0 ||
        drawPosition.Y + transformedRectangle.Height < 0 ||
        drawPosition.X >= ScreenWidth ||
        drawPosition.Y >= ScreenHeight) {
        return;
    }

    Rectangle2D clipRect = {
        .X      = 0,
        .Y      = 0,
        .Width  = font->CharWidth,
        .Height = font->CharHeight,
    };

    Rectangle2D targetRect = clipRect;

    if (drawScaled) {
        targetRect.Width  = F16ToInt(F16Mult(F16(targetRect.Width), currentGraphicsState.drawScale.X));
        targetRect.Height = F16ToInt(F16Mult(F16(targetRect.Height), currentGraphicsState.drawScale.Y));
    }

    u8 drawAnchorBackup = currentGraphicsState.drawAnchor;

    currentGraphicsState.drawAnchor = AnchorDefault;

    static uint charRow, charColumn;
    static char currentChar;

    uint charsPerLine = font->Image->Width / font->CharWidth;

    for (uint charIndex = 0; charIndex < textLength; charIndex++) {
        if (drawPosition.X + targetRect.Width < 0) {
            drawPosition.X += targetRect.Width;
            continue;
        }

        currentChar = text[charIndex];

        if (currentChar <= 127) {
            charRow    = currentChar / charsPerLine;
            charColumn = currentChar % charsPerLine;

            clipRect.Y = charRow * font->CharHeight;
            clipRect.X = charColumn * font->CharWidth;

            DrawImage(font->Image, drawPosition.X, drawPosition.Y, &clipRect);
        }

        drawPosition.X += targetRect.Width;

        if (drawPosition.X >= ScreenWidth) {
            break;
        }
    }

    currentGraphicsState.drawAnchor = drawAnchorBackup;
}

void DrawFormattedText(const BitmapFont* font, const int xPosition, const int yPosition, const string message, ...) {
    va_list argsList;
    va_start(argsList, message);
    vsnprintf(textBuffer, textBufferSize, message, argsList);
    va_end(argsList);

    DrawText(font, xPosition, yPosition, textBuffer);
}

void SetDrawAnchor(const u8 anchorMask) {
    currentGraphicsState.drawAnchor = anchorMask;
}

void SetDrawScale(const f16 xScale, const f16 yScale) {
    currentGraphicsState.drawScale.X = xScale;
    currentGraphicsState.drawScale.Y = yScale;
}

// Sound ----------------------------------------------------------------------

void SetChannelVolume(const SoundChannel channelIndex, const u8 volumePercent) {
    DrvSpuSetChannelVolume(channelIndex, volumePercent);
}

void PlayTone(const SoundChannel channelIndex, const WaveType waveType, const u16 noteFrequency, const u32 durationMs) {
    DrvSpuPlayTone(channelIndex, waveType, noteFrequency, durationMs);
}

void PauseChannel(const SoundChannel channelIndex, const bool doPause) {
    DrvSpuPauseChannel(channelIndex, doPause);
}

void PauseAllSound(const bool doPause) {
    for (uint channelIndex = 0; channelIndex < NumberOfSoundChannels; channelIndex++) {
        DrvSpuPauseChannel(channelIndex, doPause);
    }
}

void StopChannel(const SoundChannel channelIndex) {
    DrvSpuStopChannel(channelIndex);
}

void StopAllSound(void) {
    for (uint channelIndex = 0; channelIndex < NumberOfSoundChannels; channelIndex++) {
        DrvSpuStopChannel(channelIndex);
    }
}

// Storage --------------------------------------------------------------------

bool IsStorageAvailable(void) {
    return isStorageAvailable;
}

bool RefreshStorage(void) {
    if (isStorageAvailable) {
        DrvStorageFinalize();
    }

    isStorageAvailable = DrvStorageInitialize();
    return isStorageAvailable;
}

bool OpenDirectory(const string directoryPath) {
    return DrvStorageOpenDirectory(directoryPath);
}

bool GetNextDirectoryEntryInfo(StorageEntryInfo* entryInfo) {
    return DrvStorageReadDirectory(entryInfo);
}

void CloseDirectory(void) {
    DrvStorageCloseDirectory();
}

bool OpenFile(const string filePath) {
    return DrvStorageOpenFile(filePath);
}

u32 GetFileSize(void) {
    return DrvStorageGetFileSize();
}

bool ReadFile(void* readBuffer, const u16 readSize) {
    return DrvStorageReadFile(readBuffer, readSize);
}

void CloseFile(void) {
    DrvStorageCloseFile();
}
