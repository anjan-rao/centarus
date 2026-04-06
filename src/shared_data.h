#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <zephyr/kernel.h>

/* Structure for the data you want to save/pass */
struct modbus_data_packet {
    uint16_t reg_addr;
    uint16_t value;
    uint32_t timestamp;
};

/* Define the queue: 10 items max, aligned to 4 bytes */
extern struct k_msgq modbus_msgq;

#endif

