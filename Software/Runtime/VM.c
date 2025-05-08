//
// Runtime/VM.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "VM.h"

#include "Engine.h"

// General ------------------------------------------------------------

#define maxSyncTime            1000000
#define errorMessageBufferSize 100

static u8  memoryBlock[VirtualMachineMemorySize];
static u32 programMemoryOffset    = 0;
static u32 currentProgramSize     = 0;
static u32 currentInstruction     = 0;
static u32 programCounter         = 0;
static u32 programCounterSnapshot = 0;

static i32 rd, rs1, rs2, f3, f7, imm;
static i32 registers[32];

static bool syncRequested                            = false;
static f16  currentSpeedMultiplier                   = 0;
static char errorMessage[errorMessageBufferSize + 1] = {0};

#define ZERO 0
#define RA   1
#define SP   2
#define GP   3
#define TP   4
#define T0   5
#define T1   6
#define T2   7
#define S0   8
#define S1   9
#define A0   10
#define A1   11
#define A2   12
#define A3   13
#define A4   14
#define A5   15
#define A6   16
#define A7   17
#define S2   18
#define S3   19
#define S4   20
#define S5   21
#define S6   22
#define S7   23
#define S8   24
#define S9   25
#define S10  26
#define S11  27
#define T3   28
#define T4   29
#define T5   30
#define T6   31

#define assertAddress(memoryAddress, wordSize)                                                  \
    if (memoryAddress % wordSize != 0 || memoryAddress > VirtualMachineMemorySize - wordSize) { \
        return false;                                                                           \
    }

#define offsetAddress(memoryAddress)                                               \
    if (memoryAddress < 0) {                                                       \
        memoryAddress += VirtualMachineMemorySize;                                 \
    } else if (programMemoryOffset != 0 && memoryAddress >= programMemoryOffset) { \
        memoryAddress -= programMemoryOffset;                                      \
    }

static inline i32 getX(const i32 registerIndex) {
    return registerIndex < 32 ? registers[registerIndex] : 0;
}

static inline void setX(const i32 registerIndex, const i32 newValue) {
    if (registerIndex > 0 && registerIndex < 32) {
        registers[registerIndex] = newValue;
    }
}

// SysCalls -------------------------------------------------------------------

#define maxSysCalls   256
#define maxTextLength 128

typedef bool (*instructionFunction)(void);

static instructionFunction sysCallTable[maxSysCalls];
static BitmapFont*         defaultFont      = NULL;
static Point2D             targetPosition   = {.X = 0, .Y = 0};
static Rectangle2D         sourceRetangle   = {.X = 0, .Y = 0, .Width = 0, .Height = 0};
static Rectangle2D         targetRetangle   = {.X = 0, .Y = 0, .Width = 0, .Height = 0};
static u32                 activeLayerIndex = 0;
static Image               customFontImage  = {.Width = 0, .Height = 0, .Data = NULL};
static BitmapFont          customFont       = {.Image = NULL, .CharWidth = 0, .CharHeight = 0};

#define sysCallExit           1
#define sysCallSync           2
#define sysCallRandom         3
#define sysCallGetFrameTime   4
#define sysCallGetTickSeconds 5

#define sysCallGetBatteryPercentage 10

#define sysCallGetInputState        20
#define sysCallGetInputAxis         21
#define sysCallIsButtonPressed      22
#define sysCallIsButtonJustPressed  23
#define sysCallIsButtonJustReleased 24

#define sysCallClearScreen         30
#define sysCallGetColorIndex       31
#define sysCallSetTransparentColor 32
#define sysCallSetBackgroundColor  33
#define sysCallSetForegroundColor  34
#define sysCallSetDrawAnchor       35
#define sysCallSetDrawScale        36
#define sysCallSetTargetPosition   37
#define sysCallSetSourceRectangle  38
#define sysCallSetTargetRectangle  39
#define sysCallSetTextFont         40
#define sysCallDrawRectangle       41
#define sysCallDrawImage           42
#define sysCallDrawText            43
#define sysCallDrawNumber          44

#define sysCallSetChannelVolume 50
#define sysCallPlayTone         51
#define sysCallStopChannel      52
#define sysCallStopAllSound     53

#define sysCallSyncEngine              60
#define sysCallGetSprite               61
#define sysCallReleaseSprite           62
#define sysCallSetSpriteProps          63
#define sysCallSetSpriteFrames         64
#define sysCallSetActiveLayer          65
#define sysCallGetNumberOfEntities     66
#define sysCallGetEntity               67
#define sysCallReleaseEntity           68
#define sysCallSetEntityPosition       69
#define sysCallSetEntityDirection      70
#define sysCallSetEntitySpeed          71
#define sysCallSetEntityFrameIndex     72
#define sysCallSetEntityData           73
#define sysCallGetEntityTypeID         74
#define sysCallGetEntityPositionX      75
#define sysCallGetEntityPositionY      76
#define sysCallGetEntityDirectionX     77
#define sysCallGetEntityDirectionY     78
#define sysCallGetEntitySpeedX         79
#define sysCallGetEntitySpeedY         80
#define sysCallGetEntityFrameIndex     81
#define sysCallGetEntityData           82
#define sysCallGetCollidingEntityIndex 83
#define sysCallFindEntityIndex         84
#define sysCallIsEntityOnScreen        85

static bool sysInvalid(void) {
    return false;
}

static bool sysExit(void) {
    return false;
}

static bool sysSync(void) {
    setX(A0, currentSpeedMultiplier);
    syncRequested = true;
    return false;
}

static bool sysRandom(void) {
    i32 minValue = getX(A0);
    i32 maxValue = getX(A1);

    if (maxValue - minValue > 0) {
        i32 randomValue = minValue + (rand() % (maxValue + 1 - minValue));
        setX(A0, randomValue);
    } else {
        setX(A0, minValue);
    }

    return true;
}

