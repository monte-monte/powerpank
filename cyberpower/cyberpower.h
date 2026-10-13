#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace cyberpower {

static const uint8_t MAX_LINE_LENGTH = 16;  // Max characters for serial buffer

class Cyberpower;

class CyberpowerSwitch : public switch_::Switch, public Component {
  public:
    void set_parent(Cyberpower* parent) { parent_ = parent; }
    void set_command_id(uint8_t cmd) { cmd_ = cmd; }
  
  protected:
    Cyberpower* parent_{nullptr};
    uint8_t cmd_;
    void write_state(bool state) override;
};

class Cyberpower : public PollingComponent, public uart::UARTDevice {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;

  void set_model_name_sensor(text_sensor::TextSensor *sensor) { count_++; model_name_sensor_ = sensor; }
  void set_firmware_sensor(text_sensor::TextSensor *sensor) { count_++; firmware_sensor_ = sensor; }
  void set_battery_percentage_sensor(sensor::Sensor *sensor) { count_++; battery_percentage_sensor_ = sensor; }
  void set_battery_voltage_sensor(sensor::Sensor *sensor) { count_++; battery_voltage_sensor_ = sensor; }
  void set_status_sensor(text_sensor::TextSensor *sensor) { count_++; status_sensor_ = sensor; }
  void set_runtime_sensor(sensor::Sensor *sensor) { count_++; runtime_sensor_ = sensor; }
  void set_input_voltage_sensor(sensor::Sensor *sensor) { count_++; input_voltage_sensor_ = sensor; }
  void set_output_voltage_sensor(sensor::Sensor *sensor) { count_++; output_voltage_sensor_ = sensor; }
  void set_load_percentage_sensor(sensor::Sensor *sensor) { count_++; load_percentage_sensor_ = sensor; }
  void set_active_power_sensor(sensor::Sensor *sensor) { count_++; active_power_sensor_ = sensor; }
  void set_alarm_control_sensor(text_sensor::TextSensor *sensor) { count_++; alarm_control_sensor_ = sensor; }
  void set_beeper_control_sensor(text_sensor::TextSensor *sensor) { count_++; beeper_control_sensor_ = sensor; }
  void set_unknown_flag_sensor(sensor::Sensor *sensor) { count_++; unknown_flag_sensor_ = sensor; }
  void set_designed_power_sensor(sensor::Sensor *sensor) { count_++; designed_power_sensor_ = sensor; }
  void set_designed_battery_voltage_sensor(sensor::Sensor *sensor) { count_++; designed_battery_voltage_sensor_ = sensor; }
  void set_audible_alarm_control_sensor(text_sensor::TextSensor *sensor) { count_++; audible_alarm_control_sensor_ = sensor; }
  void set_remaining_time_limit_sensor(sensor::Sensor *sensor) { count_++; remaining_time_limit_sensor_ = sensor; }
  void set_low_voltage_sensor(sensor::Sensor *sensor) { count_++; low_voltage_sensor_ = sensor; }
  void set_high_voltage_sensor(sensor::Sensor *sensor) { count_++; high_voltage_sensor_ = sensor; }
  void set_test_status_sensor(text_sensor::TextSensor *sensor) { count_++; test_status_sensor_ = sensor; }
  void set_delay_shutdown_sensor(sensor::Sensor *sensor) { count_++; delay_shutdown_sensor_ = sensor; }
  void set_delay_startup_sensor(sensor::Sensor *sensor) { count_++; delay_startup_sensor_ = sensor; }
  void set_nominal_voltage_sensor(sensor::Sensor *sensor) { count_++; nominal_voltage_sensor_ = sensor; }

  void set_beeper(CyberpowerSwitch *sw) { beeper_switch_ = sw; }
  void set_power(CyberpowerSwitch *sw) { power_switch_ = sw; }
  void send_switch_cmd(uint8_t cmd_id, uint8_t state);

 protected:
  bool online_{false};
  bool state_{false};
  uint8_t parsed_data_[MAX_LINE_LENGTH];
  uint8_t parse_data(uint8_t skip = 0);  // A method to parse the data read from the sensor hardware
  uint8_t count_{0};

  int send_read_cmd(uint8_t cmd, uint8_t crc);
  int send_write_cmd(uint8_t cmd, uint32_t val, uint8_t len, uint8_t crc);

  uint8_t buffer_data_[MAX_LINE_LENGTH];
  text_sensor::TextSensor *model_name_sensor_{nullptr}; //0x1d
  text_sensor::TextSensor *firmware_sensor_{nullptr}; //0x1e
  sensor::Sensor *battery_percentage_sensor_{nullptr}; //0x20
  sensor::Sensor *battery_voltage_sensor_{nullptr}; //0x20
  sensor::Sensor *runtime_sensor_{nullptr}; //0x21
  text_sensor::TextSensor *status_sensor_{nullptr}; //0x22
  sensor::Sensor *input_voltage_sensor_{nullptr}; //0x23
  sensor::Sensor *output_voltage_sensor_{nullptr}; //0x23
  sensor::Sensor *load_percentage_sensor_{nullptr}; //0x25
  sensor::Sensor *active_power_sensor_{nullptr}; //0x27
  text_sensor::TextSensor *alarm_control_sensor_{nullptr}; //0x28
  text_sensor::TextSensor *beeper_control_sensor_{nullptr}; //0x29
  sensor::Sensor *unknown_flag_sensor_{nullptr}; //0x3d
  sensor::Sensor *designed_power_sensor_{nullptr}; //0x3e
  sensor::Sensor *designed_battery_voltage_sensor_{nullptr}; //0x3f
  text_sensor::TextSensor *audible_alarm_control_sensor_{nullptr}; //0x80
  sensor::Sensor *remaining_time_limit_sensor_{nullptr}; //0x82
  sensor::Sensor *low_voltage_sensor_{nullptr}; //0x83
  sensor::Sensor *high_voltage_sensor_{nullptr}; //0x84
  text_sensor::TextSensor *test_status_sensor_{nullptr}; //0x85
  sensor::Sensor *delay_shutdown_sensor_{nullptr}; //0x86
  sensor::Sensor *delay_startup_sensor_{nullptr}; //0x87
  sensor::Sensor *nominal_voltage_sensor_{nullptr}; //0x88

  CyberpowerSwitch *beeper_switch_{nullptr};
  CyberpowerSwitch *power_switch_{nullptr};

};

}  // namespace cyberpower
}  // namespace esphome
