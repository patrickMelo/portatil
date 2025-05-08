//
// Runtime/Drivers/CPU/Linux.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../../Drivers.h"

#include <sched.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

// CPU ------------------------------------------------------------------------

struct timespec startTime;

static void signalHandler(int signalCode) {
    Shutdown();
}

static inline u64 getTick(void) {
    static struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return ((time.tv_sec * 1000000) + (time.tv_nsec * 0.001)) - ((startTime.tv_sec * 1000000) + (startTime.tv_nsec * 0.001));
}

// Driver ---------------------------------------------------------------------

bool DrvCpuInitialize(void) {
    clock_gettime(CLOCK_MONOTONIC, &startTime);

    signal(SIGINT, signalHandler);
    signal(SIGILL, signalHandler);
    signal(SIGABRT, signalHandler);
    signal(SIGFPE, signalHandler);
    signal(SIGSEGV, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGHUP, signalHandler);
    signal(SIGQUIT, signalHandler);

    srand(startTime.tv_nsec);
    return true;
}

void DrvCpuFinalize(void) {
    // Empty
}

void DrvCpuWait(const u64 waitTime) {
    u64 startTick = getTick();

    while (getTick() - startTick < waitTime) {
        sched_yield();
    }
}

u64 DrvCpuSync(void) {
    sched_yield();
    return getTick();
}

u64 DrvCpuGetTick(void) {
    return getTick();
}

u8 DrvCpuGetAvailableCoreIndex(void) {
    return 0;
}

bool DrvCpuRunCore(const u8 coreIndex, const u8 messageSize, const u32 queueSize, const CoreFunction coreFunction) {
    return false;
}

void DrvCpuSendMessage(const u8 coreIndex, const void* messageData) {
    // Empty
}

void DrvCpuWaitMessage(const u8 coreIndex, void* messageData) {
    // Empty
}