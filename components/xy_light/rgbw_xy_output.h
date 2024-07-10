#pragma once
#include "esphome/core/log.h"
#include "esphome/core/component.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/xy_light/xy_output.h"
#include "esphome/components/xy_light/color_spaces.h"
#include "esphome/components/xy_light/rgb_profile.h"
#include "esphome/components/xy_light/white_profile.h"

namespace esphome {
namespace xy_light {

class RgbwXyOutput : public Component, public XyOutput {
 protected:
  RgbChromaTransform _rgb_profile_transform;
  WhiteChromaTransform _white_profile_transform;
  bool _calibration_logging = false;

  output::FloatOutput *_r = NULL;
  output::FloatOutput *_g = NULL;
  output::FloatOutput *_b = NULL;
  output::FloatOutput *_w = NULL;

 public:
  void set_color_XYZ(float X, float Y, float Z) override {
    auto XYZ = color_space::XYZ_Cie1931(X, Y, Z);
    auto rgb = this->_rgb_profile_transform.XYZ_to_RGB(XYZ);
    auto w = this->_white_profile_transform.XYZ_to_white_intensity(XYZ);

    if (this->_calibration_logging)
      this->log_calibration_data(rgb, w);

    if (this->_r)
      this->_r->set_level(clamp(rgb.r, 0.0f, 1.0f));

    if (this->_g)
      this->_g->set_level(clamp(rgb.g, 0.0f, 1.0f));

    if (this->_b)
      this->_b->set_level(clamp(rgb.b, 0.0f, 1.0f));

    if (this->_w)
      this->_w->set_level(clamp(w, 0.0f, 1.0f));
  }

  void enable_calibration_logging(bool enable) { this->_calibration_logging = enable; }

  void set_color_profile(RgbProfile *profile) { this->_rgb_profile_transform = profile->get_chroma_transform(); }

  void set_white_profile(WhiteProfile *profile) { this->_white_profile_transform = profile->get_chroma_transform(); }

  void set_red_output(output::FloatOutput *red) { this->_r = red; }

  void set_green_output(output::FloatOutput *green) { this->_g = green; }

  void set_blue_output(output::FloatOutput *blue) { this->_b = blue; }

  void set_white_output(output::FloatOutput *w) { this->_w = w; }

 private:
  static void log_calibration_data(color_space::RGB rgb, float w) {
    auto rgb_max = rgb.max();
    ESP_LOGI("output.rgb_w_xy_output", "Normalized: [R %.2f%%, G %.2f%%, B %.2f%%, W %.2f%%] Actual: [R %.2f%%, G %.2f%%, B %.2f%%, W %.2f%%]",
          (rgb.r / rgb_max) * 100.0f, 
          (rgb.g / rgb_max) * 100.0f, 
          (rgb.b / rgb_max) * 100.0f, 
          w,  
          rgb.r * 100.0f, 
          rgb.g * 100.0f, 
          rgb.b * 100.0f, 
          w);
  }
};
}  // namespace xy_light
}  // namespace esphome
