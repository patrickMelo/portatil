//
// Tools/Tools.h
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#ifndef PORTATIL_TOOLS_H
#define PORTATIL_TOOLS_H

#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Types ----------------------------------------------------------------------

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef u8       byte;

typedef float  f32;
typedef double f64;

typedef char* string;

#define FourCC(a, b, c, d) ((u32) (((d) << 24) | ((c) << 16) | ((b) << 8) | (a)))

#define PrintUsageReturnCode 255

// Debug ----------------------------------------------------------------------

void Info(const string tag, const string message, ...);
void Warning(const string tag, const string message, ...);
void Error(const string tag, const string message, ...);
void Debug(const string tag, const string message, ...);

// File I/O -------------------------------------------------------------------

#define MaxFiles      1024
#define MaxPathLength 4096

typedef enum FileMode {
    FileReadMode,
    FileWriteMode,
} FileMode;

typedef struct File {
        FILE*    Native;
        FileMode Mode;
        u64      Offset;
        u64      Size;
} File;

File* OpenFile(const string filePath);
File* CreateFile(const string filePath);
bool  ReadFile(File* file, const u64 readSize, byte* readOutput);
bool  WriteFile(File* file, const byte* data, const u64 writeSize);
void  CloseFile(File* file);

byte* QuickReadFile(const string filePath, u64* fileSize);
bool  QuickWriteFile(const string filePath, const byte* data, const u64 dataSize);

typedef struct FileList {
        uint NumberOfFiles;
        char Path[MaxFiles][MaxPathLength];
} FileList;

bool ListFiles(const string directoryPath, FileList* list);

#endif    // PORTATIL_TOOLS_H