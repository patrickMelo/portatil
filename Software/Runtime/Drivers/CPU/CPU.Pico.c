//
// Runtime/Drivers/CPU/Pico.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../../Drivers.h"

#include <hardware/clocks.h>
#include <pico/multicore.h>
#include <pico/stdlib.h>
#include <pico/util/queue.h>

// CPU ------------------------------------------------------------------------

static u64     startTime           = 0;
static bool    isSecondCoreRunning = false;
static queue_t secondCoreQueue;

// Driver ---------------------------------------------------------------------

bool DrvCpuInitialize(void) {
    startTime = time_us_64();
    srand(startTime);
    return true;
}

void DrvCpuFinalize(void) {
    // Empty
}

void DrvCpuWait(const u64 waitTime) {
    sleep_us(waitTime);
}

u64 DrvCpuSync(void) {
    return time_us_64() - startTime;
}

u64 DrvCpuGetTick(void) {
    return time_us_64() - startTime;
}

u8 DrvCpuGetAvailableCoreIndex(void) {
    return isSecondCoreRunning ? 0 : 1;
}

bool DrvCpuRunCore(const u8 coreIndex, const u8 messageSize, const u32 queueSize, const CoreFunction coreFunction) {
    if (isSecondCoreRunning || (coreIndex != 1)) {
        return false;
    }

    queue_init(&secondCoreQueue, messageSize, queueSize);
    multicore_launch_core1(coreFunction);

    isSecondCoreRunning = true;
    return true;
}

void DrvCpuSendMessage(const u8 coreIndex, const void* messageData) {
    if (!isSecondCoreRunning || (coreIndex != 1)) {
        return;
    }

    queue_add_blocking(&secondCoreQueue, messageData);
}

void DrvCpuWaitMessage(const u8 coreIndex, void* messageData) {
    if (!isSecondCoreRunning || (coreIndex != 1)) {
        return;
    }

    queue_remove_blocking(&secondCoreQueue, messageData);
}