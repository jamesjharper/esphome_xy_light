import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light, output
from esphome.const import CONF_ID

from . import validation as xy_cv

from .xy_output import (xy_light_ns, XyOutput, CONF_XY_OUTPUT_CALIBRATION_LOGGING)

from .rgb_profile import (RGB_PROFILE_CONFIG_SCHEMA, RgbProfile, to_rgb_profile_code)
from .cwww_profile import (CWWW_PROFILE_CONFIG_SCHEMA, CwWwProfile, to_cwww_profile_code)
from .white_profile import (WHITE_PROFILE_CONFIG_SCHEMA, WhiteProfile, to_white_profile_code)

from .rgb_xy_output import (RGB_XY_OUTPUT_CONFIG_SCHEMA, to_rgb_xy_output_code)
from .rgbw_xy_output import (RGBW_XY_OUTPUT_CONFIG_SCHEMA, to_rgbw_xy_output_code)
from .rgb_cwww_xy_output import (RGB_CWWW_XY_OUTPUT_CONFIG_SCHEMA, to_rgb_cwww_xy_output_code)

from .cwww_xy_output import (CWWW_XY_OUTPUT_CONFIG_SCHEMA, to_cwww_xy_output_code)
from .white_xy_output import (WHITE_XY_OUTPUT_CONFIG_SCHEMA, to_white_xy_output_code)

CODEOWNERS = ["@jamesjharper"]

XyLightControl = xy_light_ns.class_("XyLightControl", light.LightOutput, cg.Component)
XyLightOutput = xy_light_ns.class_("XyLightOutput")
ControlType = xy_light_ns.enum("ControlType", is_class=True)

CONF_XY_LIGHT_CONTROL_ID = "control_id"

CONF_SOURCE_COLOR_PROFILE = "source_color_profile"
CONF_SOURCE_COLOR_PROFILE_ID = "source_color_profile_id"

CONF_XY_OUTPUTS = "xy_outputs"

CONF_CONTROLS = "controls"
CONF_CONTROL_TYPE = "control_type"

CONF_CONTROL_TEMPERATURE_RANGE = "color_temperature_range"

CONF_XY_OUTPUT_TYPE__RGB = "rgb"
CONF_XY_OUTPUT_TYPE__RGB_CWWW = "rgb_cwww"
CONF_XY_OUTPUT_TYPE__RGBW = "rgbw"
CONF_XY_OUTPUT_TYPE__CWWW = "cwww"
CONF_XY_OUTPUT_TYPE__W = "white"
CONF_XY_OUTPUT_TYPE__ID = "id"

XY_OUTPUT_TYPE_VARIANT_SCHEMA = cv.All(
    cv.Schema({
        cv.Optional(CONF_XY_OUTPUT_TYPE__RGB): RGB_XY_OUTPUT_CONFIG_SCHEMA,
        cv.Optional(CONF_XY_OUTPUT_TYPE__RGB_CWWW): RGB_CWWW_XY_OUTPUT_CONFIG_SCHEMA, 
        cv.Optional(CONF_XY_OUTPUT_TYPE__RGBW): RGBW_XY_OUTPUT_CONFIG_SCHEMA,
        cv.Optional(CONF_XY_OUTPUT_TYPE__CWWW): CWWW_XY_OUTPUT_CONFIG_SCHEMA, 
        cv.Optional(CONF_XY_OUTPUT_TYPE__W): WHITE_XY_OUTPUT_CONFIG_SCHEMA, 
        cv.Optional(CONF_XY_OUTPUT_TYPE__ID): cv.use_id(XyLightOutput)
    }),
    cv.has_exactly_one_key(
        CONF_XY_OUTPUT_TYPE__RGB, 
        CONF_XY_OUTPUT_TYPE__RGB_CWWW,
        CONF_XY_OUTPUT_TYPE__RGBW,
        CONF_XY_OUTPUT_TYPE__CWWW,
        CONF_XY_OUTPUT_TYPE__W,
        CONF_XY_OUTPUT_TYPE__ID
    )
)

CONTROL_TYPES = {
    "RGB": ControlType.RGB,
    "RGB_SATURATION": ControlType.RGB_SATURATION,
    "RGB_CT": ControlType.RGB_CT,
    "RGB_CWWW": ControlType.RGB_CWWW,
    "CT": ControlType.CT,
    "CWWW": ControlType.CWWW,
    "W": ControlType.BRIGHTNESS,
}

CONTROL_CONFIG_SCHEMA = cv.All(
    light.RGB_LIGHT_SCHEMA.extend({
        cv.GenerateID(CONF_XY_LIGHT_CONTROL_ID): cv.declare_id(XyLightControl),
        cv.Required(CONF_CONTROL_TYPE): cv.enum(CONTROL_TYPES, upper=True, space="_"),
        cv.Optional(CONF_CONTROL_TEMPERATURE_RANGE): xy_cv.ct_range,
    })
)

