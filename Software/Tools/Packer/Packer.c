//
// Tools/Packer/Packer.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../Packer.h"

#define logTag "Packer"

int RunPacker(const int numberOfArguments, const string* argumentsValues) {
    if (numberOfArguments != 3) {
        return PrintUsageReturnCode;
    }

    const string sourceDirectory = argumentsValues[0];
    const string outputDirectory = argumentsValues[1];
    const string sdkHeaderPath   = argumentsValues[2];

    FileList fileList = {
        .NumberOfFiles = 0,
    };

    if (!ListFiles(sourceDirectory, &fileList)) {
        return 1;
    }

    static u8   assetType;
    static char nameBuffer[MaxPathLength];

    Info(logTag, "Packing assets from \"%s\" into \"%s\"", sourceDirectory, outputDirectory);

    for (int fileIndex = 0; fileIndex < fileList.NumberOfFiles; ++fileIndex) {
        assetType = GetAssetTypeFromFileName(fileList.Path[fileIndex]);

        switch (assetType) {
            case ImageAsset: {
                Image* rgbImage = LoadPNG(fileList.Path[fileIndex]);

                if (!rgbImage) {
                    Warning(logTag, "Skipping \"%s\" (load error)", fileList.Path[fileIndex]);
                    break;
                }

                Image* image = GetIndexedImage(rgbImage);
                DestroyImage(rgbImage);

                ExtractAssetNameFromFileName(fileList.Path[fileIndex], nameBuffer);
                AddImageAsset(nameBuffer, image);
                break;
            }

            default: {
                Warning(logTag, "Ignoring \"%s\" (unknown asset type)", fileList.Path[fileIndex]);
            }
        }
    }

    strcpy(nameBuffer, outputDirectory);
    strcat(nameBuffer, "/Assets.h");

    Info(logTag, "Writing assets header to \"%s\"", nameBuffer);

    if (!WriteAssetsHeader(nameBuffer, sdkHeaderPath)) {
        FreeAssets();
        return 1;
    }

    strcpy(nameBuffer, outputDirectory);
    strcat(nameBuffer, "/Assets.c");

    Info(logTag, "Writing assets code to \"%s\"", nameBuffer);

    if (!WriteAssetsCode(nameBuffer)) {
        FreeAssets();
        return 1;
    }

    FreeAssets();
    Info(logTag, "Done");
    return 0;
}