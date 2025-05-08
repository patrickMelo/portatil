//
// SDK/Portatil.h
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#ifndef PORTATIL_SDK_H
#define PORTATIL_SDK_H

#ifndef _DEFAULT_SOURCE
    #define _DEFAULT_SOURCE
#endif

#ifndef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 200809L
#endif

#include "Fixed.h"

// Types ----------------------------------------------------------------------

typedef unsigned int  uint;
typedef unsigned char byte;

#define bool byte

#define true  1
#define false 0

#define NULL (void*) 0

// Memory ---------------------------------------------------------------------

static inline void Copy(byte* target, byte* source, const uint size) {
    for (uint index = 0; index < size; index++) {
        target[index] = source[index];
    }
}

// General --------------------------------------------------------------------

extern void SysExit(const int exitCode);
extern uint SysSync(void);
extern int  SysRandom(int minValue, int maxValue);
extern uint SysGetFrameTime(void);
extern uint SysGetTickSeconds(void);

static inline void Exit(const int exitCode) {
    SysExit(exitCode);
}

static inline f16 Sync(void) {
    return SysSync();
}

static inline int Random(int minValue, int maxValue) {
    return SysRandom(minValue, maxValue);
}

static inline uint GetFrameTime(void) {
    return SysGetFrameTime();
}

static inline uint GetTickSeconds(void) {
    return SysGetTickSeconds();
}

// Power ----------------------------------------------------------------------

extern uint SysGetBatteryPercentage(void);

static inline uint GetBatteryPercentage(void) {
    return SysGetBatteryPercentage();
}

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

extern uint SysGetInputState(void);
extern int  SysGetInputAxis(const uint negativeButton, const uint positiveButton);
extern uint SysIsButtonPressed(const uint buttonIndex);
extern uint SysIsButtonJustPressed(const uint buttonIndex);
extern uint SysIsButtonJustReleased(const uint buttonIndex);

static inline uint GetInputState(void) {
    return SysGetInputState();
}

static inline int GetInputAxis(const Button negativeButton, const Button positiveButton) {
    return SysGetInputAxis(negativeButton, positiveButton);
}

static inline bool IsButtonPressed(const Button buttonIndex) {
    return SysIsButtonPressed(buttonIndex);
}

static inline bool IsButtonJustPressed(const Button buttonIndex) {
    return SysIsButtonJustPressed(buttonIndex);
}

static inline bool IsButtonJustReleased(const Button buttonIndex) {
    return SysIsButtonJustReleased(buttonIndex);
}

// Graphics -------------------------------------------------------------------

#define TargetFPS         30
#define TargetFrameTime   33333
#define TargetFrameTimeMs 33

#define ScreenWidth  160
#define ScreenHeight 120
#define ScreenPixels (ScreenWidth * ScreenHeight)
#define ScreenColors 256

#define AnchorTop     0b00000001
#define AnchorBottom  0b00000010
#define AnchorMiddle  0b00000011
#define AnchorLeft    0b00000100
#define AnchorRight   0b00001000
#define AnchorCenter  0b00001100
#define AnchorDefault (AnchorTop | AnchorLeft)

#define ColorNone 0xFFFFFFFF

typedef struct Point2D {
        int X, Y;
} Point2D;

typedef struct Rectangle2D {
        int X, Y, Width, Height;
} Rectangle2D;

typedef struct Image {
        uint  Width;
        uint  Height;
        byte* Data;
} Image;

extern void SysClearScreen(const uint colorIndex);
extern uint SysGetColorIndex(const uint redValue, const uint greenValue, const uint blueValue);

extern void SysSetTransparentColor(const uint colorIndex);
extern void SysSetBackgroundColor(const uint colorIndex);
extern void SysSetForegroundColor(const uint colorIndex);

extern void SysSetDrawAnchor(const uint anchorMask);
extern void SysSetDrawScale(const f16 xScale, const f16 yScale);

extern void SysSetTargetPosition(const int xPosition, const int yPosition);
extern void SysSetSourceRectangle(const int xPosition, const int yPosition, const uint sizeWidth, const uint sizeHeight);
extern void SysSetTargetRectangle(const int xPosition, const int yPosition, const uint sizeWidth, const uint sizeHeight);
extern void SysSetTextFont(const uint imageWidth, const uint imageHeight, const void* dataAddress);

extern void SysDrawRectangle(const uint colorIndex);
extern void SysDrawImage(const uint imageWidth, const uint imageHeight, const void* dataAddress);
extern void SysDrawText(const char* text);
extern void SysDrawNumber(const uint number);