static bool sysGetFrameTime(void) {
    setX(A0, GetFrameTime());
    return true;
}

static bool sysGetTickSeconds(void) {
    setX(A0, GetTick() / 1000000);
    return true;
}

static bool sysGetBatteryPercentage(void) {
    setX(A0, GetBatteryPercentageLeft());
    return true;
}

static bool sysGetInputState(void) {
    setX(A0, GetInputState());
    return true;
}

static bool sysGetInputAxis(void) {
    setX(A0, GetInputAxis(getX(A0), getX(A1)));
    return true;
}

static bool sysIsButtonPressed(void) {
    setX(A0, IsButtonPressed(getX(A0)));
    return true;
}

static bool sysIsButtonJustPressed(void) {
    setX(A0, IsButtonJustPressed(getX(A0)));
    return true;
}

static bool sysIsButtonJustReleased(void) {
    setX(A0, IsButtonJustReleased(getX(A0)));
    return true;
}

static bool sysClearScreen(void) {
    ClearScreen(getX(A0));
    return true;
}

static bool sysGetColorIndex(void) {
    setX(A0, GetNearestColorIndex(getX(A0), getX(A1), getX(A2)));

    return true;
}

static bool sysSetTransparentColor(void) {
    SetTransparentColor(getX(A0));
    return true;
}

static bool sysSetBackgroundColor(void) {
    SetBackgroundColor(getX(A0));
    return true;
}

static bool sysSetForegroundColor(void) {
    SetForegroundColor(getX(A0));
    return true;
}

static bool sysSetDrawAnchor(void) {
    SetDrawAnchor(getX(A0));
    return true;
}

static bool sysSetDrawScale(void) {
    SetDrawScale(getX(A0), getX(A1));
    return true;
}

static bool sysSetTargetPosition(void) {
    targetPosition.X = getX(A0);
    targetPosition.Y = getX(A1);
    return true;
}

static bool sysSetSourceRectangle(void) {
    sourceRetangle.X      = getX(A0);
    sourceRetangle.Y      = getX(A1);
    sourceRetangle.Width  = getX(A2);
    sourceRetangle.Height = getX(A3);
    return true;
}

static bool sysSetTargetRectangle(void) {
    targetRetangle.X      = getX(A0);
    targetRetangle.Y      = getX(A1);
    targetRetangle.Width  = getX(A2);
    targetRetangle.Height = getX(A3);
    return true;
}

static bool sysSetTextFont(void) {
    customFontImage.Width  = getX(A0);
    customFontImage.Height = getX(A1);

    u32 dataAddress = getX(A2);

    if (!dataAddress || (customFontImage.Width < 16) || (customFontImage.Height < 16)) {
        customFont.Image      = NULL;
        customFont.CharWidth  = 0;
        customFont.CharHeight = 0;
        return true;
    }

    offsetAddress(dataAddress);
    assertAddress(dataAddress, 1);

    customFontImage.Data = (u8*) (intptr_t) (memoryBlock + (intptr_t) dataAddress);

    customFont.CharWidth  = customFontImage.Width / 16;
    customFont.CharHeight = customFontImage.Height / 8;
    customFont.Image      = &customFontImage;

    return true;
}

static bool sysDrawRectangle(void) {
    DrawRectangle(&targetRetangle, getX(A0));
    return true;
}

static bool sysDrawImage(void) {
    static Image image;

    image.Width  = getX(A0);
    image.Height = getX(A1);

    u32 dataAddress = getX(A2);

    offsetAddress(dataAddress);
    assertAddress(dataAddress, 1);

    image.Data = (u8*) (intptr_t) (memoryBlock + (intptr_t) dataAddress);

    DrawImage(&image, targetPosition.X, targetPosition.Y, &sourceRetangle);
    return true;
}

static bool sysDrawText(void) {
    u32 textAddress = getX(A0);

    offsetAddress(textAddress);
    assertAddress(textAddress, 1);

    char* textData = (char*) (intptr_t) (memoryBlock + (intptr_t) textAddress);

    if (strnlen(textData, maxTextLength) > maxTextLength) {
        return false;
    }

    DrawText(customFont.Image ? &customFont : defaultFont, targetPosition.X, targetPosition.Y, textData);
    return true;
}

static bool sysDrawNumber(void) {
    DrawFormattedText(customFont.Image ? &customFont : defaultFont, targetPosition.X, targetPosition.Y, "%d", getX(A0));
    return true;
}

static bool sysSetChannelVolume(void) {
    SetChannelVolume(getX(A0), getX(A1));
    return true;
}

static bool sysPlayTone(void) {
    PlayTone(getX(A0), getX(A1), getX(A2), getX(A3));
    return true;
}

static bool sysStopChannel(void) {
    StopChannel(getX(A0));
    return true;
}

static bool sysStopAllSound(void) {
    StopAllSound();
    return true;
}

static bool sysSyncEngine(void) {
    SyncEngine(currentSpeedMultiplier);
    return true;
}

static bool sysGetSprite(void) {
    Image image;
    image.Width  = getX(A0);
    image.Height = getX(A1);

    u32 dataAddress = getX(A2);

    offsetAddress(dataAddress);
    assertAddress(dataAddress, 1);

    image.Data = (u8*) (intptr_t) (memoryBlock + (intptr_t) dataAddress);

    Sprite* sprite = GetSprite(&image);
    setX(A0, sprite ? sprite->Index : -1);
    return true;
}

static bool sysReleaseSprite(void) {
    ReleaseSprite(GetSpriteByIndex(getX(A0)));
    return true;
}

