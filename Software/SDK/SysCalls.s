#
# SDK/SysCalls.s
#
# This file is part of Portatil source code.
# Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
#

# General ---------------------------------------------------------------------

.globl	SysExit
.type	SysExit, @function

SysExit:
    add a7, zero, 1
    ecall
    ret

.globl	SysSync
.type	SysSync, @function

SysSync:
    add a7, zero, 2
    ecall
    ret

.globl	SysRandom
.type	SysRandom, @function

SysRandom:
    add a7, zero, 3
    ecall
    ret

.globl	SysGetFrameTime
.type	SysGetFrameTime, @function

SysGetFrameTime:
    add a7, zero, 4
    ecall
    ret

.globl	SysGetTickSeconds
.type	SysGetTickSeconds, @function

SysGetTickSeconds:
    add a7, zero, 5
    ecall
    ret    

# Power -----------------------------------------------------------------------

.globl	SysGetBatteryPercentage
.type	SysGetBatteryPercentage, @function

SysGetBatteryPercentage:
    add a7, zero, 10
    ecall
    ret

# Input -----------------------------------------------------------------------

.globl	SysGetInputState
.type	SysGetInputState, @function

SysGetInputState:
    add a7, zero, 20
    ecall
    ret

.globl	SysGetInputAxis
.type	SysGetInputAxis, @function

SysGetInputAxis:
    add a7, zero, 21
    ecall
    ret

.globl	SysIsButtonPressed
.type	SysIsButtonPressed, @function

SysIsButtonPressed:
    add a7, zero, 22
    ecall
    ret

.globl	SysIsButtonJustPressed
.type	SysIsButtonJustPressed, @function

SysIsButtonJustPressed:
    add a7, zero, 23
    ecall
    ret

.globl	SysIsButtonJustReleased
.type	SysIsButtonJustReleased, @function

SysIsButtonJustReleased:
    add a7, zero, 24
    ecall
    ret

# Graphics --------------------------------------------------------------------

.globl	SysClearScreen
.type	SysClearScreen, @function

SysClearScreen:
    add a7, zero, 30
    ecall
    ret

.globl	SysGetColorIndex
.type	SysGetColorIndex, @function

SysGetColorIndex:
    add a7, zero, 31
    ecall
    ret

.globl	SysSetTransparentColor
.type	SysSetTransparentColor, @function

SysSetTransparentColor:
    add a7, zero, 32
    ecall
    ret

.globl	SysSetBackgroundColor
.type	SysSetBackgroundColor, @function

SysSetBackgroundColor:
    add a7, zero, 33
    ecall
    ret

.globl	SysSetForegroundColor
.type	SysSetForegroundColor, @function

SysSetForegroundColor:
    add a7, zero, 34
    ecall
    ret

.globl	SysSetDrawAnchor
.type	SysSetDrawAnchor, @function

SysSetDrawAnchor:
    add a7, zero, 35
    ecall
    ret

.globl	SysSetDrawScale
.type	SysSetDrawScale, @function

SysSetDrawScale:
    add a7, zero, 36
    ecall
    ret

.globl	SysSetTargetPosition
.type	SysSetTargetPosition, @function

SysSetTargetPosition:
    add a7, zero, 37
    ecall
    ret

.globl	SysSetSourceRectangle
.type	SysSetSourceRectangle, @function

SysSetSourceRectangle:
    add a7, zero, 38
    ecall
    ret

.globl	SysSetTargetRectangle
.type	SysSetTargetRectangle, @function

SysSetTargetRectangle:
    add a7, zero, 39
    ecall
    ret

.globl	SysSetTextFont
.type	SysSetTextFont, @function

SysSetTextFont:
    add a7, zero, 40
    ecall
    ret    

.globl	SysDrawRectangle
.type	SysDrawRectangle, @function

SysDrawRectangle:
    add a7, zero, 41
    ecall
    ret

.globl	SysDrawImage
.type	SysDrawImage, @function

SysDrawImage:
    add a7, zero, 42
    ecall
    ret

.globl	SysDrawText
.type	SysDrawText, @function

SysDrawText:
    add a7, zero, 43
    ecall
    ret

.globl	SysDrawNumber
.type	SysDrawNumber, @function

SysDrawNumber:
    add a7, zero, 44
    ecall
    ret

# Sound -----------------------------------------------------------------------

.globl	SysSetChannelVolume
.type	SysSetChannelVolume, @function

SysSetChannelVolume:
    add a7, zero, 50
    ecall
    ret

.globl	SysPlayTone
.type	SysPlayTone, @function

SysPlayTone:
    add a7, zero, 51
    ecall
    ret

