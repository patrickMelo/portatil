//
// Runtime/Drivers/Storage/Storage.FAT32.SDCard.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../../Drivers.h"

// SD Card --------------------------------------------------------------------

#define sdSerialPort SerialPort1

#define sdSerialSlowSpeed 100000
#define sdSerialFastSpeed 10000000

#define sdStatusIdle           0b00000001
#define sdStatusEraseReset     0b00000010
#define sdStatusIllegalCommand 0b00000100
#define sdStatusCRCError       0b00001000
#define sdStatusEraseError     0b00010000
#define sdStatusAddressError   0b00100000
#define sdStatusParameterError 0b01000000

#define sdCmdReset               0
#define sdCmdSendCondition       8
#define sdCmdSendCSD             9
#define sdCmdSendCID             10
#define sdCmdSetBlockLen         16
#define sdCmdReadSingleBlock     17
#define sdAppCmdSendOpCondition  41
#define sdAppCmdSetClrCardDetect 42
#define sdCmdApp                 55
#define sdCmdReadOCR             58

#define sdCardBlockSize 512
#define sdMaxRetries    10
#define sdTimeout       1000000

static const u8 sdDummyValue = 0xFF;

static bool isV2           = false;
static bool isHighCapacity = false;
static u32  numberOfBlocks = 0;
static u64  sizeBytes      = 0;

static inline bool sdWait(void) {
    u64 startTime = DrvCpuGetTick();

    static u8 response;

    DrvSerialSelect(sdSerialPort);
    DrvSerialRead(sdSerialPort, 1, &response);

    while (response != 0xFF) {
        if (DrvCpuGetTick() - startTime >= sdTimeout) {
            DrvSerialRelease(sdSerialPort);
            return false;
        }

        DrvSerialRead(sdSerialPort, 1, &response);
    }

    DrvSerialRelease(sdSerialPort);
    return true;
}

static u8 sdCommand(const u8 command, const u32 argument, const u8 crc7, const u8 responseSize, u8* responseData) {
    u8 commandData[6] = {
        0x40 | (command & 0x3F),
        (argument >> 24) & 0xFF,
        (argument >> 16) & 0xFF,
        (argument >> 8) & 0xFF,
        argument & 0xFF,
        (crc7 << 1) | 0x01,
    };

    if (!sdWait()) {
        return false;
    }

    for (uint commandRetry = 0; commandRetry < sdMaxRetries; commandRetry++) {
        DrvSerialSelect(sdSerialPort);
        DrvSerialWrite(sdSerialPort, 6, commandData);

        u8 responseStatus = sdDummyValue;

        for (uint responseStatusRetry = 0; responseStatusRetry < sdMaxRetries; responseStatusRetry++) {
            DrvSerialRead(sdSerialPort, 1, &responseStatus);

            if (responseStatus != sdDummyValue) {
                break;
            }
        }

        if (responseStatus == sdDummyValue) {
            DrvSerialRelease(sdSerialPort);
            continue;
        }

        DrvSerialRead(sdSerialPort, responseSize, responseData);
        DrvSerialRelease(sdSerialPort);

        return responseStatus;
    }

    return sdDummyValue;
}

static inline u8 sdAppCommand(const u8 command, const u32 argument, const u8 responseSize, u8* responseData) {
    sdCommand(sdCmdApp, 0, 0, 0, NULL);
    return sdCommand(command, argument, 0, responseSize, responseData);
}

static inline void sdBoot(void) {
    // We need to send 74 or more pulses with CS high to boot the SD card.

    DrvSerialSelect(sdSerialPort);
    DrvSerialRelease(sdSerialPort);

    for (uint pulseIndex = 0; pulseIndex < 100; pulseIndex++) {
        DrvSerialWrite(sdSerialPort, 1, (u8*) &sdDummyValue);
    }
}

static inline bool sdReset(void) {
    u8 resetStatus = sdDummyValue;

    for (uint resetRetry = 0; resetRetry < sdMaxRetries; resetRetry++) {
        resetStatus = sdCommand(sdCmdReset, 0, 0x4A, 0, NULL);

        if (resetStatus == sdStatusIdle) {
            break;
        }
    }

    if (resetStatus != sdStatusIdle) {
        return false;
    }

    return true;
}

