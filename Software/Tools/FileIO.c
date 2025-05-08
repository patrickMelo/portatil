//
// Tools/FileIO.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "Packer.h"

#include <dirent.h>
#include <errno.h>

#define logTag "Tools:FileIO"

File* allocFile(const FileMode mode, FILE* native, const u64 fileSize) {
    File* file = malloc(sizeof(File));

    if (!file) {
        Error(logTag, "Could not allocate memory for the file structure");
        return NULL;
    }

    file->Offset = 0;
    file->Mode   = mode;
    file->Native = native;
    file->Size   = fileSize;

    return file;
}

File* OpenFile(const string filePath) {
    if (!filePath) {
        Error(logTag, "Invalid file path");
        return NULL;
    }

    FILE* native = fopen(filePath, "rb");

    if (!native) {
        Error(logTag, "Could not open file \"%s\" (error %d)", filePath, errno);
        return NULL;
    }

    fseek(native, 0, SEEK_END);
    u64 fileSize = ftell(native);
    fseek(native, 0, SEEK_SET);

    File* file = allocFile(FileReadMode, native, fileSize);

    if (!file) {
        fclose(native);
        return NULL;
    }

    Debug(logTag, "File \"%s\" opened for reading");
    return file;
}

File* CreateFile(const string filePath) {
    if (!filePath) {
        Error(logTag, "Invalid file path");
        return NULL;
    }

    FILE* native = fopen(filePath, "wb");

    if (!native) {
        Error(logTag, "Could not create file \"%s\" (error %d)", filePath, errno);
        return NULL;
    }

    File* file = allocFile(FileWriteMode, native, 0);

    if (!file) {
        fclose(native);
        return NULL;
    }

    Debug(logTag, "File \"%s\" created for writing", filePath);
    return file;
}

bool ReadFile(File* file, const u64 readSize, byte* readOutput) {
    if (!file || !readSize) {
        Error(logTag, "Invalid file pointer or read size");
        return false;
    }

    if (file->Mode != FileReadMode) {
        Error(logTag, "File is not in read mode");
        return false;
    }

    if (file->Offset + readSize > file->Size) {
        Error(logTag, "File offset + read size will reach outside file contents");
        return false;
    }

    if (fread(readOutput, readSize, 1, file->Native) != 1) {
        Error(logTag, "Could not read from file (error %d)", errno);
        return false;
    }

    file->Offset = ftell(file->Native);
    return true;
}

bool WriteFile(File* file, const byte* data, const u64 writeSize) {
    if (!file || !data || !writeSize) {
        Error(logTag, "Invalid file pointer, data or write size");
        return false;
    }

    if (file->Mode != FileWriteMode) {
        Error(logTag, "File is not in write mode");
        return false;
    }

    if (fwrite(data, writeSize, 1, file->Native) != 1) {
        Error(logTag, "Could not write to file (error %d)", errno);
        return false;
    }

    file->Size = file->Offset = ftell(file->Native);
    return true;
}

void CloseFile(File* file) {
    if (!file) {
        return;
    }

    fclose(file->Native);
    free(file);
}

byte* QuickReadFile(const string filePath, u64* fileSize) {
    if (!filePath || !fileSize) {
        Error(logTag, "Invalid path or size pointer");
        return NULL;
    }

    FILE* file = fopen(filePath, "rb");

    if (!file) {
        Error(logTag, "Could not open file \"%s\" (error %d)", filePath, errno);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fileSize == 0) {
        Error(logTag, "File \"%s\" has no data", filePath);
        return NULL;
    }

    byte* data = malloc(*fileSize);

    if (!data) {
        Error(logTag, "Could not allocate memory to read file \"%s\"", filePath);
        fclose(file);
        return NULL;
    }

    if (fread(data, *fileSize, 1, file) != 1) {
        Error(logTag, "Could not read from file \"%s\" (error %d)", filePath, errno);
        fclose(file);
        free(data);
        return NULL;
    }

    Debug(logTag, "%d byte(s) read from \"%s\"", *fileSize, filePath);

    fclose(file);
    return data;
}

bool QuickWriteFile(const string filePath, const byte* data, const u64 dataSize) {
    if (!data || !dataSize) {
        Error(logTag, "Invalid data or data size");
        return false;
    }

    FILE* file = fopen(filePath, "wb");

    if (!file) {
        Error(logTag, "Could not create file \"%s\" (error %d)", filePath, errno);
        return false;
    }

    if (fwrite(data, dataSize, 1, file) != 1) {
        Error(logTag, "Could not write to file \"%s\" (error %d)", filePath, errno);
        fclose(file);
        return false;
    }

    Debug(logTag, "%d byte(s) written to \"%s\"", dataSize, filePath);
    fclose(file);
    return true;
}

bool ListFiles(const string directoryPath, FileList* list) {
    if (!directoryPath || !list) {
        Error(logTag, "Invalid directory path or file list pointer");
        return false;
    }

    DIR* directory = opendir(directoryPath);

    if (!directory) {
        Error(logTag, "Could not open directory \"%s\" (error %d)", directoryPath, errno);
        return false;
    }

    struct dirent* entry;
    char           pathBuffer[MaxPathLength];

    while ((entry = readdir(directory))) {
        if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0)) {
            continue;
        }

        switch (entry->d_type) {
            case DT_DIR: {
                strcpy(pathBuffer, directoryPath);
                strcat(pathBuffer, "/");
                strcat(pathBuffer, entry->d_name);

                if (!ListFiles(pathBuffer, list)) {
                    closedir(directory);
                    return false;
                }

                break;
            }

            case DT_REG: {
                if (list->NumberOfFiles >= MaxFiles) {
                    Error(logTag, "Maximum number of files reached (%d)", MaxFiles);
                    closedir(directory);
                    return false;
                }

                strcpy(pathBuffer, directoryPath);
                strcat(pathBuffer, "/");
                strcat(pathBuffer, entry->d_name);

                strcpy(list->Path[list->NumberOfFiles], pathBuffer);
                list->NumberOfFiles++;

                break;
            }

            default: {
                continue;
            }
        }
    }

    closedir(directory);
    return true;
}