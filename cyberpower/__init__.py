import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

CODEOWNERS = ["@monte-monte"]
DEPENDENCIES = ["uart"]
CONF_CYBERPOWER_ID = "cyberpower_id"

cyberpower_ns = cg.esphome_ns.namespace("cyberpower")
Cyberpower = cyberpower_ns.class_("Cyberpower", cg.PollingComponent, uart.UARTDevice)

CONFIG_SCHEMA = (
  cv.Schema(
    {
      cv.GenerateID(): cv.declare_id(Cyberpower),
    }
  )
  .extend(cv.polling_component_schema("5s"))
  .extend(uart.UART_DEVICE_SCHEMA)
)

async def to_code(config):
  var = cg.new_Pvariable(config[CONF_ID])
  await cg.register_component(var, config)
  await uart.register_uart_device(var, config)