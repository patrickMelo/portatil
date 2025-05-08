//
// Runtime/Drivers/GPU/GPU.Generic.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../../Drivers.h"

// GPU ------------------------------------------------------------------------

static u8  framebuffer[ScreenPixels];
static u8  colorPalette[ScreenColors * 3];
static u16 transparentColor = ColorNone;
static u16 backgroundColor  = ColorNone;
static u16 foregroundColor  = ColorNone;

static void buildColorPalette(void) {
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

    static float redStep, greenStep, blueStep;

    uint colorIndex = 0;

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
}

// Driver ---------------------------------------------------------------------

bool DrvGpuInitialize(void) {
    buildColorPalette();
    memset(&framebuffer, 0, sizeof(framebuffer));
    return true;
}

void DrvGpuFinalize(void) {
    // Empty
}

void DrvGpuClear(const u8 colorIndex) {
    for (uint pixelIndex = 0; pixelIndex < ScreenPixels; pixelIndex++) {
        framebuffer[pixelIndex] = colorIndex;
    }
}

void DrvGpuSync(void) {
    DrvDisplaySync(framebuffer, colorPalette);
}

void DrvGpuSetTransparentColor(const u16 colorIndex) {
    transparentColor = colorIndex;
}

void DrvGpuSetBackgroundColor(const u16 colorIndex) {
    backgroundColor = colorIndex;
}

void DrvGpuSetForegroundColor(const u16 colorIndex) {
    foregroundColor = colorIndex;
}

