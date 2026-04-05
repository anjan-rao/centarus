#include <zephyr/kernel.h>
#include <zephyr/modbus/modbus.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <lvgl.h>
#include <stdio.h>

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
#define DISPLAY_NODE DT_CHOSEN(zephyr_display)

int main(void)
{
    /* 1. Modbus Initialization */
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
     printk("Client Modbud Init success\n");
#endif

    /* 2. Display Initialization */
    const struct device *display_dev = DEVICE_DT_GET(DISPLAY_NODE);

    if (!device_is_ready(display_dev)) {
        printk("Display device %s not ready\n", display_dev->name);
        return 0;
    }

    struct display_capabilities caps;
    display_get_capabilities(display_dev, &caps);

    /* Now that 'caps' is populated, we can define 'desc' */
    const struct display_buffer_descriptor desc = {
        .width = caps.x_resolution,
        .height = caps.y_resolution,
        .pitch = caps.x_resolution,
    };

    /* Suppress the unused variable warning until you use 'desc' in a
       display_write() call later */
    (void)desc;

    printk("Display initialized: %d x %d\n", caps.x_resolution, caps.y_resolution);

    /* 1. Create a label widget */
    lv_obj_t *hello_label = lv_label_create(lv_scr_act());

    /* 2. Set the text */
    lv_label_set_text(hello_label, "Centarus");

    /* 3. Center the label on the screen */
    lv_obj_align(hello_label, LV_ALIGN_CENTER, 0, 0);

    /* 4. Force a refresh to show the text immediately */
    lv_task_handler();

    /* 5. Wait for 1 second */
    k_sleep(K_MSEC(1000));

    /* 6. Delete the object to make it disappear */
    lv_obj_del(hello_label);

    /* 7. Refresh again to clear the screen */
    lv_task_handler();

    while (1) {
        /* LVGL timer handler should run periodically in the background */
        lv_timer_handler();
        k_sleep(K_MSEC(10));
    }

    return 0;
}
