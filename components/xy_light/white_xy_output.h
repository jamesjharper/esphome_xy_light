#pragma once
#include "esphome/core/log.h"
#include "esphome/core/component.h"
#include "esphome/components/output/float_output.h"

#include "esphome/components/xy_light/xy_output.h"
#include "esphome/components/xy_light/color_spaces.h"
#include "esphome/components/xy_light/white_profile.h"

namespace esphome {
namespace xy_light {

class WhiteXyOutput : public Component, public XyOutput {
 protected:
  WhiteChromaTransform _white_profile_transform;
  bool _calibration_logging = false;
  output::FloatOutput *_white = NULL;

 public:
  void set_color_XYZ(float X, float Y, float Z) override {
    auto XYZ = color_space::XYZ_Cie1931(X, Y, Z);
    auto w = this->_white_profile_transform.XYZ_to_white_intensity(XYZ);

    if (this->_calibration_logging)
      this->log_calibration_data(w);

    if (this->_white)
      this->_white->set_level(w);
  }

  void enable_calibration_logging(bool enable) { this->_calibration_logging = enable; }

  void set_profile(WhiteProfile *profile) { this->_white_profile_transform = profile->get_chroma_transform(); }

  void set_white_output(output::FloatOutput *white) { this->_white = _white; }

  static void log_calibration_data(float i) { ESP_LOGI("output.white_xy_output", "intensity: %.0f%%", i * 100); }
};

}  // namespace xy_light
}  // namespace esphome
