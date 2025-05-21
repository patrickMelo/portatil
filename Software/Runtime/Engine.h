//
// Runtime/Engine.h
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#ifndef PORTATIL_ENGINE_H
#define PORTATIL_ENGINE_H

#include "Kernel.h"

// Sprites --------------------------------------------------------------------

#define MaxSprites 256

typedef struct Sprite {
        u32   Index;
        bool  IsFree;
        Image Image;
        u16   TransparentColor;
        u16   FrameWidth;
        u16   FrameHeight;
        f16   FrameSpeed;
        u8    NumberOfFrames;
} Sprite;

Sprite* GetSprite(const Image* image);
Sprite* GetSpriteByIndex(const u32 spriteIndex);
void    ReleaseSprite(Sprite* sprite);

// Entities -------------------------------------------------------------------

#define MaxLayers        4
#define MaxLayerEntities 128

typedef struct Entity {
        u8           LayerIndex;
        u32          Index;
        u32          TypeID;
        Sprite*      Sprite;
        FixedPoint2D Position;
        Point2D      Direction;
        FixedPoint2D Speed;
        f16          FrameIndex;
        uint         DataAddress;
        bool         ReleaseAfterSync;
} Entity;

u32     GetNumberOfEntities(const u8 layerIndex);
Entity* GetEntity(const u8 layerIndex, const u32 typeID, const Sprite* sprite, const f16 xPosition, const f16 yPosition);
Entity* GetEntityByIndex(const u8 layerIndex, const u32 entityIndex);
void    ReleaseEntity(Entity* entity);
Entity* GetCollidingEntity(const Entity* entity, const u32 otherEntityTypeID);
bool    IsEntityOnScreen(const Entity* entity);
i32     FindEntityIndex(const u8 layerIndex, const u32 typeID, const u32 occurrenceNumber);

// Engine ---------------------------------------------------------------------

void InitializeEngine(void);
void ResetEngine(void);
u64  SyncEngine(const f16 speedMultiplier);

u64 GetEngineTime(void);

#endif    // PORTATIL_ENGINE_H