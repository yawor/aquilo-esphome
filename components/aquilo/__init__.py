import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.components import sx127x

DEPENDENCIES = ["sx127x"]

MULTI_CONF = True

aquilo_ns = cg.esphome_ns.namespace("aquilo")
Aquilo = aquilo_ns.class_("Aquilo", cg.Component)

CONF_SX127x_ID = "sx127x_id"

CONF_AQUILO_ID = "aquilo_id"
CONF_TRANSMITTER_ID = "transmitter_id"

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Aquilo),
            cv.GenerateID(CONF_SX127x_ID): cv.use_id(sx127x.SX127x),

        },
    )
    .extend(cv.COMPONENT_SCHEMA),
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    sx127x_ = await cg.get_variable(config[CONF_SX127x_ID])
    cg.add(sx127x_.register_listener(var))
