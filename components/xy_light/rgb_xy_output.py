import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import output
from esphome.const import CONF_ID


from .xy_output import (xy_light_ns, XyOutput)

from .rgb_profile import (RGB_PROFILE_CONFIG_SCHEMA, RgbProfile, to_rgb_profile_code)

from .xy_output import CONF_XY_OUTPUT_CALIBRATION_LOGGING
from .xy_output import (CONF_XY_OUTPUT_RGB_COLOR_PROFILE_ID, CONF_XY_OUTPUT_RGB_COLOR_PROFILE)
from .xy_output import (CONF_XY_OUTPUT_RED_OUTPUT_ID, CONF_XY_OUTPUT_GREEN_OUTPUT_ID, CONF_XY_OUTPUT_BLUE_OUTPUT_ID)

RgbXyOutput = xy_light_ns.class_("RgbXyOutput", cg.Component, XyOutput)

RGB_XY_OUTPUT_CONFIG_SCHEMA = cv.Schema({ 
        cv.GenerateID(CONF_ID): cv.declare_id(RgbXyOutput),

        # Calibration Logging 
        cv.Optional(CONF_XY_OUTPUT_CALIBRATION_LOGGING): cv.boolean,
         
        cv.Optional(CONF_XY_OUTPUT_RED_OUTPUT_ID): cv.use_id(output.FloatOutput),
        cv.Optional(CONF_XY_OUTPUT_GREEN_OUTPUT_ID): cv.use_id(output.FloatOutput),
        cv.Optional(CONF_XY_OUTPUT_BLUE_OUTPUT_ID): cv.use_id(output.FloatOutput),
        cv.Optional(CONF_XY_OUTPUT_RGB_COLOR_PROFILE_ID): cv.use_id(RgbProfile),
        cv.Optional(CONF_XY_OUTPUT_RGB_COLOR_PROFILE): RGB_PROFILE_CONFIG_SCHEMA,
    },
    cv.has_exactly_one_key(CONF_XY_OUTPUT_RGB_COLOR_PROFILE, CONF_XY_OUTPUT_RGB_COLOR_PROFILE_ID)
).extend(cv.COMPONENT_SCHEMA)

async def to_rgb_xy_output_code(config):
    var = cg.new_Pvariable(config[CONF_ID]) 
   
    # Color Profile
    if CONF_XY_OUTPUT_RGB_COLOR_PROFILE_ID in config:     
        color_profile = await cg.get_variable(config[CONF_XY_OUTPUT_RGB_COLOR_PROFILE_ID])
        cg.add(var.set_color_profile(color_profile))

    if CONF_XY_OUTPUT_RGB_COLOR_PROFILE in config:
        await to_rgb_profile_code(config[CONF_XY_OUTPUT_RGB_COLOR_PROFILE])
        inline_profile = await cg.get_variable(config[CONF_XY_OUTPUT_RGB_COLOR_PROFILE][CONF_ID])
        cg.add(var.set_color_profile(inline_profile))

    # RGB
    if CONF_XY_OUTPUT_RED_OUTPUT_ID in config:     
        red_output = await cg.get_variable(config[CONF_XY_OUTPUT_RED_OUTPUT_ID])
        cg.add(var.set_red_output(red_output))

    if CONF_XY_OUTPUT_GREEN_OUTPUT_ID in config:     
        green_output = await cg.get_variable(config[CONF_XY_OUTPUT_GREEN_OUTPUT_ID])
        cg.add(var.set_green_output(green_output))

    if CONF_XY_OUTPUT_BLUE_OUTPUT_ID in config:     
        blue_output = await cg.get_variable(config[CONF_XY_OUTPUT_BLUE_OUTPUT_ID])
        cg.add(var.set_blue_output(blue_output))

    if CONF_XY_OUTPUT_CALIBRATION_LOGGING in config:     
        enable_cal_log = config[CONF_XY_OUTPUT_CALIBRATION_LOGGING]
        if enable_cal_log:
            cg.add(var.enable_calibration_logging(True))    

    await cg.register_component(var, config)
