import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

from . import validation as xy_cv

from .xy_output import xy_light_ns
from .profile import CONF_PROFILE_GAMMA
from .profile import CONF_PROFILE_IMPURITY_GAMMA_DECAY
from .profile import (CONF_PROFILE_GREEN_TINT_IMPURITY, CONF_PROFILE_PURPLE_TINT_IMPURITY)
from .profile import (CONF_PROFILE_RED_CCT_IMPURITY, CONF_PROFILE_BLUE_CCT_IMPURITY)

from .profile import (CONF_PROFILE_WHITE_POINT_COLOR_TEMPERATURE, CONF_PROFILE_COLD_WHITE_COLOR_TEMPERATURE,CONF_PROFILE_WARM_WHITE_COLOR_TEMPERATURE)


from .profile import (CONF_PROFILE_MAX_WARM_WHITE_INTENSITY, CONF_PROFILE_MAX_COLD_WHITE_INTENSITY, CONF_PROFILE_MAX_COMBINED_WHITE_INTENSITY)
from .profile import (CONF_PROFILE_MIN_WARM_WHITE_INTENSITY, CONF_PROFILE_MIN_COLD_WHITE_INTENSITY, CONF_PROFILE_MIN_COMBINED_WHITE_INTENSITY)

# CWWW Profile Common 
CwWwProfile = xy_light_ns.class_("CwWwProfile", cg.Component)

CWWW_PROFILE_CONFIG_SCHEMA  = cv.Schema({ 
    cv.GenerateID(CONF_ID): cv.declare_id(CwWwProfile),

    cv.Optional(CONF_PROFILE_IMPURITY_GAMMA_DECAY): cv.positive_float,

    cv.Optional(CONF_PROFILE_GREEN_TINT_IMPURITY): xy_cv.duv,
    cv.Optional(CONF_PROFILE_PURPLE_TINT_IMPURITY): xy_cv.duv,

    cv.Optional(CONF_PROFILE_RED_CCT_IMPURITY): cv.color_temperature,
    cv.Optional(CONF_PROFILE_BLUE_CCT_IMPURITY): cv.color_temperature,

    cv.Optional(CONF_PROFILE_WHITE_POINT_COLOR_TEMPERATURE): cv.color_temperature,
    cv.Required(CONF_PROFILE_COLD_WHITE_COLOR_TEMPERATURE): cv.color_temperature,
    cv.Required(CONF_PROFILE_WARM_WHITE_COLOR_TEMPERATURE): cv.color_temperature,
    cv.Optional(CONF_PROFILE_GAMMA): cv.positive_float,

    cv.Optional(CONF_PROFILE_MAX_WARM_WHITE_INTENSITY): cv.percentage,
    cv.Optional(CONF_PROFILE_MAX_COLD_WHITE_INTENSITY): cv.percentage,
    cv.Optional(CONF_PROFILE_MAX_COMBINED_WHITE_INTENSITY): cv.percentage,

    cv.Optional(CONF_PROFILE_MIN_WARM_WHITE_INTENSITY): cv.percentage,
    cv.Optional(CONF_PROFILE_MIN_COLD_WHITE_INTENSITY): cv.percentage,
    cv.Optional(CONF_PROFILE_MIN_COMBINED_WHITE_INTENSITY): cv.percentage,

}).extend(cv.COMPONENT_SCHEMA)

async def to_cwww_profile_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
   
    if CONF_PROFILE_GAMMA in config:
        g = config[CONF_PROFILE_GAMMA]
        cg.add(var.set_gamma(g))

    if CONF_PROFILE_MAX_WARM_WHITE_INTENSITY in config:
        i = config[CONF_PROFILE_MAX_WARM_WHITE_INTENSITY]
        cg.add(var.set_max_warm_white_intensity(i))

    if CONF_PROFILE_MAX_COLD_WHITE_INTENSITY in config:
        i = config[CONF_PROFILE_MAX_COLD_WHITE_INTENSITY]
        cg.add(var.set_max_cold_white_intensity(i))

    if CONF_PROFILE_MAX_COMBINED_WHITE_INTENSITY in config:
        i = config[CONF_PROFILE_MAX_COMBINED_WHITE_INTENSITY]
        cg.add(var.set_max_combined_white_intensity(i))

    if CONF_PROFILE_MIN_WARM_WHITE_INTENSITY in config:
        i = config[CONF_PROFILE_MIN_WARM_WHITE_INTENSITY]
        cg.add(var.set_min_warm_white_intensity(i))

    if CONF_PROFILE_MIN_COLD_WHITE_INTENSITY in config:
        i = config[CONF_PROFILE_MIN_COLD_WHITE_INTENSITY]
        cg.add(var.set_min_cold_white_intensity(i))

    if CONF_PROFILE_MIN_COMBINED_WHITE_INTENSITY in config:
        i = config[CONF_PROFILE_MIN_COMBINED_WHITE_INTENSITY]
        cg.add(var.set_min_combined_white_intensity(i))

    if CONF_PROFILE_IMPURITY_GAMMA_DECAY in config:
        imp = config[CONF_PROFILE_IMPURITY_GAMMA_DECAY]
        cg.add(var.set_impurity_decay_gamma(imp))

    if CONF_PROFILE_WHITE_POINT_COLOR_TEMPERATURE in config:
        wp_cct = config[CONF_PROFILE_WHITE_POINT_COLOR_TEMPERATURE]
        cg.add(var.set_white_point_cct(wp_cct))

    if CONF_PROFILE_COLD_WHITE_COLOR_TEMPERATURE in config:
        cw_cct = config[CONF_PROFILE_COLD_WHITE_COLOR_TEMPERATURE]
        cg.add(var.set_cold_white_cct(cw_cct))

    if CONF_PROFILE_WARM_WHITE_COLOR_TEMPERATURE in config:
        ww_cct = config[CONF_PROFILE_WARM_WHITE_COLOR_TEMPERATURE]
        cg.add(var.set_warm_white_cct(ww_cct))

    if CONF_PROFILE_GREEN_TINT_IMPURITY in config:
        imp = config[CONF_PROFILE_GREEN_TINT_IMPURITY]
        cg.add(var.set_green_tint_duv_impurity(imp))

    if CONF_PROFILE_PURPLE_TINT_IMPURITY in config:
        imp = config[CONF_PROFILE_PURPLE_TINT_IMPURITY]
        cg.add(var.set_purple_tint_duv_impurity(imp))

    if CONF_PROFILE_RED_CCT_IMPURITY in config:
        imp = config[CONF_PROFILE_RED_CCT_IMPURITY]
        cg.add(var.set_red_wb_impurity(imp))

    if CONF_PROFILE_BLUE_CCT_IMPURITY in config:
        imp = config[CONF_PROFILE_BLUE_CCT_IMPURITY]
        cg.add(var.set_blue_wb_impurity(imp))

    await cg.register_component(var, config)