//
// Tools/Linker/Programs.ELF.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../Linker.h"

#define logTag "Linker:Programs:ELF"

#define elfMagicNumber FourCC(0x7F, 'E', 'L', 'F')

#define elf32Bit        1
#define elfLittleEndian 1
#define elfABISystemV   0
#define elfExecutable   2
#define elfRiscV        0xF3
#define elfLoad         0x01

typedef struct __attribute((packed)) elfFileHeader {
        u32 magicNumber;
        u8  bitType;
        u8  endianType;
        u8  fileVersion;
        u8  osABI;
        u8  abiVersion;
        u8  padding[7];
        u16 fileType;
        u16 machineType;
        u32 elfVersion;
        u32 entrypointAddress;
        u32 programHeaderOffset;
        u32 sectionHeaderOffset;
        u32 flags;
        u16 fileHeaderSize;
        u16 programHeaderEntrySize;
        u16 programHeaderEntries;
        u16 sectionHeaderEntrySize;
        u16 sectionHeaderEntries;
        u16 sectionNamesIndex;
} elfFileHeader;

typedef struct __attribute((packed)) elfProgramHeader {
        u32 type;
        u32 offset;
        u32 virtualAddress;
        u32 physicalAddress;
        u32 size;
        u32 memorySize;
        u32 flags;
        u32 alignment;
} elfProgramHeader;

Program* LoadELF(const string filePath) {
    static elfFileHeader    fileHeader;
    static elfProgramHeader programHeader;

    u64 fileSize;
    u8* fileData = QuickReadFile(filePath, &fileSize);

    if (!fileData) {
        return NULL;
    }

    memcpy(&fileHeader, fileData, sizeof(fileHeader));

    if (fileHeader.magicNumber != elfMagicNumber) {
        Error(logTag, "Invalid magic number: 0x%08x", fileHeader.magicNumber);
        free(fileData);
        return NULL;
    }

    if (fileHeader.bitType != elf32Bit) {
        Error(logTag, "Only 32-bit programs are supported");
        free(fileData);
        return NULL;
    }

    if (fileHeader.endianType != elfLittleEndian) {
        Error(logTag, "Only little-endian programs are supported");
        free(fileData);
        return NULL;
    }

    if (fileHeader.osABI != elfABISystemV) {
        Error(logTag, "Unsupported ABI: 0x%02x", fileHeader.osABI);
        free(fileData);
        return NULL;
    }

    if (fileHeader.fileType != elfExecutable) {
        Error(logTag, "Unsupported file type: 0x%04x", fileHeader.fileType);
        free(fileData);
        return NULL;
    }

    if (fileHeader.machineType != elfRiscV) {
        Error(logTag, "Unsupported machine type: 0x%04x", fileHeader.machineType);
        free(fileData);
        return NULL;
    }

    u32 programOffset = fileHeader.programHeaderOffset;

    Program* program = CreateProgram();

    if (!program) {
        free(fileData);
        return NULL;
    }

    program->EntrypointAddress = fileHeader.entrypointAddress;
    program->MemoryOffset      = 0;
    program->Size              = 0;

    u32 offsetAddress = 0;

    for (u32 entryIndex = 0; entryIndex < fileHeader.programHeaderEntries; entryIndex++) {
        memcpy(&programHeader, &fileData[programOffset], fileHeader.programHeaderEntrySize);
        programOffset += fileHeader.programHeaderEntrySize;

        if (programHeader.type != elfLoad) {
            continue;
        }

        if (program->MemoryOffset == 0 && programHeader.physicalAddress != 0) {
            program->MemoryOffset = programHeader.physicalAddress;
        }

        offsetAddress = programHeader.physicalAddress - program->MemoryOffset;

        if (offsetAddress + programHeader.size > MaxProgramSize) {
            Error(logTag, "ELF program code is too big to be loaded into program code space ( %d bytes)", programHeader.size);
            DestroyProgram(program);
            free(fileData);
            return NULL;
        }

        memcpy(&program->Data[offsetAddress], &fileData[programHeader.offset], programHeader.size);
        program->Size += programHeader.size;
    }

    free(fileData);

    Debug(logTag, "Program read from \"%s\"", filePath);
    return program;
}
