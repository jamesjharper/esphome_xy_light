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
  bool _calibration_logging = false;
  output::FloatOutput *_r = NULL;
  output::FloatOutput *_g = NULL;
  output::FloatOutput *_b = NULL;

 public:
  RgbChromaTransform rgb_profile_transform;

  void set_color_XYZ(float X, float Y, float Z) override {
    auto rgb = this->rgb_profile_transform.XYZ_to_RGB(color_space::XYZ_Cie1931(X, Y, Z));

    if (this->_calibration_logging)
      this->log_calibration_data(rgb);

    rgb = rgb.clamp_truncate();
    if (this->_r)
      this->_r->set_level(rgb.r);

    if (this->_g)
      this->_g->set_level(rgb.g);

    if (this->_b)
      this->_b->set_level(rgb.b);
  }

  void enable_calibration_logging(bool enable) { this->_calibration_logging = enable; }

  void set_color_profile(RgbProfile *profile) { this->rgb_profile_transform = profile->get_chroma_transform(); }

  void set_red_output(output::FloatOutput *red) { this->_r = red; }

  void set_green_output(output::FloatOutput *green) { this->_g = green; }

  void set_blue_output(output::FloatOutput *blue) { this->_b = blue; }

 private:
  static void log_calibration_data(color_space::RGB rgb) {
    auto rgb_max = rgb.max();

    ESP_LOGI("output.rgb_xy_output", "Normalized: [R %.2f%%, G %.2f%%, B %.2f%%] Actual: [R %.2f%%, G %.2f%%, B %.2f%%]",
      (rgb.r / rgb_max) * 100.0f, 
      (rgb.g / rgb_max) * 100.0f, 
      (rgb.b / rgb_max) * 100.0f, 
      rgb.r * 100.0f, 
      rgb.g * 100.0f, 
      rgb.b * 100.0f
    );
  }
};
}  // namespace xy_light
}  // namespace esphome
