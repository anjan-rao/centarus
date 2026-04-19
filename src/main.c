#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "shared_data.h"

K_MSGQ_DEFINE(modbus_msgq, sizeof(struct modbus_data_packet), 10, 4);

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

static const struct gpio_dt_spec hb_led = GPIO_DT_SPEC_GET(DT_ALIAS(heartbeat_led), gpios);

int main(void)
{
    struct modbus_data_packet incoming_data;

    if (!gpio_is_ready_dt(&hb_led)) {
        LOG_ERR("LED device not ready");
        return 0;
    }

    gpio_pin_configure_dt(&hb_led, GPIO_OUTPUT_ACTIVE);
    LOG_INF("Application started. Heartbeat active.\n");

    while (1) {
        /* Toggle LED every 500ms */
        gpio_pin_toggle_dt(&hb_led);

        /* Wait for data with a timeout so the loop (and LED) keeps running */
        if (k_msgq_get(&modbus_msgq, &incoming_data, K_MSEC(500)) == 0) {
            LOG_INF("[%u] Reg: %d, Val: %d\n",
                    incoming_data.timestamp,
                    incoming_data.reg_addr,
                    incoming_data.value);
        }
    }
    return 0;
}
