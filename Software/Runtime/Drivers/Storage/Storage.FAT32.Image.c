//
// Runtime/Drivers/Storage/Storage.FAT32.Image.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../../Drivers.h"

// Media ----------------------------------------------------------------------

#define sectorSize 512
#define filePath   "fat32.img"

static FILE* mediaFile = NULL;

bool Fat32InitializeMedia(void) {
    if (mediaFile) {
        return false;
    }

    mediaFile = fopen(filePath, "rb");
    return mediaFile != NULL;
}

void Fat32FinalizeMedia(void) {
    if (!mediaFile) {
        return;
    }

    fclose(mediaFile);
    mediaFile = NULL;
}

u32 Fat32GetMediaSize(void) {
    if (!mediaFile) {
        return 0;
    }

    fseek(mediaFile, 0, SEEK_END);
    return ftell(mediaFile);
}

bool Fat32ReadMedia(const u32 sectorIndex, u8* sectorData) {
    if (!mediaFile) {
        return false;
    }

    fseek(mediaFile, sectorIndex * sectorSize, SEEK_SET);
    return fread(sectorData, sectorSize, 1, mediaFile) == 1;
}
