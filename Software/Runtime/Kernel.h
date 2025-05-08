//
// Runtime/Kernel.h
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#ifndef PORTATIL_KERNEL_H
#define PORTATIL_KERNEL_H

#ifndef _DEFAULT_SOURCE
    #define _DEFAULT_SOURCE
#endif

#ifndef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 200809L
#endif

#include "Fixed.h"

#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Types ----------------------------------------------------------------------

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef unsigned char byte;
typedef unsigned int  uint;

typedef char* string;

#define FourCC(a, b, c, d) ((u32) (((d) << 24) | ((c) << 16) | ((b) << 8) | (a)))

// General --------------------------------------------------------------------

typedef void (*KernelFunction)(const u64 frameTime);

bool Boot(KernelFunction bootFuction);
void ChangeState(KernelFunction stateFunction);
void Shutdown(void);

u64  GetTick(void);
u64  GetFrameTime(void);
u64  GetBusyFrameTime(void);
void Sleep(const u64 waitTime);

// Power ----------------------------------------------------------------------

u8   GetBatteryPercentageLeft(void);
void DrawBatteryIndicator(void);

// Input ----------------------------------------------------------------------

#define NumberOfButtons 8

typedef enum Button {
    ButtonUp    = 0b00000001,
    ButtonDown  = 0b00000010,
    ButtonLeft  = 0b00000100,
    ButtonRight = 0b00001000,
    ButtonA     = 0b00010000,
    ButtonB     = 0b00100000,
    ButtonX     = 0b01000000,
    ButtonY     = 0b10000000,
} Button;

u8 GetInputState(void);
i8 GetInputAxis(const Button negativeButton, const Button positiveButton);

bool IsButtonPressed(const Button button);
bool IsButtonJustPressed(const Button button);
bool IsButtonJustReleased(const Button button);

// Graphics -------------------------------------------------------------------

#define TargetFPS         30
#define TargetFrameTime   33333
#define TargetFrameTimeMs 33

#define ScreenWidth  160
#define ScreenHeight 120
#define ScreenPixels (ScreenWidth * ScreenHeight)
#define ScreenColors 256

#define ColorNone 0xFFFF

typedef struct Point2D {
        i32 X, Y;
} Point2D;

typedef struct FixedPoint2D {
        f16 X, Y;
} FixedPoint2D;

typedef struct Rectangle2D {
        i32 X, Y, Width, Height;
} Rectangle2D;

typedef struct FixedRectangle2D {
        f16 X, Y, Width, Height;
} FixedRectangle2D;

typedef struct Image {
        u16 Width;
        u16 Height;
        u8* Data;
} Image;

typedef struct BitmapFont {
        Image* Image;
        u8     CharWidth;
        u8     CharHeight;
} BitmapFont;

void SetTransparentColor(const u16 colorIndex);
void SetBackgroundColor(const u16 colorIndex);
void SetForegroundColor(const u16 colorIndex);

u8 GetNearestColorIndex(const u8 redValue, const u8 greenValue, const u8 blueValue);

BitmapFont* GetDefaultFont(void);

void ClearScreen(const u8 colorIndex);
void DrawRectangle(const Rectangle2D* rectangle, const u8 colorIndex);
void DrawImage(const Image* image, const int xPosition, const int yPosition, const Rectangle2D* clipRect);
void DrawText(const BitmapFont* font, const int xPosition, const int yPosition, const string text);
void DrawFormattedText(const BitmapFont* font, const int xPosition, const int yPosition, const string message, ...);

#define AnchorTop     0b00000001
#define AnchorBottom  0b00000010
#define AnchorMiddle  0b00000011
#define AnchorLeft    0b00000100
#define AnchorRight   0b00001000
#define AnchorCenter  0b00001100
#define AnchorDefault (AnchorTop | AnchorLeft)

void SetDrawAnchor(const u8 anchorMask);
void SetDrawScale(const f16 xScale, const f16 yScale);

void ResetDrawState(void);
void SaveDrawState(void);
void RestoreDrawState(void);

// Sound ----------------------------------------------------------------------

#define SoundBufferSize 735    // (Sound Frequency / Target FPS)
#define SoundFrequency  22050
#define SoundBits       8
#define PlayForever     0

typedef enum WaveType {
    SawtoothWave,
    SquareWave,
    TriangleWave,
    NumberOfWaveTypes,
} WaveType;

typedef enum SoundChannel {
    SoundChannel1,
    SoundChannel2,
    SoundChannel3,
    SoundChannel4,
    NumberOfSoundChannels,
} SoundChannel;

void SetChannelVolume(const SoundChannel channelIndex, const u8 volumePercent);
void PlayTone(const SoundChannel channelIndex, const WaveType waveType, const u16 noteFrequency, const u32 durationMs);
void PauseChannel(const SoundChannel channelIndex, const bool doPause);
void PauseAllSound(const bool doPause);
void StopChannel(const SoundChannel channelIndex);
void StopAllSound(void);

// Storage --------------------------------------------------------------------

#define StorageMaxPathLength       4096
#define StorageMaxNameLength       128
#define StorageMaxDirectoryEntries 256

#define StorageEntryDirectoryFlag 0b10000000
#define StorageEntryProgramFlag   0b00000001

#define IsDirectory(flags) (flags & StorageEntryDirectoryFlag)
#define IsProgram(flags)   (flags & StorageEntryProgramFlag)

typedef struct StorageEntryInfo {
        char Name[StorageMaxNameLength + 1];
        u8   Flags;
} StorageEntryInfo;

bool IsStorageAvailable(void);
bool RefreshStorage(void);

bool OpenDirectory(const string directoryPath);
bool GetNextDirectoryEntryInfo(StorageEntryInfo* entryInfo);
void CloseDirectory(void);

bool OpenFile(const string filePath);
u32  GetFileSize(void);
bool ReadFile(void* readBuffer, const u16 readSize);
void CloseFile(void);

#endif    // PORTATIL_KERNEL_H