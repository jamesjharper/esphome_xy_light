import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

from . import validation as xy_cv

from .xy_output import xy_light_ns

from .profile import CONF_PROFILE_GAMMA
from .profile import CONF_PROFILE_IMPURITY_GAMMA_DECAY
from .profile import (CONF_PROFILE_GREEN_TINT_IMPURITY, CONF_PROFILE_PURPLE_TINT_IMPURITY)
from .profile import (CONF_PROFILE_RED_CCT_IMPURITY, CONF_PROFILE_BLUE_CCT_IMPURITY)
from .profile import CONF_PROFILE_WHITE_POINT_COLOR_TEMPERATURE

# W Profile Common 
WhiteProfile = xy_light_ns.class_("WhiteProfile", cg.Component)

WHITE_PROFILE_CONFIG_SCHEMA  = cv.Schema({ 
    cv.GenerateID(CONF_ID): cv.declare_id(WhiteProfile),

    cv.Optional(CONF_PROFILE_IMPURITY_GAMMA_DECAY): cv.positive_float,

    cv.Optional(CONF_PROFILE_GREEN_TINT_IMPURITY): xy_cv.duv,
    cv.Optional(CONF_PROFILE_PURPLE_TINT_IMPURITY): xy_cv.duv,

    cv.Optional(CONF_PROFILE_RED_CCT_IMPURITY): cv.color_temperature,
    cv.Optional(CONF_PROFILE_BLUE_CCT_IMPURITY): cv.color_temperature,

    cv.Optional(CONF_PROFILE_WHITE_POINT_COLOR_TEMPERATURE): cv.color_temperature,
    cv.Optional(CONF_PROFILE_GAMMA): cv.positive_float

}).extend(cv.COMPONENT_SCHEMA)

async def to_white_profile_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
   
    if CONF_PROFILE_GAMMA in config:
        g = config[CONF_PROFILE_GAMMA]
        cg.add(var.set_gamma(g))

    if CONF_PROFILE_IMPURITY_GAMMA_DECAY in config:
        imp = config[CONF_PROFILE_IMPURITY_GAMMA_DECAY]
        cg.add(var.set_impurity_decay_gamma(imp))

    if CONF_PROFILE_WHITE_POINT_COLOR_TEMPERATURE in config:
        wp_cct = config[CONF_PROFILE_WHITE_POINT_COLOR_TEMPERATURE]
        cg.add(var.set_white_point_cct(wp_cct))

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