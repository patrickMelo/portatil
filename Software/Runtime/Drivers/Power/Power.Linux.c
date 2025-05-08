//
// Runtime/Drivers/Power/Power.Linux.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../../Drivers.h"

#include <dirent.h>
#include <linux/limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Power ----------------------------------------------------------------------

#define classPath       "/sys/class/power_supply"
#define batteryTypeName "Battery"
#define readBufferSize  512

static char pathBuffer[PATH_MAX];
static char readBuffer[readBufferSize + 1];
static char capacityPath[PATH_MAX];
static bool batteryFound = false;

static bool readFile(const string filePath) {
    FILE* file = fopen(filePath, "rb");

    if (!file) {
        return false;
    }

    fseek(file, 0, SEEK_END);
    u16 fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fileSize > readBufferSize) {
        fileSize = readBufferSize;
    }

    u16 readSize = fread(readBuffer, 1, fileSize, file);
    fclose(file);

    if (readSize > 0) {
        readBuffer[readSize - 1] = 0;
    }

    return readSize > 0;
}

static inline bool isBatteryDevice(const string deviceName) {
    sprintf(pathBuffer, "%s/%s/type", classPath, deviceName);

    if (!readFile(pathBuffer)) {
        return false;
    }

    return strcmp(readBuffer, batteryTypeName) == 0;
}

// Driver ---------------------------------------------------------------------

bool DrvPowerInitialize(void) {
    DIR*           directory = opendir(classPath);
    struct dirent* entry;

    while ((entry = readdir(directory))) {
        if ((strcmp(entry->d_name, ".") == 0) ||
            (strcmp(entry->d_name, "..") == 0)) {
            continue;
        }

        if (isBatteryDevice(entry->d_name)) {
            sprintf(capacityPath, "%s/%s/capacity", classPath, entry->d_name);
            batteryFound = true;
            break;
        }
    }

    closedir(directory);
    return true;
}

void DrvPowerFinalize(void) {
    batteryFound = false;
}

u8 DrvPowerSync(void) {
    if (!batteryFound) {
        return 0;
    }

    return readFile(capacityPath) ? strtol(readBuffer, NULL, 10) : 0;
}
