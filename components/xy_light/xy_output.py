import esphome.config_validation as cv
import esphome.codegen as cg

xy_light_ns = cg.esphome_ns.namespace("xy_light")
XyOutput = xy_light_ns.output_ns.class_("XyOutput")

CONF_XY_OUTPUT_RGB_COLOR_PROFILE_ID = "rgb_profile_id"
CONF_XY_OUTPUT_RGB_COLOR_PROFILE = "rgb_profile"

CONF_XY_OUTPUT_CWWW_COLOR_PROFILE_ID = "cwww_profile_id"
CONF_XY_OUTPUT_CWWW_COLOR_PROFILE = "cwww_profile"
CONF_XY_OUTPUT_WHITE_COLOR_PROFILE_ID = "white_profile_id"
CONF_XY_OUTPUT_WHITE_COLOR_PROFILE = "white_profile"

CONF_XY_OUTPUT_RED_OUTPUT_ID = "red"
CONF_XY_OUTPUT_GREEN_OUTPUT_ID = "green"
CONF_XY_OUTPUT_BLUE_OUTPUT_ID = "blue"
CONF_XY_OUTPUT_WHITE_OUTPUT_ID = "white"
CONF_XY_OUTPUT_WARM_WHITE_OUTPUT_ID = "warm_white"
CONF_XY_OUTPUT_COLD_WHITE_OUTPUT_ID = "cold_white"

CONF_XY_OUTPUT_CALIBRATION_LOGGING = "calibration_logging"


