/* SPDX-License-Identifier: MIT */
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/byteorder.h>

LOG_MODULE_REGISTER(a2_rdy_probe, LOG_LEVEL_INF);
static const struct i2c_dt_spec bus = I2C_DT_SPEC_GET(DT_NODELABEL(corcell_a2));
static const struct gpio_dt_spec rdy = GPIO_DT_SPEC_GET(DT_NODELABEL(corcell_a2), irq_gpios);
static struct gpio_callback callback;
static atomic_t edges;

static void edge_callback(const struct device *port, struct gpio_callback *cb, uint32_t pins) {
    ARG_UNUSED(port);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);
    atomic_inc(&edges);
}

static void probe(void *a, void *b, void *c) {
    ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);
    LOG_INF("A2 RDY probe v3: sensor driver OFF; no reset/config writes; cursor intentionally OFF");
    if (!i2c_is_ready_dt(&bus) || !gpio_is_ready_dt(&rdy)) {
        LOG_ERR("I2C or GPIO controller not ready");
        return;
    }
    int err = gpio_pin_configure_dt(&rdy, GPIO_INPUT);
    if (err) { LOG_ERR("RDY input setup failed: %d", err); return; }
    gpio_init_callback(&callback, edge_callback, BIT(rdy.pin));
    err = gpio_add_callback(rdy.port, &callback);
    if (err) { LOG_ERR("RDY callback failed: %d", err); return; }
    err = gpio_pin_interrupt_configure_dt(&rdy, GPIO_INT_EDGE_BOTH);
    if (err) {
        gpio_remove_callback(rdy.port, &callback);
        LOG_ERR("RDY edge setup failed: %d", err);
        return;
    }
    LOG_INF("RDY %s pin %u, active-low; observing 5 seconds without I2C access",
            rdy.port->name, rdy.pin);
    for (int i = 0; i < 5; ++i) {
        k_msleep(1000);
        LOG_INF("RDY physical=%d logical-active=%d edges=%ld",
                gpio_pin_get_raw(rdy.port, rdy.pin), gpio_pin_get_dt(&rdy),
                (long)atomic_get(&edges));
    }
    /* One read of the read-only ID block, never a configuration write.
     * Prefer an active RDY window. If none arrives, report that explicitly
     * before one force-comms read (datasheet 12.9.2 default stretch method).
     */
    int ready = gpio_pin_get_dt(&rdy);
    int64_t deadline = k_uptime_get() + 1000;
    while (ready == 0 && k_uptime_get() < deadline) {
        k_msleep(1);
        ready = gpio_pin_get_dt(&rdy);
    }
    if (ready < 0) { LOG_ERR("RDY read failed: %d", ready); goto done; }
    LOG_INF("ID read: RDY active=%d (%s)", ready,
            ready ? "RDY window" : "forced; no RDY window observed");
    const uint8_t address[] = {0x00, 0x10};
    uint8_t id[10];
    err = i2c_write_read_dt(&bus, address, sizeof(address), id, sizeof(id));
    if (err) {
        LOG_ERR("ID read failed: %d", err);
    } else {
        LOG_INF("ID=0x%04x version=%u.%u SHA=0x%08x", sys_get_le16(id),
                sys_get_le16(id + 2), sys_get_le16(id + 4),
                (unsigned int)sys_get_le32(id + 6));
    }
    k_msleep(1000);
    LOG_INF("After read: RDY physical=%d edges=%ld; probe complete",
            gpio_pin_get_raw(rdy.port, rdy.pin), (long)atomic_get(&edges));
done:
    gpio_pin_interrupt_configure_dt(&rdy, GPIO_INT_DISABLE);
    gpio_remove_callback(rdy.port, &callback);
}

K_THREAD_DEFINE(a2_probe_thread, 1536, probe, NULL, NULL, NULL, 10, 0, 12000);