static inline void ClearScreen(const uint colorIndex) {
    SysClearScreen(colorIndex);
}

static inline uint GetColorIndex(const uint redValue, const uint greenValue, const uint blueValue) {
    return SysGetColorIndex(redValue, greenValue, blueValue);
}

static inline void SetTransparentColor(const uint colorIndex) {
    SysSetTransparentColor(colorIndex);
}

static inline void SetBackgroundColor(const uint colorIndex) {
    SysSetBackgroundColor(colorIndex);
}

static inline void SetForegroundColor(const uint colorIndex) {
    SysSetForegroundColor(colorIndex);
}

static inline void SetDrawAnchor(const uint anchorMask) {
    SysSetDrawAnchor(anchorMask);
}

static inline void SetDrawScale(const f16 xScale, const f16 yScale) {
    SysSetDrawScale(xScale, yScale);
}

static inline void SetTextFont(const Image* image) {
    if (image) {
        SysSetTextFont(image->Width, image->Height, image->Data);
    } else {
        SysSetTextFont(0, 0, NULL);
    }
}

static inline void DrawRectangle(const int xPosition, const int yPosition, const uint sizeWidth, const uint sizeHeight, const uint colorIndex) {
    SysSetTargetRectangle(xPosition, yPosition, sizeWidth, sizeHeight);
    SysDrawRectangle(colorIndex);
}

static inline void DrawImage(const Image* image, const int xPosition, const int yPosition, const Rectangle2D* clipRect) {
    SysSetSourceRectangle(clipRect->X, clipRect->Y, clipRect->Width, clipRect->Height);
    SysSetTargetPosition(xPosition, yPosition);
    SysDrawImage(image->Width, image->Height, image->Data);
}

static inline void DrawText(const int xPosition, const int yPosition, const char* text) {
    SysSetTargetPosition(xPosition, yPosition);
    SysDrawText(text);
}

static inline void DrawNumber(const int xPosition, const int yPosition, const uint number) {
    SysSetTargetPosition(xPosition, yPosition);
    SysDrawNumber(number);
}

// Sound ----------------------------------------------------------------------

#define SoundFrequency 22050
#define SoundBits      8
#define PlayForever    0

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

extern void SysSetChannelVolume(const uint channelIndex, const uint volumePercent);
extern void SysPlayTone(const uint channelIndex, const uint waveType, const uint noteFrequency, const uint durationMs);
extern void SysStopChannel(const uint channelIndex);
extern void SysStopAllSound(void);

static inline void SetChannelVolume(const SoundChannel channelIndex, const uint volumePercent) {
    SysSetChannelVolume(channelIndex, volumePercent);
}

static inline void PlayTone(const SoundChannel channelIndex, const WaveType waveType, const uint noteFrequency, const uint durationMs) {
    SysPlayTone(channelIndex, waveType, noteFrequency, durationMs);
}

static inline void StopChannel(const SoundChannel channelIndex) {
    SysStopChannel(channelIndex);
}

static inline void StopAllSound(void) {
    SysStopAllSound();
}

// Engine ---------------------------------------------------------------------

#define MaxLayers        4
#define MaxLayerEntities 128
#define MaxSprites       256

extern void SysSyncEngine(void);

extern int  SysGetSprite(const uint imageWidth, const uint imageHeight, const void* dataAddress);
extern void SysReleaseSprite(const uint spriteID);
extern void SysSetSpriteProps(const uint spriteID, const uint transparentColor, const uint frameWidth, const uint frameHeight);
extern void SysSetSpriteFrames(const uint spriteID, const uint numberOfFrames, const uint framesPerSecond);

extern void SysSetActiveLayer(const uint layerIndex);
extern uint SysGetNumberOfEntities(void);

extern int  SysGetEntity(const uint typeID, const uint spriteID, const f16 xPosition, const f16 yPosition);
extern void SysReleaseEntity(const uint entityIndex);

extern void SysSetEntityPosition(const uint entityIndex, const f16 xPosition, const f16 yPosition);
extern void SysSetEntityDirection(const uint entityIndex, const int xDirection, const int yDirection);
extern void SysSetEntitySpeed(const uint entityIndex, const f16 xSpeed, const f16 ySpeed);
extern void SysSetEntityFrameIndex(const uint entityIndex, const f16 frameIndex);
extern void SysSetEntityData(const uint entityIndex, const void* dataAddress);