static bool sysSetSpriteProps(void) {
    Sprite* sprite = GetSpriteByIndex(getX(A0));

    if (sprite) {
        sprite->TransparentColor = getX(A1);
        sprite->FrameWidth       = getX(A2);
        sprite->FrameHeight      = getX(A3);
    }

    return true;
}

static bool sysSetSpriteFrames(void) {
    Sprite* sprite = GetSpriteByIndex(getX(A0));

    if (sprite) {
        sprite->NumberOfFrames = getX(A1);
        sprite->FrameSpeed     = F16Div(F16(getX(A2)), F16(TargetFPS));
    }

    return true;
}

static bool sysSetActiveLayer(void) {
    uint layerIndex = getX(A0);

    if (layerIndex < MaxLayers) {
        activeLayerIndex = layerIndex;
    }

    return true;
}

static bool sysGetNumberOfEntities(void) {
    setX(A0, GetNumberOfEntities(activeLayerIndex));
    return true;
}

static bool sysGetEntity(void) {
    Sprite* sprite = GetSpriteByIndex(getX(A1));

    if (!sprite) {
        setX(A0, 0);
        return true;
    }

    Entity* entity = GetEntity(activeLayerIndex, getX(A0), sprite, getX(A2), getX(A3));
    setX(A0, entity ? entity->Index : -1);

    return true;
}

static bool sysReleaseEntity(void) {
    ReleaseEntity(GetEntityByIndex(activeLayerIndex, getX(A0)));
    return true;
}

static bool sysSetEntityPosition(void) {
    Entity* entity = GetEntityByIndex(activeLayerIndex, getX(A0));

    if (entity) {
        entity->Position.X = getX(A1);
        entity->Position.Y = getX(A2);
    }

    return true;
}

static bool sysSetEntityDirection(void) {
    Entity* entity = GetEntityByIndex(activeLayerIndex, getX(A0));

    if (entity) {
        entity->Direction.X = getX(A1);
        entity->Direction.Y = getX(A2);
    }

    return true;
}

static bool sysSetEntitySpeed(void) {
    Entity* entity = GetEntityByIndex(activeLayerIndex, getX(A0));

    if (entity) {
        entity->Speed.X = getX(A1);
        entity->Speed.Y = getX(A2);
    }

    return true;
}

static bool sysSetEntityFrameIndex(void) {
    Entity* entity = GetEntityByIndex(activeLayerIndex, getX(A0));

    if (entity) {
        entity->FrameIndex = getX(A1);
    }

    return true;
}

static bool sysSetEntityData(void) {
    Entity* entity = GetEntityByIndex(activeLayerIndex, getX(A0));

    if (entity) {
        entity->DataAddress = getX(A1);
    }

    return true;
}

static bool sysGetEntityTypeID(void) {
    Entity* entity = GetEntityByIndex(activeLayerIndex, getX(A0));
    setX(A0, entity ? entity->TypeID : -1);
    return true;
}

static bool sysGetEntityPositionX(void) {
    Entity* entity = GetEntityByIndex(activeLayerIndex, getX(A0));
    setX(A0, entity ? entity->Position.X : 0);
    return true;
}

static bool sysGetEntityPositionY(void) {
    Entity* entity = GetEntityByIndex(activeLayerIndex, getX(A0));
    setX(A0, entity ? entity->Position.Y : 0);
    return true;
}

static bool sysGetEntityDirectionX(void) {
    Entity* entity = GetEntityByIndex(activeLayerIndex, getX(A0));
    setX(A0, entity ? entity->Direction.X : 0);
    return true;
}

static bool sysGetEntityDirectionY(void) {
    Entity* entity = GetEntityByIndex(activeLayerIndex, getX(A0));
    setX(A0, entity ? entity->Direction.Y : 0);
    return true;
}

static bool sysGetEntitySpeedX(void) {
    Entity* entity = GetEntityByIndex(activeLayerIndex, getX(A0));
    setX(A0, entity ? entity->Speed.X : 0);
    return true;
}

static bool sysGetEntitySpeedY(void) {
    Entity* entity = GetEntityByIndex(activeLayerIndex, getX(A0));
    setX(A0, entity ? entity->Speed.Y : 0);
    return true;
}

static bool sysGetEntityFrameIndex(void) {
    Entity* entity = GetEntityByIndex(activeLayerIndex, getX(A0));
    setX(A0, entity ? entity->FrameIndex : F16(-1));
    return true;
}

static bool sysGetEntityData(void) {
    Entity* entity = GetEntityByIndex(activeLayerIndex, getX(A0));
    setX(A0, entity ? entity->DataAddress : 0);
    return true;
}

static bool sysGetCollidingEntityIndex(void) {
    Entity* entity = GetEntityByIndex(activeLayerIndex, getX(A0));

    if (entity) {
        Entity* otherEntity = GetCollidingEntity(entity, getX(A1));
        setX(A0, otherEntity ? otherEntity->Index : -1);
    } else {
        setX(A0, -1);
    }

    return true;
}

static bool sysFindEntityIndex(void) {
    setX(A0, FindEntityIndex(activeLayerIndex, getX(A0), getX(A1)));
    return true;
}

static bool sysIsEntityOnScreen(void) {
    setX(A0, IsEntityOnScreen(GetEntityByIndex(activeLayerIndex, getX(A0))));
    return true;
}

