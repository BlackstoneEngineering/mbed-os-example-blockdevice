/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "mbed.h"
#include <stdio.h>
#include <algorithm>

// Block devices
// #include "SPIFBlockDevice.h"
// #include "DataFlashBlockDevice.h"
#include "mbed-os/hal/qspi_api.h"
#include "mbed-os/drivers/QSPI.h"
#include "mbed-os/features/filesystem/bd/QSPIFBlockDevice.h"
#define DEVICE_QSPI

// #include "SDBlockDevice.h"
// #include "HeapBlockDevice.h"


// Physical block device, can be any device that supports the BlockDevice API
// SPIFBlockDevice bd(
//         MBED_CONF_SPIF_DRIVER_SPI_MOSI,
//         MBED_CONF_SPIF_DRIVER_SPI_MISO,
//         MBED_CONF_SPIF_DRIVER_SPI_CLK,
//         MBED_CONF_SPIF_DRIVER_SPI_CS);

// SPIFBlockDevice bd(PE_15, PE_14, PE_13, PE_12);
// SPIFBlockDevice bd(PE_12, PE_13, PE_10, PE_11);
QSPIFBlockDevice bd(PE_12, PE_13, PE_14, PE_15,PE_10,PE_11,0,8000000);


// Entry point for the example
int main() {
    printf("--- Mbed OS block device example ---\n");

    // Initialize the block device
    printf("bd.init()\n");
    int err = bd.init();
    printf("bd.init -> %d\n", err);

    // Get device geometry
    bd_size_t read_size    = bd.get_read_size();
    bd_size_t program_size = bd.get_program_size();
    bd_size_t erase_size   = bd.get_erase_size();
    bd_size_t size         = bd.size();

    printf("--- Block device geometry ---\n");
    printf("read_size:    %lld B\n", read_size);
    printf("program_size: %lld B\n", program_size);
    printf("erase_size:   %lld B\n", erase_size);
    printf("size:         %lld B\n", size);
    printf("---\n");

    // Allocate a block with enough space for our data, aligned to the
    // nearest program_size. This is the minimum size necessary to write
    // data to a block.
    size_t buffer_size = sizeof("Hello Storage!") + program_size-1;
    buffer_size = buffer_size - (buffer_size % program_size);
    printf("Buffer Size: %d \r\n",buffer_size);
    // char *buffer = new char[buffer_size];
    char buffer[buffer_size] = {0};

    // Read what is currently stored on the block device. We haven't written
    // yet so this may be garbage
    printf("bd.read(%p, %d, %d)\n", buffer, 0, buffer_size);
    err = bd.read(buffer, 0, buffer_size);
    printf("bd.read -> %d\n", err);

    printf("--- Stored data ---\n");
    for (size_t i = 0; i < buffer_size; i += 16) {
        for (size_t j = 0; j < 16; j++) {
            if (i+j < buffer_size) {
                printf("%02x ", buffer[i+j]);
            } else {
                printf("   ");
            }
        }

        printf(" %.*s\n", buffer_size - i, &buffer[i]);
    }
    printf("---\n");

    // Update buffer with our string we want to store
    strncpy(buffer, "Hello Storage!", buffer_size);

    // Write data to first block, write occurs in two parts,
    // an erase followed by a program
    printf("bd.erase(%d, %lld)\r\n", 0, erase_size);
    err = bd.erase(0, erase_size);
    printf("bd.erase -> %d\n", err);

    printf("bd.program(%p, %d, %d)\n", buffer, 0, buffer_size);
    err = bd.program(buffer, 0, buffer_size);
    printf("bd.program -> %d\n", err);

    // Clobber the buffer so we don't get old data
    memset(buffer, 0xcc, buffer_size);

    // Read the data from the first block, note that the program_size must be
    // a multiple of the read_size, so we don't have to check for alignment
    printf("bd.read(%p, %d, %d)\n", buffer, 0, buffer_size);
    err = bd.read(buffer, 0, buffer_size);
    printf("bd.read -> %d\n", err);

    printf("--- Stored data ---\n");
    for (size_t i = 0; i < buffer_size; i += 16) {
        for (size_t j = 0; j < 16; j++) {
            if (i+j < buffer_size) {
                printf("%02x ", buffer[i+j]);
            } else {
                printf("   ");
            }
        }

        printf(" %.*s\n", buffer_size - i, &buffer[i]);
    }
    printf("---\n");

    // Deinitialize the block device
    printf("bd.deinit()\n");
    err = bd.deinit();
    printf("bd.deinit -> %d\n", err);

    printf("--- done! ---\n");
}

