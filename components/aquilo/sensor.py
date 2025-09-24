import esphome.codegen as cg
from esphome.components import sensor, globals
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_DISTANCE,
    DEVICE_CLASS_SIGNAL_STRENGTH,
    DEVICE_CLASS_VOLTAGE,
    ENTITY_CATEGORY_DIAGNOSTIC,
    ICON_BATTERY,
    ICON_COUNTER,
    ICON_GAUGE,
    ICON_SIGNAL,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_NONE,
    UNIT_DECIBEL,
    UNIT_DECIBEL_MILLIWATT,
    UNIT_MILLIMETER,
    UNIT_MILLIVOLT,
)

from . import (
    CONF_AQUILO_ID,
    CONF_TRANSMITTER_ID,
    Aquilo,
    aquilo_ns,
)

AquiloSensor = aquilo_ns.class_("AquiloSensor", cg.Component)

CONF_DISTANCE = "distance"
CONF_VOLTAGE = "voltage"
CONF_MEASUREMENT_COUNT = "measurement_count"
CONF_PREV_MEASUREMENT_COUNT = "prev_measurement_count"
CONF_RSSI = "rssi"
CONF_SNR = "snr"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(AquiloSensor),
    cv.GenerateID(CONF_AQUILO_ID): cv.use_id(Aquilo),
    cv.Required(CONF_TRANSMITTER_ID): cv.hex_uint32_t,
    cv.Optional(CONF_DISTANCE): sensor.sensor_schema(
        unit_of_measurement=UNIT_MILLIMETER,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_DISTANCE,
        state_class=STATE_CLASS_MEASUREMENT,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    ),
    cv.Optional(CONF_VOLTAGE): sensor.sensor_schema(
        unit_of_measurement=UNIT_MILLIVOLT,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_VOLTAGE,
        state_class=STATE_CLASS_MEASUREMENT,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    ),
    cv.Optional(CONF_MEASUREMENT_COUNT): sensor.sensor_schema(
        icon=ICON_COUNTER,
        accuracy_decimals=0,
        state_class=STATE_CLASS_NONE,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    ),
    cv.Optional(CONF_PREV_MEASUREMENT_COUNT): sensor.sensor_schema(
        icon=ICON_COUNTER,
        accuracy_decimals=0,
        state_class=STATE_CLASS_NONE,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    ),
    cv.Optional(CONF_RSSI): sensor.sensor_schema(
        unit_of_measurement=UNIT_DECIBEL_MILLIWATT,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_SIGNAL_STRENGTH,
        state_class=STATE_CLASS_MEASUREMENT,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    ),
    cv.Optional(CONF_SNR): sensor.sensor_schema(
        unit_of_measurement=UNIT_DECIBEL,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_SIGNAL_STRENGTH,
        state_class=STATE_CLASS_MEASUREMENT,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    ),
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.set_transmitter_id(config[CONF_TRANSMITTER_ID]))

    if CONF_DISTANCE in config:
        sens = await sensor.new_sensor(config[CONF_DISTANCE])
        cg.add(var.set_distance_sensor(sens))
    if CONF_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_VOLTAGE])
        cg.add(var.set_voltage_sensor(sens))
    if CONF_MEASUREMENT_COUNT in config:
        sens = await sensor.new_sensor(config[CONF_MEASUREMENT_COUNT])
        cg.add(var.set_measurement_count_sensor(sens))
    if CONF_PREV_MEASUREMENT_COUNT in config:
        sens = await sensor.new_sensor(config[CONF_PREV_MEASUREMENT_COUNT])
        cg.add(var.set_prev_measurement_count_sensor(sens))
    if CONF_RSSI in config:
        sens = await sensor.new_sensor(config[CONF_RSSI])
        cg.add(var.set_rssi_sensor(sens))
    if CONF_SNR in config:
        sens = await sensor.new_sensor(config[CONF_SNR])
        cg.add(var.set_snr_sensor(sens))

    aquilo = await cg.get_variable(config[CONF_AQUILO_ID])
    cg.add(aquilo.register_listener(var))
