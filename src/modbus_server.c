#include <zephyr/kernel.h>
#include <zephyr/modbus/modbus.h>
#include "shared_data.h"

#define MODBUS_NODE DT_ALIAS(modbus0)

K_MSGQ_DEFINE(modbus_msgq, sizeof(struct modbus_data_packet), 10, 4);

static int holding_reg_write(uint16_t addr, uint16_t reg)
{
    struct modbus_data_packet pkt = {
        .reg_addr = addr,
        .value = reg,
        .timestamp = k_uptime_get_32()
    };

    /* Non-blocking put into queue */
    if (k_msgq_put(&modbus_msgq, &pkt, K_NO_WAIT) != 0) {
        // Queue full - handle overflow if necessary
    }
    return 0;
}

static struct modbus_user_callbacks server_cbs = {
    .holding_reg_wr = holding_reg_write,
};

static struct modbus_iface_param server_param = {
    .mode = MODBUS_MODE_RTU,
    .serial = {.baud = 19200, .parity = UART_CFG_PARITY_NONE},
    .server = {.user_cb = &server_cbs}
};

void modbus_server_thread(void *p1, void *p2, void *p3)
{
    int iface = modbus_iface_get_by_name(DT_NODE_FULL_NAME(MODBUS_NODE));
    
    if (modbus_init_server(iface, server_param)) {
        return;
    }

    while (1) {
        /* In Zephyr, the stack handles server responses in the background 
           once initialized, but we keep the thread alive. */
        k_sleep(K_FOREVER);
    }
}

K_THREAD_DEFINE(modbus_srv_tid, 1024, modbus_server_thread, NULL, NULL, NULL, 7, 0, 0);