void initializeSysCalls(void) {
    defaultFont = GetDefaultFont();

    for (int sysCall = 0; sysCall < maxSysCalls; sysCall++) {
        sysCallTable[sysCall] = sysInvalid;
    }

    sysCallTable[sysCallExit]           = sysExit;
    sysCallTable[sysCallSync]           = sysSync;
    sysCallTable[sysCallRandom]         = sysRandom;
    sysCallTable[sysCallGetFrameTime]   = sysGetFrameTime;
    sysCallTable[sysCallGetTickSeconds] = sysGetTickSeconds;

    sysCallTable[sysCallGetBatteryPercentage] = sysGetBatteryPercentage;

    sysCallTable[sysCallGetInputState]        = sysGetInputState;
    sysCallTable[sysCallGetInputAxis]         = sysGetInputAxis;
    sysCallTable[sysCallIsButtonPressed]      = sysIsButtonPressed;
    sysCallTable[sysCallIsButtonJustPressed]  = sysIsButtonJustPressed;
    sysCallTable[sysCallIsButtonJustReleased] = sysIsButtonJustReleased;

    sysCallTable[sysCallClearScreen]         = sysClearScreen;
    sysCallTable[sysCallGetColorIndex]       = sysGetColorIndex;
    sysCallTable[sysCallSetTransparentColor] = sysSetTransparentColor;
    sysCallTable[sysCallSetBackgroundColor]  = sysSetBackgroundColor;
    sysCallTable[sysCallSetForegroundColor]  = sysSetForegroundColor;
    sysCallTable[sysCallSetDrawAnchor]       = sysSetDrawAnchor;
    sysCallTable[sysCallSetDrawScale]        = sysSetDrawScale;
    sysCallTable[sysCallSetTargetPosition]   = sysSetTargetPosition;
    sysCallTable[sysCallSetSourceRectangle]  = sysSetSourceRectangle;
    sysCallTable[sysCallSetTargetRectangle]  = sysSetTargetRectangle;
    sysCallTable[sysCallSetTextFont]         = sysSetTextFont;
    sysCallTable[sysCallDrawRectangle]       = sysDrawRectangle;
    sysCallTable[sysCallDrawImage]           = sysDrawImage;
    sysCallTable[sysCallDrawText]            = sysDrawText;
    sysCallTable[sysCallDrawNumber]          = sysDrawNumber;

    sysCallTable[sysCallSetChannelVolume] = sysSetChannelVolume;
    sysCallTable[sysCallPlayTone]         = sysPlayTone;
    sysCallTable[sysCallStopChannel]      = sysStopChannel;
    sysCallTable[sysCallStopAllSound]     = sysStopAllSound;

    sysCallTable[sysCallSyncEngine]              = sysSyncEngine;
    sysCallTable[sysCallGetSprite]               = sysGetSprite;
    sysCallTable[sysCallReleaseSprite]           = sysReleaseSprite;
    sysCallTable[sysCallSetSpriteProps]          = sysSetSpriteProps;
    sysCallTable[sysCallSetSpriteFrames]         = sysSetSpriteFrames;
    sysCallTable[sysCallSetActiveLayer]          = sysSetActiveLayer;
    sysCallTable[sysCallGetNumberOfEntities]     = sysGetNumberOfEntities;
    sysCallTable[sysCallGetEntity]               = sysGetEntity;
    sysCallTable[sysCallReleaseEntity]           = sysReleaseEntity;
    sysCallTable[sysCallSetEntityPosition]       = sysSetEntityPosition;
    sysCallTable[sysCallSetEntityDirection]      = sysSetEntityDirection;
    sysCallTable[sysCallSetEntitySpeed]          = sysSetEntitySpeed;
    sysCallTable[sysCallSetEntityFrameIndex]     = sysSetEntityFrameIndex;
    sysCallTable[sysCallSetEntityData]           = sysSetEntityData;
    sysCallTable[sysCallGetEntityTypeID]         = sysGetEntityTypeID;
    sysCallTable[sysCallGetEntityPositionX]      = sysGetEntityPositionX;
    sysCallTable[sysCallGetEntityPositionY]      = sysGetEntityPositionY;
    sysCallTable[sysCallGetEntityDirectionX]     = sysGetEntityDirectionX;
    sysCallTable[sysCallGetEntityDirectionY]     = sysGetEntityDirectionY;
    sysCallTable[sysCallGetEntitySpeedX]         = sysGetEntitySpeedX;
    sysCallTable[sysCallGetEntitySpeedY]         = sysGetEntitySpeedY;
    sysCallTable[sysCallGetEntityFrameIndex]     = sysGetEntityFrameIndex;
    sysCallTable[sysCallGetEntityData]           = sysGetEntityData;
    sysCallTable[sysCallGetCollidingEntityIndex] = sysGetCollidingEntityIndex;
    sysCallTable[sysCallFindEntityIndex]         = sysFindEntityIndex;
    sysCallTable[sysCallIsEntityOnScreen]        = sysIsEntityOnScreen;
}

static bool doSysCall(void) {
    u32 sysCall = getX(A7);

    if (sysCall >= maxSysCalls) {
        return false;
    }

    return sysCallTable[sysCall]();
}

// Instructions ---------------------------------------------------------------

#define decode1(start)  ((currentInstruction >> start) & 0b1)
#define decode3(start)  ((currentInstruction >> start) & 0b111)
#define decode4(start)  ((currentInstruction >> start) & 0b1111)
#define decode5(start)  ((currentInstruction >> start) & 0b11111)
#define decode6(start)  ((currentInstruction >> start) & 0b111111)
#define decode7(start)  ((currentInstruction >> start) & 0b1111111)
#define decode8(start)  ((currentInstruction >> start) & 0b11111111)
#define decode10(start) ((currentInstruction >> start) & 0b1111111111)
#define decode12(start) ((currentInstruction >> start) & 0b111111111111)
#define decode20(start) ((currentInstruction >> start) & 0b11111111111111111111)

