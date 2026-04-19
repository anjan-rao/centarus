#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/modbus/modbus.h>
#include "shared_data.h"

/* Keep only ONE registration. Use DBG to see detailed Modbus traffic. */
LOG_MODULE_REGISTER(modbus_client, LOG_LEVEL_DBG);

#ifdef CONFIG_MODBUS_ROLE_CLIENT

void modbus_client_thread(void *p1, void *p2, void *p3)
{
    /* Use the label defined in your overlay */

    const char iface_name[] = "MODBUS_0";
    int iface = modbus_iface_get_by_name(iface_name);
    LOG_INF("Modbus interface index: %d for device: %s",
         iface,
         DEVICE_DT_NAME(DT_ALIAS(modbus0)));
    uint16_t read_val;
    int err;

    static struct modbus_iface_param client_param = {
        .mode = MODBUS_MODE_RTU,
        .rx_timeout = 50000,
        .serial = {.baud = 115200, .parity = UART_CFG_PARITY_NONE}
    };

    if (iface < 0) {
        LOG_ERR("Could not find Modbus interface");
        return;
    }

    if (modbus_init_client(iface, client_param)) {
        LOG_ERR("Failed to initialize Modbus Client");
        return;
    }

    LOG_INF("Modbus Client polling started...");

    while (1) {
        /* Poll Server ID 1, Holding Register 0 (often mapped to '1' in PLC tools) */
        err = modbus_read_holding_regs(iface, 1, 0, &read_val, 1);

        if (err == 0) {
            LOG_INF("SUCCESS: Read Reg 0 = %d", read_val);
            struct modbus_data_packet pkt = {
                .reg_addr = 0,
                .value = read_val,
                .timestamp = k_uptime_get_32()
            };
            k_msgq_put(&modbus_msgq, &pkt, K_NO_WAIT);
        } else {
            /* Error -116 is ETIMEDOUT (Wiring/ID issue) */
            LOG_WRN("FAILED: err %d", err);
        }

        k_msleep(20000);
    }
}

K_THREAD_DEFINE(modbus_cli_tid, 2048, modbus_client_thread, NULL, NULL, NULL, 7, 0, 0);
#endif
