//
// Runtime/Drivers/Display/Display.SDL.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../../Drivers.h"

#include <math.h>
#include <SDL2/SDL.h>

// Display --------------------------------------------------------------------

#define displayWidth  320
#define displayHeight 240
#define windowScale   3

static SDL_Window*  sdlWindow        = NULL;
static SDL_Surface* sdlWindowSurface = NULL;
static SDL_Surface* sdlBlitSurface   = NULL;
static SDL_Rect     sdlWindowRect    = {0, 0, displayWidth* windowScale, displayHeight* windowScale};

// Driver ---------------------------------------------------------------------

bool DrvDisplayInitialize(void) {
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
        return false;
    }

    sdlWindow = SDL_CreateWindow("Portatil Desktop", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, displayWidth * windowScale, displayHeight * windowScale, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    if (!sdlWindow) {
        return false;
    }

    sdlWindowSurface = SDL_GetWindowSurface(sdlWindow);

    if (!sdlWindowSurface) {
        SDL_DestroyWindow(sdlWindow);
        sdlWindow = NULL;
        return false;
    }

    sdlBlitSurface = SDL_CreateRGBSurface(0, ScreenWidth, ScreenHeight, 24, 0, 0, 0, 0);

    if (!sdlBlitSurface) {
        SDL_DestroyWindow(sdlWindow);
        sdlWindow        = NULL;
        sdlWindowSurface = NULL;
        return false;
    }

    SDL_FillRect(sdlBlitSurface, NULL, 0);
    return true;
}

void DrvDisplayFinalize(void) {
    if (sdlBlitSurface) {
        SDL_FreeSurface(sdlBlitSurface);
    }

    if (sdlWindow) {
        SDL_DestroyWindow(sdlWindow);
    }

    SDL_QuitSubSystem(SDL_INIT_VIDEO);

    sdlBlitSurface   = NULL;
    sdlWindowSurface = NULL;
    sdlWindow        = NULL;
}

void DrvDisplaySync(const u8* framebufferData, const u8* colorPalette) {
    SDL_LockSurface(sdlBlitSurface);

    u8* surfacePixels = sdlBlitSurface->pixels;
    u8  colorIndex;

    u32 sourceOffset, targetOffset;

    for (u16 pixelY = 0; pixelY < ScreenHeight; pixelY++) {
        for (u16 pixelX = 0; pixelX < ScreenWidth; pixelX++) {
            sourceOffset = (pixelY * ScreenWidth) + pixelX;
            colorIndex   = framebufferData[sourceOffset];

            targetOffset = (pixelY * sdlBlitSurface->pitch) + (pixelX * 3);

            surfacePixels[targetOffset]     = colorPalette[colorIndex * 3 + 2];
            surfacePixels[targetOffset + 1] = colorPalette[colorIndex * 3 + 1];
            surfacePixels[targetOffset + 2] = colorPalette[colorIndex * 3];
        }
    }

    SDL_UnlockSurface(sdlBlitSurface);

    SDL_BlitScaled(sdlBlitSurface, NULL, sdlWindowSurface, &sdlWindowRect);
    SDL_UpdateWindowSurface(sdlWindow);
}
