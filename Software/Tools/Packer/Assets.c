//
// Tools/Packer/Assets.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../Packer.h"

#define logTag "Packer:Assets"

#define writeBufferSize 4096

static char writeBuffer[writeBufferSize];

u8 GetAssetTypeFromFileName(const string fileName) {
    u16 extensionStart = strlen(fileName) - 4;

    if (strncasecmp(&fileName[extensionStart], ".png", 4) == 0) {
        return ImageAsset;
    }

    return UnknownAsset;
}

void ExtractAssetNameFromFileName(const string fileName, string assetName) {
    u16 nameLength = strlen(fileName);
    int nameStart  = -1;
    int nameEnd    = -1;

    for (u16 charIndex = nameLength - 1; charIndex >= 0; --charIndex) {
        if (fileName[charIndex] == '.' && nameEnd == -1) {
            nameEnd = charIndex - 1;
            continue;
        }

        if (fileName[charIndex] == '/' && nameStart == -1) {
            nameStart = charIndex + 1;
            break;
        }
    }

    if (nameStart < 0) {
        nameStart = 0;
    }

    if (nameEnd < 0) {
        nameEnd = nameLength - 1;
    }

    nameLength = nameEnd - nameStart + 1;
    memcpy(assetName, &fileName[nameStart], nameLength);
    assetName[nameLength] = 0;
}

typedef struct assetInfo {
        u8    type;
        char  name[MaxPathLength];
        void* asset;
} assetInfo;

static assetInfo assets[MaxAssets];
static u8        nextFreeIndex = 0;

bool addAsset(const string name, const uint assetType, void* asset) {
    if (!name || !asset) {
        Error(logTag, "Invalid asset name or pointer");
        return false;
    }

    if (nextFreeIndex >= MaxAssets) {
        Error(logTag, "Maximum number of assets reached: %d", MaxAssets);
        return false;
    }

    for (u16 index = 0; index < nextFreeIndex; ++index) {
        if (strcmp(assets[index].name, name) == 0) {
            Error(logTag, "Asset named \"%d\" already exists", name);
            return false;
        }
    }

    assets[nextFreeIndex].type = assetType;
    strcpy(assets[nextFreeIndex].name, name);
    assets[nextFreeIndex].asset = asset;

    Info(logTag, "Asset \"%s\" added at index %d", name, nextFreeIndex);
    nextFreeIndex++;
    return true;
}

bool AddImageAsset(const string name, Image* image) {
    return addAsset(name, ImageAsset, image);
}

void writeImageAssetHeader(File* file, assetInfo* asset) {
    static const string imageTemplate = "extern const Image %sImage;\nextern const Rectangle2D %sRectangle;\n";
    sprintf(writeBuffer, imageTemplate, asset->name, asset->name);
    WriteFile(file, (byte*) writeBuffer, strlen(writeBuffer));
}

bool WriteAssetsHeader(const string filePath, const string sdkHeaderPath) {
    File* headerFile = CreateFile(filePath);

    if (!headerFile) {
        Error(logTag, "Could not create assets header file");
        return false;
    }

    static const string headerStartTemplate =
        "#ifndef PORTATIL_ASSETS_H \n\
#define PORTATIL_ASSETS_H \n\
\n\
#include \"%s\"\n\
\n";

    sprintf(writeBuffer, headerStartTemplate, sdkHeaderPath);
    WriteFile(headerFile, (byte*) writeBuffer, strlen(writeBuffer));

    for (u16 index = 0; index < nextFreeIndex; ++index) {
        switch (assets[index].type) {
            case ImageAsset: {
                writeImageAssetHeader(headerFile, &assets[index]);
                break;
            }

            default: {
                Error(logTag, "Unknown asset type for asset %d: %d", index, assets[index].type);
            }
        }
    }

    static const string headerEnd = "\n#endif // PORTATIL_ASSETS_H\n";

    WriteFile(headerFile, (byte*) headerEnd, strlen(headerEnd));
    CloseFile(headerFile);

    return true;
}

void writeImageAssetCode(File* file, assetInfo* asset) {
    static const string imageDataStartTemplate = "static const byte %sImageData[] = {\n    ";

    sprintf(writeBuffer, imageDataStartTemplate, asset->name);
    WriteFile(file, (byte*) writeBuffer, strlen(writeBuffer));

    Image* image      = (Image*) asset->asset;
    u16    rowCounter = 0;

    for (u16 dataIndex = 0; dataIndex < image->Width * image->Height; ++dataIndex) {
        sprintf(writeBuffer, "0x%02X, ", image->Data[dataIndex]);
        WriteFile(file, (byte*) writeBuffer, strlen(writeBuffer));

        rowCounter++;

        if (rowCounter >= image->Width) {
            WriteFile(file, (byte*) "\n    ", 5);
            rowCounter = 0;
        }
    }

    static const string imageMetadataTemplate =
        "};\n\n\
const Image %sImage = {\n\
    .Width = %d, \n\
    .Height = %d, \n\
    .Data = (byte*) %sImageData,\n};\n\n\
const Rectangle2D %sRectangle = {\n\
    .X = 0,\n\
    .Y = 0,\n\
    .Width = %d,\n\
    .Height = %d,\n};\n\n";

    sprintf(writeBuffer, imageMetadataTemplate, asset->name, image->Width, image->Height, asset->name, asset->name, image->Width, image->Height);
    WriteFile(file, (byte*) writeBuffer, strlen(writeBuffer));
}

bool WriteAssetsCode(const string filePath) {
    File* codeFile = CreateFile(filePath);

    if (!codeFile) {
        Error(logTag, "Could not create assets code file");
        return false;
    }

    static const string codeStart =
        "#include \"Assets.h\" \n\n\
// clang-format off\n\n";

    WriteFile(codeFile, (byte*) codeStart, strlen(codeStart));

    for (u16 index = 0; index < nextFreeIndex; ++index) {
        switch (assets[index].type) {
            case ImageAsset: {
                writeImageAssetCode(codeFile, &assets[index]);
                break;
            }

            default: {
                Error(logTag, "Unknown asset type for asset %d: %d", index, assets[index].type);
            }
        }
    }

    static const string codeEnd = "// clang-format on\n";

    WriteFile(codeFile, (byte*) codeEnd, strlen(codeEnd));
    CloseFile(codeFile);

    return true;
}

void FreeAssets(void) {
    for (u16 index = 0; index < nextFreeIndex; ++index) {
        switch (assets[index].type) {
            case ImageAsset: {
                DestroyImage((Image*) assets[index].asset);
                break;
            }

            default: {
                Error(logTag, "Unknown asset type for asset %d: %d", index, assets[index].type);
            }
        }
    }
}