u8 DrvGpuGetNearestColorIndex(const u8 redValue, const u8 greenValue, const u8 blueValue) {
    u8  nearestIndex    = 0;
    int nearestDistance = INT32_MAX;

    static int colorDistance;
    static int redDiff, greenDiff, blueDiff;

    for (uint colorIndex = 0; colorIndex < ScreenColors; ++colorIndex) {
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

void DrvGpuDraw(const Image* image, const Point2D* position, const Rectangle2D* clipRect) {
    Rectangle2D offsetTargetRect = {
        .X      = position->X,
        .Y      = position->Y,
        .Width  = clipRect->Width,
        .Height = clipRect->Height,
    };

    Rectangle2D offsetClipRect = *clipRect;

    if ((offsetTargetRect.X > ScreenWidth) || (offsetTargetRect.Y > ScreenHeight) ||
        (offsetTargetRect.X + offsetTargetRect.Width < 0) || (offsetTargetRect.Y + offsetTargetRect.Height < 0)) {
        return;
    }

    if (offsetTargetRect.X < 0) {
        offsetClipRect.X -= offsetTargetRect.X;
        offsetClipRect.Width += offsetTargetRect.X;

        offsetTargetRect.Width += offsetTargetRect.X;
        offsetTargetRect.X = 0;
    }

    if (offsetTargetRect.X + offsetTargetRect.Width > ScreenWidth) {
        offsetTargetRect.Width -= (offsetTargetRect.X + offsetTargetRect.Width) - ScreenWidth;
        offsetClipRect.Width = offsetTargetRect.Width;
    }

    if (offsetTargetRect.Y < 0) {
        offsetClipRect.Y -= offsetTargetRect.Y;
        offsetClipRect.Height += offsetTargetRect.Y;

        offsetTargetRect.Height += offsetTargetRect.Y;
        offsetTargetRect.Y = 0;
    }

    if (offsetTargetRect.Y + offsetTargetRect.Height > ScreenHeight) {
        offsetTargetRect.Height -= (offsetTargetRect.Y + offsetTargetRect.Height) - ScreenHeight;
        offsetClipRect.Height = offsetTargetRect.Height;
    }

    static u32 sourcePixelIndex, targetPixelIndex;
    static u8  pixelColor;

    for (u16 pixelY = 0; pixelY < offsetTargetRect.Height; pixelY++) {
        for (u16 pixelX = 0; pixelX < offsetTargetRect.Width; pixelX++) {
            sourcePixelIndex = ((offsetClipRect.Y + pixelY) * image->Width) + (offsetClipRect.X + pixelX);
            pixelColor       = image->Data[sourcePixelIndex];

            if (pixelColor == transparentColor) {
                if (backgroundColor == ColorNone) {
                    continue;
                }

                pixelColor = backgroundColor;
            } else if (foregroundColor != ColorNone) {
                pixelColor = foregroundColor;
            }

            targetPixelIndex              = ((offsetTargetRect.Y + pixelY) * ScreenWidth) + (offsetTargetRect.X + pixelX);
            framebuffer[targetPixelIndex] = pixelColor;
        }
    }
}

void DrvGpuDrawScaled(const Image* image, const Rectangle2D* sourceRect, const Rectangle2D* targetRect) {
    Rectangle2D offsetSourceRect = *sourceRect;
    Rectangle2D offsetTargetRect = *targetRect;

    if ((offsetTargetRect.X > ScreenWidth) || (offsetTargetRect.Y > ScreenHeight) ||
        (offsetTargetRect.X + offsetTargetRect.Width < 0) || (offsetTargetRect.Y + offsetTargetRect.Height < 0)) {
        return;
    }

    static i16 offsetDifference;

    f16 sourcePixelWidth  = F16Div(F16(sourceRect->Width), F16(targetRect->Width));
    f16 sourcePixelHeight = F16Div(F16(sourceRect->Height), F16(targetRect->Height));

    if (offsetTargetRect.X < 0) {
        offsetDifference = F16ToInt(F16Mult(F16(offsetTargetRect.X), sourcePixelWidth));
        offsetSourceRect.X -= offsetDifference;
        offsetSourceRect.Width += offsetDifference;

        offsetTargetRect.Width += offsetTargetRect.X;
        offsetTargetRect.X = 0;
    }

    if (offsetTargetRect.X + offsetTargetRect.Width > ScreenWidth) {
        offsetTargetRect.Width -= (offsetTargetRect.X + offsetTargetRect.Width) - ScreenWidth;
        offsetSourceRect.Width = F16ToInt(F16Mult(F16(offsetTargetRect.Width), sourcePixelWidth));
    }

    if (offsetTargetRect.Y < 0) {
        offsetDifference = F16ToInt(F16Mult(F16(offsetTargetRect.Y), sourcePixelHeight));
        offsetSourceRect.Y -= offsetDifference;
        offsetSourceRect.Height += offsetDifference;

        offsetTargetRect.Height += offsetTargetRect.Y;
        offsetTargetRect.Y = 0;
    }

    if (offsetTargetRect.Y + offsetTargetRect.Height > ScreenHeight) {
        offsetTargetRect.Height -= (offsetTargetRect.Y + offsetTargetRect.Height) - ScreenHeight;
        offsetSourceRect.Height = F16ToInt(F16Mult(F16(offsetTargetRect.Height), sourcePixelHeight));
    }

    static u16 sourcePixelX, sourcePixelY;
    static u32 sourcePixelIndex, targetPixelIndex;
    static u8  pixelColor;

    for (u16 pixelY = 0; pixelY < offsetTargetRect.Height; pixelY++) {
        for (u16 pixelX = 0; pixelX < offsetTargetRect.Width; pixelX++) {
            sourcePixelX = offsetSourceRect.X + F16ToInt(F16Mult(F16(pixelX), sourcePixelWidth));
            sourcePixelY = offsetSourceRect.Y + F16ToInt(F16Mult(F16(pixelY), sourcePixelHeight));

            sourcePixelIndex = (sourcePixelY * image->Width) + sourcePixelX;
            pixelColor       = image->Data[sourcePixelIndex];

            if (pixelColor == transparentColor) {
                if (backgroundColor == ColorNone) {
                    continue;
                }

                pixelColor = backgroundColor;
            } else if (foregroundColor != ColorNone) {
                pixelColor = foregroundColor;
            }

            targetPixelIndex              = ((offsetTargetRect.Y + pixelY) * ScreenWidth) + (offsetTargetRect.X + pixelX);
            framebuffer[targetPixelIndex] = pixelColor;
        }
    }
}

void DrvGpuDrawRectangle(const Rectangle2D* rectangle, const u8 colorIndex) {
    Rectangle2D offsetRectangle = *rectangle;

    if ((offsetRectangle.X > ScreenWidth) || (offsetRectangle.Y > ScreenHeight) ||
        (offsetRectangle.X + offsetRectangle.Width < 0) || (offsetRectangle.Y + offsetRectangle.Height < 0)) {
        return;
    }

    if (offsetRectangle.X < 0) {
        offsetRectangle.Width += offsetRectangle.X;
        offsetRectangle.X = 0;
    }

    if (offsetRectangle.X + offsetRectangle.Width > ScreenWidth) {
        offsetRectangle.Width -= (offsetRectangle.X + offsetRectangle.Width) - ScreenWidth;
    }

    if (offsetRectangle.Y < 0) {
        offsetRectangle.Height += offsetRectangle.Y;
        offsetRectangle.Y = 0;
    }

    if (offsetRectangle.Y + offsetRectangle.Height > ScreenHeight) {
        offsetRectangle.Height -= (offsetRectangle.Y + offsetRectangle.Height) - ScreenHeight;
    }

    static u32 pixelIndex;

    for (u16 pixelY = 0; pixelY < offsetRectangle.Height; pixelY++) {
        for (u16 pixelX = 0; pixelX < offsetRectangle.Width; pixelX++) {
            pixelIndex              = ((offsetRectangle.Y + pixelY) * ScreenWidth) + (offsetRectangle.X + pixelX);
            framebuffer[pixelIndex] = colorIndex;
        }
    }
}