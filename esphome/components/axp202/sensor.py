import esphome.codegen as cg
import esphome.config_validation as cv

from esphome import automation, pins
from esphome.components import i2c, sensor
from esphome.const import (
    CONF_ID,
    CONF_BATTERY_LEVEL,
    UNIT_PERCENT,
    ICON_BATTERY,
    CONF_INTERRUPT_PIN,
    CONF_ON_CLICK,
    CONF_TRIGGER_ID,
)

DEPENDENCIES = ["i2c"]

axp202_ns = cg.esphome_ns.namespace("axp202")
AXP202Component = axp202_ns.class_(
    "AXP202Component", cg.PollingComponent, i2c.I2CDevice
)

ClickTrigger = axp202_ns.class_("ClickTrigger", automation.Trigger.template())

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AXP202Component),
            cv.Required(CONF_INTERRUPT_PIN): cv.All(
                pins.internal_gpio_input_pin_schema
            ),
            cv.Optional(CONF_ON_CLICK): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(ClickTrigger),
                }
            ),
            cv.Optional(CONF_BATTERY_LEVEL): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=1,
                icon=ICON_BATTERY,
            ),
        }
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(i2c.i2c_device_schema(0x35))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    interrupt_pin = await cg.gpio_pin_expression(config[CONF_INTERRUPT_PIN])
    cg.add(var.set_interrupt_pin(interrupt_pin))

    if CONF_BATTERY_LEVEL in config:
        conf = config[CONF_BATTERY_LEVEL]
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_batterylevel_sensor(sens))

    for conf in config.get(CONF_ON_CLICK, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [], conf)
