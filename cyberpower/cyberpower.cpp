#include "esphome/core/log.h"
#include "cyberpower.h"

namespace esphome {
namespace cyberpower {

static const char *TAG = "cyberpower.sensor";

uint8_t switch_power_off_cmd[] = {0x43, 0xC6, 0x00, 0x00, 0x04};
uint8_t switch_power_on_cmd[] = {0x43, 0xC7, 0x00, 0x00, 0x87};
uint8_t switch_beep_on_cmd[] = {0x42, 0xC0, 0x02, 0xdc};
uint8_t switch_beep_off_cmd[] = {0x42, 0xC0, 0x01, 0x76};

#define RESPONSE_TIMEOUT 30
#define REQUEST_DELAY    50
#define CRC_POLY         0xd5

int Cyberpower::send_read_cmd(uint8_t cmd, uint8_t crc) {
  uint32_t timer = 0;
  uint8_t byte = 0;
  uint8_t long_message = 0;
  uint8_t len = 0;
  uint8_t data_pos = 0;
  uint8_t read_crc = 0;

  if (!crc) {
    uint8_t crc_buf[2] = {0x41, cmd};
    crc = crc8(crc_buf, 2, 0, CRC_POLY, 0);
  }

  memset(parsed_data_, 0, MAX_LINE_LENGTH);
  this->write(0x41);
  this->write(cmd);
  this->write(crc);
  this->flush();
  timer = millis() + RESPONSE_TIMEOUT;
  while ((int32_t)(millis() - timer) < 0) {
    if (this->available() && this->read_byte(&byte)) break;
  }
  if (byte != 0x40) return 0;
  timer = millis() + RESPONSE_TIMEOUT;
  while ((int32_t)(millis() - timer) < 0) {
    if (this->available() && this->read_byte(&byte)) break;
  }
  long_message = !(byte&0x40);
  len = byte&0xf;
  timer = millis() + RESPONSE_TIMEOUT;
  if (!long_message) {
    while ((int32_t)(millis() - timer) < 0) {
      if (this->available() && this->read_byte(&byte)) break;
    }
    if (byte != cmd) return 0;
    timer = millis() + RESPONSE_TIMEOUT;
  }
  while ((int32_t)(millis() - timer) < 0 && len) {
    if (this->available() && this->read_byte(&parsed_data_[data_pos])) {
      data_pos++;
      len--;
      timer = millis() + RESPONSE_TIMEOUT;
    }
  }
  while ((int32_t)(millis() - timer) < 0) {
    if (this->available() && this->read_byte(&read_crc)) break;
  }
  this->write(0x40);
  this->flush();
  if (long_message) {
    timer = millis() + RESPONSE_TIMEOUT;
    while ((int32_t)(millis() - timer) < 0) {
      if (this->available() && this->read_byte(&byte)) break;
    }
    // ESP_LOGW(TAG, "byte = %02x", byte);
    len = byte&0xf;
    timer = millis() + RESPONSE_TIMEOUT;
    while ((int32_t)(millis() - timer) < 0 && len) {
      if (this->available() && this->read_byte(&parsed_data_[data_pos])) {
        // ESP_LOGW(TAG, "parsed_data_[data_pos] = %02x", parsed_data_[data_pos]);
        data_pos++;
        len--;
        timer = millis() + RESPONSE_TIMEOUT;
      }
    }
    while ((int32_t)(millis() - timer) < 0) {
      if (this->available() && this->read_byte(&read_crc)) break;
    }
    this->write(0x40);
    this->flush();
  }

  return data_pos+1;
}

void Cyberpower::setup() {
  uint8_t buffer_pos = 0;
  uint8_t len = 0;
  // this->write(0x40);
  this->flush();
  delay(REQUEST_DELAY);
  while (this->available() && buffer_pos < MAX_LINE_LENGTH && this->read_byte(&this->buffer_data_[buffer_pos++]));
  
  len = send_read_cmd(0x3f, 0x3f);
  if (len) {
    if (this->power_switch_) this->power_switch_->publish_state(true);
    delay(REQUEST_DELAY);
  } else {
    if (this->power_switch_) this->power_switch_->publish_state(false);
    return;
  }
  if (this->designed_battery_voltage_sensor_) {
    if (len) {
      uint32_t value = this->parsed_data_[1] << 8 | this->parsed_data_[0];
      this->designed_battery_voltage_sensor_->publish_state(((float)value)/10);
    }
  }
  
  if (this->model_name_sensor_) {
    len = send_read_cmd(0x1d, 0xe4);
    if (len) {
      std::string model((char*)this->parsed_data_, len);
      this->model_name_sensor_->publish_state(model.c_str());
    }
    delay(REQUEST_DELAY);
  }

  if (this->firmware_sensor_) {
    len = send_read_cmd(0x1e, 0x4e);
    if (len) {
      std::string string((char*)this->parsed_data_, len);
      this->firmware_sensor_->publish_state(string.c_str());
    }
    delay(REQUEST_DELAY);
  }
  
  if (this->unknown_flag_sensor_) {
    len = send_read_cmd(0x3d, 0x40);
    if (len) {
      this->unknown_flag_sensor_->publish_state((float)this->parsed_data_[0]);
    } else {
      ESP_LOGW(TAG, "No data received");
      this->status_set_warning();
    }
    delay(REQUEST_DELAY);
  }

  if (this->designed_power_sensor_) {
    len = send_read_cmd(0x3e, 0xea);
    if (len > 1) {
      uint32_t value = (this->parsed_data_[1] << 8) | this->parsed_data_[0];
      this->designed_power_sensor_->publish_state((float)value);
    } else {
      ESP_LOGW(TAG, "No data received");
      this->status_set_warning();
    }
    delay(REQUEST_DELAY);
  }

  if (this->nominal_voltage_sensor_) {
    len = send_read_cmd(0x88, 0x72);
    if (len > 1) {
      uint32_t value = (this->parsed_data_[1] << 8) | this->parsed_data_[0];
      this->nominal_voltage_sensor_->publish_state((float)value);
    } else {
      ESP_LOGW(TAG, "No data received");
      this->status_set_warning();
    }
    delay(REQUEST_DELAY);
  }
}

void Cyberpower::update() {
  // Work to be done at each update interval
  uint8_t len;
  // ESP_LOGW(TAG, "Update");

  len = send_read_cmd(0x3f, 0x3f);
  if (len) {
    if (this->power_switch_) this->power_switch_->publish_state(true);
    delay(REQUEST_DELAY);
  } else {
    if (this->power_switch_) this->power_switch_->publish_state(false);
    return;
  }

  if (this->battery_percentage_sensor_) {
    len = send_read_cmd(0x20, 0x10);
    if (len) {
      this->battery_percentage_sensor_->publish_state((float)this->parsed_data_[0]);
    } else {
      ESP_LOGW(TAG, "No data received");
      this->status_set_warning();  // We can indicate a warning if no data was read
    }
    delay(REQUEST_DELAY);
  }
  
  if (this->battery_voltage_sensor_) {
    if (!this->battery_percentage_sensor_ || !len) len = send_read_cmd(0x20, 0x10);
    if (len > 1) {
      this->battery_voltage_sensor_->publish_state(((float)this->parsed_data_[1])/10);
    } else {
      ESP_LOGW(TAG, "No data received, len = %d", len);
      this->status_set_warning();
    }
    delay(REQUEST_DELAY);
  }

  if (this->runtime_sensor_) {
    len = send_read_cmd(0x21, 0xc5);
    if (len > 1) {
      uint16_t value = (this->parsed_data_[1] << 8) | this->parsed_data_[0];
      this->runtime_sensor_->publish_state((float)value);
    } else {
      ESP_LOGW(TAG, "No data received");
      this->status_set_warning();
    }
    delay(REQUEST_DELAY);
  }

  if (this->status_sensor_) {
    len = send_read_cmd(0x22, 0x6f);
    if (len) {
      switch (this->parsed_data_[0])
      {
      case 1:
        this->status_sensor_->publish_state("AC present");
        break;
      case 3:
        this->status_sensor_->publish_state("AC present, charging");
        break;
      case 4:
        this->status_sensor_->publish_state("Discharging");
        break;
      case 5:
        this->status_sensor_->publish_state("AC present, discharging");
        break;
      case 9:
        this->status_sensor_->publish_state("AC present, fully charged");
        break;
      default:
        this->status_sensor_->publish_state(std::to_string(this->parsed_data_[0]).c_str());
      }
    } else {
      ESP_LOGW(TAG, "No data received");
      this->status_set_warning();
    }
    delay(REQUEST_DELAY);
  }

  if (this->input_voltage_sensor_) {
    len = send_read_cmd(0x23, 0xba);
    if (len > 1) {
      uint16_t value = (this->parsed_data_[1] << 8) | this->parsed_data_[0];
      this->input_voltage_sensor_->publish_state((float)value);
    } else {
      ESP_LOGW(TAG, "No data received");
      this->status_set_warning();
    }
    delay(REQUEST_DELAY);
  }

  if (this->output_voltage_sensor_) {
    if (!this->input_voltage_sensor_ || !len) len = send_read_cmd(0x23, 0xba);
    if (len > 3) {
      uint16_t value = (this->parsed_data_[3] << 8) | this->parsed_data_[2];
      this->output_voltage_sensor_->publish_state((float)value);
    } else {
      ESP_LOGW(TAG, "No data received");
      this->status_set_warning();
    }
    delay(REQUEST_DELAY);
  }

  if (this->load_percentage_sensor_) {
    len = send_read_cmd(0x25, 0x3b);
    if (len) {
      this->load_percentage_sensor_->publish_state((float)this->parsed_data_[0]);
    } else {
      ESP_LOGW(TAG, "No data received");
      this->status_set_warning();
    }
    delay(REQUEST_DELAY);
  }

  if (this->active_power_sensor_) {
    len = send_read_cmd(0x27, 0x44);
    if (len > 1) {
      uint16_t value = (this->parsed_data_[1] << 8) | this->parsed_data_[0];
      this->active_power_sensor_->publish_state((float)value);
    } else {
      ESP_LOGW(TAG, "Active power. No data received, len = %d", len);
      this->status_set_warning();
    }
    delay(REQUEST_DELAY);
  }

  if (this->alarm_control_sensor_) {
    len = send_read_cmd(0x28, 0x39);
    if (len) {
      switch (this->parsed_data_[0])
      {
      case 0:
        this->alarm_control_sensor_->publish_state("None");
        break;
      case 1:
        this->alarm_control_sensor_->publish_state("Overload");
        break;
      case 2:
        this->alarm_control_sensor_->publish_state("Boost");
        break;
      case 4:
        this->alarm_control_sensor_->publish_state("Buck");
        break;
      default:
        this->alarm_control_sensor_->publish_state(std::to_string(this->parsed_data_[0]).c_str());
      }
    } else {
      ESP_LOGW(TAG, "No data received");
      this->status_set_warning();
    }
    delay(REQUEST_DELAY);
  }

  if (this->beeper_control_sensor_) {
    len = send_read_cmd(0x29, 0xec);
    if (len) {
      switch (this->parsed_data_[0])
      {
      case 1:
        this->beeper_control_sensor_->publish_state("Disabled");
        break;
      case 2:
        this->beeper_control_sensor_->publish_state("Enabled");
        break;
      case 3:
        this->beeper_control_sensor_->publish_state("Muted");
        break;
      }
    } else {
      ESP_LOGW(TAG, "No data received");
      this->status_set_warning();
    }
    delay(REQUEST_DELAY);
  }

  if (this->audible_alarm_control_sensor_) {
    len = send_read_cmd(0x80, 0x5b);
    if (len) {
      switch (this->parsed_data_[0])
      {
      case 1:
        this->audible_alarm_control_sensor_->publish_state("Disabled");
        break;
      case 2:
        this->audible_alarm_control_sensor_->publish_state("Enabled");
        break;
      case 3:
        this->audible_alarm_control_sensor_->publish_state("Muted");
        break;
      }
    } else {
      ESP_LOGW(TAG, "No data received");
      this->status_set_warning();
    }
    delay(REQUEST_DELAY);
  }

  if (this->remaining_time_limit_sensor_) {
    len = send_read_cmd(0x82, 0x24);
    if (len > 1) {
      uint16_t value = (this->parsed_data_[1] << 8) | this->parsed_data_[0];
      this->remaining_time_limit_sensor_->publish_state((float)value);
    } else {
      ESP_LOGW(TAG, "No data received");
      this->status_set_warning();
    }
    delay(REQUEST_DELAY);
  }

  if (this->low_voltage_sensor_) {
    len = send_read_cmd(0x83, 0xf1);
    if (len > 1) {
      uint16_t value = (this->parsed_data_[1] << 8) | this->parsed_data_[0];
      this->low_voltage_sensor_->publish_state((float)value);
    } else {
      ESP_LOGW(TAG, "No data received");
      this->status_set_warning();
    }
    delay(REQUEST_DELAY);
  }

  if (this->high_voltage_sensor_) {
    len = send_read_cmd(0x84, 0xa5);
    if (len > 1) {
      uint16_t value = (this->parsed_data_[1] << 8) | this->parsed_data_[0];
      this->high_voltage_sensor_->publish_state((float)value);
    } else {
      ESP_LOGW(TAG, "No data received");
      this->status_set_warning();
    }
    delay(REQUEST_DELAY);
  }

  if (this->test_status_sensor_) {
    len = send_read_cmd(0x85, 0x70);
    if (len) {
      switch (this->parsed_data_[0])
      {
      case 6:
        this->test_status_sensor_->publish_state("No test");
        break;
      default:
        this->test_status_sensor_->publish_state(std::to_string(this->parsed_data_[0]).c_str());
      }
    } else {
      ESP_LOGW(TAG, "No data received");
      this->status_set_warning();
    }
    delay(REQUEST_DELAY);
  }

  if (this->delay_shutdown_sensor_) {
    len = send_read_cmd(0x86, 0xda);
    if (len > 1) {
      uint16_t value = (this->parsed_data_[1] << 8) | this->parsed_data_[0];
      this->delay_shutdown_sensor_->publish_state((float)value);
    } else {
      ESP_LOGW(TAG, "No data received");
      this->status_set_warning();
    }
    delay(REQUEST_DELAY);
  }

  if (this->delay_startup_sensor_) {
    len = send_read_cmd(0x87, 0x0f);
    if (len > 1) {
      uint16_t value = (this->parsed_data_[1] << 8) | this->parsed_data_[0];
      this->delay_startup_sensor_->publish_state((float)value);
    } else {
      ESP_LOGW(TAG, "No data received");
      this->status_set_warning();
    }
    delay(REQUEST_DELAY);
  }
}

void Cyberpower::dump_config(){
  ESP_LOGCONFIG(TAG, "Cyberpower sensor count = %d", this->count_);
  // uint8_t len = send_read_cmd(0x1e, 0x4e);
  // if (len) {
  //   std::string string((char*)this->parsed_data_, len);
  //   this->firmware_sensor_->publish_state(string.c_str());
  // }
  // uint8_t len = send_read_cmd(0x1d, 0xe4);
  // if (len) {
  //   std::string model((char*)this->parsed_data_, len);
  //   // this->model_name_sensor_->publish_state(model.c_str());
  //   ESP_LOGW(TAG, "len = %d, %s", len, model.c_str());
  // }
}

uint8_t Cyberpower::parse_data(uint8_t skip) {
  uint8_t bytes = 0;
  if (this->buffer_data_[1-skip] & 0x40) {
    for (int i = 0; i < (this->buffer_data_[1-skip]&0xf); i++) {
      this->parsed_data_[i] = this->buffer_data_[i+3-skip];
      bytes++;
    }
  } else {
    for (int i = 0; i < (this->buffer_data_[1-skip]&0xf); i++) {
      this->parsed_data_[i] = this->buffer_data_[i+2-skip];
      bytes++;
    }
    for (int i = 0; i < (this->buffer_data_[11-skip]&0xf); i++) {
      this->parsed_data_[bytes] = this->buffer_data_[i+12-skip];
      bytes++;
    }
  }
  return bytes;
}

void Cyberpower::send_switch_cmd(uint8_t cmd_id, uint8_t state) {
  if (cmd_id == 0x03 && state) {
    this->write_array(switch_beep_on_cmd, sizeof(switch_beep_on_cmd));
    this->flush();
    if (this->beeper_switch_) this->beeper_switch_->publish_state(true);
  } else if (cmd_id == 0x03 && !state) {
    this->write_array(switch_beep_off_cmd, sizeof(switch_beep_off_cmd));
    this->flush();
    if (this->beeper_switch_) this->beeper_switch_->publish_state(false);
  } else if (cmd_id == 0x86 && state) {
    this->write_array(switch_power_on_cmd, sizeof(switch_power_on_cmd));
    this->flush();
    this->state_ = true;
    if (this->power_switch_) this->power_switch_->publish_state(true);
  } else if (cmd_id == 0x86 && !state) {
    this->write_array(switch_power_off_cmd, sizeof(switch_power_off_cmd));
    this->flush();
    this->state_ = false;
    if (this->power_switch_) this->power_switch_->publish_state(false);
  }
}

void CyberpowerSwitch::write_state(bool state) {
  if (this->parent_) this->parent_->send_switch_cmd(this->cmd_, state);
  
  this->publish_state(state);
}

}  // namespace empty_UART_sensor
}  // namespace esphome
