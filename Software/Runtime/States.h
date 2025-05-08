//
// Runtime/States.h
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#ifndef PORTATIL_STATES_H
#define PORTATIL_STATES_H

#include "Assets.h"
#include "Engine.h"
#include "Kernel.h"
#include "VM.h"

// Error ----------------------------------------------------------------------

void InitializeError(void);
void ShowError(KernelFunction nextStateFunction, const string message);

void ErrorState(const u64 frameTime);

// Shell ----------------------------------------------------------------------

void InitializeShell(void);
void ShellState(const u64 frameTime);
void DrawShellOverlay(const string title, const string leftOption, const string rightOption);

// In Game --------------------------------------------------------------------

void InitializeInGame(void);
void InGameState(const u64 frameTime);

// Pause Menu -----------------------------------------------------------------

void PauseMenuState(const u64 frameTime);

#endif    // define PORTATIL_STATES_H