//
// Tools/Linker/Programs.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../Linker.h"

#define logTag "Linker:Programs"

#define programMagicNumber   FourCC('P', 'V', 'M', 'P')
#define programVersionNumber 1

typedef struct __attribute((packed)) programFileHeader {
        u32 magicNumber;
        u16 versionNumber;
        u32 programSize;
        u32 entrypointAddress;
        u32 memoryOffset;
} programFileHeader;

Program* CreateProgram(void) {
    Program* program = malloc(sizeof(Program));

    if (!program) {
        Error(logTag, "Could not allocate memory to create the program");
        return NULL;
    }

    program->Data = malloc(MaxProgramSize);

    if (!program->Data) {
        Error(logTag, "Could not allocate memory to hold the program data");
        free(program);
        return NULL;
    }

    program->EntrypointAddress = 0;
    program->MemoryOffset      = 0;
    program->Size              = 0;

    return program;
}

void DestroyProgram(Program* program) {
    if (!program) {
        return;
    }

    free(program->Data);
    free(program);
}

bool SaveProgram(const string filePath, Program* program) {
    if (!filePath || !program) {
        Error(logTag, "Invalid program pointer or file path");
        return false;
    }

    programFileHeader fileHeader = {
        .magicNumber       = programMagicNumber,
        .entrypointAddress = program->EntrypointAddress,
        .memoryOffset      = program->MemoryOffset,
        .programSize       = program->Size,
        .versionNumber     = programVersionNumber,
    };

    Debug(logTag,
          "File header: 0x%08x, 0x%08x, 0x%08x, %d, %d",
          fileHeader.magicNumber,
          fileHeader.entrypointAddress,
          fileHeader.memoryOffset,
          fileHeader.programSize,
          fileHeader.versionNumber);

    File* programFile = CreateFile(filePath);

    if (!programFile) {
        Error(logTag, "Could not create the file to save to program to");
        return false;
    }

    if (!WriteFile(programFile, (const byte*) &fileHeader, sizeof(programFileHeader))) {
        Error(logTag, "Could write the program header to the file");
        CloseFile(programFile);
        return false;
    }

    if (!WriteFile(programFile, program->Data, program->Size)) {
        Error(logTag, "Could write the program data to the file");
        CloseFile(programFile);
        return false;
    }

    CloseFile(programFile);

    Info(logTag, "Program saved to %s", filePath);
    return true;
}