static inline void decodeR(void) {
    rd  = decode5(7);
    f3  = decode3(12);
    rs1 = decode5(15);
    rs2 = decode5(20);
    f7  = decode7(25);
}

static inline void decodeI(void) {
    rd  = decode5(7);
    f3  = decode3(12);
    rs1 = decode5(15);
    rs2 = decode5(20);
    f7  = decode7(25);
    imm = decode12(20);
}

static inline void decodeS(void) {
    f3  = decode3(12);
    rs1 = decode5(15);
    rs2 = decode5(20);
    imm = decode5(7) | (decode7(25) << 5);
}

static inline void decodeB(void) {
    f3  = decode3(12);
    rs1 = decode5(15);
    rs2 = decode5(20);
    imm = (decode1(7) << 11) | (decode4(8) << 1) | (decode6(25) << 5) | (decode1(31) << 12);
}

static inline void decodeU(void) {
    rd  = decode5(7);
    imm = decode20(12) << 12;
}

static inline void decodeJ(void) {
    rd  = decode5(7);
    imm = (decode8(12) << 12) | (decode1(20) << 11) | (decode10(21) << 1) | (decode1(31) << 20);
}

static inline i32 SignExtend(i32 intValue, const i32 numberOfBits) {
    return (intValue >> (numberOfBits - 1)) == 1 ? intValue | (0xFFFFFFFF << numberOfBits) : intValue;
}

static bool opInvalid() {
    return false;
}

// JAL
static bool opJump() {
    decodeJ();
    imm = SignExtend(imm, 21);

    u32 targetAddress = programCounterSnapshot + imm;

    offsetAddress(targetAddress);
    assertAddress(targetAddress, 4);

    setX(rd, programCounter);
    programCounter = targetAddress;

    return true;
}

// JALR
static bool opIndirectJump() {
    decodeI();
    imm = SignExtend(imm, 12);

    i32 targetAddress = (getX(rs1) + imm) & 0xFFFFFFFE;

    offsetAddress(targetAddress);
    assertAddress(targetAddress, 4);

    setX(rd, programCounter);
    programCounter = targetAddress;

    return true;
}

static bool opImmediate() {
    decodeI();
    imm = SignExtend(imm, 12);

    switch (f3) {
        case 0b000: {    // ADDI
            setX(rd, getX(rs1) + imm);
            break;
        }

        case 0b001: {    // SLLI
            setX(rd, getX(rs1) << imm);
            break;
        }

        case 0b010: {    // SLTI
            setX(rd, getX(rs1) < imm ? 1 : 0);
            break;
        }

        case 0b011: {    // SLTIU
            setX(rd, (u32) getX(rs1) < (u32) imm ? 1 : 0);
            break;
        }

        case 0b100: {    // XORI
            setX(rd, getX(rs1) ^ imm);
            break;
        }

        case 0b101: {
            switch (f7) {
                case 0b0000000: {    // SRLI
                    setX(rd, (u32) getX(rs1) >> rs2);
                    break;
                }

                case 0b0100000: {    // SRAI
                    i32 rsValue     = getX(rs1);
                    i32 shiftAmount = rs2 & 0x1F;

                    if (shiftAmount > 0) {
                        if (rsValue >> 31) {
                            rsValue = (rsValue >> shiftAmount) | (0xFFFFFFFF << (32 - shiftAmount));
                        } else {
                            rsValue >>= shiftAmount;
                        }
                    }

                    setX(rd, rsValue);
                    break;
                }

                default: {
                    return false;
                }
            }

            break;
        }

        case 0b110: {    // ORI
            setX(rd, getX(rs1) | imm);
            break;
        }

        case 0b111: {    // ANDI
            setX(rd, getX(rs1) & imm);
            break;
        }

        default: {
            return false;
        }
    }

    return true;
}

