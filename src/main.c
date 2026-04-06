#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "shared_data.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void)
{
    struct modbus_data_packet incoming_data;

    LOG_INF("Application started. Waiting for Modbus data...\n");

    while (1) {
        /* Block until data is available in the queue */
        if (k_msgq_get(&modbus_msgq, &incoming_data, K_FOREVER) == 0) {
            LOG_INF("[%u] Reg: %d, Val: %d\n",
                    incoming_data.timestamp,
                    incoming_data.reg_addr,
                    incoming_data.value);
        }
    }
    return 0;
}
