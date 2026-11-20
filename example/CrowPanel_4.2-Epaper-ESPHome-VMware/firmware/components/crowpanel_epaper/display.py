import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import display, spi
from esphome.const import CONF_ID, CONF_LAMBDA, CONF_DC_PIN, CONF_RESET_PIN, CONF_BUSY_PIN

DEPENDENCIES = ['spi']
ns = cg.esphome_ns.namespace('crowpanel_epaper')
CrowPanelEPaper = ns.class_('CrowPanelEPaper', display.DisplayBuffer, spi.SPIDevice)
CONFIG_SCHEMA = display.FULL_DISPLAY_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(CrowPanelEPaper),
    cv.Required(CONF_DC_PIN): pins.gpio_output_pin_schema,
    cv.Required(CONF_RESET_PIN): pins.gpio_output_pin_schema,
    cv.Required(CONF_BUSY_PIN): pins.gpio_input_pin_schema,
    cv.Required('power_pin'): pins.gpio_output_pin_schema,
}).extend(cv.polling_component_schema('never')).extend(spi.spi_device_schema())
FINAL_VALIDATE_SCHEMA = spi.final_validate_device_schema('crowpanel_epaper', require_mosi=True, require_miso=False)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await display.register_display(var, config)
    await spi.register_spi_device(var, config, write_only=True)
    for name in [CONF_DC_PIN, CONF_RESET_PIN, CONF_BUSY_PIN, 'power_pin']:
        pin = await cg.gpio_pin_expression(config[name])
        cg.add(getattr(var, 'set_'+name)(pin))
    if CONF_LAMBDA in config:
        writer = await cg.process_lambda(config[CONF_LAMBDA], [(display.DisplayRef, 'it')], return_type=cg.void)
        cg.add(var.set_writer(writer))