static bool opRegister() {
    decodeR();

    switch (f3) {
        case 0b000: {
            switch (f7) {
                case 0b0000000: {    // ADD
                    setX(rd, getX(rs1) + getX(rs2));
                    break;
                }

                case 0b0000001: {    // MUL
                    setX(rd, getX(rs1) * getX(rs2));
                    break;
                }

                case 0b0100000: {    // SUB
                    setX(rd, getX(rs1) - getX(rs2));
                    break;
                }

                default: {
                    return false;
                }
            }

            break;
        }

        case 0b001: {
            switch (f7) {
                case 0b0000000: {    // SLL
                    setX(rd, getX(rs1) << getX(rs2));
                    break;
                }

                case 0b0000001: {    // MULH
                    i64 rs1Value = getX(rs1);
                    i64 rs2Value = getX(rs2);

                    setX(rd, (i32) ((rs1Value * rs2Value) >> 32));
                    break;
                }

                default: {
                    return false;
                }
            }

            break;
        }

        case 0b010: {
            switch (f7) {
                case 0b0000000: {    // SLT
                    setX(rd, getX(rs1) < getX(rs2) ? 1 : 0);
                    break;
                }

                case 0b0000001: {    // MULHSU
                    i64 rs1Value = getX(rs1);
                    u64 rs2Value = (u32) getX(rs2);

                    setX(rd, (i32) ((rs1Value * rs2Value) >> 32));

                    break;
                }

                default: {
                    return false;
                }
            }

            break;
        }

        case 0b011: {
            switch (f7) {
                case 0b0000000: {    // SLTU
                    setX(rd, (u32) getX(rs1) < (u32) getX(rs2) ? 1 : 0);
                    break;
                }

                case 0b0000001: {    // MULHU
                    u64 rs1Value = (u32) getX(rs1);
                    u64 rs2Value = (u32) getX(rs2);

                    setX(rd, (i32) ((rs1Value * rs2Value) >> 32));
                    break;
                }

                default: {
                    return false;
                }
            }

            break;
        }

        case 0b100: {
            switch (f7) {
                case 0b0000000: {    // XOR
                    setX(rd, getX(rs1) ^ getX(rs2));
                    break;
                }

                case 0b0000001: {    // DIV
                    i32 rs1Value = getX(rs1);
                    i32 rs2Value = getX(rs2);

                    if (rs2Value == 0) {
                        setX(rd, -1);
                    } else if (rs1Value == INT32_MIN && rs2Value == -1) {
                        setX(rd, INT32_MIN);
                    } else {
                        setX(rd, rs1Value / rs2Value);
                    }

                    break;
                }

                default: {
                    return false;
                }
            }

            break;
        }

        case 0b101: {
            switch (f7) {
                case 0b0000000: {    // SRL
                    setX(rd, (u32) getX(rs1) >> (getX(rs2) & 0x1F));
                    break;
                }

                case 0b0000001: {    // DIVU
                    u32 rs1Value = (u32) getX(rs1);
                    u32 rs2Value = (u32) getX(rs2);

                    if (rs2Value == 0) {
                        setX(rd, 0xFFFFFFFF);
                    } else {
                        setX(rd, (i32) (rs1Value / rs2Value));
                    }

                    break;
                }

                case 0b0100000: {    // SRA
                    i32 rsValue     = getX(rs1);
                    i32 shiftAmount = getX(rs2) & 0x1F;

                    if (shiftAmount > 0) {
                        if (rsValue >> 31) {
                            rsValue = (rsValue >> shiftAmount) | (0xFFFFFFFF << (32 - shiftAmount));
                        } else {
                            rsValue >>= shiftAmount;
                        }
                    }

                    setX(rd, rsValue);
                    break;
                }

                default: {
                    return false;
                }
            }

            break;
        }

        case 0b110: {
            switch (f7) {
                case 0b0000000: {    // OR
                    setX(rd, getX(rs1) | getX(rs2));
                    break;
                }

                case 0b0000001: {    // REM
                    i32 rs1Value = getX(rs1);
                    i32 rs2Value = getX(rs2);

                    if (rs2Value == 0) {
                        setX(rd, rs1Value);
                    } else if (rs1Value == INT32_MIN && rs2Value == -1) {
                        setX(rd, 0);
                    } else {
                        setX(rd, rs1Value % rs2Value);
                    }

                    break;
                }

                default: {
                    return false;
                }
            }

            break;
        }

        case 0b111: {
            switch (f7) {
                case 0b0000000: {    // AND
                    setX(rd, getX(rs1) & getX(rs2));
                    break;
                }

                case 0b0000001: {    // REMU
                    u32 rs1Value = (u32) getX(rs1);
                    u32 rs2Value = (u32) getX(rs2);

                    if (rs2Value == 0) {
                        setX(rd, rs1Value);
                    } else {
                        setX(rd, (i32) (rs1Value % rs2Value));
                    }

                    break;
                }

                default: {
                    return false;
                }
            }

            break;
        }

        default: {
            return false;
        }
    }

    return true;
}

static bool opAUIPC() {
    decodeU();
    setX(rd, programCounterSnapshot + imm);
    return true;
}

static bool opLUI() {
    decodeU();
    setX(rd, imm);
    return true;
}

static bool opSystem() {
    decodeI();

    switch (f3) {
        case 0b000: {
            switch (imm) {
                case 0b0: {    // ECALL
                    return doSysCall();
                }

                case 0b1: {    // EBREAK
                    return false;
                }

                case 0b000100000010: {    // SRET
                    break;
                }

                case 0b001100000010: {    // MRET
                    break;
                }

                default: {
                    return false;
                }
            }
        }

        case 0b001: {    // CSRRW
            break;
        }

        case 0b010: {    // CSRRS
            break;
        }

        case 0b011: {    // CSRRC
            break;
        }

        case 0b101: {    // CSRRWI
            break;
        }

        case 0b110: {    // CSRRSI
            break;
        }

        case 0b111: {    // CSRRCI
            break;
        }

        default: {
            return false;
        }
    }

    return true;
}

static bool opBranch() {
    decodeB();
    imm = SignExtend(imm, 13);

    i32 branchAddress = programCounterSnapshot + imm;

    offsetAddress(branchAddress);
    assertAddress(branchAddress, 4);

    switch (f3) {
        case 0b000: {    // BEQ
            if (getX(rs1) == getX(rs2)) {
                programCounter = branchAddress;
            }

            break;
        }

        case 0b001: {    // BNE
            if (getX(rs1) != getX(rs2)) {
                programCounter = branchAddress;
            }

            break;
        }

        case 0b100: {    // BLT
            if (getX(rs1) < getX(rs2)) {
                programCounter = branchAddress;
            }

            break;
        }

        case 0b101: {    // BGE
            if (getX(rs1) >= getX(rs2)) {
                programCounter = branchAddress;
            }

            break;
        }

        case 0b110: {    // BLTU
            if ((u32) getX(rs1) < (u32) getX(rs2)) {
                programCounter = branchAddress;
            }

            break;
        }

        case 0b111: {    // BGEU
            if ((u32) getX(rs1) >= (u32) getX(rs2)) {
                programCounter = branchAddress;
            }

            break;
        }

        default: {
            return false;
        }
    }

    return true;
}