extern int   SysGetEntityTypeID(const uint entityIndex);
extern f16   SysGetEntityPositionX(const uint entityIndex);
extern f16   SysGetEntityPositionY(const uint entityIndex);
extern int   SysGetEntityDirectionX(const uint entityIndex);
extern int   SysGetEntityDirectionY(const uint entityIndex);
extern f16   SysGetEntitySpeedX(const uint entityIndex);
extern f16   SysGetEntitySpeedY(const uint entityIndex);
extern f16   SysGetEntityFrameIndex(const uint entityIndex);
extern void* SysGetEntityData(const uint entityIndex);
extern int   SysGetCollidingEntityIndex(const uint entityIndex, const uint otherTypeID);

extern int SysFindEntityIndex(const uint typeID, const uint occurrenceNumber);
extern int SysIsEntityOnScreen(const uint entityIndex);

static inline void SyncEngine(void) {
    SysSyncEngine();
}

static inline uint GetSprite(const Image* image, const uint transparentColor, const uint frameWidth, const uint frameHeight) {
    uint spriteID = SysGetSprite(image->Width, image->Height, image->Data);
    SysSetSpriteProps(spriteID, transparentColor, frameWidth, frameHeight);
    return spriteID;
}

static inline void ConfigureSprite(const uint spriteID, const uint numberOfFrames, const uint framesPerSecond) {
    SysSetSpriteFrames(spriteID, numberOfFrames, framesPerSecond);
}

static inline void ReleaseSprite(const uint spriteID) {
    SysReleaseSprite(spriteID);
}

static inline void SetActiveLayer(const uint layerIndex) {
    SysSetActiveLayer(layerIndex);
}

static inline uint GetNumberOfEntities(void) {
    return SysGetNumberOfEntities();
}

static inline uint GetEntity(const uint typeID, const uint spriteID, const f16 xPosition, const f16 yPosition) {
    return SysGetEntity(typeID, spriteID, xPosition, yPosition);
}

static inline void ReleaseEntity(const uint entityIndex) {
    SysReleaseEntity(entityIndex);
}

static inline void SetEntityPosition(const uint entityIndex, const f16 xPosition, const f16 yPosition) {
    SysSetEntityPosition(entityIndex, xPosition, yPosition);
}

static inline void SetEntityDirection(const uint entityIndex, const int xDirection, const int yDirection) {
    SysSetEntityDirection(entityIndex, xDirection, yDirection);
}

static inline void SetEntitySpeed(const uint entityIndex, const f16 xSpeed, const f16 ySpeed) {
    SysSetEntitySpeed(entityIndex, xSpeed, ySpeed);
}

static inline void SetEntityFrameIndex(const uint entityIndex, const f16 frameIndex) {
    SysSetEntityFrameIndex(entityIndex, frameIndex);
}

static inline void SetEntityData(const uint entityIndex, const void* dataAddress) {
    SysSetEntityData(entityIndex, dataAddress);
}

static inline int GetEntityTypeID(const uint entityIndex) {
    return SysGetEntityTypeID(entityIndex);
}

static inline f16 GetEntityPositionX(const uint entityIndex) {
    return SysGetEntityPositionX(entityIndex);
}

static inline f16 GetEntityPositionY(const uint entityIndex) {
    return SysGetEntityPositionY(entityIndex);
}

static inline int GetEntityDirectionX(const uint entityIndex) {
    return SysGetEntityDirectionX(entityIndex);
}

static inline int GetEntityDirectionY(const uint entityIndex) {
    return SysGetEntityDirectionY(entityIndex);
}

static inline f16 GetEntitySpeedX(const uint entityIndex) {
    return SysGetEntitySpeedX(entityIndex);
}

static inline f16 GetEntitySpeedY(const uint entityIndex) {
    return SysGetEntitySpeedY(entityIndex);
}

static inline f16 GetEntityFrameIndex(const uint entityIndex) {
    return SysGetEntityFrameIndex(entityIndex);
}

static inline void* GetEntityData(const uint entityIndex) {
    return SysGetEntityData(entityIndex);
}

static inline int GetCollidingEntityIndex(const uint entityIndex, const uint otherTypeID) {
    return SysGetCollidingEntityIndex(entityIndex, otherTypeID);
}

static inline int FindEntityIndex(const uint typeID, const uint occurrenceNumber) {
    return SysFindEntityIndex(typeID, occurrenceNumber);
}

static inline bool IsEntityOnScreen(const uint entityIndex) {
    return SysIsEntityOnScreen(entityIndex);
}

#endif    // PORTATIL_SDK_H