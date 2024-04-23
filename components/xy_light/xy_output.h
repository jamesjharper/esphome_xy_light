#pragma once

namespace esphome {
namespace xy_light {

class XyOutput {
 public:
  virtual void set_color_XYZ(float X, float Y, float Z) = 0;
};

};  // namespace xy_light
};  // namespace esphome