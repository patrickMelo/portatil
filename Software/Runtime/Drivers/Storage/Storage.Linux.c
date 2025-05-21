//
// Runtime/Drivers/Storage/Storage.Linux.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../../Drivers.h"

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

// Storage --------------------------------------------------------------------

static char  rootDirectoryPath[PATH_MAX + 1];
static DIR*  currentDirectory = NULL;
static FILE* currentFile      = NULL;
static u32   currentFileSize  = 0;
static u64   busyTime         = 0;

static inline void getAbsolutePath(string pathBuffer, string relativePath) {
    strncpy(pathBuffer, rootDirectoryPath, PATH_MAX);
    strcat(pathBuffer, "/");
    strncat(pathBuffer, relativePath, 64);
}

#define startTimer() u64 startTime = DrvCpuGetTick()
#define stopTimer()  busyTime += DrvCpuGetTick() - startTime

// Driver ---------------------------------------------------------------------

bool DrvStorageInitialize(void) {
    busyTime = 0;
    return getcwd(rootDirectoryPath, PATH_MAX) != NULL;
}

void DrvStorageFinalize(void) {
    DrvStorageCloseFile();
    DrvStorageCloseDirectory();
}

bool DrvStorageOpenDirectory(const string directoryPath) {
    DrvStorageCloseDirectory();

    startTimer();

    static char absolutePath[PATH_MAX + 1];
    getAbsolutePath(absolutePath, directoryPath);
    currentDirectory = opendir(absolutePath);

    stopTimer();
    return currentDirectory != NULL;
}

bool DrvStorageReadDirectory(StorageEntryInfo* entryInfo) {
    if (!currentDirectory) {
        return false;
    }

    startTimer();

    static char extensionBuffer[5];
    static u16  nameLength;

    struct dirent* entry = readdir(currentDirectory);

    while (entry) {
        if ((strncmp(entry->d_name, ".", 1) == 0) || (strncmp(entry->d_name, "..", 2) == 0)) {
            entry = readdir(currentDirectory);
            continue;
        }

        entryInfo->Flags = 0;

        switch (entry->d_type) {
            case DT_DIR: {
                strncpy(entryInfo->Name, entry->d_name, StorageMaxNameLength);
                entryInfo->Flags |= StorageEntryDirectoryFlag;
                stopTimer();
                return true;
            }

            case DT_REG: {
                strncpy(entryInfo->Name, entry->d_name, StorageMaxNameLength);
                nameLength = strnlen(entry->d_name, StorageMaxNameLength);

                if (nameLength > 4) {
                    strncpy(extensionBuffer, &entry->d_name[nameLength - 4], 4);

                    if (strncasecmp(extensionBuffer, ".rvp", 4) == 0) {
                        entryInfo->Flags |= StorageEntryProgramFlag;
                    }
                }

                stopTimer();
                return true;
            }
        }

        entry = readdir(currentDirectory);
    }

    stopTimer();
    return false;
}

void DrvStorageCloseDirectory(void) {
    if (!currentDirectory) {
        return;
    }

    closedir(currentDirectory);
    currentDirectory = NULL;
}

bool DrvStorageOpenFile(const string filePath) {
    DrvStorageCloseFile();

    startTimer();

    static char absolutePath[PATH_MAX + 1];
    getAbsolutePath(absolutePath, filePath);

    currentFile = fopen(absolutePath, "rb");

    if (currentFile) {
        fseek(currentFile, 0, SEEK_END);
        currentFileSize = ftell(currentFile);
        fseek(currentFile, 0, SEEK_SET);
    } else {
        currentFileSize = 0;
    }

    stopTimer();
    return currentFile != NULL;
}

u32 DrvStorageGetFileSize(void) {
    return currentFileSize;
}

bool DrvStorageReadFile(void* readBuffer, const u32 readSize) {
    if (!currentFile) {
        return false;
    }

    startTimer();
    bool readResult = fread(readBuffer, readSize, 1, currentFile) == 1;
    stopTimer();

    return readResult;
}

void DrvStorageCloseFile(void) {
    if (!currentFile) {
        return;
    }

    fclose(currentFile);

    currentFile     = NULL;
    currentFileSize = 0;
}

void DrvStorageResetTime(void) {
    busyTime = 0;
}

u64 DrvStorageGetTime(void) {
    return busyTime;
}