//
// Runtime/States/Shell.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../States.h"

// General --------------------------------------------------------------------

static StorageEntryInfo directoryEntries[StorageMaxDirectoryEntries];
static char             directoryPath[StorageMaxPathLength + 1];
static char             entryPath[StorageMaxPathLength + 1];
static u16              numberOfEntries    = 0;
static u16              selectedEntryIndex = 0;
static u8               barColor           = 0;
static u8               selectionColor     = 0;
static BitmapFont*      defaultFont        = NULL;
static u8               barHeight          = 0;
static i16              firstEntryIndex    = 0;
static i16              lastEntryIndex     = 0;
static u8               disabledColor      = 0;
static bool             reloadStorage      = false;
static bool             oneGameCheck       = false;

static void updateDrawIndexes(void) {
    i8 entriesPerPage = (ScreenHeight - (barHeight * 2)) / (defaultFont->CharHeight + 1);

    if (numberOfEntries <= entriesPerPage) {
        firstEntryIndex = 0;
        lastEntryIndex  = numberOfEntries - 1;
        return;
    }

    firstEntryIndex = selectedEntryIndex - (entriesPerPage / 2);

    if (firstEntryIndex < 0) {
        firstEntryIndex = 0;
    }

    lastEntryIndex = firstEntryIndex + entriesPerPage - 1;

    if (lastEntryIndex >= numberOfEntries) {
        lastEntryIndex  = numberOfEntries - 1;
        firstEntryIndex = lastEntryIndex - entriesPerPage + 1;
    }
}

// Entries --------------------------------------------------------------------

static i8 compareEntries(const StorageEntryInfo* entryA, const StorageEntryInfo* entryB) {
    if (entryA->Flags != entryB->Flags) {
        return entryA->Flags > entryB->Flags ? -1 : 1;
    }

    return strcasecmp(entryA->Name, entryB->Name);
}

static void sortEntries(const u16 startIndex, const u16 endIndex) {
    if ((i16) endIndex - (i16) startIndex < 1) {
        return;
    }

    static StorageEntryInfo tempEntry;
    StorageEntryInfo        pivotEntry = directoryEntries[endIndex];
    u16                     pivotIndex = startIndex;

    for (u16 currentIndex = startIndex; currentIndex < endIndex; currentIndex++) {
        if (compareEntries(&directoryEntries[currentIndex], &pivotEntry) < 0) {
            memcpy(&tempEntry, &directoryEntries[pivotIndex], sizeof(StorageEntryInfo));
            memcpy(&directoryEntries[pivotIndex], &directoryEntries[currentIndex], sizeof(StorageEntryInfo));
            memcpy(&directoryEntries[currentIndex], &tempEntry, sizeof(StorageEntryInfo));
            pivotIndex++;
        }
    }

    memcpy(&tempEntry, &directoryEntries[pivotIndex], sizeof(StorageEntryInfo));
    memcpy(&directoryEntries[pivotIndex], &directoryEntries[endIndex], sizeof(StorageEntryInfo));
    memcpy(&directoryEntries[endIndex], &tempEntry, sizeof(StorageEntryInfo));

    sortEntries(startIndex, pivotIndex - 1);
    sortEntries(pivotIndex + 1, endIndex);
}

static void updateCurrentEntryPath(void) {
    entryPath[0] = 0;

    if (numberOfEntries == 0) {
        return;
    }

    strncat(entryPath, directoryPath, StorageMaxPathLength);
    strcat(entryPath, "/");
    strncat(entryPath, directoryEntries[selectedEntryIndex].Name, StorageMaxNameLength);
}

static void refreshDirectoryEntries() {
    numberOfEntries    = 0;
    selectedEntryIndex = 0;

    if (!OpenDirectory(directoryPath)) {
        updateDrawIndexes();
        return;
    }

    while (numberOfEntries < StorageMaxDirectoryEntries) {
        if (!GetNextDirectoryEntryInfo(&directoryEntries[numberOfEntries])) {
            break;
        }

        numberOfEntries++;
    }

    CloseDirectory();

    sortEntries(0, numberOfEntries - 1);

    updateCurrentEntryPath();
    updateDrawIndexes();
}

static void selectPreviousEntry(void) {
    if (numberOfEntries == 0) {
        return;
    }

    if (selectedEntryIndex == 0) {
        selectedEntryIndex = numberOfEntries - 1;
    } else {
        selectedEntryIndex--;
    }

    updateCurrentEntryPath();
    updateDrawIndexes();
}

static void selectNextEntry(void) {
    if (numberOfEntries == 0) {
        return;
    }

    if (selectedEntryIndex == numberOfEntries - 1) {
        selectedEntryIndex = 0;
    } else {
        selectedEntryIndex++;
    }

    updateCurrentEntryPath();
    updateDrawIndexes();
}

static StorageEntryInfo* getSelectedEntryInfo(void) {
    if (numberOfEntries == 0) {
        return NULL;
    }

    return &directoryEntries[selectedEntryIndex];
}

static void resetEntries() {
    numberOfEntries    = 0;
    selectedEntryIndex = 0;
    directoryPath[0]   = 0;
    entryPath[0]       = 0;

    refreshDirectoryEntries();
}

// Directories ----------------------------------------------------------------

static void enterParentDirectory(void) {
    if (!IsStorageAvailable() || directoryPath[0] == 0) {
        return;
    }

    u16 pathLength = strnlen(directoryPath, StorageMaxPathLength);

    for (int charIndex = pathLength - 1; charIndex >= 0; charIndex--) {
        if (directoryPath[charIndex] == '/') {
            directoryPath[charIndex] = 0;
            refreshDirectoryEntries();
            return;
        }
    }

    directoryPath[0] = 0;
    entryPath[0]     = 0;

    refreshDirectoryEntries();
}

