//
// Tools/Packer.h
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#ifndef PORTATIL_PACKER_H
#define PORTATIL_PACKER_H

#include "Tools.h"

// Packer ---------------------------------------------------------------------

int RunPacker(const int numberOfArguments, const string* argumentsValues);

// Images ---------------------------------------------------------------------

#define MaxImageWidth  256
#define MaxImageHeight 256

typedef struct Image {
        u16 Width;
        u16 Height;
        u8  BitsPerPixel;
        u8* Data;
} Image;

Image* CreateImage(const u16 width, const u16 height, const u8 bitsPerPixel);
void   DestroyImage(Image* image);
void   DitherImage(Image* image);
Image* GetIndexedImage(Image* image);

Image* LoadPNG(const string filePath);

// Assets ---------------------------------------------------------------------

#define MaxAssets 100

enum {
    UnknownAsset,
    ImageAsset
};

u8   GetAssetTypeFromFileName(const string fileName);
void ExtractAssetNameFromFileName(const string fileName, string assetName);

bool AddImageAsset(const string name, Image* image);
bool WriteAssetsHeader(const string filePath, const string sdkHeaderPath);
bool WriteAssetsCode(const string filePath);
void FreeAssets(void);

#endif    // PORTATIL_PACKER_H