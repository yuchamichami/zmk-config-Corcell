target_sources_ifdef(CONFIG_ZMK_INPUT_PROCESSOR_VERTICAL_SCROLL app PRIVATE
  src/pointing/input_processor_vertical_scroll.c
)

target_sources_ifdef(CONFIG_ZMK_INPUT_PROCESSOR_EVENT_DEBUG app PRIVATE
  src/debug/input_processor_event_debug.c
)

target_sources_ifdef(CONFIG_CORCHIBI2_IQS5XX_DEBUG_PROBE app PRIVATE
  src/debug/iqs5xx_debug_probe.c
)

target_sources_ifdef(CONFIG_CORCHIBI2_BATTERY_VOLTAGE_DIVIDER app PRIVATE
  src/battery/corchibi2_battery_voltage_divider.c
)
