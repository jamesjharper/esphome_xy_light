#pragma once
#include <set>
#include "esphome/core/optional.h"
#include "esphome/core/component.h"
#include "esphome/components/light/light_output.h"
#include "esphome/components/light/light_state.h"
#include "esphome/components/output/float_output.h"

#include "esphome/components/xy_light/color_spaces.h"
#include "esphome/components/xy_light/rgb_profile.h"
#include "esphome/components/xy_light/xy_output.h"

namespace esphome {
namespace xy_light {

inline static bool almost_eq(float a, float b) {
    return fabs(a - b) < 0.005f;
}

class XyLightOutput {
 protected: 

  RgbChromaTransform _gamut_transform;
  color_space::Cie2dColorSpace _white_point = color_space::Xy_Cie1931();

  std::vector<XyOutput *> _outputs;

  float _brightness, _saturation = 1.0f;
  bool _calibration_logging = false;

  color_space::RGB _rgb;
  optional<color_space::Xy_Cie1931> _xy = {};
  
 public:

  XyLightOutput() {
    this->_gamut_transform.set_sRGB();
    this->_white_point = this->_gamut_transform.get_white_point();
  }

  void set_source_color_profile(RgbProfile *profile) {
    this->_gamut_transform = profile->get_chroma_transform(); 
    this->_white_point = this->_gamut_transform.get_white_point();
  }

  void add_output(XyOutput *output) { this->_outputs.push_back(output); }

  void set_color_temperature_value(float mired) {
    if(!this->_calibration_logging) {
      auto ct_uv_1960 = color_space::Cct::from_mireds(mired).uv;
      this->_white_point = ct_uv_1960.as_xy_cie1931();
    }
  }

  void set_color_saturation_value(float i) {
    this->_saturation = i;
  }

  void set_brightness_value(float i) {
    this->_brightness = i;
  }

  void set_rgb_value(float r, float g, float b) {
    this->_rgb = color_space::RGB(r,g,b);
  }

  void set_xy_value(float x, float y) {
    this->_xy = color_space::Xy_Cie1931(x, y);
  }

  void enable_calibration_logging(bool enable) { this->_calibration_logging = enable; }

  void apply() {
    if (this->_xy.has_value()) {
      // Use xy values if they have been given
      this->apply_xyY(this->_xy.value(), 1.0f);
    } else {
      // Otherwise convert RGB values to xy from source colour space
      auto xyY = this->_gamut_transform.RGB_to_XYZ(this->_rgb).as_xyY_cie1931();
      auto xy = xyY.as_xy_cie1931();
      auto Y = xyY.Y;
      this->apply_xyY(xy, Y);
    }
  }

  void apply_xyY(color_space::Xy_Cie1931 xy, float Y) {
    if (!almost_eq(this->_saturation, 1.0f)) {
      xy = this->_gamut_transform.adjust_saturation(xy, this->_saturation);
    } 

    if (!almost_eq(this->_brightness, 1.0f)) {
      Y *= this->_brightness;
    }

    // Adjust white balance
    auto XYZ = color_space::xyY_Cie1931(xy.x, xy.y, Y).as_XYZ_cie1931();
    XYZ = this->_gamut_transform.adjust_white_balance(XYZ, this->_white_point);

    if(this->_calibration_logging) {
      XyLightOutput::log_calibration_data(XYZ);
    }
        
    for (auto output : this->_outputs){
        output->set_color_XYZ(XYZ.X, XYZ.Y, XYZ.Z);
    }
  }

  static void log_calibration_data(color_space::XYZ_Cie1931 XYZ) {
    auto xyY = XYZ.as_xyY_cie1931();
    ESP_LOGI("output.xy_light_output", "xy: [%.3f%, %.3f%], XYZ: [%.3f%, %.3f%, %.3f%], Approx Color Temperature: %.0f K%", 
      xyY.x, xyY.y, 
      XYZ.X, XYZ.Y, XYZ.Z, 
      xyY.cct_kelvin_approx());
  }
};

enum class ControlAttributes : std::uint8_t {
  BRIGHTNESS = 1,
  RGB = 2,
  XY = 4,
  CT = 8,
  CW_WW = 16,
  SATURATION = 32,
  INTERLOCK = 64
};

constexpr inline ControlAttributes operator|(ControlAttributes a, ControlAttributes b) {
  return static_cast<ControlAttributes>(static_cast<std::uint8_t>(a) | static_cast<std::uint8_t>(b));
}

constexpr inline ControlAttributes operator&(ControlAttributes a, ControlAttributes b) {
  return static_cast<ControlAttributes>(static_cast<std::uint8_t>(a) & static_cast<std::uint8_t>(b));
}

enum class ControlType : std::uint8_t {
  RGB = (std::uint8_t)(ControlAttributes::RGB),
  RGB_SATURATION = (std::uint8_t) (ControlAttributes::RGB | ControlAttributes::SATURATION),
  RGB_CT = (std::uint8_t) (ControlAttributes::RGB | ControlAttributes::CT | ControlAttributes::INTERLOCK),
  RGB_CWWW = (std::uint8_t) (ControlAttributes::RGB | ControlAttributes::CW_WW),

