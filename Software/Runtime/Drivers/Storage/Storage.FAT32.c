//
// Runtime/Drivers/Storage/Storage.FAT32.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../../Drivers.h"

// FAT32 ----------------------------------------------------------------------

extern bool Fat32InitializeMedia(void);
extern void Fat32FinalizeMedia(void);
extern u32  Fat32GetMediaSize(void);
extern bool Fat32ReadMedia(const u32 sectorIndex, u8* sectorData);

#define fat32SectorSize                512
#define fat32DirectoryEntriesPerSector 16

#define fat32ReadOnlyFlag  0b00000001
#define fat32HiddenFlag    0b00000010
#define fat32SystemFlag    0b00000100
#define fat32VolumeIdFlag  0b00001000
#define fat32DirectoryFlag 0b00010000
#define fat32ArchiveFlag   0b00100000

#define fat32LFNAttribute 0x0F

typedef struct __attribute__((packed)) fat32DirectoryEntry {
        char shortName[8];
        char shortExtension[3];
        u8   fileAttributes;
        u8   userAttributes;
        u8   createTime;
        u16  passwordHash;
        u16  createDate;
        u16  lastAccessDate;
        u16  firstClusterHigh;
        u16  lastModifiedTime;
        u16  lastModifiedDate;
        u16  firstClusterLow;
        u32  fileSize;
        char longName[256];
} fat32DirectoryEntry;

typedef struct __attribute__((packed)) fat32LfnDirectoryEntry {
        u8   sequenceNumber;
        char nameBuffer1[10];
        u8   fileAttributes;
        u8   lfnType;
        u8   dosFileNameChecksum;
        char nameBuffer2[12];
        u16  firstClusterHigh;
        char nameBuffer3[4];
} fat32LfnDirectoryEntry;

typedef struct fat32DirectoryPointer {
        u32 startCluster;
        u32 clusterIndex;
        u32 sectorIndex;
        u32 entryIndex;
} fat32DirectoryPointer;

static struct __attribute__((packed)) {
        u8   jmpBoot[3];
        char oemName[8];
        u16  bytesPerSector;
        u8   sectorsPerCluster;
        u16  reservedSectorCount;
        u8   numberOfFats;
        u16  rootEntriesCount;
        u16  totalSectors16;
        u8   mediaType;
        u16  fatSize16;
        u16  sectorsPerTrack;
        u16  numberOfHeads;
        u32  hiddenSectorsCount;
        u32  totalSectors32;
        u32  fatSize32;
        u16  extFlags;
        u8   fsVersion[2];
        u32  rootCluster;
        u16  fsInfo;
        u16  backupBootSector;
        u8   reserved1[12];
        u8   driveNumber;
        u8   reserved2;
        u8   bootSignature;
        u32  volumeID;
        char volumeLabel[11];
        char fsType[8];
        u8   padding1[420];
        u16  signatureWord;
} bootSector;

static struct __attribute__((packed)) {
        u32 leadSignature;
        u8  reserved1[480];
        u32 structureSignature;
        u32 freeClusterCount;
        u32 nextFreeCluster;
        u8  reserved2[12];
        u32 trailSignature;
} fsInfo;

static u32 fatStartSector  = 0;
static u32 dataStartSector = 0;

static fat32DirectoryPointer rootDirectory;

static u8  sectorData[fat32SectorSize];
static u32 lastReadSector = 0xFFFFFFFF;

static bool fat32ReadSector(const u32 sectorIndex) {
    if (lastReadSector == sectorIndex) {
        return true;
    }

    if (!Fat32ReadMedia(sectorIndex, sectorData)) {
        lastReadSector = 0xFFFFFFFF;
        return false;
    }

    lastReadSector = sectorIndex;
    return true;
}

static u32 fat32GetValue(const u32 clusterIndex) {
    u32 clusterSector = fatStartSector + (clusterIndex / 128);
    u32 clusterOffset = (clusterIndex % 128) * 4;

    if (!fat32ReadSector(clusterSector)) {
        return 0;
    }

    return (*(u32*) &sectorData[clusterOffset]) & 0x0FFFFFFF;
}

