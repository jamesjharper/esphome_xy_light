#pragma once
#include "esphome/core/log.h"
#include "esphome/core/component.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/xy_light/xy_output.h"
#include "esphome/components/xy_light/cwww_profile.h"

namespace esphome {
namespace xy_light {

class CwWwXyOutput : public Component, public XyOutput {
 protected:
  CwWwChromaTransform _cwww_profile_transform;
  bool _calibration_logging = false;
  output::FloatOutput *_warm_white = NULL;
  output::FloatOutput *_cold_white = NULL;

 public:
  void set_color_XYZ(float X, float Y, float Z) override {
    auto XYZ = color_space::XYZ_Cie1931(X, Y, Z);
    auto cwww = this->_cwww_profile_transform.XYZ_to_CwWw(XYZ);

    if (this->_calibration_logging)
      this->log_calibration_data(cwww);

    if (this->_cold_white)
      this->_cold_white->set_level(cwww.cw);

    if (this->_warm_white)
      this->_warm_white->set_level(cwww.ww);
  }

  void enable_calibration_logging(bool enable) { this->_calibration_logging = enable; }

  void set_profile(CwWwProfile *profile) { this->_cwww_profile_transform = profile->get_chroma_transform(); }

  void set_warm_white_output(output::FloatOutput *warm_white) { this->_warm_white = warm_white; }

  void set_cold_white_output(output::FloatOutput *cold_white) { this->_cold_white = cold_white; }

  static void log_calibration_data(color_space::CwWw cwww) {
    auto cwww_max = cwww.max();
    ESP_LOGI("output.cwww_xy_output", "Normalized: [CW %.2f%%, WW %.2f%%] Actual: [CW %.2f%%, WW %.2f%%]",
          (cwww.cw / cwww_max) * 100.0f, 
          (cwww.ww / cwww_max) * 100.0f, 
          cwww.cw * 100.0f,  
          cwww.ww * 100.0f);
  }
};

}  // namespace xy_light
}  // namespace esphome
