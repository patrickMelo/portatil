//
// Tools/Debug.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "Tools.h"

#include <stdarg.h>

#define printBufferSize 2048

static char   printBuffer[printBufferSize + 1];
static string messageFormats[4] = {
    "\033[1;32m(I) [%s] %s\033[0m\n",
    "\033[1;33m(W) [%s] %s\033[0m\n",
    "\033[1;31m(E) [%s] %s\033[0m\n",
    "\033[1;35m(D) [%s] %s\033[0m\n",
};

#define printMessage(formatIndex)                               \
    va_list argsList;                                           \
    va_start(argsList, message);                                \
    vsnprintf(printBuffer, printBufferSize, message, argsList); \
    va_end(argsList);                                           \
    printf(messageFormats[formatIndex], tag, printBuffer);

void Info(const string tag, const string message, ...) {
    printMessage(0);
}

void Warning(const string tag, const string message, ...) {
    printMessage(1);
}

void Error(const string tag, const string message, ...) {
    printMessage(2);
}

void Debug(const string tag, const string message, ...) {
    printMessage(3);
}