static bool opLoad() {
    decodeI();
    imm = SignExtend(imm, 12);

    i32 memoryAddress = getX(rs1) + imm;

    offsetAddress(memoryAddress);
    assertAddress(memoryAddress, 1);

    switch (f3) {
        case 0b000: {    // LB
            i32 byteValue = memoryBlock[memoryAddress];
            SignExtend(byteValue, 8);
            setX(rd, byteValue);
            break;
        }

        case 0b001: {    // LH
            i32 halfValue = memoryBlock[memoryAddress] | (memoryBlock[memoryAddress + 1] << 8);
            SignExtend(halfValue, 16);
            setX(rd, halfValue);
            break;
        }

        case 0b010: {    // LW
            i32 wordValue = memoryBlock[memoryAddress] | (memoryBlock[memoryAddress + 1] << 8) | (memoryBlock[memoryAddress + 2] << 16) | (memoryBlock[memoryAddress + 3] << 24);
            setX(rd, wordValue);
            break;
        }

        case 0b100: {    // LBU
            setX(rd, memoryBlock[memoryAddress]);
            break;
        }

        case 0b101: {    // LHU
            setX(rd, *(u16*) &memoryBlock[memoryAddress]);
            break;
        }

        default: {
            return false;
        }
    }

    return true;
}

static bool opStore() {
    decodeS();
    imm = SignExtend(imm, 12);

    i32 memoryAddress = getX(rs1) + imm;

    offsetAddress(memoryAddress);
    assertAddress(memoryAddress, 1);

    i32 registerValue = getX(rs2);

    switch (f3) {
        case 0b000: {    // SB
            memoryBlock[memoryAddress] = registerValue & 0xFF;
            break;
        }

        case 0b001: {    // SH
            memoryBlock[memoryAddress]     = registerValue & 0xFF;
            memoryBlock[memoryAddress + 1] = (registerValue >> 8) & 0xFF;
            break;
        }

        case 0b010: {    // SW
            memoryBlock[memoryAddress]     = registerValue & 0xFF;
            memoryBlock[memoryAddress + 1] = (registerValue >> 8) & 0xFF;
            memoryBlock[memoryAddress + 2] = (registerValue >> 16) & 0xFF;
            memoryBlock[memoryAddress + 3] = (registerValue >> 24) & 0xFF;
            break;
        }

        default: {
            return false;
        }
    }

    return true;
}

static bool opNop() {
    return true;
}

static bool opFence() {
    return true;
}

#define numberOfInstructions 128

static const instructionFunction instructionSet[128] = {
    /* 0000000 */ opNop,
    /* 0000001 */ opInvalid,
    /* 0000010 */ opInvalid,
    /* 0000011 */ opLoad,
    /* 0000100 */ opInvalid,
    /* 0000101 */ opInvalid,
    /* 0000110 */ opInvalid,
    /* 0000111 */ opInvalid,
    /* 0001000 */ opInvalid,
    /* 0001001 */ opInvalid,
    /* 0001010 */ opInvalid,
    /* 0001011 */ opInvalid,
    /* 0001100 */ opInvalid,
    /* 0001101 */ opInvalid,
    /* 0001110 */ opInvalid,
    /* 0001111 */ opFence,
    /* 0010000 */ opInvalid,
    /* 0010001 */ opInvalid,
    /* 0010010 */ opInvalid,
    /* 0010011 */ opImmediate,
    /* 0010100 */ opInvalid,
    /* 0010101 */ opInvalid,
    /* 0010110 */ opInvalid,
    /* 0010111 */ opAUIPC,
    /* 0011000 */ opInvalid,
    /* 0011001 */ opInvalid,
    /* 0011010 */ opInvalid,
    /* 0011011 */ opInvalid,
    /* 0011100 */ opInvalid,
    /* 0011101 */ opInvalid,
    /* 0011110 */ opInvalid,
    /* 0011111 */ opInvalid,
    /* 0100000 */ opInvalid,
    /* 0100001 */ opInvalid,
    /* 0100010 */ opInvalid,
    /* 0100011 */ opStore,
    /* 0100100 */ opInvalid,
    /* 0100101 */ opInvalid,
    /* 0100110 */ opInvalid,
    /* 0100111 */ opInvalid,
    /* 0101000 */ opInvalid,
    /* 0101001 */ opInvalid,
    /* 0101010 */ opInvalid,
    /* 0101011 */ opInvalid,
    /* 0101100 */ opInvalid,
    /* 0101101 */ opInvalid,
    /* 0101110 */ opInvalid,
    /* 0101111 */ opInvalid,
    /* 0110000 */ opInvalid,
    /* 0110001 */ opInvalid,
    /* 0110010 */ opInvalid,
    /* 0110011 */ opRegister,
    /* 0110100 */ opInvalid,
    /* 0110101 */ opInvalid,
    /* 0110110 */ opInvalid,
    /* 0110111 */ opLUI,
    /* 0111000 */ opInvalid,
    /* 0111001 */ opInvalid,
    /* 0111010 */ opInvalid,
    /* 0111011 */ opInvalid,
    /* 0111100 */ opInvalid,
    /* 0111101 */ opInvalid,
    /* 0111110 */ opInvalid,
    /* 0111111 */ opInvalid,
    /* 1000000 */ opInvalid,
    /* 1000001 */ opInvalid,
    /* 1000010 */ opInvalid,
    /* 1000011 */ opInvalid,
    /* 1000100 */ opInvalid,
    /* 1000101 */ opInvalid,
    /* 1000110 */ opInvalid,
    /* 1000111 */ opInvalid,
    /* 1001000 */ opInvalid,
    /* 1001001 */ opInvalid,
    /* 1001010 */ opInvalid,
    /* 1001011 */ opInvalid,
    /* 1001100 */ opInvalid,
    /* 1001101 */ opInvalid,
    /* 1001110 */ opInvalid,
    /* 1001111 */ opInvalid,
    /* 1010000 */ opInvalid,
    /* 1010001 */ opInvalid,
    /* 1010010 */ opInvalid,
    /* 1010011 */ opInvalid,
    /* 1010100 */ opInvalid,
    /* 1010101 */ opInvalid,
    /* 1010110 */ opInvalid,
    /* 1010111 */ opInvalid,
    /* 1011000 */ opInvalid,
    /* 1011001 */ opInvalid,
    /* 1011010 */ opInvalid,
    /* 1011011 */ opInvalid,
    /* 1011100 */ opInvalid,
    /* 1011101 */ opInvalid,
    /* 1011110 */ opInvalid,
    /* 1011111 */ opInvalid,
    /* 1100000 */ opInvalid,
    /* 1100001 */ opInvalid,
    /* 1100010 */ opInvalid,
    /* 1100011 */ opBranch,
    /* 1100100 */ opInvalid,
    /* 1100101 */ opInvalid,
    /* 1100110 */ opInvalid,
    /* 1100111 */ opIndirectJump,
    /* 1101000 */ opInvalid,
    /* 1101001 */ opInvalid,
    /* 1101010 */ opInvalid,
    /* 1101011 */ opInvalid,
    /* 1101100 */ opInvalid,
    /* 1101101 */ opInvalid,
    /* 1101110 */ opInvalid,
    /* 1101111 */ opJump,
    /* 1110000 */ opInvalid,
    /* 1110001 */ opInvalid,
    /* 1110010 */ opInvalid,
    /* 1110011 */ opSystem,
    /* 1110100 */ opInvalid,
    /* 1110101 */ opInvalid,
    /* 1110110 */ opInvalid,
    /* 1110111 */ opInvalid,
    /* 1111000 */ opInvalid,
    /* 1111001 */ opInvalid,
    /* 1111010 */ opInvalid,
    /* 1111011 */ opInvalid,
    /* 1111100 */ opInvalid,
    /* 1111101 */ opInvalid,
    /* 1111110 */ opInvalid,
    /* 1111111 */ opInvalid,
};

