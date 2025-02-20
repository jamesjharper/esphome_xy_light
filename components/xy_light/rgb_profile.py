import esphome.codegen as cg
import esphome.config_validation as cv

from esphome.const import CONF_ID

from . import cie
from . import validation as xy_cv

from .xy_output import xy_light_ns

from .profile import (
    CONF_PROFILE_STANDARD_PROFILE, 
    CONF_PROFILE_STANDARD_PROFILE__LED, 
    CONF_PROFILE_STANDARD_PROFILE__SRGB, 
    CONF_PROFILE_STANDARD_PROFILE__AdobeRGB_D55, 
    CONF_PROFILE_STANDARD_PROFILE__AdobeRGB_D65, 
    CONF_PROFILE_STANDARD_PROFILE__ACES_AP0,
    CONF_PROFILE_STANDARD_PROFILE__ACES_AP1
)
from .profile import (CONF_PROFILE_RED_XY, CONF_PROFILE_RED_WAVELENGTH, CONF_PROFILE_RED_INTENSITY, CONF_PROFILE_RED_MAX_INTENSITY, CONF_PROFILE_RED_MIN_INTENSITY, CONF_PROFILE_RED_GAMMA)
from .profile import (CONF_PROFILE_GREEN_XY, CONF_PROFILE_GREEN_WAVELENGTH, CONF_PROFILE_GREEN_INTENSITY, CONF_PROFILE_GREEN_MAX_INTENSITY, CONF_PROFILE_GREEN_MIN_INTENSITY, CONF_PROFILE_GREEN_GAMMA)
from .profile import (CONF_PROFILE_BLUE_XY, CONF_PROFILE_BLUE_WAVELENGTH, CONF_PROFILE_BLUE_INTENSITY, CONF_PROFILE_BLUE_MAX_INTENSITY, CONF_PROFILE_BLUE_MIN_INTENSITY, CONF_PROFILE_BLUE_GAMMA)
from .profile import (CONF_PROFILE_WHITE_POINT_XY, CONF_PROFILE_WHITE_POINT_COLOR_TEMPERATURE)
from .profile import CONF_PROFILE_GAMMA


# RGB Profile Common 
RgbProfile = xy_light_ns.class_("RgbProfile", cg.Component)

RGB_PROFILE_CONFIG_SCHEMA = cv.All(
    cv.Schema({ 
        cv.GenerateID(CONF_ID): cv.declare_id(RgbProfile),

        cv.Optional(CONF_PROFILE_STANDARD_PROFILE): cv.one_of(
            CONF_PROFILE_STANDARD_PROFILE__LED, 
            CONF_PROFILE_STANDARD_PROFILE__SRGB, 
            CONF_PROFILE_STANDARD_PROFILE__AdobeRGB_D55, 
            CONF_PROFILE_STANDARD_PROFILE__AdobeRGB_D65,
            CONF_PROFILE_STANDARD_PROFILE__ACES_AP0,
            CONF_PROFILE_STANDARD_PROFILE__ACES_AP1
        ),

        # Red
        cv.Optional(CONF_PROFILE_RED_XY): xy_cv.cie_xy,
        cv.Optional(CONF_PROFILE_RED_WAVELENGTH): xy_cv.wavelength,
        cv.Optional(CONF_PROFILE_RED_INTENSITY): cv.percentage,
        cv.Optional(CONF_PROFILE_RED_MAX_INTENSITY): cv.percentage,
        cv.Optional(CONF_PROFILE_RED_MIN_INTENSITY): cv.percentage,
        cv.Optional(CONF_PROFILE_RED_GAMMA): cv.positive_float,

        # Green
        cv.Optional(CONF_PROFILE_GREEN_XY): xy_cv.cie_xy,
        cv.Optional(CONF_PROFILE_GREEN_WAVELENGTH): xy_cv.wavelength,
        cv.Optional(CONF_PROFILE_GREEN_INTENSITY): cv.percentage,
        cv.Optional(CONF_PROFILE_GREEN_MAX_INTENSITY): cv.percentage,
        cv.Optional(CONF_PROFILE_GREEN_MIN_INTENSITY): cv.percentage,
        cv.Optional(CONF_PROFILE_GREEN_GAMMA): cv.positive_float,

        # Blue
        cv.Optional(CONF_PROFILE_BLUE_XY): xy_cv.cie_xy,   
        cv.Optional(CONF_PROFILE_BLUE_WAVELENGTH): xy_cv.wavelength,
        cv.Optional(CONF_PROFILE_BLUE_INTENSITY): cv.percentage,
        cv.Optional(CONF_PROFILE_BLUE_MAX_INTENSITY): cv.percentage,
        cv.Optional(CONF_PROFILE_BLUE_MIN_INTENSITY): cv.percentage,
        cv.Optional(CONF_PROFILE_BLUE_GAMMA): cv.positive_float,

        # White point
        cv.Optional(CONF_PROFILE_WHITE_POINT_XY): xy_cv.cie_xy,
        cv.Optional(CONF_PROFILE_WHITE_POINT_COLOR_TEMPERATURE): cv.color_temperature,

        # Gamma
        cv.Optional(CONF_PROFILE_GAMMA): cv.positive_float

    })
    .extend(cv.COMPONENT_SCHEMA),
    cv.has_exactly_one_key(CONF_PROFILE_RED_XY, CONF_PROFILE_RED_WAVELENGTH, CONF_PROFILE_STANDARD_PROFILE),
    cv.has_exactly_one_key(CONF_PROFILE_GREEN_XY, CONF_PROFILE_GREEN_WAVELENGTH, CONF_PROFILE_STANDARD_PROFILE),
    cv.has_exactly_one_key(CONF_PROFILE_BLUE_XY, CONF_PROFILE_BLUE_WAVELENGTH, CONF_PROFILE_STANDARD_PROFILE)
)

