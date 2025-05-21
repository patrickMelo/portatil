//
// Runtime/Drivers/Serial/Serial.Pico.c
//
// This file is part of Portatil source code.
// Copyright 2025 Patrick L. Melo <patrick@patrickmelo.com.br>
//

#include "../../Drivers.h"

#include <hardware/dma.h>
#include <hardware/spi.h>
#include <pico/stdlib.h>

// SPI ------------------------------------------------------------------------

typedef struct SPIState {
        bool        isConfigured;
        bool        isSelected;
        u8          rxPin, csPin, sckPin, txPin;
        i8          dmaChannel;
        spi_inst_t* hwInstance;
} SPIState;

static SPIState spi[NumberOfSerialPorts] = {
    {
     .isConfigured = false,
     .isSelected   = false,
     .dmaChannel   = -1,
     .rxPin        = 0,
     .csPin        = 1,
     .sckPin       = 2,
     .txPin        = 3,
     .hwInstance   = spi0,
     },
    {
     .isConfigured = false,
     .isSelected   = false,
     .dmaChannel   = -1,
     .rxPin        = 8,
     .csPin        = 9,
     .sckPin       = 10,
     .txPin        = 11,
     .hwInstance   = spi1,
     },
};

static inline void releasePort(SerialPortNumber portNumber) {
    if (spi[portNumber].dmaChannel >= 0) {
        dma_channel_cleanup(spi[portNumber].dmaChannel);
        dma_channel_unclaim(spi[portNumber].dmaChannel);
    }

    gpio_put(spi[portNumber].csPin, 0);
    spi_deinit(spi[portNumber].hwInstance);

    spi[portNumber].isConfigured = false;
    spi[portNumber].isSelected   = false;
}

// Driver ---------------------------------------------------------------------

bool DrvSerialInitialize(void) {
    // Empty
    return true;
}

void DrvSerialFinalize(void) {
    for (u8 portNumber = 0; portNumber < NumberOfSerialPorts; portNumber++) {
        if (!spi[portNumber].isConfigured) {
            continue;
        }

        releasePort(portNumber);
    }
}

bool DrvSerialConfigure(const SerialPortNumber portNumber, const u32 speedHz, const bool useDMA) {
    if ((portNumber >= NumberOfSerialPorts)) {
        return false;
    }

    if (spi[portNumber].isConfigured) {
        releasePort(portNumber);
    }

    spi_init(spi[portNumber].hwInstance, speedHz);

    if (useDMA) {
        spi[portNumber].dmaChannel = dma_claim_unused_channel(false);

        if (spi[portNumber].dmaChannel < 0) {
            spi_deinit(spi[portNumber].hwInstance);
            return false;
        }

        dma_channel_config dmaConfig = dma_channel_get_default_config(spi[portNumber].dmaChannel);

        channel_config_set_transfer_data_size(&dmaConfig, DMA_SIZE_8);
        channel_config_set_dreq(&dmaConfig, spi_get_dreq(spi[portNumber].hwInstance, true));
        channel_config_set_read_increment(&dmaConfig, true);
        channel_config_set_write_increment(&dmaConfig, false);

        dma_channel_configure(spi[portNumber].dmaChannel, &dmaConfig, &spi_get_hw(spi[portNumber].hwInstance)->dr, NULL, 0, false);
    }

    spi_set_format(spi[portNumber].hwInstance, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_set_function(spi[portNumber].rxPin, GPIO_FUNC_SPI);
    gpio_set_function(spi[portNumber].csPin, GPIO_FUNC_SPI);
    gpio_set_function(spi[portNumber].sckPin, GPIO_FUNC_SPI);
    gpio_set_function(spi[portNumber].txPin, GPIO_FUNC_SPI);

    gpio_put(spi[portNumber].csPin, 1);
    gpio_pull_up(spi[portNumber].rxPin);

    spi[portNumber].isSelected   = true;
    spi[portNumber].isConfigured = true;
    return true;
}

bool DrvSerialSelect(const SerialPortNumber portNumber) {
    if ((portNumber >= NumberOfSerialPorts) || (!spi[portNumber].isConfigured) || (spi[portNumber].isSelected)) {
        return false;
    }

    gpio_put(spi[portNumber].csPin, 0);
    spi[portNumber].isSelected = true;
    return true;
}

void DrvSerialRelease(const SerialPortNumber portNumber) {
    if ((portNumber >= NumberOfSerialPorts) || (!spi[portNumber].isSelected)) {
        return;
    }

    gpio_put(spi[portNumber].csPin, 1);
    spi[portNumber].isSelected = false;
}

void DrvSerialWait(const SerialPortNumber portNumber) {
    if ((portNumber >= NumberOfSerialPorts) || (!spi[portNumber].isSelected)) {
        return;
    }

    if (spi[portNumber].dmaChannel >= 0) {
        if (dma_channel_is_busy(spi[portNumber].dmaChannel)) {
            dma_channel_wait_for_finish_blocking(spi[portNumber].dmaChannel);
        }
    } else {
        while (spi_is_busy(spi[portNumber].hwInstance)) {
            // Empty
        }
    }
}

void DrvSerialSetSpeed(const SerialPortNumber portNumber, const u32 speedHz) {
    if ((portNumber >= NumberOfSerialPorts) || (!spi[portNumber].isConfigured)) {
        return;
    }

    spi_set_baudrate(spi[portNumber].hwInstance, speedHz);
}

bool DrvSerialRead(const SerialPortNumber portNumber, const u32 size, u8* buffer) {
    if ((portNumber >= NumberOfSerialPorts) || (!spi[portNumber].isSelected)) {
        return false;
    }

    if (spi[portNumber].dmaChannel >= 0) {
        dma_channel_transfer_to_buffer_now(spi[portNumber].dmaChannel, buffer, size);
    } else {
        spi_read_blocking(spi[portNumber].hwInstance, 0xFF, buffer, size);
    }

    return true;
}

bool DrvSerialWrite(const SerialPortNumber portNumber, const u32 size, u8* buffer) {
    if ((portNumber >= NumberOfSerialPorts) || (!spi[portNumber].isSelected)) {
        return false;
    }

    if (spi[portNumber].dmaChannel >= 0) {
        dma_channel_transfer_from_buffer_now(spi[portNumber].dmaChannel, buffer, size);
    } else {
        spi_write_blocking(spi[portNumber].hwInstance, buffer, size);
    }

    DrvCpuWait(1);    // for some reason, if we don't do anything after the write, SPI slows to a crawl.
    return true;
}