static inline u32 fat32GetClusterDataSector(const u32 clusterIndex) {
    return dataStartSector + ((clusterIndex - 2) * bootSector.sectorsPerCluster);
}

static void fat32GetShortName(const fat32DirectoryEntry* directoryEntry, string nameBuffer) {
    u8 bufferIndex = 0;

    for (u8 charIndex = 0; charIndex < 8; charIndex++) {
        if (directoryEntry->shortName[charIndex] <= ' ') {
            break;
        }

        nameBuffer[bufferIndex++] = directoryEntry->shortName[charIndex];
    }

    if (directoryEntry->shortExtension[0] > ' ') {
        nameBuffer[bufferIndex++] = '.';

        for (u8 charIndex = 0; charIndex < 3; charIndex++) {
            if (directoryEntry->shortExtension[charIndex] <= ' ') {
                break;
            }

            nameBuffer[bufferIndex++] = directoryEntry->shortExtension[charIndex];
        }
    }

    nameBuffer[bufferIndex] = 0;
}

static inline void fat32ResetDirectory(fat32DirectoryPointer* directory) {
    directory->clusterIndex = directory->startCluster;
    directory->sectorIndex  = 0;
    directory->entryIndex   = 0;
}

static inline bool fat32AdvanceDirectory(fat32DirectoryPointer* directory) {
    directory->entryIndex += 1;

    if (directory->entryIndex >= fat32DirectoryEntriesPerSector) {
        directory->entryIndex = 0;
        directory->sectorIndex += 1;

        if (directory->sectorIndex >= bootSector.sectorsPerCluster) {
            u32 nextClusterIndex = fat32GetValue(directory->clusterIndex);

            if (nextClusterIndex < 2 || nextClusterIndex >= bootSector.fatSize32) {
                return false;
            }

            directory->clusterIndex = nextClusterIndex;
            directory->sectorIndex  = 0;
        }
    }

    return true;
}

static inline bool fat32AdvanceFile(fat32DirectoryPointer* file) {
    file->sectorIndex += 1;

    if (file->sectorIndex >= bootSector.sectorsPerCluster) {
        u32 nextClusterIndex = fat32GetValue(file->clusterIndex);

        if (nextClusterIndex < 2 || nextClusterIndex >= bootSector.fatSize32) {
            return false;
        }

        file->clusterIndex = nextClusterIndex;
        file->sectorIndex  = 0;
    }

    return true;
}

static bool fat32ReadDirectoryEntry(const fat32DirectoryPointer* directory, fat32DirectoryEntry* entry) {
    u32 clusterSector = fat32GetClusterDataSector(directory->clusterIndex);
    u32 entryOffset   = directory->entryIndex * 32;

    if (!fat32ReadSector(clusterSector + directory->sectorIndex)) {
        return false;
    }

    memcpy(entry, &sectorData[entryOffset], 32);
    return entry->shortName[0] != 0;
}

static bool fat32ProcessLongName(const fat32LfnDirectoryEntry* entry, string nameBuffer) {
    bool isLastEntry = (entry->sequenceNumber >> 6) & 0b1;

    if (isLastEntry) {
        memset(nameBuffer, 0, 256);
    }

    u8 entryNumber = entry->sequenceNumber & 0x1F;
    u8 bufferIndex = (entryNumber - 1) * 13;

    // TODO: handle UCS-2 (for now we just skip the character's second byte)

    for (u8 charIndex = 0; charIndex < 10; charIndex += 2) {
        nameBuffer[bufferIndex++] = entry->nameBuffer1[charIndex];
    }

    for (u8 charIndex = 0; charIndex < 12; charIndex += 2) {
        nameBuffer[bufferIndex++] = entry->nameBuffer2[charIndex];
    }

    for (u8 charIndex = 0; charIndex < 4; charIndex += 2) {
        nameBuffer[bufferIndex++] = entry->nameBuffer3[charIndex];
    }

    return entryNumber > 1;
}

