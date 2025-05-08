//
// Runtime/VM.h
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#ifndef PORTATIL_VM_H
#define PORTATIL_VM_H

#include "Kernel.h"

// Virtual Machine ------------------------------------------------------------

#define VirtualMachineMemorySize 65536    // 64K

bool   InitializeVirtualMachine(void);
bool   SyncVirtualMachine(const f16 speedMultiplier);
string GetVirtualMachineError(void);

// Programs -------------------------------------------------------------------

#define MaxProgramSize 65536    // 64K

bool LoadProgramFromStorage(void);

#endif    // PORTATIL_VM_H