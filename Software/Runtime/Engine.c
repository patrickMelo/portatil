//
// Runtime/Engine.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "Engine.h"

static u64 busyTime     = 0;
static u64 lastBusyTime = 0;

#define startTimer() u64 startTime = GetTick()
#define stopTimer()  busyTime += GetTick() - startTime

// Sprites --------------------------------------------------------------------

static u32    nextFreeSpriteIndex = 0;
static Sprite sprites[MaxSprites];

Sprite* GetSprite(const Image* image) {
    if (nextFreeSpriteIndex >= MaxSprites) {
        return NULL;
    }

    startTimer();

    sprites[nextFreeSpriteIndex].IsFree           = false;
    sprites[nextFreeSpriteIndex].Image            = *image;
    sprites[nextFreeSpriteIndex].TransparentColor = 0;
    sprites[nextFreeSpriteIndex].FrameWidth       = 0;
    sprites[nextFreeSpriteIndex].FrameHeight      = 0;
    sprites[nextFreeSpriteIndex].FrameSpeed       = 0;
    sprites[nextFreeSpriteIndex].NumberOfFrames   = 0;

    Sprite* sprite = &sprites[nextFreeSpriteIndex];

    if (nextFreeSpriteIndex < MaxSprites) {
        for (u32 spriteIndex = nextFreeSpriteIndex + 1; nextFreeSpriteIndex < MaxSprites; nextFreeSpriteIndex++) {
            if (sprites[spriteIndex].IsFree) {
                nextFreeSpriteIndex = spriteIndex;
                break;
            }
        }
    }

    stopTimer();
    return sprite;
}

Sprite* GetSpriteByIndex(const u32 spriteIndex) {
    if (spriteIndex >= MaxSprites || sprites[spriteIndex].IsFree) {
        return NULL;
    }

    return &sprites[spriteIndex];
}

void ReleaseSprite(Sprite* sprite) {
    sprite->IsFree = true;

    if (sprite->Index < nextFreeSpriteIndex) {
        nextFreeSpriteIndex = sprite->Index;
    }
}

// Entities -------------------------------------------------------------------

static u32    numberOfEntities[MaxLayers];
static Entity entities[MaxLayers][MaxLayerEntities];

static void drawEntity(Entity* entity) {
    u8 framesPerRow = entity->Sprite->Image.Width / entity->Sprite->FrameWidth;
    u8 frameRow     = F16ToInt(entity->FrameIndex) / framesPerRow;
    u8 frameColumn  = F16ToInt(entity->FrameIndex) % framesPerRow;

    Rectangle2D frameRect = {
        .X      = frameColumn * entity->Sprite->FrameWidth,
        .Y      = frameRow * entity->Sprite->FrameHeight,
        .Width  = entity->Sprite->FrameWidth,
        .Height = entity->Sprite->FrameHeight,
    };

    SetTransparentColor(entity->Sprite->TransparentColor);
    DrawImage(&entity->Sprite->Image, F16ToInt(entity->Position.X), F16ToInt(entity->Position.Y), &frameRect);
}

u32 GetNumberOfEntities(const u8 layerIndex) {
    return layerIndex < MaxLayers ? numberOfEntities[layerIndex] : 0;
}

Entity* GetEntity(const u8 layerIndex, const u32 typeID, const Sprite* sprite, const f16 xPosition, const f16 yPosition) {
    if (layerIndex >= MaxLayers || numberOfEntities[layerIndex] >= MaxLayerEntities) {
        return 0;
    }

    startTimer();

    Entity* entity = &entities[layerIndex][numberOfEntities[layerIndex]++];

    entity->TypeID      = typeID;
    entity->Position.X  = xPosition;
    entity->Position.Y  = yPosition;
    entity->Sprite      = (Sprite*) sprite;
    entity->FrameIndex  = 0;
    entity->Direction.X = 0;
    entity->Direction.Y = 0;
    entity->Speed.X     = 0;
    entity->Speed.Y     = 0;

    entity->ReleaseAfterSync = false;

    stopTimer();
    return entity;
}

Entity* GetEntityByIndex(const u8 layerIndex, const u32 entityIndex) {
    if (layerIndex >= MaxLayers || entityIndex >= numberOfEntities[layerIndex]) {
        return NULL;
    }

    return &entities[layerIndex][entityIndex];
}

void ReleaseEntity(Entity* entity) {
    if (!entity) {
        return;
    }

    entity->ReleaseAfterSync = true;
}

