```yaml
# example configuration:

external_components:
  - source: 
      type: local
      path: custom_components

uart:
  id: uart_bus
  tx_pin: GPIO15
  rx_pin: GPIO13
  baud_rate: 2400

cyberpower:
  id: ups
  uart_id: uart_bus

text_sensor:
  - platform: cyberpower
    model:
      name: "Model name"
    firmware_version:
      name: "Firmware version"
    status:
      name: "Status"
    alarm_control:
      name: "Warning"
    audible_alarm:
      name: "Audible alarm control"
    beeper_control:
      name: "Beeper control"
    test_status:
      name: "Test status"
      
sensor:
  - platform: cyberpower
    battery_level:
      name: "Battery level"
    battery_voltage:
      name: "Battery voltage"
    runtime:
      name: "Runtime"
    designed_power:
      name: "Designed power"
    designed_battery_voltage:
      name: "Designed battery voltage"
    remaining_time_limit:
      name: "Remaining time limit"
    output_voltage:
      name: "Output voltage"
    input_voltage:
      name: "Input voltage"
    load_percentage:
      name: "Load percentage"
    active_power:
      name: "Active power"
    unknown_flag:
      name: "Unknown flag"
    low_voltage_transfer:
      name: "Low voltage transfer"
    high_voltage_transfer:
      name: "High voltage transfer"
    delay_shutdown:
      name: "Sutdown delay"
    delay_startup:
      name: "Startup delay"
    nominal_voltage:
      name: "Nominal voltage"
    
switch:
  - platform: cyberpower
    beeper:
      name: "UPS Beeper"
    power:
      name: "UPS Load Power"
```
