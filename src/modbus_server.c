#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/modbus/modbus.h>
#include "shared_data.h"

#include <zephyr/storage/flash_map.h>
#include <zephyr/devicetree.h>
#include <zephyr/fs/fcb.h>
#include <zephyr/drivers/flash.h>
#include <errno.h>

#include "shared_data.h"

#ifdef CONFIG_MODBUS_ROLE_SERVER

LOG_MODULE_REGISTER(modbus_server, LOG_LEVEL_DBG);

/* * In Zephyr 4.x, the non-deprecated way to get a partition ID is
 * using DT_FIXED_PARTITION_ID on a DT_NODELABEL.
 */
#define MODBUS_PARTITION_ID DT_FIXED_PARTITION_ID(DT_NODELABEL(modbus_partition))
#define FCB_MAGIC 0x54425553

static struct fcb my_fcb;
static struct flash_sector sectors[16];
static const struct flash_area *fa_ptr;

void init_storage(void) {
    int rc;
    rc = flash_area_open(MODBUS_PARTITION_ID, &fa_ptr);
    if (rc) { return; }

    my_fcb.f_magic = FCB_MAGIC;
    my_fcb.f_sector_cnt = 16;
    my_fcb.f_sectors = sectors;

    rc = fcb_init(MODBUS_PARTITION_ID, &my_fcb);

    // If it fails with -5 (EIO) or is empty, erase it so FCB can take over
    if (rc != 0) {
        LOG_WRN("Formatting FCB storage...");
        flash_area_erase(fa_ptr, 0, fa_ptr->fa_size);
        rc = fcb_init(MODBUS_PARTITION_ID, &my_fcb);
    }

    if (rc == 0) {
        LOG_INF("FCB initialized successfully");
    }
}

static int holding_reg_read(uint16_t addr, uint16_t *reg)
{
    /* For testing: return a dummy value (e.g., 123) for any register read */
    *reg = 123;
    LOG_DBG("Client read register %d, sending value %d", addr, *reg);
    return 0;
}

static int holding_reg_write(uint16_t addr, uint16_t reg)
{
    struct modbus_data_packet pkt = {
        .reg_addr = addr,
        .value = reg,
        .timestamp = k_uptime_get_32()
    };

    struct fcb_entry loc;
    int rc = fcb_append(&my_fcb, sizeof(pkt), &loc);

    if (rc == -ENOSPC) {
        LOG_WRN("FCB Full, rotating oldest sector...\n");
        fcb_rotate(&my_fcb);
        rc = fcb_append(&my_fcb, sizeof(pkt), &loc);
    }

    if (rc == 0) {
        /* Use the fa_ptr obtained from flash_area_open */
        rc = flash_area_write(fa_ptr, loc.fe_data_off, &pkt, sizeof(pkt));
        if (rc == 0) {
            fcb_append_finish(&my_fcb, &loc);
            LOG_INF("Stored to QSPI: Reg %d = %d\n", addr, reg);
        }
    }

    k_msgq_put(&modbus_msgq, &pkt, K_NO_WAIT);
    return 0;
}

#define MODBUS_NODE DT_ALIAS(modbus0)


static struct modbus_user_callbacks server_cbs = {
    .holding_reg_wr = holding_reg_write,
    .holding_reg_rd = holding_reg_read,
};

static struct modbus_iface_param server_param = {
    .mode = MODBUS_MODE_RTU,
    .rx_timeout = 50000,
    .serial = {
        .baud = 115200,
        .parity = UART_CFG_PARITY_NONE
    },
    .server = {
        .user_cb = &server_cbs,
        .unit_id = 1,
    }
};


// Apply the same fix to the server thread
void modbus_server_thread(void *p1, void *p2, void *p3)
{
    init_storage();

    const char iface_name[] = "MODBUS_0";
    int iface = modbus_iface_get_by_name(iface_name);
    LOG_INF("Modbus interface index: %d for device: %s",
         iface,
         DEVICE_DT_NAME(DT_ALIAS(modbus0)));

    if (iface < 0) {
        LOG_ERR("Failed to get Modbus interface");
        return;
    }

    if (modbus_init_server(iface, server_param)) {
        LOG_ERR("Modbus server failed to initialize!");
        return;
    }
    LOG_INF("Modbus server initialized on Unit ID 1");

    while (1) {
        /* In Zephyr, the stack handles server responses in the background
           once initialized, but we keep the thread alive. */
        k_sleep(K_FOREVER);
    }
}

K_THREAD_DEFINE(modbus_srv_tid, 2048, modbus_server_thread, NULL, NULL, NULL, 7, 0, 0);

#endif