CONFIG_SCHEMA = cv.All(
    cv.COMPONENT_SCHEMA.extend({
        cv.GenerateID(CONF_ID): cv.declare_id(XyLightOutput),
        cv.Optional(CONF_SOURCE_COLOR_PROFILE_ID): cv.use_id(RgbProfile),
        cv.Optional(CONF_SOURCE_COLOR_PROFILE): RGB_PROFILE_CONFIG_SCHEMA,
        cv.Required(CONF_CONTROLS): cv.ensure_list(CONTROL_CONFIG_SCHEMA),
        cv.Optional(CONF_XY_OUTPUTS): cv.ensure_list(XY_OUTPUT_TYPE_VARIANT_SCHEMA),
        cv.Optional(CONF_XY_OUTPUT_CALIBRATION_LOGGING): cv.boolean
    }),
    cv.has_at_most_one_key(CONF_SOURCE_COLOR_PROFILE_ID, CONF_SOURCE_COLOR_PROFILE)
)

async def to_control_code(config, var_light_output):
    var_light_control = cg.new_Pvariable(config[CONF_XY_LIGHT_CONTROL_ID])
    cg.add(var_light_control.set_xy_light_output(var_light_output))

    if CONF_CONTROL_TEMPERATURE_RANGE in config:
        ct_range = config[CONF_CONTROL_TEMPERATURE_RANGE]
        cg.add(var_light_control.set_color_temperature_range(ct_range[0], ct_range[1]))

    if CONF_CONTROL_TYPE in config:
        control_type = await cg.templatable(config[CONF_CONTROL_TYPE], [], ControlType)
        cg.add(var_light_control.set_control_type(control_type))

    await register_xy_light_(var_light_control, config)


async def to_code(config):
    var_light_output = cg.new_Pvariable(config[CONF_ID])

    if CONF_SOURCE_COLOR_PROFILE_ID in config:
        profile = await cg.get_variable(config[CONF_SOURCE_COLOR_PROFILE_ID])
        cg.add(var_light_output.set_source_color_profile(profile))

    if CONF_SOURCE_COLOR_PROFILE in config:
        await to_rgb_profile_code(config[CONF_SOURCE_COLOR_PROFILE])
        inline_profile = await cg.get_variable(config[CONF_SOURCE_COLOR_PROFILE][CONF_ID])
        cg.add(var_light_output.set_source_color_profile(inline_profile))

    if CONF_XY_OUTPUT_CALIBRATION_LOGGING in config:     
        enable_cal_log = config[CONF_XY_OUTPUT_CALIBRATION_LOGGING]
        if enable_cal_log:
            cg.add(var_light_output.enable_calibration_logging(True))    

    if CONF_XY_OUTPUTS in config:
        for output in config[CONF_XY_OUTPUTS]:
            await to_xy_output_code(var_light_output, output)

    if CONF_CONTROLS in config:
        for control_config in config[CONF_CONTROLS]:
            await to_control_code(control_config, var_light_output)

async def register_xy_light_(var_light_control, config):
    light_var = cg.new_Pvariable(config[CONF_ID], var_light_control)
    await cg.register_component(light_var, config)
    await light.setup_light_core_(light_var, var_light_control, config)

    cg.add(cg.App.register_light(light_var))


async def to_xy_output_code(var, config):
    if CONF_XY_OUTPUT_TYPE__RGB in config: 
        await unpack_variant_to_code(var, config[CONF_XY_OUTPUT_TYPE__RGB], to_rgb_xy_output_code)

    elif CONF_XY_OUTPUT_TYPE__CWWW in config: 
        await unpack_variant_to_code(var, config[CONF_XY_OUTPUT_TYPE__CWWW], to_cwww_xy_output_code)

    elif CONF_XY_OUTPUT_TYPE__RGBW in config: 
        await unpack_variant_to_code(var, config[CONF_XY_OUTPUT_TYPE__RGBW], to_rgbw_xy_output_code)

    elif CONF_XY_OUTPUT_TYPE__RGB_CWWW in config: 
        await unpack_variant_to_code(var, config[CONF_XY_OUTPUT_TYPE__RGB_CWWW], to_rgb_cwww_xy_output_code)

    elif CONF_XY_OUTPUT_TYPE__W in config: 
        await unpack_variant_to_code(var, config[CONF_XY_OUTPUT_TYPE__W], to_white_xy_output_code) 

    elif CONF_XY_OUTPUT_TYPE__ID in config: 
        var_output = await cg.get_variable(config[CONF_XY_OUTPUT_TYPE__ID])
        cg.add(var.add_output(var_output)) 
 
async def unpack_variant_to_code(var, config, to_code_method):
    await to_code_method(config)
    var_output = await cg.get_variable(config[CONF_ID])
    cg.add(var.add_output(var_output))
