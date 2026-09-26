target_sources_ifdef(CONFIG_ZMK_INPUT_PROCESSOR_VERTICAL_SCROLL app PRIVATE
  src/pointing/input_processor_vertical_scroll.c
)

target_sources_ifdef(CONFIG_CORCELL_BATTERY_VOLTAGE_DIVIDER app PRIVATE
  src/battery/corcell_battery_voltage_divider.c
)

target_sources_ifdef(CONFIG_CORCELL_POWER_ON_LED app PRIVATE
  src/status/power_on_led.c
)