async def to_rgb_profile_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
   
    if CONF_PROFILE_STANDARD_PROFILE in config:
        profile_standard = config[CONF_PROFILE_STANDARD_PROFILE]
        if CONF_PROFILE_STANDARD_PROFILE__LED == profile_standard:
            cg.add(var.use_typical_led())
        elif CONF_PROFILE_STANDARD_PROFILE__SRGB == profile_standard:
            cg.add(var.use_sRGB())
        elif CONF_PROFILE_STANDARD_PROFILE__AdobeRGB_D55 == profile_standard:
            cg.add(var.set_AdobeRGB_D55())
        elif CONF_PROFILE_STANDARD_PROFILE__AdobeRGB_D55 == profile_standard:
            cg.add(var.use_AdobeRGB_D65())
        elif CONF_PROFILE_STANDARD_PROFILE__ACES_AP0 == profile_standard:
            cg.add(var.use_ACES_AP0())
        elif CONF_PROFILE_STANDARD_PROFILE__ACES_AP1 == profile_standard:
            cg.add(var.use_ACES_AP1())


    if CONF_PROFILE_GAMMA in config:
        g = config[CONF_PROFILE_GAMMA]
        cg.add(var.set_gamma(g))
        
    # Red Calibrations
    if CONF_PROFILE_RED_XY in config:
        xy = config[CONF_PROFILE_RED_XY]
        cg.add(var.set_red_xy(xy[0], xy[1]))

    if CONF_PROFILE_RED_WAVELENGTH in config:
        # at build time, convert wavelength into x,y
        xy = cie.wavelength_to_xy(config[CONF_PROFILE_RED_WAVELENGTH])
        cg.add(var.set_red_xy(xy[0], xy[1]))

    if CONF_PROFILE_RED_INTENSITY in config:
        i = config[CONF_PROFILE_RED_INTENSITY]
        cg.add(var.set_weighted_red_intensity(i))

    if CONF_PROFILE_RED_MAX_INTENSITY in config:
        i = config[CONF_PROFILE_RED_MAX_INTENSITY]
        cg.add(var.set_max_red_intensity(i))

    if CONF_PROFILE_RED_MIN_INTENSITY in config:
        i = config[CONF_PROFILE_RED_MIN_INTENSITY]
        cg.add(var.set_min_red_intensity(i))

    if CONF_PROFILE_RED_GAMMA in config:
        i = config[CONF_PROFILE_RED_GAMMA]
        cg.add(var.set_red_gamma(i))

    # Green Calibrations
    if CONF_PROFILE_GREEN_XY in config:
        xy = config[CONF_PROFILE_GREEN_XY]
        cg.add(var.set_green_xy(xy[0], xy[1]))

    if CONF_PROFILE_GREEN_WAVELENGTH in config:
        # at build time, convert wavelength into x,y
        xy = cie.wavelength_to_xy(config[CONF_PROFILE_GREEN_WAVELENGTH])
        cg.add(var.set_green_xy(xy[0], xy[1]))

    if CONF_PROFILE_GREEN_INTENSITY in config:
        i = config[CONF_PROFILE_GREEN_INTENSITY]
        cg.add(var.set_weighted_green_intensity(i))

    if CONF_PROFILE_GREEN_MAX_INTENSITY in config:
        i = config[CONF_PROFILE_GREEN_MAX_INTENSITY]
        cg.add(var.set_max_green_intensity(i))

    if CONF_PROFILE_GREEN_MIN_INTENSITY in config:
        i = config[CONF_PROFILE_GREEN_MIN_INTENSITY]
        cg.add(var.set_min_green_intensity(i))

    if CONF_PROFILE_GREEN_GAMMA in config:
        i = config[CONF_PROFILE_GREEN_GAMMA]
        cg.add(var.set_green_gamma(i))

    # Blue Calibrations
    if CONF_PROFILE_BLUE_XY in config:
        xy = config[CONF_PROFILE_BLUE_XY]
        cg.add(var.set_blue_xy(xy[0], xy[1]))

    if CONF_PROFILE_BLUE_WAVELENGTH in config:
        # at build time, convert wavelength into x,y
        xy = cie.wavelength_to_xy(config[CONF_PROFILE_BLUE_WAVELENGTH])
        cg.add(var.set_blue_xy(xy[0], xy[1]))

    if CONF_PROFILE_BLUE_INTENSITY in config:
        i = config[CONF_PROFILE_BLUE_INTENSITY]
        cg.add(var.set_weighted_blue_intensity(i))

    if CONF_PROFILE_BLUE_MAX_INTENSITY in config:
        i = config[CONF_PROFILE_BLUE_MAX_INTENSITY]
        cg.add(var.set_max_blue_intensity(i))

    if CONF_PROFILE_BLUE_MIN_INTENSITY in config:
        i = config[CONF_PROFILE_BLUE_MIN_INTENSITY]
        cg.add(var.set_min_blue_intensity(i))

    if CONF_PROFILE_BLUE_GAMMA in config:
        i = config[CONF_PROFILE_BLUE_GAMMA]
        cg.add(var.set_blue_gamma(i))

    # White point Calibrations
    if CONF_PROFILE_WHITE_POINT_COLOR_TEMPERATURE in config:
        k = config[CONF_PROFILE_WHITE_POINT_COLOR_TEMPERATURE]
        cg.add(var.set_white_point_cct(k))

    if CONF_PROFILE_WHITE_POINT_XY in config:
        xy = config[CONF_PROFILE_WHITE_POINT_XY]
        cg.add(var.set_white_point_xy(xy[0], xy[1]))
    
    await cg.register_component(var, config)
    