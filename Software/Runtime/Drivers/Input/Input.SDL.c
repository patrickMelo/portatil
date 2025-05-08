//
// Runtime/Drivers/Input/Input.SDL.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../../Drivers.h"

#include <SDL2/SDL.h>

// Input ----------------------------------------------------------------------

static u8 currentState = 0;

// Driver ---------------------------------------------------------------------

bool DrvInputInitialize(void) {
    currentState = 0;
    return true;
}

void DrvInputFinalize(void) {
    // Empty
}

u8 DrvInputSync(void) {
    static SDL_Event sdlEvent;

    while (SDL_PollEvent(&sdlEvent)) {
        switch (sdlEvent.type) {
            case SDL_QUIT: {
                Shutdown();
                return 0;
            }

            case SDL_KEYDOWN:
                switch (sdlEvent.key.keysym.sym) {
                    case SDLK_w: {
                        currentState |= ButtonUp;
                        break;
                    }

                    case SDLK_s: {
                        currentState |= ButtonDown;
                        break;
                    }

                    case SDLK_a: {
                        currentState |= ButtonLeft;
                        break;
                    }

                    case SDLK_d: {
                        currentState |= ButtonRight;
                        break;
                    }

                    case SDLK_k: {
                        currentState |= ButtonB;
                        break;
                    }

                    case SDLK_l: {
                        currentState |= ButtonA;
                        break;
                    }

                    case SDLK_i: {
                        currentState |= ButtonY;
                        break;
                    }

                    case SDLK_o: {
                        currentState |= ButtonX;
                        break;
                    }
                }

                break;

            case SDL_KEYUP:
                switch (sdlEvent.key.keysym.sym) {
                    case SDLK_w: {
                        currentState &= ~ButtonUp;
                        break;
                    }

                    case SDLK_s: {
                        currentState &= ~ButtonDown;
                        break;
                    }

                    case SDLK_a: {
                        currentState &= ~ButtonLeft;
                        break;
                    }

                    case SDLK_d: {
                        currentState &= ~ButtonRight;
                        break;
                    }

                    case SDLK_k: {
                        currentState &= ~ButtonB;
                        break;
                    }

                    case SDLK_l: {
                        currentState &= ~ButtonA;
                        break;
                    }

                    case SDLK_i: {
                        currentState &= ~ButtonY;
                        break;
                    }

                    case SDLK_o: {
                        currentState &= ~ButtonX;
                        break;
                    }
                }
                break;

            default:
                break;
        }
    }

    return currentState;
}