.globl	SysStopChannel
.type	SysStopChannel, @function

SysStopChannel:
    add a7, zero, 52
    ecall
    ret

.globl	SysStopAllSound
.type	SysSysStopAllSoundPlayTone, @function

SysStopAllSound:
    add a7, zero, 53
    ecall
    ret

# Engine ----------------------------------------------------------------------

.globl	SysSyncEngine
.type	SysSyncEngine, @function

SysSyncEngine:
    add a7, zero, 60
    ecall
    ret

.globl	SysGetSprite
.type	SysGetSprite, @function

SysGetSprite:
    add a7, zero, 61
    ecall
    ret

.globl	SysReleaseSprite
.type	SysReleaseSprite, @function

SysReleaseSprite:
    add a7, zero, 62
    ecall
    ret

.globl	SysSetSpriteProps
.type	SysSetSpriteProps, @function

SysSetSpriteProps:
    add a7, zero, 63
    ecall
    ret

.globl	SysSetSpriteFrames
.type	SysSetSpriteFrames, @function

SysSetSpriteFrames:
    add a7, zero, 64
    ecall
    ret

.globl	SysSetActiveLayer
.type	SysSetActiveLayer, @function

SysSetActiveLayer:
    add a7, zero, 65
    ecall
    ret

.globl	SysGetNumberOfEntities
.type	SysGetNumberOfEntities, @function

SysGetNumberOfEntities:
    add a7, zero, 66
    ecall
    ret

.globl	SysGetEntity
.type	SysGetEntity, @function

SysGetEntity:
    add a7, zero, 67
    ecall
    ret

.globl	SysReleaseEntity
.type	SysReleaseEntity, @function

SysReleaseEntity:
    add a7, zero, 68
    ecall
    ret

.globl	SysSetEntityPosition
.type	SysSetEntityPosition, @function

SysSetEntityPosition:
    add a7, zero, 69
    ecall
    ret

.globl	SysSetEntityDirection
.type	SysSetEntityDirection, @function

SysSetEntityDirection:
    add a7, zero, 70
    ecall
    ret

.globl	SysSetEntitySpeed
.type	SysSetEntitySpeed, @function

SysSetEntitySpeed:
    add a7, zero, 71
    ecall
    ret

.globl	SysSetEntityFrameIndex
.type	SysSetEntityFrameIndex, @function

SysSetEntityFrameIndex:
    add a7, zero, 72
    ecall
    ret

.globl	SysSetEntityData
.type	SysSetEntityData, @function

SysSetEntityData:
    add a7, zero, 73
    ecall
    ret


.globl	SysGetEntityTypeID
.type	SysGetEntityTypeID, @function

SysGetEntityTypeID:
    add a7, zero, 74
    ecall
    ret

.globl	SysGetEntityPositionX
.type	SysGetEntityPositionX, @function

SysGetEntityPositionX:
    add a7, zero, 75
    ecall
    ret

.globl	SysGetEntityPositionY
.type	SysGetEntityPositionY, @function

SysGetEntityPositionY:
    add a7, zero, 76
    ecall
    ret

.globl	SysGetEntityDirectionX
.type	SysGetEntityDirectionX, @function

SysGetEntityDirectionX:
    add a7, zero, 77
    ecall
    ret

.globl	SysGetEntityDirectionY
.type	SysGetEntityDirectionY, @function

SysGetEntityDirectionY:
    add a7, zero, 78
    ecall
    ret


.globl	SysGetEntitySpeedX
.type	SysGetEntitySpeedX, @function

SysGetEntitySpeedX:
    add a7, zero, 79
    ecall
    ret

.globl	SysGetEntitySpeedY
.type	SysGetEntitySpeedY, @function

SysGetEntitySpeedY:
    add a7, zero, 80
    ecall
    ret

.globl	SysGetEntityFrameIndex
.type	SysGetEntityFrameIndex, @function

SysGetEntityFrameIndex:
    add a7, zero, 81
    ecall
    ret

.globl	SysGetEntityData
.type	SysGetEntityData, @function

SysGetEntityData:
    add a7, zero, 82
    ecall
    ret

.globl	SysGetCollidingEntityIndex
.type	SysGetCollidingEntityIndex, @function

SysGetCollidingEntityIndex:
    add a7, zero, 83
    ecall
    ret



.globl	SysFindEntityIndex
.type	SysFindEntityIndex, @function

SysFindEntityIndex:
    add a7, zero, 84
    ecall
    ret

.globl	SysIsEntityOnScreen
.type	SysIsEntityOnScreen, @function

SysIsEntityOnScreen:
    add a7, zero, 85
    ecall
    ret
