//
// Tools/Tools.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "Tools.h"

#include "Linker.h"
#include "Packer.h"

#define logTag "Tools"

typedef int (*toolMainFunction)(const int numberOfArguments, const string* argumentsValues);

typedef struct toolInfo {
        const string     runCommand;
        const string     usageHelp;
        toolMainFunction mainFunction;
} toolInfo;

#define numberOfAvailableTools 2

const toolInfo availableTools[numberOfAvailableTools] = {
    {
     .runCommand   = "packer",
     .usageHelp    = "<assets directory> <output directory> <sdk header path>",
     .mainFunction = RunPacker,
     },
    {
     .runCommand   = "linker",
     .usageHelp    = "<elf program file> <output program file>",
     .mainFunction = RunLinker,
     },
};

void printUsage(const string exeName) {
    Info(logTag, "Usage: %s <tool> <parameters>", exeName);
    Info(logTag, "");
    Info(logTag, "Available tools:");
    Info(logTag, "");

    for (uint toolIndex = 0; toolIndex < numberOfAvailableTools; toolIndex++) {
        Info(logTag, "%s %s", availableTools[toolIndex].runCommand, availableTools[toolIndex].usageHelp);
    }

    Info(logTag, "");
}

int main(const int numberOfArguments, const string* argumentsValues) {
    if (numberOfArguments < 2) {
        printUsage(argumentsValues[0]);
        return 1;
    }

    string requestedTool      = argumentsValues[1];
    int    requestedToolIndex = -1;

    for (uint toolIndex = 0; toolIndex < numberOfAvailableTools; toolIndex++) {
        if (strncmp(requestedTool, availableTools[toolIndex].runCommand, strlen(availableTools[toolIndex].runCommand)) == 0) {
            requestedToolIndex = toolIndex;
            break;
        }
    }

    if (requestedToolIndex < 0) {
        printUsage(argumentsValues[0]);
        return 1;
    }

    int toolReturnCode = availableTools[requestedToolIndex].mainFunction(numberOfArguments - 2, &argumentsValues[2]);

    if (toolReturnCode == PrintUsageReturnCode) {
        printUsage(argumentsValues[0]);
        return 1;
    }

    return toolReturnCode;
}