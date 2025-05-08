//
// Tools/Packer/Images.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../Packer.h"

#define logTag       "Packer:Images"
#define screenColors 256

static u8   colorPalette[screenColors * 3];
static bool isColorPaletteBuilt = false;

static void buildColorPalette(void) {
    if (isColorPaletteBuilt) {
        return;
    }

    static const u8 minValues[48] = {
        0, 0, 0,      // While/Gray/Black
        32, 0, 0,     // Red
        32, 8, 0,     // Red/Orange
        32, 16, 0,    // Orange
        32, 16, 0,    // Orange/Yellow
        32, 32, 0,    // Yellow
        16, 32, 0,    // Lime
        0, 32, 0,     // Green
        0, 32, 16,    // Green/Teal
        0, 32, 32,    // Teal
        0, 16, 32,    // Teal/Blue
        0, 0, 32,     // Blue
        8, 0, 32,     // Blue/Purple
        16, 0, 32,    // Purple
        32, 0, 32,    // Fuchsia
        32, 0, 16,    // Fuchsia/Red
    };

    static const u8 midValues[48] = {
        128, 128, 128,    // While/Gray/Black
        255, 0, 0,        // Red
        255, 64, 0,       // Red/Orange
        255, 128, 0,      // Orange
        255, 192, 0,      // Orange/Yellow
        255, 255, 0,      // Yellow
        128, 255, 0,      // Lime
        0, 255, 0,        // Green
        0, 255, 128,      // Green/Teal
        0, 255, 255,      // Teal
        0, 128, 255,      // Teal/Blue
        0, 0, 255,        // Blue
        64, 0, 255,       // Blue/Purple
        128, 0, 255,      // Purple
        255, 0, 255,      // Fuchsia
        255, 0, 128,      // Fuchsia/Red
    };

    static const u8 maxValues[48] = {
        255, 255, 255,    // While/Gray/Black
        255, 224, 224,    // Red
        255, 224, 224,    // Red/Orange
        255, 240, 224,    // Orange
        255, 255, 224,    // Orange/Yellow
        255, 255, 224,    // Yellow
        240, 255, 224,    // Lime
        224, 255, 224,    // Green
        224, 255, 240,    // Green/Teal
        224, 255, 255,    // Teal
        224, 240, 255,    // Teal/Blue
        224, 224, 255,    // Blue
        240, 224, 255,    // Blue/Purple
        240, 224, 255,    // Purple
        255, 224, 255,    // Fuchsia
        255, 224, 240,    // Fuchsia/Red
    };

    uint  colorIndex = 0;
    float redStep, greenStep, blueStep;

    for (uint rowIndex = 0; rowIndex < 16; rowIndex++) {
        redStep   = (float) (midValues[rowIndex * 3] - minValues[rowIndex * 3]) / 7.0f;
        greenStep = (float) (midValues[rowIndex * 3 + 1] - minValues[rowIndex * 3 + 1]) / 7.0f;
        blueStep  = (float) (midValues[rowIndex * 3 + 2] - minValues[rowIndex * 3 + 2]) / 7.0f;

        for (uint columnIndex = 0; columnIndex < 8; columnIndex++) {
            colorPalette[colorIndex * 3]     = minValues[rowIndex * 3] + floor((float) columnIndex * redStep);
            colorPalette[colorIndex * 3 + 1] = minValues[rowIndex * 3 + 1] + floor((float) columnIndex * greenStep);
            colorPalette[colorIndex * 3 + 2] = minValues[rowIndex * 3 + 2] + floor((float) columnIndex * blueStep);
            colorIndex++;
        }

        redStep   = (float) (maxValues[rowIndex * 3] - midValues[rowIndex * 3]) / 8.0f;
        greenStep = (float) (maxValues[rowIndex * 3 + 1] - midValues[rowIndex * 3 + 1]) / 8.0f;
        blueStep  = (float) (maxValues[rowIndex * 3 + 2] - midValues[rowIndex * 3 + 2]) / 8.0f;

        for (uint columnIndex = 1; columnIndex < 9; columnIndex++) {
            colorPalette[colorIndex * 3]     = midValues[rowIndex * 3] + floor((float) columnIndex * redStep);
            colorPalette[colorIndex * 3 + 1] = midValues[rowIndex * 3 + 1] + floor((float) columnIndex * greenStep);
            colorPalette[colorIndex * 3 + 2] = midValues[rowIndex * 3 + 2] + floor((float) columnIndex * blueStep);
            colorIndex++;
        }
    }

    isColorPaletteBuilt = true;
}

static u8 getNearestColorIndex(const u8 redValue, const u8 greenValue, const u8 blueValue) {
    u8  nearestIndex    = 0;
    int nearestDistance = INT32_MAX;
    int colorDistance;

    int redDiff, greenDiff, blueDiff;

    for (uint colorIndex = 0; colorIndex < screenColors; ++colorIndex) {
        redDiff   = colorPalette[colorIndex * 3] - redValue;
        greenDiff = colorPalette[colorIndex * 3 + 1] - greenValue;
        blueDiff  = colorPalette[colorIndex * 3 + 2] - blueValue;

        colorDistance = (2 * redDiff * redDiff) + (4 * greenDiff * greenDiff) + (3 * blueDiff * blueDiff);

        if (colorDistance < nearestDistance) {
            nearestDistance = colorDistance;
            nearestIndex    = colorIndex;
        }
    }

    return nearestIndex;
}