// Virtual Machine ------------------------------------------------------------

static bool resetVirtualMachine(const u32 entrypointAddress, const u32 memoryOffset, const u32 programSize) {
    if ((entrypointAddress > VirtualMachineMemorySize - 4) || (memoryOffset > VirtualMachineMemorySize - 4) || (programSize > VirtualMachineMemorySize)) {
        return false;
    }

    memset(memoryBlock, 0, VirtualMachineMemorySize);
    memset(registers, 0, sizeof(registers));

    currentInstruction     = 0;
    syncRequested          = false;
    currentSpeedMultiplier = 0;

    rd = rs1 = rs2 = f3 = f7 = imm = 0;

    programCounter         = entrypointAddress;
    programCounterSnapshot = programCounter;
    programMemoryOffset    = memoryOffset;
    currentProgramSize     = programSize;

    offsetAddress(programCounter);
    assertAddress(programCounter, 4);
    setX(SP, VirtualMachineMemorySize);

    return true;
}

bool InitializeVirtualMachine(void) {
    initializeSysCalls();
    return true;
}

bool SyncVirtualMachine(const f16 speedMultiplier) {
    bool returnValue = false;
    bool isLocked    = false;
    u64  startTime   = GetTick();

    u32 instructionCounter = 0;
    u8  opCode;

    while (true) {
        if (programCounter > currentProgramSize - 4) {
            sprintf(errorMessage, "invalid pc: %d", programCounter);
            break;
        }

        programCounterSnapshot = programCounter;
        currentSpeedMultiplier = speedMultiplier;
        currentInstruction     = *(u32*) &memoryBlock[programCounter];
        opCode                 = currentInstruction & 0x7F;

        programCounter += 4;
        instructionCounter++;

        if (opCode > 127) {
            sprintf(errorMessage, "invalid opcode: %d", opCode);
            break;
        }

        syncRequested = false;

        if (!instructionSet[opCode]()) {
            if (syncRequested) {
                returnValue = true;
                break;
            }

            if (currentInstruction == 115) {
                sprintf(errorMessage, "invalid syscall: %d", getX(A7));
            } else {
                sprintf(errorMessage, "instruction error");
            }

            break;
        }

        if (programCounter == programCounterSnapshot) {
            if (isLocked) {
                sprintf(errorMessage, "program locked");
                break;
            }

            isLocked = true;
        } else {
            isLocked = false;
        }

        if (instructionCounter >= 100000) {
            if (GetTick() - startTime > maxSyncTime) {
                sprintf(errorMessage, "sync timeout");
                break;
            }
        }
    }

    if (returnValue) {
        errorMessage[0] = 0;
    }

    return returnValue;
}

string GetVirtualMachineError(void) {
    return errorMessage[0] == 0 ? NULL : errorMessage;
}

// Programs -------------------------------------------------------------------

#define programMagicNumber   FourCC('P', 'V', 'M', 'P')
#define programVersionNumber 1

typedef struct __attribute((packed)) programFileHeader {
        u32 magicNumber;
        u16 versionNumber;
        u32 programSize;
        u32 entrypointAddress;
        u32 memoryOffset;
} programFileHeader;

bool LoadProgramFromStorage(void) {
    programFileHeader fileHeader;

    if (!ReadFile(&fileHeader, sizeof(programFileHeader))) {
        return false;
    }

    if (fileHeader.magicNumber != programMagicNumber) {
        return false;
    }

    u32 fileSize = GetFileSize();

    if (fileSize != fileHeader.programSize + sizeof(programFileHeader)) {
        return false;
    }

    if (fileHeader.programSize > VirtualMachineMemorySize) {
        return false;
    }

    if (!resetVirtualMachine(fileHeader.entrypointAddress, fileHeader.memoryOffset, fileHeader.programSize)) {
        return false;
    }

    return ReadFile(memoryBlock, fileHeader.programSize);
}
