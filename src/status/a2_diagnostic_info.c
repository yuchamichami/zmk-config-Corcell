/* SPDX-License-Identifier: MIT */
#include <zephyr/drivers/i2c.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(corcell_a2_info, LOG_LEVEL_INF);

/* Run after the bus, before IQS9151 init/IRQ registration, avoiding concurrent
 * RDY-window transactions. Match the driver's initial force-comms read, using
 * the default clock-stretch method described in datasheet v1.1 section 12.9.2.
 */
BUILD_ASSERT(CONFIG_I2C_INIT_PRIORITY < 79);
BUILD_ASSERT(CONFIG_INPUT_IQS9151_INIT_PRIORITY > 79);

static const struct i2c_dt_spec a2 = I2C_DT_SPEC_GET(DT_NODELABEL(corcell_a2));

static int a2_diagnostic_info_init(void) {
    if (!i2c_is_ready_dt(&a2)) {
        LOG_WRN("A2 version probe: I2C bus not ready");
        return 0;
    }

    /* Appendix A.1: product 0x1000, major 0x1002, minor 0x1004, SHA 0x1006.
     * One contiguous read-only transaction, with STOP; no configuration write.
     */
    const uint8_t address[] = {0x00, 0x10};
    uint8_t info[10];
    int err = i2c_write_read_dt(&a2, address, sizeof(address), info, sizeof(info));
    if (err) {
        LOG_WRN("A2 version probe failed (%d); sensor driver will still attempt init", err);
        return 0;
    }
    if (sys_get_le16(info) != 0x09bc) {
        LOG_WRN("A2 version probe: unexpected product 0x%04x", sys_get_le16(info));
        return 0;
    }
    LOG_INF("A2 product=0x%04x version=%u.%u product-SHA=0x%08x",
            sys_get_le16(info), sys_get_le16(info + 2), sys_get_le16(info + 4),
            (unsigned int)sys_get_le32(info + 6));
    return 0;
}

SYS_INIT(a2_diagnostic_info_init, POST_KERNEL, 79);
