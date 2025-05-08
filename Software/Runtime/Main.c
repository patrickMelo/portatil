//
// Runtime/Main.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "Engine.h"
#include "Kernel.h"
#include "States.h"
#include "VM.h"

#define bootSplashTime 1000000

static bool bootSplashDrawn = false;

static void bootFunction(const u64 frameTime) {
    if (!bootSplashDrawn) {
        bootSplashDrawn = true;

        ClearScreen(0);
        SetTransparentColor(0);
        SetDrawAnchor(AnchorMiddle | AnchorCenter);
        DrawText(GetDefaultFont(), ScreenWidth / 2, ScreenHeight / 2, "Portatil");

        return;
    }

    Sleep(bootSplashTime);

    InitializeError();
    InitializeShell();
    InitializeInGame();
    InitializeVirtualMachine();
    InitializeEngine();

    ChangeState(ShellState);
}

int main(const int numberOfArguments, const string* argumentsValues) {
    return Boot(bootFunction) ? 0 : 1;
}