/*
 * Copyright (c) 2020 The ZMK Contributors
 * Copyright (c) 2026 The Corell contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT corell_battery_voltage_divider

#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct corell_battery_value {
    uint16_t adc_raw;
    uint16_t millivolts;
    uint8_t state_of_charge;
};

struct corell_io_channel_config {
    uint8_t channel;
};

struct corell_bvd_config {
    struct corell_io_channel_config io_channel;
    struct gpio_dt_spec power;
    uint32_t output_ohm;
    uint32_t full_ohm;
    const uint16_t *mv_to_pct_thresholds;
    size_t mv_to_pct_thresholds_len;
};

struct corell_bvd_data {
    const struct device *adc;
    struct adc_channel_cfg acc;
    struct adc_sequence as;
    struct corell_battery_value value;
};

static uint8_t corell_bvd_mv_to_pct(const struct corell_bvd_config *cfg,
                                       uint16_t millivolts) {
    const uint16_t *thresholds = cfg->mv_to_pct_thresholds;
    size_t last = cfg->mv_to_pct_thresholds_len - 1;

    if (millivolts <= thresholds[0]) {
        return 0;
    }

    if (millivolts >= thresholds[last]) {
        return 100;
    }

    for (size_t i = 1; i <= last; i++) {
        if (millivolts > thresholds[i]) {
            continue;
        }

        uint32_t low_mv = thresholds[i - 1];
        uint32_t high_mv = thresholds[i];
        uint32_t low_pct = (i - 1) * 100U / last;
        uint32_t high_pct = i * 100U / last;

        return low_pct + (millivolts - low_mv) * (high_pct - low_pct) / (high_mv - low_mv);
    }

    return 100;
}

static int corell_battery_channel_get(const struct corell_battery_value *value,
                                         enum sensor_channel chan,
                                         struct sensor_value *val_out) {
    switch (chan) {
    case SENSOR_CHAN_GAUGE_VOLTAGE:
        val_out->val1 = value->millivolts / 1000;
        val_out->val2 = (value->millivolts % 1000) * 1000U;
        break;

    case SENSOR_CHAN_GAUGE_STATE_OF_CHARGE:
        val_out->val1 = value->state_of_charge;
        val_out->val2 = 0;
        break;

    default:
        return -ENOTSUP;
    }

    return 0;
}

static int corell_bvd_sample_fetch(const struct device *dev, enum sensor_channel chan) {
    struct corell_bvd_data *drv_data = dev->data;
    const struct corell_bvd_config *drv_cfg = dev->config;
    struct adc_sequence *as = &drv_data->as;

    if (chan != SENSOR_CHAN_GAUGE_VOLTAGE && chan != SENSOR_CHAN_GAUGE_STATE_OF_CHARGE &&
        chan != SENSOR_CHAN_ALL) {
        LOG_DBG("Selected channel is not supported: %d", chan);
        return -ENOTSUP;
    }

    int rc = 0;

#if DT_INST_NODE_HAS_PROP(0, power_gpios)
    rc = gpio_pin_set_dt(&drv_cfg->power, 1);
    if (rc != 0) {
        LOG_DBG("Failed to enable ADC power GPIO: %d", rc);
        return rc;
    }

    k_sleep(K_MSEC(10));
#endif

    rc = adc_read(drv_data->adc, as);
    as->calibrate = false;

    if (rc == 0) {
        int32_t val = drv_data->value.adc_raw;

        adc_raw_to_millivolts(adc_ref_internal(drv_data->adc), drv_data->acc.gain, as->resolution,
                              &val);

        uint16_t millivolts = val * (uint64_t)drv_cfg->full_ohm / drv_cfg->output_ohm;
        uint8_t percent = corell_bvd_mv_to_pct(drv_cfg, millivolts);

        LOG_DBG("ADC raw %d ~ %d mV => %d mV, %d%%", drv_data->value.adc_raw, val, millivolts,
                percent);

        drv_data->value.millivolts = millivolts;
        drv_data->value.state_of_charge = percent;
    } else {
        LOG_DBG("Failed to read ADC: %d", rc);
    }

#if DT_INST_NODE_HAS_PROP(0, power_gpios)
    int rc2 = gpio_pin_set_dt(&drv_cfg->power, 0);
    if (rc2 != 0) {
        LOG_DBG("Failed to disable ADC power GPIO: %d", rc2);
        return rc2;
    }
#endif

    return rc;
}

static int corell_bvd_channel_get(const struct device *dev, enum sensor_channel chan,
                                     struct sensor_value *val) {
    struct corell_bvd_data *drv_data = dev->data;
    return corell_battery_channel_get(&drv_data->value, chan, val);
}

static const struct sensor_driver_api corell_bvd_api = {
    .sample_fetch = corell_bvd_sample_fetch,
    .channel_get = corell_bvd_channel_get,
};

static int corell_bvd_init(const struct device *dev) {
    struct corell_bvd_data *drv_data = dev->data;
    const struct corell_bvd_config *drv_cfg = dev->config;

    if (drv_data->adc == NULL) {
        LOG_ERR("ADC failed to retrieve ADC driver");
        return -ENODEV;
    }

#if DT_INST_NODE_HAS_PROP(0, power_gpios)
    if (!device_is_ready(drv_cfg->power.port)) {
        LOG_ERR("GPIO port for power control is not ready");
        return -ENODEV;
    }

    int rc = gpio_pin_configure_dt(&drv_cfg->power, GPIO_OUTPUT_INACTIVE);
    if (rc != 0) {
        LOG_ERR("Failed to control feed %u: %d", drv_cfg->power.pin, rc);
        return rc;
    }
#endif

    drv_data->as = (struct adc_sequence){
        .channels = BIT(drv_cfg->io_channel.channel),
        .buffer = &drv_data->value.adc_raw,
        .buffer_size = sizeof(drv_data->value.adc_raw),
        .oversampling = 4,
        .calibrate = true,
    };

#ifdef CONFIG_ADC_NRFX_SAADC
    drv_data->acc = (struct adc_channel_cfg){
        .gain = ADC_GAIN_1_6,
        .reference = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 40),
        .input_positive = SAADC_CH_PSELP_PSELP_AnalogInput0 + drv_cfg->io_channel.channel,
    };

    drv_data->as.resolution = 12;
#else
#error Unsupported ADC
#endif

    int rc = adc_channel_setup(drv_data->adc, &drv_data->acc);
    LOG_DBG("AIN%u setup returned %d", drv_cfg->io_channel.channel, rc);

    return rc;
}

static struct corell_bvd_data corell_bvd_data = {
    .adc = DEVICE_DT_GET(DT_IO_CHANNELS_CTLR(DT_DRV_INST(0)))};

static const uint16_t corell_bvd_mv_to_pct_thresholds[] =
    DT_INST_PROP(0, mv_to_pct_thresholds);

static const struct corell_bvd_config corell_bvd_cfg = {
    .io_channel =
        {
            DT_IO_CHANNELS_INPUT(DT_DRV_INST(0)),
        },
#if DT_INST_NODE_HAS_PROP(0, power_gpios)
    .power = GPIO_DT_SPEC_INST_GET(0, power_gpios),
#endif
    .output_ohm = DT_INST_PROP(0, output_ohms),
    .full_ohm = DT_INST_PROP(0, full_ohms),
    .mv_to_pct_thresholds = corell_bvd_mv_to_pct_thresholds,
    .mv_to_pct_thresholds_len = ARRAY_SIZE(corell_bvd_mv_to_pct_thresholds),
};

DEVICE_DT_INST_DEFINE(0, &corell_bvd_init, NULL, &corell_bvd_data, &corell_bvd_cfg,
                      POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY, &corell_bvd_api);