static bool fat32GetLongName(fat32DirectoryPointer* directory, fat32DirectoryEntry* entry) {
    static fat32DirectoryEntry lfnEntryInfo;

    bool gotLongName = false;

    while (fat32ReadDirectoryEntry(directory, &lfnEntryInfo)) {
        if (((u8) lfnEntryInfo.shortName[0]) == 0xE5) {    // Skip deleted entries
            fat32AdvanceDirectory(directory);
            continue;
        }

        if (lfnEntryInfo.fileAttributes != fat32LFNAttribute) {
            break;
        }

        fat32AdvanceDirectory(directory);
        gotLongName = true;

        if (!fat32ProcessLongName((fat32LfnDirectoryEntry*) &lfnEntryInfo, entry->longName)) {
            break;
        }
    }

    return gotLongName;
}

static bool fat32GetNextDirectoryEntry(fat32DirectoryPointer* directory, fat32DirectoryEntry* entry) {
    bool gotLongName = fat32GetLongName(directory, entry);

    if (!fat32ReadDirectoryEntry(directory, entry)) {
        return false;
    }

    if (!gotLongName) {
        fat32GetShortName(entry, entry->longName);
    }

    fat32AdvanceDirectory(directory);
    return true;
}

static bool fat32CheckBootSector(void) {
    if (!Fat32ReadMedia(0, (u8*) &bootSector)) {
        return false;
    }

    if ((bootSector.jmpBoot[0] != 0xEB) && (bootSector.jmpBoot[0] != 0xE9)) {
        return false;
    }

    if ((bootSector.numberOfFats != 2) ||
        (bootSector.rootEntriesCount != 0) ||
        (bootSector.totalSectors16 != 0) ||
        (bootSector.fatSize16 != 0) ||
        (bootSector.totalSectors32 == 0) ||
        (bootSector.fatSize32 == 0) ||
        (bootSector.signatureWord != 0xAA55) ||
        (bootSector.bytesPerSector != fat32SectorSize)) {
        return false;
    }

    if (!Fat32ReadMedia(bootSector.fsInfo, (u8*) &fsInfo)) {
        return false;
    }

    if ((fsInfo.leadSignature != 0x41615252) ||
        (fsInfo.structureSignature != 0x61417272) ||
        (fsInfo.trailSignature != 0xAA550000)) {
        return false;
    }

    u32 numberOfFatSectors = bootSector.fatSize32 * bootSector.numberOfFats;

    fatStartSector  = bootSector.reservedSectorCount;
    dataStartSector = fatStartSector + numberOfFatSectors;

    rootDirectory.startCluster = bootSector.rootCluster;
    fat32ResetDirectory(&rootDirectory);

    return true;
}

// Storage --------------------------------------------------------------------

static bool                  isDirectoryOpen = false;
static fat32DirectoryPointer currentDirectory;
static fat32DirectoryEntry   currentDirectoryEntry;
static bool                  isFileOpen = false;
static fat32DirectoryPointer currentFile;
static fat32DirectoryEntry   currentFileEntry;
static u32                   currentFileOffset = 0;

static bool findEntry(fat32DirectoryPointer* directory, const string entryPath, fat32DirectoryPointer* pointer, fat32DirectoryEntry* entry) {
    if (!entryPath) {
        return false;
    }

    if (entryPath[0] == 0) {
        memcpy(pointer, directory, sizeof(fat32DirectoryPointer));
        return true;
    }

    u16 pathLength = strnlen(entryPath, StorageMaxPathLength);

    static char processedPath[StorageMaxPathLength];
    strncpy(processedPath, entryPath, pathLength);

    if (processedPath[0] == '/') {
        pathLength--;
        strncpy(processedPath, &entryPath[1], pathLength);
    }

    processedPath[pathLength] = 0;

    u16 separatorIndex = 0;

    while (separatorIndex < pathLength) {
        if (processedPath[separatorIndex] == '/') {
            break;
        }

        separatorIndex++;
    }

    fat32ResetDirectory(directory);

    while (fat32GetNextDirectoryEntry(directory, entry)) {
        u16 nameLength = strnlen(entry->longName, 256);

        if ((nameLength == separatorIndex) && (strncasecmp(entry->longName, processedPath, separatorIndex) == 0)) {
            pointer->startCluster = ((u32) entry->firstClusterHigh << 16) | (u32) entry->firstClusterLow;
            fat32ResetDirectory(pointer);

            if (separatorIndex < pathLength) {
                return findEntry(pointer, &processedPath[separatorIndex + 1], pointer, entry);
            } else {
                return true;
            }
        }
    }

    return false;
}

