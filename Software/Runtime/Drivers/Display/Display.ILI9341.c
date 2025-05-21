//
// Runtime/Drivers/Display/Display.ILI9341.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../../Drivers.h"

// Serial ---------------------------------------------------------------------

#define serialPortNumber SerialPort0
#define serialSpeed      62500000

#define dcPin    4
#define resetPin 5

static const u8 cmdNop                  = 0x00;
static const u8 cmdSoftwareReset        = 0x01;
static const u8 cmdSleep                = 0x10;
static const u8 cmdWakeUp               = 0x11;
static const u8 cmdDisplayOff           = 0x28;
static const u8 cmdDisplayOn            = 0x29;
static const u8 cmdSetColumnAddress     = 0x2A;
static const u8 cmdSetPageAddress       = 0x2B;
static const u8 cmdWriteMemory          = 0x2C;
static const u8 cmdMemoryAccessControl  = 0x36;
static const u8 cmdSetPixelFormat       = 0x3A;
static const u8 cmdNormalFrameControl   = 0xB1;
static const u8 cmdFunctionControl      = 0xB6;
static const u8 cmdPowerControl1        = 0xC0;
static const u8 cmdPowerControl2        = 0xC1;
static const u8 cmdVCOMControl1         = 0xC5;
static const u8 cmdVCOMControl2         = 0xC7;
static const u8 cmdPowerControlA        = 0xCB;
static const u8 cmdPowerControlB        = 0xCF;
static const u8 cmdDriverTimingControlA = 0xE8;
static const u8 cmdDriverTimingControlB = 0xEA;
static const u8 cmdPowerOnSequence      = 0xED;
static const u8 cmdPumpRatioControl     = 0xF7;

#define sendCommand(command)                             \
    DrvGpioDigitalWrite(dcPin, false);                   \
    DrvSerialWrite(serialPortNumber, 1, (u8*) &command); \
    DrvGpioDigitalWrite(dcPin, true);

#define writeData(data, size) DrvSerialWrite(serialPortNumber, size, (u8*) data)

#define writeData8(data)  writeData(&data, 1)
#define writeData16(data) writeData(&data, 2)

#define executeCommand(command, data, size) \
    sendCommand(command);                   \
    writeData(data, size)

static inline void setAddress(const u16 x0,
                              const u16 y0,
                              const u16 x1,
                              const u16 y1) {
    u16 sx0 = (x0 << 8) | (x0 >> 8);
    u16 sx1 = (x1 << 8) | (x1 >> 8);
    u16 sy0 = (y0 << 8) | (y0 >> 8);
    u16 sy1 = (y1 << 8) | (y1 >> 8);

    sendCommand(cmdSetColumnAddress);
    writeData16(sx0);
    writeData16(sx1);

    sendCommand(cmdSetPageAddress);
    writeData16(sy0);
    writeData16(sy1);
}

static bool initializeDisplay(void) {
    if (!DrvGpioConfigure(resetPin, GpioDigital, GpioOutput)) {
        return false;
    }

    if (!DrvGpioConfigure(dcPin, GpioDigital, GpioOutput)) {
        return false;
    }

    DrvGpioDigitalWrite(resetPin, 0);
    Sleep(50000);
    DrvGpioDigitalWrite(resetPin, 1);
    Sleep(50000);

    sendCommand(cmdDisplayOff);
    sendCommand(cmdSoftwareReset);

    executeCommand(cmdPowerControlB, "\x00\xC1\x30", 3);
    executeCommand(cmdPowerOnSequence, "\x64\x03\x12\x81", 4);
    executeCommand(cmdDriverTimingControlA, "\x85\x00\x78", 3);
    executeCommand(cmdPowerControlA, "\x39\x2C\x00\x34\x02", 5);
    executeCommand(cmdPumpRatioControl, "\x20", 1);
    executeCommand(cmdDriverTimingControlB, "\x00\x00", 2);
    executeCommand(cmdPowerControl1, "\x23", 1);
    executeCommand(cmdPowerControl2, "\x10", 1);
    executeCommand(cmdVCOMControl1, "\x3E\x28", 2);
    executeCommand(cmdVCOMControl2, "\x86", 1);
    executeCommand(cmdFunctionControl, "\x08\x82\x27\x00", 4);
    executeCommand(cmdMemoryAccessControl, "\xE8", 1);       // Rotated 90 Degrees
    executeCommand(cmdSetPixelFormat, "\x55", 1);            // 16-bit Colors
    executeCommand(cmdNormalFrameControl, "\x00\x10", 2);    // 119Hz

    sendCommand(cmdWakeUp);
    Sleep(50000);
    sendCommand(cmdDisplayOn);
    Sleep(50000);

    return true;
}

