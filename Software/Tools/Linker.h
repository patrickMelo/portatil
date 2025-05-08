//
// Tools/Linker.h
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#ifndef PORTATIL_LINKER_H
#define PORTATIL_LINKER_H

#include "Tools.h"

// Linker ---------------------------------------------------------------------

int RunLinker(const int numberOfArguments, const string* argumentsValues);

// Programs -------------------------------------------------------------------

#define MaxProgramSize 65536    // 64K

typedef struct Program {
        u32 Size;
        u32 EntrypointAddress;
        u32 MemoryOffset;
        u8* Data;
} Program;

Program* CreateProgram(void);
void     DestroyProgram(Program* program);
bool     SaveProgram(const string filePath, Program* program);

Program* LoadELF(const string filePath);

#endif    // PORTATIL_PACKER_H