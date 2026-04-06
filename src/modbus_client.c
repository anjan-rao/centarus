#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/modbus/modbus.h>

#include "shared_data.h"

LOG_MODULE_REGISTER(modbus_client, LOG_LEVEL_INF);

#define MODBUS_NODE DT_ALIAS(modbus0)

K_MSGQ_DEFINE(modbus_msgq, sizeof(struct modbus_data_packet), 10, 4);

static struct modbus_iface_param client_param = {
    .mode = MODBUS_MODE_RTU,
    .serial = {.baud = 19200, .parity = UART_CFG_PARITY_NONE}
};

void modbus_client_thread(void *p1, void *p2, void *p3)
{
    int iface = modbus_iface_get_by_name(DT_NODE_FULL_NAME(MODBUS_NODE));
    uint16_t read_val;

    if (modbus_init_client(iface, client_param)) {
        LOG_ERR("Failed to initialize Modbus Client on %s", DT_NODE_FULL_NAME(MODBUS_NODE));
        return;
    }

    while (1) {
        /* Example: Polling Holding Register 1 from Server Address 1 */
        if (modbus_read_holding_regs(iface, 1, 1, &read_val, 1) == 0) {
            LOG_DBG("Poll success: Reg 1 = %d", read_val);
            struct modbus_data_packet pkt = {
                .reg_addr = 1,
                .value = read_val,
                .timestamp = k_uptime_get_32()
            };
            k_msgq_put(&modbus_msgq, &pkt, K_NO_WAIT);
        } else {
            LOG_WRN("Modbus poll timeout/error");
        }
        k_msleep(1000); // Poll interval
    }
}

K_THREAD_DEFINE(modbus_cli_tid, 1024, modbus_client_thread, NULL, NULL, NULL, 7, 0, 0);

