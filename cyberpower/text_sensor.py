import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import (
  CONF_STATUS,
)
from . import Cyberpower, cyberpower_ns, CONF_CYBERPOWER_ID

DEPENDENCIES = ["cyberpower"]

CONF_MODEL = "model"
CONF_FIRMWARE = "firmware_version"
CONF_ALARM_CONTROL = "alarm_control"
CONF_AUDIBLE_ALARM = "audible_alarm"
CONF_BEEPER_CONTROL = "beeper_control"
CONF_TEST_STATUS = "test_status"

TYPES = {
  CONF_MODEL: ("set_model_name_sensor", text_sensor.text_sensor_schema(
  )),
  CONF_FIRMWARE: ("set_firmware_sensor", text_sensor.text_sensor_schema(
  )),
  CONF_STATUS: ("set_status_sensor", text_sensor.text_sensor_schema(
  )),
  CONF_ALARM_CONTROL: ("set_alarm_control_sensor", text_sensor.text_sensor_schema(
  )),
  CONF_BEEPER_CONTROL: ("set_beeper_control_sensor", text_sensor.text_sensor_schema(
  )),
  CONF_AUDIBLE_ALARM: ("set_audible_alarm_control_sensor", text_sensor.text_sensor_schema(
  )),
  CONF_TEST_STATUS: ("set_test_status_sensor", text_sensor.text_sensor_schema(
  )),
}

CONFIG_SCHEMA = cv.Schema({
  cv.GenerateID(CONF_CYBERPOWER_ID): cv.use_id(Cyberpower),
}).extend({cv.Optional(key): schema for key, (_, schema) in TYPES.items()})

async def to_code(config):
  hub = await cg.get_variable(config[CONF_CYBERPOWER_ID])
  for key, (func_name, _) in TYPES.items():
    if key in config:
      sens = await text_sensor.new_text_sensor(config[key])
      cg.add(getattr(hub, func_name)(sens))