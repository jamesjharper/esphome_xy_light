#pragma once
#include "esphome/core/log.h"
#include "esphome/core/component.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/xy_light/color_spaces.h"
#include "esphome/components/xy_light/rgb_profile.h"
#include "esphome/components/xy_light/cwww_profile.h"

namespace esphome {
namespace xy_light {

class RgbCwWwXyOutput : public Component, public XyOutput {
 protected:

  bool _calibration_logging = false;

  output::FloatOutput *_r = NULL;
  output::FloatOutput *_g = NULL;
  output::FloatOutput *_b = NULL;

  output::FloatOutput *_cw = NULL;
  output::FloatOutput *_ww = NULL;

 public:
  RgbChromaTransform rgb_profile_transform;
  CwWwChromaTransform cwww_profile_transform;
  
  void set_color_XYZ(float X, float Y, float Z) override {
    auto XYZ = color_space::XYZ_Cie1931(X, Y, Z);
    auto rgb = this->rgb_profile_transform.XYZ_to_RGB(XYZ);
    auto cwww = this->cwww_profile_transform.XYZ_to_CwWw(XYZ);

    if (this->_calibration_logging)
      this->log_calibration_data(rgb, cwww);

    cwww = cwww.clamp_truncate();
    rgb = rgb.clamp_truncate();
    if (this->_r)
      this->_r->set_level(rgb.r);

    if (this->_g)
      this->_g->set_level(rgb.g);

    if (this->_b)
      this->_b->set_level(rgb.b);

    if (this->_cw)
      this->_cw->set_level(cwww.cw);

    if (this->_ww)
      this->_ww->set_level(cwww.ww);
  }

  void enable_calibration_logging(bool enable) { this->_calibration_logging = enable; }

  void set_color_profile(RgbProfile *profile) { this->rgb_profile_transform = profile->get_chroma_transform(); }

  void set_cwww_profile(CwWwProfile *profile) { this->cwww_profile_transform = profile->get_chroma_transform(); }

  void set_red_output(output::FloatOutput *red) { this->_r = red; }

  void set_green_output(output::FloatOutput *green) { this->_g = green; }

  void set_blue_output(output::FloatOutput *blue) { this->_b = blue; }

  void set_cold_white_output(output::FloatOutput *cw) { this->_cw = cw; }

  void set_warm_white_output(output::FloatOutput *ww) { this->_ww = ww; }

 private:
  static void log_calibration_data(color_space::RGB rgb, color_space::CwWw cwww) {
    auto rgb_max = rgb.max();
    auto cwww_max = cwww.max();

    ESP_LOGI("output.rgb_cwww_xy_output", "Normalized: [R %.2f%%, G %.2f%%, B %.2f%%, CW %.2f%%, WW %.2f%%] Actual: [R %.2f%%, G %.2f%%, B %.2f%%, CW %.2f%%, WW %.2f%%]",
             (rgb.r / rgb_max) * 100.0f, 
             (rgb.g / rgb_max) * 100.0f, 
             (rgb.b / rgb_max) * 100.0f, 
             (cwww.cw / cwww_max) * 100.0f, 
             (cwww.ww / cwww_max) * 100.0f, 
             rgb.r * 100.0f, 
             rgb.g * 100.0f, 
             rgb.b * 100.0f, 
             cwww.cw * 100.0f,  
             cwww.ww * 100.0f);
  }
};
}  // namespace xy_light
}  // namespace esphome
