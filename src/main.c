#include <zephyr/kernel.h>
#include <zephyr/modbus/modbus.h>
#include <zephyr/device.h>

static int iface;

#if defined(SERVER_ROLE)
static struct modbus_user_callbacks server_cbs = {
    .holding_reg_rd = NULL,
    .holding_reg_wr = NULL,
};

static struct modbus_iface_param server_param = {
    .mode = MODBUS_MODE_RTU,
    .serial = {
        .baud = 19200,
        .parity = UART_CFG_PARITY_NONE,
    },
    .server = {
        .user_cb = &server_cbs,
    }
};
#endif

#if defined(CLIENT_ROLE)
static struct modbus_iface_param client_param = {
    .mode = MODBUS_MODE_RTU,
    .serial = {
        .baud = 19200,
        .parity = UART_CFG_PARITY_NONE,
    },
};
#endif

#define MODBUS_NODE DT_ALIAS(modbus0)

int main(void)
{
    /* Modbus Initialization */
    iface = modbus_iface_get_by_name(DT_NODE_FULL_NAME(MODBUS_NODE));
    if (iface < 0) {
        printk("Failed to get Modbus interface\n");
        return 0;
    }

#if defined(SERVER_ROLE)
    if (modbus_init_server(iface, server_param)) {
        printk("Server Modbus Init Failed\n");
    }
    printk("Server Init success\n");
#elif defined(CLIENT_ROLE)
    if (modbus_init_client(iface, client_param)) {
        printk("Client Init Failed\n");
    }
     printk("Client Modbus Init success\n");
#endif

    while (1) {
        // Your Modbus logic goes here
        k_sleep(K_MSEC(1000));
    }

    return 0;
}