  XY = (std::uint8_t)(ControlAttributes::XY),
  XY_SATURATION = (std::uint8_t) (ControlAttributes::XY | ControlAttributes::SATURATION),
  XY_CT = (std::uint8_t) (ControlAttributes::XY | ControlAttributes::CT | ControlAttributes::INTERLOCK),
  XY_CWWW = (std::uint8_t) (ControlAttributes::XY | ControlAttributes::CW_WW),

  CT = (std::uint8_t) (ControlAttributes::CT),
  CWWW = (std::uint8_t) (ControlAttributes::CW_WW),
  BRIGHTNESS = (std::uint8_t) (ControlAttributes::BRIGHTNESS),
  SATURATION = (std::uint8_t) (ControlAttributes::SATURATION)
};

class XyLightControl : public light::LightOutput, public Component  {
 protected:
  ControlAttributes _control_attributes;
  light::LightTraits _traits;

  XyLightOutput* _xy_output_light;

  public:
  void set_color_temperature_range(float min_mired, float max_mired) {
    this->_traits.set_min_mireds(min_mired);
    this->_traits.set_max_mireds(max_mired);
  }

  void set_control_type(ControlType control_type) {
    this->set_control_attributes(static_cast<ControlAttributes>(static_cast<std::uint32_t>(control_type)));
  }

  void set_control_attributes(ControlAttributes attr) {

    this->_control_attributes = attr | this->_control_attributes;
    std::set<light::ColorMode> supported_color_modes;

    if ((uint8_t)(this->_control_attributes & ControlAttributes::CT)) {
        supported_color_modes.insert(light::ColorMode::COLOR_TEMPERATURE);
    }

    if ((uint8_t)(this->_control_attributes & ControlAttributes::CW_WW)) {
        supported_color_modes.insert(light::ColorMode::COLD_WARM_WHITE);
    }

    if ((uint8_t)(this->_control_attributes & ControlAttributes::BRIGHTNESS)) {
        supported_color_modes.insert(light::ColorMode::WHITE);
    }

    if ((uint8_t)(this->_control_attributes & ControlAttributes::RGB)) {
        supported_color_modes.insert(light::ColorMode::RGB);
    }

    this->_traits.set_supported_color_modes(supported_color_modes);
  }

  void set_xy_light_output(XyLightOutput* _xy_output_light) {
    this->_xy_output_light = _xy_output_light;
  }

  light::LightTraits get_traits() override {
    return this->_traits;
  }

  void write_state(light::LightState *state) override {
    if(!this->_xy_output_light)
       return;

    if ((uint8_t)(this->_control_attributes & (ControlAttributes::CT | ControlAttributes::CW_WW))) {
        auto ct = state->current_values.get_color_temperature();
        this->_xy_output_light->set_color_temperature_value(ct);
    }

    auto intensity = state->current_values.get_state() * state->current_values.get_brightness();
  
    if ((uint8_t)(this->_control_attributes & ControlAttributes::SATURATION)) {
        this->_xy_output_light->set_color_saturation_value(intensity);
    } else {
        this->_xy_output_light->set_brightness_value(intensity);
    }

    if ((uint8_t)(this->_control_attributes & ControlAttributes::RGB)) {
        // be careful not to decompress gamma here, as this will be done by the profile
        this->_xy_output_light->set_rgb_value(
            state->current_values.get_red(),
            state->current_values.get_green(),
            state->current_values.get_blue()
        );
    }

    // Not supported by esphome at this time
    //if (this->_control_attributes & ControlAttributes::XY) {
        // this->_xy_output_light->set_xy_value(...);
    //}

    this->_xy_output_light->apply();
  }
};


}  // namespace xy_light
}  // namespace esphome