void finalizeDisplay(void) {
    sendCommand(cmdSleep);
    Sleep(50000);
    sendCommand(cmdDisplayOff);
    Sleep(50000);
}

// Display --------------------------------------------------------------------

#define displayWidth  320
#define displayHeight 240

#define blitRows   12
#define blitWidth  displayWidth
#define blitHeight (displayHeight / blitRows)
#define blitPixels (blitWidth * blitHeight)

static u16 blitBuffer[blitPixels];
static u8  localFrameBuffer[ScreenPixels];
static u16 displayColorPalette[ScreenColors * 3];
static u8  coreIndex;

static u64 lastSyncTick = 0;
static u64 busyTime     = 0;

static inline u16 toDisplayColor(const u8* color) {
    // RRRRR GGGGGG BBBBB
    u16 displayColor = ((color[0] >> 3) << 11) |
                       ((color[1] >> 2) << 5) |
                       ((color[2] >> 3));

    return (displayColor << 8) | (displayColor >> 8);
}

static inline void fillBlitBuffer(const u32 startY) {
    static u32 pixelIndex;
    static u16 pixelColor;
    static u32 lineStart;

    for (u16 pixelY = 0; pixelY < blitHeight / 2; pixelY++) {
        lineStart = ((startY + pixelY) * ScreenWidth);

        for (u16 pixelX = 0; pixelX < ScreenWidth; pixelX++) {
            pixelIndex = lineStart + pixelX;
            pixelColor = displayColorPalette[localFrameBuffer[pixelIndex]];

            blitBuffer[(pixelY * 2 * blitWidth) + (pixelX * 2)]           = pixelColor;
            blitBuffer[(pixelY * 2 * blitWidth) + (pixelX * 2 + 1)]       = pixelColor;
            blitBuffer[((pixelY * 2 + 1) * blitWidth) + (pixelX * 2)]     = pixelColor;
            blitBuffer[((pixelY * 2 + 1) * blitWidth) + (pixelX * 2 + 1)] = pixelColor;
        }
    }
}

void syncCore(void) {
    for (;;) {
        DrvCpuWaitMessage(coreIndex, NULL);

        u64 startTime = DrvCpuGetTick();

        if (startTime - lastSyncTick > TargetFrameTime) {
            continue;
        }

        DrvSerialWait(serialPortNumber);

        setAddress(0, 0, displayWidth - 1, displayHeight - 1);
        sendCommand(cmdWriteMemory);

        u16 pixelY = 0;

        for (u8 rowIndex = 0; rowIndex < blitRows; rowIndex++) {
            DrvSerialWait(serialPortNumber);
            fillBlitBuffer(pixelY);
            pixelY += blitHeight / 2;

            DrvSerialWrite(serialPortNumber, blitPixels * 2, (u8*) blitBuffer);
        }

        busyTime = DrvCpuGetTick() - startTime;
    }
}

// Driver ---------------------------------------------------------------------

bool DrvDisplayInitialize(void) {
    if (!DrvSerialConfigure(serialPortNumber, serialSpeed, true)) {
        return false;
    }

    if (!initializeDisplay()) {
        return false;
    }

    memset(blitBuffer, 0, blitPixels * 2);

    coreIndex = DrvCpuGetAvailableCoreIndex();

    if ((coreIndex == 0) || !DrvCpuRunCore(coreIndex, 1, 10, syncCore)) {
        finalizeDisplay();
        return false;
    }

    busyTime = 0;
    return true;
}

void DrvDisplayFinalize(void) {
    finalizeDisplay();
}

void DrvDisplaySetColorPallete(const u8* colorPalette) {
    for (u32 colorIndex = 0; colorIndex < ScreenColors; colorIndex++) {
        displayColorPalette[colorIndex] = toDisplayColor(&colorPalette[colorIndex * 3]);
    }
}

u64 DrvDisplaySync(const u8* framebufferData) {
    lastSyncTick = DrvCpuGetTick();

    memcpy(&localFrameBuffer, framebufferData, ScreenPixels);

    DrvCpuSendMessage(coreIndex, NULL);

    return busyTime;
}

u64 DrvDisplayGetTime(void) {
    return busyTime;
}
