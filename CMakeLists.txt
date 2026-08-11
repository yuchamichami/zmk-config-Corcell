target_sources_ifdef(CONFIG_ZMK_INPUT_PROCESSOR_VERTICAL_SCROLL app PRIVATE
  src/pointing/input_processor_vertical_scroll.c
)

target_sources_ifdef(CONFIG_ZMK_INPUT_PROCESSOR_MOTION_DEBUG app PRIVATE
  src/pointing/input_processor_motion_debug.c
)

target_sources_ifdef(CONFIG_CORELL_BATTERY_VOLTAGE_DIVIDER app PRIVATE
  src/battery/corell_battery_voltage_divider.c
)
