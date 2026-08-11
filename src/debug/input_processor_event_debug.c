/*
 * Copyright (c) 2026
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_input_processor_event_debug

#include <zephyr/device.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <drivers/input_processor.h>

LOG_MODULE_REGISTER(corell_event_debug, LOG_LEVEL_INF);

struct event_debug_config {
    uint32_t min_interval_ms;
};

struct event_debug_data {
    uint32_t seq;
    int64_t last_log_ms;
};

static int event_debug_handle_event(const struct device *dev, struct input_event *event,
                                    uint32_t param1, uint32_t param2,
                                    struct zmk_input_processor_state *state) {
    const struct event_debug_config *cfg = dev->config;
    struct event_debug_data *data = dev->data;

    ARG_UNUSED(param1);
    ARG_UNUSED(param2);
    ARG_UNUSED(state);

    int64_t now = k_uptime_get();
    if (cfg->min_interval_ms != 0 && now - data->last_log_ms < cfg->min_interval_ms) {
        return ZMK_INPUT_PROC_CONTINUE;
    }

    LOG_INF("trackpad_event seq=%u dt_ms=%lld type=%u code=%u value=%d sync=%u", data->seq++,
            data->last_log_ms == 0 ? 0 : now - data->last_log_ms, event->type, event->code,
            event->value, event->sync);
    data->last_log_ms = now;

    return ZMK_INPUT_PROC_CONTINUE;
}

static struct zmk_input_processor_driver_api event_debug_driver_api = {
    .handle_event = event_debug_handle_event,
};

#define EVENT_DEBUG_INST(n)                                                                       \
    static const struct event_debug_config event_debug_config_##n = {                              \
        .min_interval_ms = DT_INST_PROP_OR(n, min_interval_ms, 0),                                 \
    };                                                                                             \
    static struct event_debug_data event_debug_data_##n = {};                                      \
    DEVICE_DT_INST_DEFINE(n, NULL, NULL, &event_debug_data_##n, &event_debug_config_##n,           \
                          POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                        \
                          &event_debug_driver_api);

DT_INST_FOREACH_STATUS_OKAY(EVENT_DEBUG_INST)