Entity* GetCollidingEntity(const Entity* entity, const u32 otherEntityTypeID) {
    if (!entity) {
        return NULL;
    }

    startTimer();

    Rectangle2D entityRect = {
        .X      = F16ToInt(entity->Position.X),
        .Y      = F16ToInt(entity->Position.Y),
        .Width  = entity->Sprite->FrameWidth,
        .Height = entity->Sprite->FrameHeight,
    };

    for (u32 otherEntityIndex = 0; otherEntityIndex < numberOfEntities[entity->LayerIndex]; otherEntityIndex++) {
        if (otherEntityIndex == entity->Index || entities[entity->LayerIndex][otherEntityIndex].TypeID != otherEntityTypeID) {
            continue;
        }

        Rectangle2D otherEntityRect = {
            .X      = F16ToInt(entities[entity->LayerIndex][otherEntityIndex].Position.X),
            .Y      = F16ToInt(entities[entity->LayerIndex][otherEntityIndex].Position.Y),
            .Width  = entities[entity->LayerIndex][otherEntityIndex].Sprite->FrameWidth,
            .Height = entities[entity->LayerIndex][otherEntityIndex].Sprite->FrameHeight,
        };

        if ((otherEntityRect.X < entityRect.X + entityRect.Width) &&
            (otherEntityRect.X + otherEntityRect.Width > entityRect.X) &&
            (otherEntityRect.Y < entityRect.Y + entityRect.Height) &&
            (otherEntityRect.Y + otherEntityRect.Height > entityRect.Y)) {
            stopTimer();
            return &entities[entity->LayerIndex][otherEntityIndex];
        }
    }

    stopTimer();
    return NULL;
}

bool IsEntityOnScreen(const Entity* entity) {
    if (!entity) {
        return false;
    }

    return (entity->Position.X >= -F16(entity->Sprite->FrameWidth)) &&
           (entity->Position.Y >= -F16(entity->Sprite->FrameHeight)) &&
           (entity->Position.X < F16(ScreenWidth)) &&
           (entity->Position.Y < F16(ScreenHeight));
}

i32 FindEntityIndex(const u8 layerIndex, const u32 typeID, const u32 occurrenceNumber) {
    if ((layerIndex >= MaxLayers) || (occurrenceNumber == 0)) {
        return -1;
    }

    startTimer();

    u32 occurrencesFound = 0;

    for (u32 entityIndex = 0; entityIndex < numberOfEntities[layerIndex]; entityIndex++) {
        if (entities[layerIndex][entityIndex].TypeID == typeID) {
            occurrencesFound++;

            if (occurrencesFound == occurrenceNumber) {
                stopTimer();
                return entityIndex;
            }
        }
    }

    stopTimer();
    return -1;
}

// Engine ---------------------------------------------------------------------

void InitializeEngine(void) {
    ResetEngine();
}

void ResetEngine(void) {
    startTimer();

    for (u32 spriteIndex = 0; spriteIndex < MaxSprites; ++spriteIndex) {
        sprites[spriteIndex].Index  = spriteIndex;
        sprites[spriteIndex].IsFree = true;
    }

    nextFreeSpriteIndex = 0;

    for (u32 layerIndex = 0; layerIndex < MaxLayers; layerIndex++) {
        numberOfEntities[layerIndex] = 0;

        for (u32 entityIndex = 0; entityIndex < MaxLayerEntities; ++entityIndex) {
            entities[layerIndex][entityIndex].LayerIndex = layerIndex;
            entities[layerIndex][entityIndex].Index      = entityIndex;
        }
    }

    stopTimer();
}

u64 SyncEngine(const f16 speedMultiplier) {
    startTimer();

    for (u8 layerIndex = 0; layerIndex < MaxLayers; layerIndex++) {
        for (u32 entityIndex = 0; entityIndex < numberOfEntities[layerIndex]; ++entityIndex) {
            Entity* entity = &entities[layerIndex][entityIndex];

            if (entity->Sprite->FrameSpeed != 0) {
                entity->FrameIndex += F16Mult(entity->Sprite->FrameSpeed, speedMultiplier);

                if (F16ToInt(entity->FrameIndex) >= entity->Sprite->NumberOfFrames) {
                    entity->FrameIndex = 0;
                }
            }

            if (entity->Direction.X != 0) {
                entity->Position.X += F16Mult(entity->Speed.X, speedMultiplier) * entity->Direction.X;
            }

            if (entity->Direction.Y != 0) {
                entity->Position.Y += F16Mult(entity->Speed.Y, speedMultiplier) * entity->Direction.Y;
            }

            drawEntity(entity);
        }
    }

    for (u8 layerIndex = 0; layerIndex < MaxLayers; layerIndex++) {
        u32 entityIndex = 0;

        while (entityIndex < numberOfEntities[layerIndex]) {
            if (entities[layerIndex][entityIndex].ReleaseAfterSync) {
                numberOfEntities[layerIndex]--;

                if (entityIndex < numberOfEntities[layerIndex]) {
                    u32 indexBackup = entities[layerIndex][entityIndex].Index;
                    memcpy((byte*) &entities[layerIndex][entityIndex], (byte*) &entities[layerIndex][numberOfEntities[layerIndex]], sizeof(Entity));
                    entities[layerIndex][entityIndex].Index = indexBackup;
                }

                continue;
            }

            entityIndex++;
        }
    }

    stopTimer();

    lastBusyTime = busyTime;
    busyTime     = 0;

    return lastBusyTime;
}

u64 GetEngineTime(void) {
    return lastBusyTime;
}