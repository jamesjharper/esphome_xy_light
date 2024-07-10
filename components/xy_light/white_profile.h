#pragma once

#include "esphome/core/log.h"
#include "esphome/components/logger/logger.h"
#include "esphome/components/xy_light/color_spaces.h"

namespace esphome {
namespace xy_light {

class WhiteChromaTransform {
 protected:
  float _gamma = 1.00f;

  // The "perfect" white light, has an xy value which lays somewhere alone the Planckian locus.
  // To ensure the light has a smooth transition between "true" white light and other hues,
  // a permited impurity value is used to attenuate the light s brightness as it fall future from the Planckian locus.
  float _green_tint_duv_impurity = 0.06f;
  float _purple_tint_duv_impurity = 0.05f;

  float _red_wb_impurity_threshold_mired = 100.0f;
  float _blue_wb_impurity_threshold_mired = 10.0f;

  float _red_wb_impurity_k;
  float _blue_wb_impurity_k;
  float _white_point_k;

  // Greater the rate of decay:
  // - more colour accurate
  // - less bright when showing green/purple hues
  // - at extreme values light will appear to flicker as it transitions between certain hues

  // The lesser the rate of decay:
  //  - more washed out the colours,
  //  - brighter light
  //  - and will have less obvious shifts in brightness when moving between green/purple to red/blue/white hues
  float _impurity_attn_decay_gamma = 1.5f;

 public:
  void set_gamma(float g) { this->_gamma = g; }

  void set_white_point(float mired) {
    auto wp = color_space::ColorTemperature::from_mired(mired);
    this->_white_point_k = wp.as_kelvin();
    this->_red_wb_impurity_k = wp.add_mired(this->_red_wb_impurity_threshold_mired).as_kelvin();
    this->_blue_wb_impurity_k = wp.sub_mired(this->_blue_wb_impurity_threshold_mired).as_kelvin();
  }

  void set_red_wb_impurity(float mired) {
    this->_red_wb_impurity_k = color_space::ColorTemperature::from_kelvin(this->_white_point_k)
                                   .add_mired(this->_red_wb_impurity_threshold_mired)
                                   .as_kelvin();
  }

  void set_blue_wb_impurity(float mired) {
    this->_blue_wb_impurity_k = color_space::ColorTemperature::from_kelvin(this->_white_point_k)
                                    .sub_mired(this->_blue_wb_impurity_threshold_mired)
                                    .as_kelvin();
  }

  void set_green_tint_duv_impurity(float duv) { this->_green_tint_duv_impurity = duv; }

  void set_purple_tint_duv_impurity(float duv) { this->_purple_tint_duv_impurity = duv; }

  void set_impurity_decay_gamma(float g) { this->_impurity_attn_decay_gamma = g; }

 private:
  float tint_impurity_attenuation_factor(color_space::xyY_Cie1931 t_xyY) {
    return color_space::Xy_Cie1931(t_xyY.x, t_xyY.y)
        .as_uv_cie1960()
        .tint_impurity(this->_green_tint_duv_impurity, this->_purple_tint_duv_impurity);
  }

  float wb_impurity_attenuation_factor(float k) {
    if (k > this->_white_point_k) {
      return clamp(1 - ((k - this->_white_point_k) / this->_blue_wb_impurity_k), 0.0f, 1.0f);
    } else if (k < this->_white_point_k) {
      return clamp(1 - ((this->_white_point_k - k) / this->_red_wb_impurity_k), 0.0f, 1.0f);
    }

    return 1.0;
  }

 public:
  float XYZ_to_white_intensity(color_space::XYZ_Cie1931 XYZ) {
    auto t_xyY = XYZ.as_xyY_cie1931();

    // Reduce the brightness the future the target colour is from the planckian locus
    auto tint_impurity_attn = this->tint_impurity_attenuation_factor(t_xyY);

    // Exit early if we can, as calculating approximate color temp is very cpu intensive
    if (tint_impurity_attn == 0.0f) {
      return 0.0f;
    }

    auto k = t_xyY.cct_kelvin_approx();
    auto wb_impurity_attn = this->wb_impurity_attenuation_factor(k);

    // Rather then multiplying the attenuations, take the smallest of the two.
    // this is to prevent a too rapid of a roll off towards the extreme ends of colour temperatures.
    float impurity_attn = std::min(wb_impurity_attn, tint_impurity_attn);

    if (impurity_attn == 0.0f) {
      return 0.0f;
    }

    auto brightness = powf(impurity_attn, this->_impurity_attn_decay_gamma) * t_xyY.Y;
    return color_space::exp_gamma_compress(brightness, this->_gamma);
  }
};

class WhiteProfile : public Component {
 protected:
  WhiteChromaTransform _chroma_transform;

 public:
  void set_gamma(float g) { this->_chroma_transform.set_gamma(g); }

  void set_tint_duv_impurity(float duv) {
    this->_chroma_transform.set_green_tint_duv_impurity(duv);
    this->_chroma_transform.set_purple_tint_duv_impurity(duv);
  }

  void set_green_tint_duv_impurity(float duv) { this->_chroma_transform.set_green_tint_duv_impurity(duv); }

  void set_purple_tint_duv_impurity(float duv) { this->_chroma_transform.set_purple_tint_duv_impurity(duv); }

  void set_white_point_cct(float mireds) { this->_chroma_transform.set_white_point(mireds); }

  void set_impurity_decay_gamma(float g) { this->_chroma_transform.set_impurity_decay_gamma(g); }

  void set_red_wb_impurity(float mireds) { this->_chroma_transform.set_red_wb_impurity(mireds); }

  void set_blue_wb_impurity(float mireds) { this->_chroma_transform.set_blue_wb_impurity(mireds); }

  WhiteChromaTransform get_chroma_transform() { return this->_chroma_transform; }
};

}  // namespace xy_light
}  // namespace esphome
