//
// Tools/Linker/Linker.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../Linker.h"

#define logTag "Linker"

int RunLinker(const int numberOfArguments, const string* argumentsValues) {
    if (numberOfArguments != 2) {
        return PrintUsageReturnCode;
    }

    const string elfProgram    = argumentsValues[0];
    const string outputProgram = argumentsValues[1];

    Info(logTag, "Linking program from \"%s\" to \"%s\"", elfProgram, outputProgram);

    Program* loadedProgram = LoadELF(elfProgram);

    if (!loadedProgram) {
        return 1;
    }

    if (!SaveProgram(outputProgram, loadedProgram)) {
        return 1;
    }

    Info(logTag, "Done");
    return 0;
}