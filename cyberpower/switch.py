import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import (
  ICON_POWER,
)
from . import Cyberpower, cyberpower_ns, CONF_CYBERPOWER_ID

DEPENDENCIES = ["cyberpower"]

CyberpowerSwitch = cyberpower_ns.class_("CyberpowerSwitch", switch.Switch)

CONF_BEEPER = "beeper"
CONF_POWER = "power"

# Tuple mapping: (Command ID, default icon, switch configuration schema)
TYPES = {
  CONF_BEEPER: (0x03, "set_beeper", switch.switch_schema(CyberpowerSwitch,
    icon="mdi:volume-high",
  )),
  CONF_POWER: (0x86, "set_power", switch.switch_schema(CyberpowerSwitch,
    icon=ICON_POWER,
  )),
}

CONFIG_SCHEMA = cv.Schema({
  cv.GenerateID(CONF_CYBERPOWER_ID): cv.use_id(Cyberpower),
}).extend({cv.Optional(key): schema for key, (_, _, schema) in TYPES.items()})

async def to_code(config):
  hub = await cg.get_variable(config[CONF_CYBERPOWER_ID])
  for key, (command_id, func_name, _) in TYPES.items():
    if key in config:
      sw = await switch.new_switch(config[key])
      cg.add(sw.set_parent(hub))
      cg.add(sw.set_command_id(command_id))
      cg.add(getattr(hub, func_name)(sw))