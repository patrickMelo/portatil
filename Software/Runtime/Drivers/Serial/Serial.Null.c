//
// Runtime/Drivers/Serial/Serial.Null.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../../Drivers.h"

// Driver ---------------------------------------------------------------------

bool DrvSerialInitialize(void) {
    return true;
}

void DrvSerialFinalize(void) {
    // Empty
}

bool DrvSerialConfigure(const SerialPortNumber portNumber, const u32 speedHz, const bool useDMA) {
    return false;
}

bool DrvSerialSelect(const SerialPortNumber portNumber) {
    return false;
}

void DrvSerialRelease(const SerialPortNumber portNumber) {
    // Empty
}

void DrvSerialWait(const SerialPortNumber portNumber) {
    // Empty
}

void DrvSerialSetSpeed(const SerialPortNumber portNumber, const u32 speedHz) {
    // Empty
}

bool DrvSerialRead(const SerialPortNumber portNumber, const u32 size, u8* buffer) {
    return false;
}

bool DrvSerialWrite(const SerialPortNumber portNumber, const u32 size, u8* buffer) {
    return false;
}