static void enterSelectedDirectory(void) {
    if (!IsStorageAvailable() || numberOfEntries == 0 || !IsDirectory(directoryEntries[selectedEntryIndex].Flags)) {
        return;
    }

    strcat(directoryPath, "/");
    strncat(directoryPath, directoryEntries[selectedEntryIndex].Name, StorageMaxNameLength);

    refreshDirectoryEntries();
}

// User Interface -------------------------------------------------------------

static void drawOverlay(void) {
    string leftOption  = directoryPath[0] != 0 ? "(B) Back" : NULL;
    string rightOption = NULL;

    if (numberOfEntries > 0) {
        if (IsProgram(directoryEntries[selectedEntryIndex].Flags)) {
            rightOption = "Play (A)";
        } else if (IsDirectory(directoryEntries[selectedEntryIndex].Flags)) {
            rightOption = "Enter (A)";
        }
    }

    DrawShellOverlay("Portatil", leftOption, rightOption);
}

static void drawEntries(void) {
    SetTransparentColor(0);

    u16 currentY = barHeight + 1;

    for (u16 entryIndex = firstEntryIndex; entryIndex <= lastEntryIndex; entryIndex++) {
        if (entryIndex == selectedEntryIndex) {
            DrawRectangle(&(Rectangle2D) {0, currentY, ScreenWidth, barHeight}, selectionColor);
        }

        if (IsDirectory(directoryEntries[entryIndex].Flags)) {
            DrawImage(&FolderIconImage, 1, currentY + 1, &FolderIconRectangle);
        } else if (IsProgram(directoryEntries[entryIndex].Flags)) {
            DrawImage(&ProgramIconImage, 1, currentY + 1, &ProgramIconRectangle);
        } else {
            SetForegroundColor(disabledColor);
        }

        DrawText(defaultFont, 9, currentY + 1, directoryEntries[entryIndex].Name);
        SetForegroundColor(ColorNone);

        currentY += barHeight;
    }

    if (firstEntryIndex > 0) {
        SetDrawAnchor(AnchorTop | AnchorRight);
        DrawImage(&ScrollUpImage, ScreenWidth - 1, barHeight + 1, &ScrollUpRectangle);
    }

    if (lastEntryIndex < numberOfEntries - 1) {
        SetDrawAnchor(AnchorBottom | AnchorRight);
        DrawImage(&ScrollDownImage, ScreenWidth - 1, ScreenHeight - barHeight - 1, &ScrollDownRectangle);
    }

    SetDrawAnchor(AnchorDefault);
}

// Shell ----------------------------------------------------------------------

static void loadGame(const string filePath) {
    if (!OpenFile(filePath)) {
        ShowError(ShellState, "file error");
        return;
    }

    bool gameLoaded = LoadProgramFromStorage();

    CloseFile();

    if (gameLoaded) {
        ResetEngine();
        ChangeState(InGameState);
    } else {
        ShowError(ShellState, "program load error");
    }
}

static void handleInput(void) {
    if (IsButtonJustPressed(ButtonUp)) {
        selectPreviousEntry();
        return;
    }

    if (IsButtonJustPressed(ButtonDown)) {
        selectNextEntry();
        return;
    }

    if (IsButtonJustPressed(ButtonB)) {
        enterParentDirectory();
        return;
    }

    if (IsButtonJustPressed(ButtonA)) {
        StorageEntryInfo* selectedEntry = getSelectedEntryInfo();

        if (selectedEntry) {
            if (IsDirectory(selectedEntry->Flags)) {
                enterSelectedDirectory();
            } else if (IsProgram(selectedEntry->Flags)) {
                loadGame(entryPath);
            }
        }
    }
}

void InitializeShell(void) {
    defaultFont = GetDefaultFont();
    barHeight   = defaultFont->CharHeight + 1;

    barColor       = GetNearestColorIndex(64, 64, 64);
    selectionColor = GetNearestColorIndex(80, 160, 200);

    firstEntryIndex = -1;
    lastEntryIndex  = -1;

    disabledColor = GetNearestColorIndex(64, 64, 64);

    resetEntries();
    oneGameCheck = true;
}

void ShellState(const u64 frameTime) {
    if (reloadStorage) {
        reloadStorage = false;
        RefreshStorage();
        resetEntries();
        oneGameCheck = true;
        return;
    }

    if (oneGameCheck) {
        oneGameCheck = false;

        if (numberOfEntries == 1 && IsProgram(directoryEntries[0].Flags)) {
            loadGame(entryPath);
            return;
        }
    }

    handleInput();

    ClearScreen(0);
    ResetDrawState();

    if (IsStorageAvailable()) {
        drawEntries();
    } else {
        reloadStorage = true;
        ShowError(ShellState, "insert SD card");
    }

    drawOverlay();
}

void DrawShellOverlay(const string title, const string leftOption, const string rightOption) {
    SaveDrawState();
    ResetDrawState();

    SetTransparentColor(0);

    if (title) {
        DrawRectangle(&(Rectangle2D) {0, 0, ScreenWidth, barHeight}, barColor);
        DrawText(defaultFont, 1, 1, title);
        DrawBatteryIndicator();
    }

    SetDrawAnchor(AnchorBottom | AnchorLeft);
    DrawRectangle(&(Rectangle2D) {0, ScreenHeight, ScreenWidth, barHeight}, barColor);

    if (leftOption) {
        DrawText(defaultFont, 1, ScreenHeight, leftOption);
    }

    if (rightOption) {
        SetDrawAnchor(AnchorBottom | AnchorRight);
        DrawText(defaultFont, ScreenWidth, ScreenHeight, rightOption);
    }

    RestoreDrawState();
}