static u8 clampU8(int value) {
    if (value < 0) {
        return 0;
    }

    if (value > 255) {
        return 255;
    }

    return value;
}

Image* CreateImage(const u16 width, const u16 height, const u8 bitsPerPixel) {
    if ((!width) || (!height) || (width > MaxImageWidth) || (height > MaxImageHeight)) {
        Error(logTag, "Invalid image dimensions: %dx%d", width, height);
        return NULL;
    }

    Image* image = malloc(sizeof(Image));

    if (!image) {
        Error(logTag, "Could not allocate memory to create the image");
        return NULL;
    }

    image->Width        = width;
    image->Height       = height;
    image->BitsPerPixel = bitsPerPixel;
    image->Data         = malloc(width * height * (bitsPerPixel / 8));

    if (!image->Data) {
        Error(logTag, "Could not allocate memory to hold the image data");
        free(image);
        return NULL;
    }

    return image;
}

void DestroyImage(Image* image) {
    if (!image) {
        return;
    }

    free(image->Data);
    free(image);
}

#define pixelAddress(x, y) (((y) * image->Width) + ((x) * 3))

void DitherImage(Image* image) {
    if (!image || image->BitsPerPixel != 24) {
        Error(logTag, "Invalid image pointer or bits per pixel");
        return;
    }

    if (!isColorPaletteBuilt) {
        buildColorPalette();
    }

    // Floyd-Steinberg Dithering

    u8  oldR, oldG, oldB, newR, newG, newB, nearestIndex;
    i16 rError, gError, bError;

    for (uint y = 0; y < image->Height - 1; ++y) {
        for (uint x = 1; x < image->Width - 1; ++x) {
            oldR = image->Data[pixelAddress(x, y)];
            oldG = image->Data[pixelAddress(x, y) + 1];
            oldB = image->Data[pixelAddress(x, y) + 2];

            nearestIndex = getNearestColorIndex(oldR, oldG, oldB);

            newR = colorPalette[nearestIndex * 3];
            newG = colorPalette[nearestIndex * 3 + 1];
            newB = colorPalette[nearestIndex * 3 + 2];

            image->Data[pixelAddress(x, y)]     = newR;
            image->Data[pixelAddress(x, y) + 1] = newG;
            image->Data[pixelAddress(x, y) + 2] = newB;

            rError = oldR - newR;
            gError = oldG - newG;
            bError = oldB - newB;

            image->Data[pixelAddress(x + 1, y)]     = clampU8(image->Data[pixelAddress(x + 1, y)] + rError * 0.4375f);
            image->Data[pixelAddress(x + 1, y) + 1] = clampU8(image->Data[pixelAddress(x + 1, y) + 1] + gError * 0.4375f);
            image->Data[pixelAddress(x + 1, y) + 2] = clampU8(image->Data[pixelAddress(x + 1, y) + 2] + bError * 0.4375f);

            image->Data[pixelAddress(x - 1, y + 1)]     = clampU8(image->Data[pixelAddress(x - 1, y + 1)] + rError * 0.1875f);
            image->Data[pixelAddress(x - 1, y + 1) + 1] = clampU8(image->Data[pixelAddress(x - 1, y + 1) + 1] + gError * 0.1875f);
            image->Data[pixelAddress(x - 1, y + 1) + 2] = clampU8(image->Data[pixelAddress(x - 1, y + 1) + 2] + bError * 0.1875f);

            image->Data[pixelAddress(x, y + 1)]     = clampU8(image->Data[pixelAddress(x, y + 1)] + rError * 0.3125f);
            image->Data[pixelAddress(x, y + 1) + 1] = clampU8(image->Data[pixelAddress(x, y + 1) + 1] + gError * 0.3125f);
            image->Data[pixelAddress(x, y + 1) + 2] = clampU8(image->Data[pixelAddress(x, y + 1) + 2] + bError * 0.3125f);

            image->Data[pixelAddress(x + 1, y + 1)]     = clampU8(image->Data[pixelAddress(x + 1, y + 1)] + rError * 0.0625f);
            image->Data[pixelAddress(x + 1, y + 1) + 1] = clampU8(image->Data[pixelAddress(x + 1, y + 1) + 1] + gError * 0.0625f);
            image->Data[pixelAddress(x + 1, y + 1) + 2] = clampU8(image->Data[pixelAddress(x + 1, y + 1) + 2] + bError * 0.0625f);
        }
    }
}

Image* GetIndexedImage(Image* image) {
    if (!image || image->BitsPerPixel != 24) {
        Error(logTag, "Invalid image pointer or bits per pixel");
        return NULL;
    }

    if (!isColorPaletteBuilt) {
        buildColorPalette();
    }

    Image* indexedImage = CreateImage(image->Width, image->Height, 8);

    if (!indexedImage) {
        return NULL;
    }

    uint pixelCount = image->Width * image->Height;

    for (uint colorIndex = 0; colorIndex < pixelCount; ++colorIndex) {
        indexedImage->Data[colorIndex] = getNearestColorIndex(image->Data[colorIndex * 3], image->Data[colorIndex * 3 + 1], image->Data[colorIndex * 3 + 2]);
    }

    return indexedImage;
}