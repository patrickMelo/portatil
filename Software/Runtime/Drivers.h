//
// Runtime/Drivers.h
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#ifndef PORTATIL_DRIVERS_H
#define PORTATIL_DRIVERS_H

#include "Kernel.h"

// CPU ------------------------------------------------------------------------

typedef void (*CoreFunction)(void);

bool DrvCpuInitialize(void);
void DrvCpuFinalize(void);

void DrvCpuWait(const u64 waitTime);
u64  DrvCpuSync(void);
u64  DrvCpuGetTick(void);

u8   DrvCpuGetAvailableCoreIndex(void);
bool DrvCpuRunCore(const u8 coreIndex, const u8 messageSize, const u32 queueSize, const CoreFunction coreFunction);
void DrvCpuSendMessage(const u8 coreIndex, const void* messageData);
void DrvCpuWaitMessage(const u8 coreIndex, void* messageData);

// Display --------------------------------------------------------------------

bool DrvDisplayInitialize(void);
void DrvDisplayFinalize(void);

void DrvDisplaySync(const u8* framebufferData, const u8* colorPalette);

// GPIO -----------------------------------------------------------------------

typedef enum GpioMode {
    GpioAnalog,
    GpioDigital,
    NumberOfGpioModes,
} GpioMode;

typedef enum GpioDirection {
    GpioInput,
    GpioOutput,
    NumberOfGpioDirections,
} GpioDirection;

bool DrvGpioInitialize(void);
void DrvGpioFinalize(void);

bool DrvGpioConfigure(const u8 pinNumber, const GpioMode mode, const GpioDirection direction);
bool DrvGpioDigitalRead(const u8 pinNumber);
void DrvGpioDigitalWrite(const u8 pinNumber, const bool value);
u16  DrvGpioAnalogRead(const u8 pinNumber);

// GPU ------------------------------------------------------------------------

bool DrvGpuInitialize(void);
void DrvGpuFinalize(void);

void DrvGpuClear(const u8 colorIndex);
void DrvGpuSync(void);

void DrvGpuSetTransparentColor(const u16 colorIndex);
void DrvGpuSetBackgroundColor(const u16 colorIndex);
void DrvGpuSetForegroundColor(const u16 colorIndex);

u8 DrvGpuGetNearestColorIndex(const u8 redValue, const u8 greenValue, const u8 blueValue);

void DrvGpuDraw(const Image* image, const Point2D* position, const Rectangle2D* clipRect);
void DrvGpuDrawScaled(const Image* image, const Rectangle2D* sourceRect, const Rectangle2D* targetRect);
void DrvGpuDrawRectangle(const Rectangle2D* rectangle, const u8 colorIndex);

// Input ----------------------------------------------------------------------

bool DrvInputInitialize(void);
void DrvInputFinalize(void);

u8 DrvInputSync(void);

// Power ----------------------------------------------------------------------

bool DrvPowerInitialize(void);
void DrvPowerFinalize(void);

u8 DrvPowerSync(void);

// Serial ---------------------------------------------------------------------

typedef enum SerialPortNumber {
    SerialPort0,
    SerialPort1,
    NumberOfSerialPorts,
} SerialPortNumber;

bool DrvSerialInitialize(void);
void DrvSerialFinalize(void);

bool DrvSerialConfigure(const SerialPortNumber portNumber, const u32 speedHz, const bool useDMA);
void DrvSerialSetSpeed(const SerialPortNumber portNumber, const u32 speedHz);

bool DrvSerialSelect(const SerialPortNumber portNumber);
void DrvSerialRelease(const SerialPortNumber portNumber);

void DrvSerialWait(const SerialPortNumber portNumber);
bool DrvSerialRead(const SerialPortNumber portNumber, const u32 size, u8* buffer);
bool DrvSerialWrite(const SerialPortNumber portNumber, const u32 size, u8* buffer);

// Speaker --------------------------------------------------------------------

bool DrvSpeakerInitialize(void);
void DrvSpeakerFinalize(void);

void DrvSpeakerSync(const i8* soundData);

// SPU ------------------------------------------------------------------------

bool DrvSpuInitialize(void);
void DrvSpuFinalize(void);

void DrvSpuSync(void);

void DrvSpuSetChannelVolume(const SoundChannel channelIndex, const u8 volumePercent);
void DrvSpuPlayTone(const SoundChannel channelIndex, const WaveType waveType, const u16 noteFrequency, const u32 durationMs);
void DrvSpuPauseChannel(const SoundChannel channelIndex, const bool doPause);
void DrvSpuStopChannel(const SoundChannel channelIndex);

// Storage --------------------------------------------------------------------

bool DrvStorageInitialize(void);
void DrvStorageFinalize(void);

bool DrvStorageOpenDirectory(const string directoryPath);
bool DrvStorageGetNextDirectoryEntryInfo(StorageEntryInfo* entryInfo);
void DrvStorageCloseDirectory(void);

bool DrvStorageOpenFile(const string filePath);
u32  DrvStorageGetFileSize(void);
bool DrvStorageReadFile(void* readBuffer, const u32 readSize);
void DrvStorageCloseFile(void);

#endif    // PORTATIL_DRIVERS_H