import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
  CONF_BATTERY_LEVEL,
  DEVICE_CLASS_BATTERY,
  DEVICE_CLASS_VOLTAGE,
  DEVICE_CLASS_POWER,
  STATE_CLASS_MEASUREMENT,
  UNIT_PERCENT,
  UNIT_VOLT,
  UNIT_SECOND,
  UNIT_MINUTE,
  UNIT_WATT,
)
from . import Cyberpower, cyberpower_ns, CONF_CYBERPOWER_ID

DEPENDENCIES = ["cyberpower"]

CONF_BATTERY_VOLTAGE = "battery_voltage"
CONF_RUNTIME = "runtime"

CONF_DESIGNED_POWER = "designed_power"
CONF_DESIGNED_BATTERY_VOLTAGE = "designed_battery_voltage"
CONF_REMAINING_TIME_LIMIT = "remaining_time_limit"
CONF_OUTPUT_VOLTAGE = "output_voltage"
CONF_INPUT_VOLTAGE = "input_voltage"
CONF_LOAD_PERCENTAGE = "load_percentage"
CONF_ACTIVE_POWER = "active_power"
CONF_UNKNOWN_FLAG = "unknown_flag"
CONF_VOLTAGE_LOW = "low_voltage_transfer"
CONF_VOLTAGE_HIGH = "high_voltage_transfer"
CONF_DELAY_SHUTDOWN = "delay_shutdown"
CONF_DELAY_STARTUP = "delay_startup"
CONF_NOMINAL_VOLTAGE = "nominal_voltage"

TYPES = {
  CONF_BATTERY_LEVEL: ("set_battery_percentage_sensor", sensor.sensor_schema(
    unit_of_measurement=UNIT_PERCENT,
    accuracy_decimals=0,
    device_class=DEVICE_CLASS_BATTERY,
    state_class=STATE_CLASS_MEASUREMENT,
  )),
  CONF_BATTERY_VOLTAGE: ("set_battery_voltage_sensor", sensor.sensor_schema(
    unit_of_measurement=UNIT_VOLT,
    accuracy_decimals=1,
    device_class=DEVICE_CLASS_BATTERY,
    state_class=STATE_CLASS_MEASUREMENT,
  )),
  CONF_RUNTIME: ("set_runtime_sensor", sensor.sensor_schema(
    unit_of_measurement=UNIT_SECOND,
    accuracy_decimals=0,
    state_class=STATE_CLASS_MEASUREMENT,
  )),
  CONF_INPUT_VOLTAGE: ("set_input_voltage_sensor", sensor.sensor_schema(
    unit_of_measurement=UNIT_VOLT,
    accuracy_decimals=1,
    device_class=DEVICE_CLASS_VOLTAGE,
    state_class=STATE_CLASS_MEASUREMENT,
  )),
  CONF_OUTPUT_VOLTAGE: ("set_output_voltage_sensor", sensor.sensor_schema(
    unit_of_measurement=UNIT_VOLT,
    accuracy_decimals=1,
    device_class=DEVICE_CLASS_VOLTAGE,
    state_class=STATE_CLASS_MEASUREMENT,
  )),
  CONF_LOAD_PERCENTAGE: ("set_load_percentage_sensor", sensor.sensor_schema(
    unit_of_measurement=UNIT_PERCENT,
    accuracy_decimals=0,
    state_class=STATE_CLASS_MEASUREMENT,
  )),
  CONF_ACTIVE_POWER: ("set_active_power_sensor", sensor.sensor_schema(
    unit_of_measurement=UNIT_WATT,
    accuracy_decimals=0,
    device_class=DEVICE_CLASS_POWER,
    state_class=STATE_CLASS_MEASUREMENT,
  )),
  CONF_UNKNOWN_FLAG: ("set_unknown_flag_sensor", sensor.sensor_schema(
    accuracy_decimals=0,
  )),
  CONF_DESIGNED_POWER: ("set_designed_power_sensor", sensor.sensor_schema(
    unit_of_measurement=UNIT_WATT,
    accuracy_decimals=0,
    device_class=DEVICE_CLASS_POWER,
  )),
  CONF_DESIGNED_BATTERY_VOLTAGE: ("set_designed_battery_voltage_sensor", sensor.sensor_schema(
    unit_of_measurement=UNIT_VOLT,
    accuracy_decimals=1,
    device_class=DEVICE_CLASS_VOLTAGE,
  )),
  CONF_REMAINING_TIME_LIMIT: ("set_remaining_time_limit_sensor", sensor.sensor_schema(
    unit_of_measurement=UNIT_MINUTE,
    accuracy_decimals=0,
  )),
  CONF_VOLTAGE_LOW: ("set_low_voltage_sensor", sensor.sensor_schema(
    unit_of_measurement=UNIT_VOLT,
    accuracy_decimals=0,
    device_class=DEVICE_CLASS_VOLTAGE,
  )),
  CONF_VOLTAGE_HIGH: ("set_high_voltage_sensor", sensor.sensor_schema(
    unit_of_measurement=UNIT_VOLT,
    accuracy_decimals=0,
    device_class=DEVICE_CLASS_VOLTAGE,
  )),
  CONF_DELAY_SHUTDOWN: ("set_delay_shutdown_sensor", sensor.sensor_schema(
    unit_of_measurement=UNIT_MINUTE,
    accuracy_decimals=0,
    state_class=STATE_CLASS_MEASUREMENT,
  )),
  CONF_DELAY_STARTUP: ("set_delay_startup_sensor", sensor.sensor_schema(
    unit_of_measurement=UNIT_MINUTE,
    accuracy_decimals=0,
    state_class=STATE_CLASS_MEASUREMENT,
  )),
  CONF_NOMINAL_VOLTAGE: ("set_nominal_voltage_sensor", sensor.sensor_schema(
    unit_of_measurement=UNIT_VOLT,
    accuracy_decimals=0,
  )),
}

CONFIG_SCHEMA = cv.Schema({
  cv.GenerateID(CONF_CYBERPOWER_ID): cv.use_id(Cyberpower),
}).extend({cv.Optional(key): schema for key, (_, schema) in TYPES.items()})

async def to_code(config):
  hub = await cg.get_variable(config[CONF_CYBERPOWER_ID])
  for key, (func_name, _) in TYPES.items():
    if key in config:
      sens = await sensor.new_sensor(config[key])
      cg.add(getattr(hub, func_name)(sens))