import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import output
from esphome.const import CONF_ID

from .xy_output import (xy_light_ns, XyOutput)

from .cwww_profile import (CWWW_PROFILE_CONFIG_SCHEMA, CwWwProfile, to_cwww_profile_code)

from .xy_output import CONF_XY_OUTPUT_CALIBRATION_LOGGING
from .xy_output import (CONF_XY_OUTPUT_CWWW_COLOR_PROFILE_ID, CONF_XY_OUTPUT_CWWW_COLOR_PROFILE)
from .xy_output import (CONF_XY_OUTPUT_WARM_WHITE_OUTPUT_ID, CONF_XY_OUTPUT_COLD_WHITE_OUTPUT_ID)

CwWwXyOutput = xy_light_ns.class_("CwWwXyOutput", cg.Component, XyOutput)

CWWW_XY_OUTPUT_CONFIG_SCHEMA = cv.Schema({ 
        cv.GenerateID(CONF_ID): cv.declare_id(CwWwXyOutput),
        cv.Optional(CONF_XY_OUTPUT_CALIBRATION_LOGGING): cv.boolean,
        cv.Optional(CONF_XY_OUTPUT_COLD_WHITE_OUTPUT_ID): cv.use_id(output.FloatOutput),
        cv.Optional(CONF_XY_OUTPUT_WARM_WHITE_OUTPUT_ID): cv.use_id(output.FloatOutput),
        cv.Optional(CONF_XY_OUTPUT_CWWW_COLOR_PROFILE_ID): cv.use_id(CwWwProfile),
        cv.Optional(CONF_XY_OUTPUT_CWWW_COLOR_PROFILE): CWWW_PROFILE_CONFIG_SCHEMA,
    },
    cv.has_exactly_one_key(CONF_XY_OUTPUT_CWWW_COLOR_PROFILE, CONF_XY_OUTPUT_CWWW_COLOR_PROFILE_ID)
).extend(cv.COMPONENT_SCHEMA)

async def to_cwww_xy_output_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
   
    # Color Profile
    if CONF_XY_OUTPUT_CWWW_COLOR_PROFILE_ID in config:     
        profile = await cg.get_variable(config[CONF_XY_OUTPUT_CWWW_COLOR_PROFILE_ID])
        cg.add(var.set_profile(profile))

    if CONF_XY_OUTPUT_CWWW_COLOR_PROFILE in config:
        await to_cwww_profile_code(config[CONF_XY_OUTPUT_CWWW_COLOR_PROFILE])
        inline_profile = await cg.get_variable(config[CONF_XY_OUTPUT_CWWW_COLOR_PROFILE][CONF_ID])
        cg.add(var.set_profile(inline_profile))

    if CONF_XY_OUTPUT_COLD_WHITE_OUTPUT_ID in config:     
        cold_white_output = await cg.get_variable(config[CONF_XY_OUTPUT_COLD_WHITE_OUTPUT_ID])
        cg.add(var.set_cold_white_output(cold_white_output))

    if CONF_XY_OUTPUT_WARM_WHITE_OUTPUT_ID in config:     
        warm_white_output = await cg.get_variable(config[CONF_XY_OUTPUT_WARM_WHITE_OUTPUT_ID])
        cg.add(var.set_warm_white_output(warm_white_output))

    if CONF_XY_OUTPUT_CALIBRATION_LOGGING in config:     
        enable_cal_log = config[CONF_XY_OUTPUT_CALIBRATION_LOGGING]
        if enable_cal_log:
            cg.add(var.enable_calibration_logging(True))   

    await cg.register_component(var, config)
