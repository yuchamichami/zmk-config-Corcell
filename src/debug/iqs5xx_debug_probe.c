/*
 * Copyright (c) 2026
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(corchibi2_iqs5xx_probe, LOG_LEVEL_INF);

#if !DT_HAS_CHOSEN(corchibi2_debug_trackpad)
#error "CONFIG_CORCHIBI2_IQS5XX_DEBUG_PROBE requires chosen corchibi2,debug-trackpad"
#endif

#define TRACKPAD_NODE DT_CHOSEN(corchibi2_debug_trackpad)

#if !DT_NODE_HAS_STATUS(TRACKPAD_NODE, okay)
#error "chosen corchibi2,debug-trackpad must point to an enabled IQS5xx node"
#endif

static const struct i2c_dt_spec trackpad_i2c = I2C_DT_SPEC_GET(TRACKPAD_NODE);
static const struct gpio_dt_spec rdy_gpio = GPIO_DT_SPEC_GET(TRACKPAD_NODE, rdy_gpios);

static int read_reg8(uint16_t addr, uint16_t reg, uint8_t *val) {
    uint8_t reg_buf[2] = {reg >> 8, reg & 0xff};

    return i2c_write_read(trackpad_i2c.bus, addr, reg_buf, sizeof(reg_buf), val, 1);
}

static int read_reg16(uint16_t addr, uint16_t reg, uint16_t *val) {
    uint8_t reg_buf[2] = {reg >> 8, reg & 0xff};
    uint8_t buf[2];
    int ret = i2c_write_read(trackpad_i2c.bus, addr, reg_buf, sizeof(reg_buf), buf, sizeof(buf));

    if (ret < 0) {
        return ret;
    }

    *val = ((uint16_t)buf[0] << 8) | buf[1];
    return 0;
}

static void end_comm_window(uint16_t addr) {
    uint8_t buf[4] = {0xee, 0xee, 0x00, 0x00};
    int ret = i2c_write(trackpad_i2c.bus, buf, sizeof(buf), addr);

    if (ret < 0) {
        LOG_DBG("addr=0x%02x end_comm failed ret=%d", addr, ret);
    }
}

static void probe_addr(uint16_t addr) {
    uint8_t sys0 = 0;
    uint8_t sys1 = 0;
    uint8_t fingers = 0;
    uint16_t rel_x = 0;

    int ret_sys0 = read_reg8(addr, 0x000f, &sys0);
    int ret_sys1 = read_reg8(addr, 0x0010, &sys1);
    int ret_fingers = read_reg8(addr, 0x0011, &fingers);
    int ret_rel_x = read_reg16(addr, 0x0012, &rel_x);

    if (ret_sys0 < 0 && ret_sys1 < 0 && ret_fingers < 0 && ret_rel_x < 0) {
        LOG_INF("probe addr=0x%02x no response sys0=%d sys1=%d fingers=%d rel_x=%d", addr,
                ret_sys0, ret_sys1, ret_fingers, ret_rel_x);
        return;
    }

    LOG_INF("probe addr=0x%02x ok sys0=%d/0x%02x sys1=%d/0x%02x fingers=%d/0x%02x rel_x=%d/%d",
            addr, ret_sys0, sys0, ret_sys1, sys1, ret_fingers, fingers, ret_rel_x,
            (int16_t)rel_x);
    end_comm_window(addr);
}

static int iqs5xx_debug_probe_init(void) {
    LOG_INF("IQS5xx debug probe start bus=%s dt_addr=0x%02x", trackpad_i2c.bus->name,
            trackpad_i2c.addr);

    if (!device_is_ready(trackpad_i2c.bus)) {
        LOG_ERR("I2C bus is not ready: %s", trackpad_i2c.bus->name);
        return 0;
    }

    if (!gpio_is_ready_dt(&rdy_gpio)) {
        LOG_ERR("RDY GPIO is not ready");
    } else {
        int ret = gpio_pin_configure_dt(&rdy_gpio, GPIO_INPUT);
        if (ret < 0) {
            LOG_ERR("RDY configure failed ret=%d", ret);
        }

        LOG_INF("RDY gpio port=%s pin=%u dt_flags=0x%x", rdy_gpio.port->name, rdy_gpio.pin,
                rdy_gpio.dt_flags);
        for (int i = 0; i < 8; i++) {
            LOG_INF("RDY sample[%d]=%d", i, gpio_pin_get_dt(&rdy_gpio));
            k_msleep(20);
        }
    }

    probe_addr(trackpad_i2c.addr);
    probe_addr(0x74);
    probe_addr(0x75);
    probe_addr(0x73);

    LOG_INF("IQS5xx debug probe done");
    return 0;
}

SYS_INIT(iqs5xx_debug_probe_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