// Driver ---------------------------------------------------------------------

bool DrvStorageInitialize(void) {
    if (!Fat32InitializeMedia()) {
        return false;
    }

    if (!fat32CheckBootSector()) {
        Fat32FinalizeMedia();
        return false;
    }

    return true;
}

void DrvStorageFinalize(void) {
    Fat32FinalizeMedia();
}

bool DrvStorageOpenDirectory(const string directoryPath) {
    DrvStorageCloseDirectory();

    if (!findEntry(&rootDirectory, directoryPath, &currentDirectory, &currentDirectoryEntry)) {
        return false;
    }

    fat32ResetDirectory(&currentDirectory);
    isDirectoryOpen = true;
    return true;
}

bool DrvStorageGetNextDirectoryEntryInfo(StorageEntryInfo* entryInfo) {
    if (!isDirectoryOpen) {
        return false;
    }

    static fat32DirectoryEntry entry;

    if (!fat32GetNextDirectoryEntry(&currentDirectory, &entry)) {
        return false;
    }

    if ((entry.fileAttributes & fat32VolumeIdFlag) || (entry.shortName[0] == '.')) {
        return DrvStorageGetNextDirectoryEntryInfo(entryInfo);
    }

    strncpy(entryInfo->Name, entry.longName, StorageMaxNameLength);
    entryInfo->Flags = 0;

    if (entry.fileAttributes & fat32DirectoryFlag) {
        entryInfo->Flags |= StorageEntryDirectoryFlag;
    } else {
        static char extensionBuffer[5];

        u8 nameLength = strnlen(entryInfo->Name, StorageMaxNameLength);

        if (nameLength > 4) {
            strncpy(extensionBuffer, &entryInfo->Name[nameLength - 4], 4);

            if (strncasecmp(extensionBuffer, ".rvp", 4) == 0) {
                entryInfo->Flags |= StorageEntryProgramFlag;
            }
        }
    }

    return true;
}

void DrvStorageCloseDirectory(void) {
    isDirectoryOpen = false;
}

bool DrvStorageOpenFile(const string filePath) {
    DrvStorageCloseFile();

    if (!findEntry(&rootDirectory, filePath, &currentFile, &currentFileEntry)) {
        return false;
    }

    isFileOpen        = true;
    currentFileOffset = 0;
    return true;
}

u32 DrvStorageGetFileSize(void) {
    if (!isFileOpen) {
        return 0;
    }

    return currentFileEntry.fileSize;
}

bool DrvStorageReadFile(void* readBuffer, const u32 readSize) {
    if (!isFileOpen) {
        return false;
    }

    u32 bytesLeft = readSize;
    u32 readIndex = 0;

    static u32 clusterSector;
    static u32 sectorBytes, sectorOffset, bytesToRead;

    while (bytesLeft > 0) {
        sectorOffset = currentFileOffset % fat32SectorSize;
        sectorBytes  = fat32SectorSize - sectorOffset;

        bytesToRead = sectorBytes <= bytesLeft ? sectorBytes : bytesLeft;

        clusterSector = fat32GetClusterDataSector(currentFile.clusterIndex);

        if (!fat32ReadSector(clusterSector + currentFile.sectorIndex)) {
            return false;
        }

        memcpy(&((u8*) readBuffer)[readIndex], &sectorData[sectorOffset], bytesToRead);

        currentFileOffset += bytesToRead;
        readIndex += bytesToRead;
        bytesLeft -= bytesToRead;
        sectorBytes -= bytesToRead;

        if ((sectorBytes == 0) && (bytesLeft > 0) && !fat32AdvanceFile(&currentFile)) {
            return false;
        }
    }

    return true;
}

void DrvStorageCloseFile(void) {
    isFileOpen = false;
}
