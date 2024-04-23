#pragma once
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/core/component.h"
#include "esphome/components/output/float_output.h"

#include "esphome/components/xy_light/xy_output.h"
#include "esphome/components/xy_light/color_spaces.h"
#include "esphome/components/xy_light/rgb_profile.h"

namespace esphome {
namespace xy_light {

class RgbXyOutput : public Component, public XyOutput {
 protected:
  RgbChromaTransform _rgb_profile_transform;
  bool _calibration_logging = false;
  output::FloatOutput *_r = NULL;
  output::FloatOutput *_g = NULL;
  output::FloatOutput *_b = NULL;

 public:
  void set_color_XYZ(float X, float Y, float Z) override {
    auto rgb = this->_rgb_profile_transform.XYZ_to_RGB(color_space::XYZ_Cie1931(X, Y, Z));

    if (this->_calibration_logging)
      this->log_calibration_data(rgb);

    if (this->_r)
      this->_r->set_level(clamp(rgb.r, 0.0f, 1.0f));

    if (this->_g)
      this->_g->set_level(clamp(rgb.g, 0.0f, 1.0f));

    if (this->_b)
      this->_b->set_level(clamp(rgb.b, 0.0f, 1.0f));
  }

  void enable_calibration_logging(bool enable) { this->_calibration_logging = enable; }

  void set_color_profile(RgbProfile *profile) { this->_rgb_profile_transform = profile->get_chroma_transform(); }

  void set_red_output(output::FloatOutput *red) { this->_r = red; }

  void set_green_output(output::FloatOutput *green) { this->_g = green; }

  void set_blue_output(output::FloatOutput *blue) { this->_b = blue; }

 private:
  static void log_calibration_data(color_space::RGB rgb) {
    auto max = rgb.max();
    ESP_LOGI("output.rgb_xy_output", "Red: %.0f%, Green: %.0f%, Blue: %.0f%, Lum: %.0f%", (rgb.r / max) * 100,
             (rgb.g / max) * 100, (rgb.b / max) * 100, max * 100);
  }
};
}  // namespace xy_light
}  // namespace esphome
