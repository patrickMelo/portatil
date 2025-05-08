//
// Tools/Packer/Images.PNG.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../Packer.h"

#include <png.h>

#define logTag "Packer:Images:PNG"

typedef struct dataBlock {
        byte* data;
        u64   index;
        u64   size;
} dataBlock;

static void readFromMemory(png_structp png, png_bytep data, png_size_t readSize) {
    dataBlock* sourceData = png_get_io_ptr(png);

    if (sourceData->index + readSize > sourceData->size) {
        png_error(png, "insufficient data");
        Error(logTag, "There is not sufficient data to read from the buffer");
        return;
    }

    memcpy(data, &sourceData->data[sourceData->index], readSize);
    sourceData->index += readSize;
}

static void pngError(png_structp png, png_const_charp errorMessage) {
    Error(logTag, (string) errorMessage);
}

Image* LoadPNG(const string filePath) {
    static dataBlock pngData;
    pngData.data = QuickReadFile(filePath, &pngData.size);

    if (!pngData.data) {
        return NULL;
    }

    pngData.index = 8;

    if (!png_check_sig(pngData.data, 8)) {
        Error(logTag, "Invalid  signature");
        return NULL;
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, pngError, NULL);

    if (!png) {
        Error(logTag, "Could not create reading structure");
        free(pngData.data);
        return NULL;
    }

    png_infop pngInfo = png_create_info_struct(png);

    if (!pngInfo) {
        Error(logTag, "Could not create info structure");
        png_destroy_read_struct(&png, NULL, NULL);
        free(pngData.data);
        return NULL;
    }

    png_set_read_fn(png, &pngData, readFromMemory);
    png_set_sig_bytes(png, 8);
    png_read_info(png, pngInfo);

    png_uint_32 imageWidth      = 0;
    png_uint_32 imageHeight     = 0;
    int         bitDepth        = 0;
    int         colorType       = -1;
    png_uint_32 getHeaderResult = png_get_IHDR(png, pngInfo, &imageWidth, &imageHeight, &bitDepth, &colorType, NULL, NULL, NULL);

    if (getHeaderResult != 1) {
        Error(logTag, "Error reading header chunk");
        png_destroy_info_struct(png, &pngInfo);
        png_destroy_read_struct(&png, NULL, NULL);
        free(pngData.data);
        return NULL;
    }

    if (bitDepth != 8) {
        Error(logTag, "Unsupported bit depth: %d", bitDepth);
        png_destroy_info_struct(png, &pngInfo);
        png_destroy_read_struct(&png, NULL, NULL);
        free(pngData.data);
        return NULL;
    }

    if (colorType != PNG_COLOR_TYPE_RGB) {
        Error(logTag, "Unsupported color type: %d", colorType);
        png_destroy_info_struct(png, &pngInfo);
        png_destroy_read_struct(&png, NULL, NULL);
        free(pngData.data);
        return NULL;
    }

    Image* image = CreateImage(imageWidth, imageHeight, 24);

    if (!image) {
        Error(logTag, "Could not create the new image");
        png_destroy_info_struct(png, &pngInfo);
        png_destroy_read_struct(&png, NULL, NULL);
        free(pngData.data);
        return NULL;
    }

    byte* rowBuffer = malloc(png_get_rowbytes(png, pngInfo));

    if (!rowBuffer) {
        Error(logTag, "Could not allocate memory for the row buffer");
        DestroyImage(image);
        png_destroy_info_struct(png, &pngInfo);
        png_destroy_read_struct(&png, NULL, NULL);
        free(pngData.data);
        return NULL;
    }

    u32 pixelIndex;

    for (u32 y = 0; y < image->Height; ++y) {
        png_read_row(png, rowBuffer, NULL);

        for (u32 x = 0; x < image->Width; ++x) {
            pixelIndex                      = (y * image->Width) + x;
            image->Data[pixelIndex * 3]     = rowBuffer[x * 3];
            image->Data[pixelIndex * 3 + 1] = rowBuffer[x * 3 + 1];
            image->Data[pixelIndex * 3 + 2] = rowBuffer[x * 3 + 2];
        }
    }

    png_destroy_info_struct(png, &pngInfo);
    png_destroy_read_struct(&png, NULL, NULL);

    free(rowBuffer);
    free(pngData.data);

    Debug(logTag, "Image read from \"%s\"", filePath);
    return image;
}