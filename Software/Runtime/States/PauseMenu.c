//
// Runtime/States/PauseMenu.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../States.h"

void PauseMenuState(const u64 frameTime) {
    if (IsButtonJustPressed(ButtonB)) {
        StopAllSound();
        ChangeState(ShellState);
        return;
    }

    if (IsButtonJustPressed(ButtonA)) {
        PauseAllSound(false);
        ChangeState(InGameState);
        return;
    }

    DrawShellOverlay("Game Paused", "(B) Exit", "Resume (A)");
}
