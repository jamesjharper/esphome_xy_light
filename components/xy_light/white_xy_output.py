import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import output
from esphome.const import CONF_ID

from .xy_output import (xy_light_ns, XyOutput)
from .white_profile import (WHITE_PROFILE_CONFIG_SCHEMA, WhiteProfile, to_white_profile_code)

from .xy_output import CONF_XY_OUTPUT_CALIBRATION_LOGGING
from .xy_output import (CONF_XY_OUTPUT_WHITE_COLOR_PROFILE_ID, CONF_XY_OUTPUT_WHITE_COLOR_PROFILE)
from .xy_output import CONF_XY_OUTPUT_WHITE_OUTPUT_ID
from .xy_output import CONF_XY_OUTPUT_CALIBRATION_LOGGING

WhiteXyOutput = xy_light_ns.class_("WhiteXyOutput", cg.Component, XyOutput)

WHITE_XY_OUTPUT_CONFIG_SCHEMA = cv.Schema({ 
        cv.GenerateID(CONF_ID): cv.declare_id(WhiteXyOutput),
        cv.Optional(CONF_XY_OUTPUT_CALIBRATION_LOGGING): cv.boolean,
        cv.Optional(CONF_XY_OUTPUT_WHITE_OUTPUT_ID): cv.use_id(output.FloatOutput),
        cv.Optional(CONF_XY_OUTPUT_WHITE_COLOR_PROFILE_ID): cv.use_id(WhiteProfile),
        cv.Optional(CONF_XY_OUTPUT_WHITE_COLOR_PROFILE): WHITE_PROFILE_CONFIG_SCHEMA,
    },
    cv.has_exactly_one_key(CONF_XY_OUTPUT_WHITE_COLOR_PROFILE, CONF_XY_OUTPUT_WHITE_COLOR_PROFILE_ID)
).extend(cv.COMPONENT_SCHEMA)

async def to_white_xy_output_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
   
    # Color Profile
    if CONF_XY_OUTPUT_WHITE_COLOR_PROFILE_ID in config:     
        profile = await cg.get_variable(config[CONF_XY_OUTPUT_WHITE_COLOR_PROFILE_ID])
        cg.add(var.set_profile(profile))

    if CONF_XY_OUTPUT_WHITE_COLOR_PROFILE in config:
        await to_cwww_profile_code(config[CONF_XY_OUTPUT_WHITE_COLOR_PROFILE])
        inline_profile = await cg.get_variable(config[CONF_XY_OUTPUT_WHITE_COLOR_PROFILE][CONF_ID])
        cg.add(var.set_profile(inline_profile))

    if CONF_XY_OUTPUT_WHITE_OUTPUT_ID in config:     
        white_output = await cg.get_variable(config[CONF_XY_OUTPUT_WHITE_OUTPUT_ID])
        cg.add(var.set_white_output(white_output))

    if CONF_XY_OUTPUT_CALIBRATION_LOGGING in config:     
        enable_cal_log = config[CONF_XY_OUTPUT_CALIBRATION_LOGGING]
        if enable_cal_log:
            cg.add(var.enable_calibration_logging(True))   

    await cg.register_component(var, config)