static inline bool sdSetupV2(void) {
    // Initialize V2 card:
    // Send ACMD41 with HCS set to 1 (to support SDHC and SDXC)
    // Wait for card to be ready (when response is 0)
    // If ACMD41 never returns 0 it is probably a SDUC card (cannot operate in SPI mode)

    u8  status    = sdAppCommand(sdAppCmdSendOpCondition, 1 << 30, 0, NULL);
    u64 startTime = DrvCpuGetTick();

    while (status != 0) {
        if (status != sdStatusIdle) {
            return false;
        }

        if (DrvCpuGetTick() - startTime >= sdTimeout) {
            return false;
        }

        DrvCpuWait(10000);
        status = sdAppCommand(sdAppCmdSendOpCondition, 1 << 30, 0, NULL);
    }

    // Send CMD58 (Get CCS)
    // If CCS = 0 SD card is standard capacity
    // If CCS = 1 SD card is high or extended capacity

    static u32 ocrRegister;
    status = sdCommand(sdCmdReadOCR, 0, 0, 4, (u8*) &ocrRegister);

    if (status != 0) {
        return false;
    }

    isHighCapacity = (ocrRegister >> 30) & 0x1;

    status = sdAppCommand(sdAppCmdSetClrCardDetect, 0, 0, NULL);

    if (status != 0) {
        return false;
    }

    return true;
}

static inline bool sdSetup(void) {
    static u32 response;

    u8 status = sdCommand(sdCmdSendCondition, 0x1AA, 0x43, 4, (u8*) &response);

    isV2 = status != sdStatusIllegalCommand;

    if ((!isV2) ||
        (status != sdStatusIdle) ||
        (response & 0xFFF != 0x1AA)) {
        return false;
    }

    if (!sdSetupV2()) {
        return false;
    }

    status = sdCommand(sdCmdSetBlockLen, sdCardBlockSize, 0, 0, NULL);

    if (status != 0) {
        return false;
    }

    return true;
}

static inline void sdReadBlock(const uint blockSize, u8* readData) {
    u8   readByte  = sdDummyValue;
    u64  startTime = DrvCpuGetTick();
    uint timeoutMs = 100;

    DrvSerialSelect(sdSerialPort);

    while (readByte != 0xFE) {
        if (DrvCpuGetTick() - startTime >= timeoutMs * 1000) {
            DrvSerialRelease(sdSerialPort);
            return;
        }

        DrvSerialRead(sdSerialPort, 1, &readByte);
    }

    DrvSerialRead(sdSerialPort, blockSize, readData);

    static u16 crc16;

    DrvSerialRead(sdSerialPort, 2, (u8*) &crc16);
    DrvSerialRelease(sdSerialPort);
}

static inline void sdExtractCSD(u8 data[16]) {
    u8  csdVersion = data[0] >> 6;
    u8  readBlLen, cSizeMult;
    u32 cSize;

    switch (csdVersion) {
        case 0x00: {    // Version 1.0
            readBlLen = data[5] & 0b1111;
            cSize     = (((u16) data[6] & 0b11) << 10) | ((u16) data[7] << 2) | ((u16) data[8] >> 6);
            cSizeMult = ((data[9] & 0b11) << 1) | (data[10] >> 7);

            sizeBytes = (((u64) cSize + 1) * ((u64) 1 << (cSizeMult + 2))) * ((u64) 1 << readBlLen);
            break;
        }

        case 0x01: {    // Version 2.0
            cSize = (((u32) data[7] & 0b111111) << 16) | ((u32) data[8] << 8) | (data[9]);

            sizeBytes = ((u64) cSize + 1) * (512 * 1024);
            break;
        }

        case 0x10: {    // Version 3.0
            cSize = (((u32) data[6] & 0b1111) << 24) | ((u32) data[7] << 16) | ((u32) data[8] << 8) | (data[9]);

            sizeBytes = ((u64) cSize + 1) * (512 * 1024);
            break;
        }
    }
}

static inline bool sdGetInformation() {
    u8 status = sdCommand(sdCmdSendCSD, 0, 0, 0, NULL);

    if (status != 0) {
        return false;
    }

    static u8 csdResponse[16];

    sdReadBlock(16, csdResponse);
    sdExtractCSD(csdResponse);

    numberOfBlocks = sizeBytes / sdCardBlockSize;

    return true;
}

static bool sdReadSector(const u32 sectorIndex, u8* readData) {
    u8 status = sdCommand(sdCmdReadSingleBlock, sectorIndex, 0, 0, NULL);

    if (status != 0) {
        return false;
    }

    sdReadBlock(sdCardBlockSize, readData);
    return true;
}

// Media ----------------------------------------------------------------------

bool Fat32InitializeMedia(void) {
    if (!DrvSerialConfigure(sdSerialPort, sdSerialSlowSpeed, false)) {
        return false;
    }

    DrvSerialSelect(sdSerialPort);
    DrvSerialWrite(sdSerialPort, 1, (u8*) &sdDummyValue);
    DrvSerialRelease(sdSerialPort);

    sdBoot();

    if (!sdReset() || !sdSetup()) {
        return false;
    }

    DrvSerialSetSpeed(sdSerialPort, sdSerialFastSpeed);

    if (!sdGetInformation()) {
        return false;
    }

    return true;
}

void Fat32FinalizeMedia(void) {
    // Empty
}

u32 Fat32GetMediaSize(void) {
    return sizeBytes;
}

bool Fat32ReadMedia(const u32 sectorIndex, u8* sectorData) {
    return sdReadSector(sectorIndex, sectorData);
}
