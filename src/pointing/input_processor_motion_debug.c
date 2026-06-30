/*
 * Copyright (c) 2026
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_input_processor_motion_debug

#include <stdlib.h>

#include <zephyr/device.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <drivers/input_processor.h>

LOG_MODULE_REGISTER(corchibi2_motion_debug, LOG_LEVEL_INF);

struct motion_debug_config {
    uint8_t type;
    uint32_t min_interval_ms;
    bool log_zero;
};

struct motion_debug_data {
    int16_t x;
    int16_t y;
    bool has_x;
    bool has_y;
    uint32_t seq;
    int64_t last_log_ms;
};

static const char *dominant_axis(int16_t x, int16_t y) {
    int32_t ax = abs((int32_t)x);
    int32_t ay = abs((int32_t)y);

    if (ax == ay) {
        return "even";
    }

    return ax > ay ? "x" : "y";
}

static void clear_report(struct motion_debug_data *data) {
    data->x = 0;
    data->y = 0;
    data->has_x = false;
    data->has_y = false;
}

static int motion_debug_handle_event(const struct device *dev, struct input_event *event,
                                     uint32_t param1, uint32_t param2,
                                     struct zmk_input_processor_state *state) {
    const struct motion_debug_config *cfg = dev->config;
    struct motion_debug_data *data = dev->data;

    ARG_UNUSED(param1);
    ARG_UNUSED(param2);
    ARG_UNUSED(state);

    if (event->type != cfg->type) {
        return ZMK_INPUT_PROC_CONTINUE;
    }

    switch (event->code) {
    case INPUT_REL_X:
        data->x += event->value;
        data->has_x = true;
        break;
    case INPUT_REL_Y:
        data->y += event->value;
        data->has_y = true;
        break;
    default:
        return ZMK_INPUT_PROC_CONTINUE;
    }

    if (!event->sync) {
        return ZMK_INPUT_PROC_CONTINUE;
    }

    if (!data->has_x && !data->has_y) {
        return ZMK_INPUT_PROC_CONTINUE;
    }

    if (!cfg->log_zero && data->x == 0 && data->y == 0) {
        clear_report(data);
        return ZMK_INPUT_PROC_CONTINUE;
    }

    int64_t now = k_uptime_get();
    if (cfg->min_interval_ms == 0 || now - data->last_log_ms >= cfg->min_interval_ms) {
        int64_t dt = data->last_log_ms == 0 ? 0 : now - data->last_log_ms;
        int32_t ax = abs((int32_t)data->x);
        int32_t ay = abs((int32_t)data->y);

        LOG_INF("twist_raw seq=%u dt_ms=%lld x=%d y=%d abs_x=%d abs_y=%d dom=%s",
                data->seq++, dt, data->x, data->y, ax, ay, dominant_axis(data->x, data->y));
        data->last_log_ms = now;
    }

    clear_report(data);
    return ZMK_INPUT_PROC_CONTINUE;
}

static struct zmk_input_processor_driver_api motion_debug_driver_api = {
    .handle_event = motion_debug_handle_event,
};

#define MOTION_DEBUG_INST(n)                                                                       \
    static const struct motion_debug_config motion_debug_config_##n = {                            \
        .type = DT_INST_PROP_OR(n, type, INPUT_EV_REL),                                            \
        .min_interval_ms = DT_INST_PROP_OR(n, min_interval_ms, 0),                                 \
        .log_zero = DT_INST_PROP(n, log_zero),                                                     \
    };                                                                                             \
    static struct motion_debug_data motion_debug_data_##n = {};                                    \
    DEVICE_DT_INST_DEFINE(n, NULL, NULL, &motion_debug_data_##n, &motion_debug_config_##n,         \
                          POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                        \
                          &motion_debug_driver_api);

DT_INST_FOREACH_STATUS_OKAY(MOTION_DEBUG_INST)
