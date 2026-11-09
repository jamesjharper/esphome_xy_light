#pragma once
#include "esphome/core/component.h"
#include "esphome/components/xy_light/color_spaces.h"
#include "esphome/components/xy_light/cwww_attenuation.h"

namespace esphome {
namespace xy_light {

struct CwWwProjection {
  bool valid{false};
  float estimated_color_temperature_mired{0.0f};
  float projected_color_temperature_mired{0.0f};
  float impurity{0.0f};
  float attenuation{0.0f};
  float brightness_scale{0.0f};
};

class CwWwChromaTransform {
 protected:
  float _gamma = 1.00f;

  // The "perfect" white light, has an xy value which lays somewhere alone the Planckian locus.
  // To ensure the light has a smooth transition between "true" white light and other hues,
  // a permited impurity value is used to attenuate the light s brightness as it fall future from the Planckian locus.
  float _green_tint_duv_impurity = 0.06f;
  float _purple_tint_duv_impurity = 0.05f;

  float _red_wb_impurity_threshold_mired = 250.0f;
  float _blue_wb_impurity_threshold_mired = 80.0f;

  float _red_wb_impurity_k;
  float _blue_wb_impurity_k;

  float _warm_white_k;
  float _warm_white_mired;
  float _cold_white_k;
  float _cold_white_mired;
  optional<float> _white_point_mired;

  color_space::CwWwIntensityCalibration _int_cal;

  // Greater the rate of decay:
  // - more colour accurate
  // - less bright when showing green/purple hues
  // - at extreme values light will appear to flicker as it transitions between certain hues

  // The lesser the rate of decay:
  //  - more washed out the colours,
  //  - brighter light
  //  - and will have less obvious shifts in brightness when moving between green/purple to red/blue/white hues
  //float _impurity_attn_decay_gamma = 1.5f;
  float _impurity_attn_decay_gamma = 3.0f;
  CwWwAttenuationFunction _attenuation_function = CwWwAttenuationFunction::INVERSE_GAMMA;

 public:
  void set_gamma(float g) { this->_gamma = g; }

  void set_max_cold_white_intensity(float i) { this->_int_cal.max_cw = i; }
  void set_max_warm_white_intensity(float i) { this->_int_cal.max_ww = i; }
  void set_max_combined_white_intensity(float i) { this->_int_cal.max_combined = i; }

  void set_min_cold_white_intensity(float i) { this->_int_cal.min_cw = i; }
  void set_min_warm_white_intensity(float i) { this->_int_cal.min_ww = i; }
  void set_min_combined_white_intensity(float i) { this->_int_cal.min_combined = i; }

  void set_warm_white(float mired) {
    auto ww = color_space::ColorTemperature::from_mired(mired);
    this->_warm_white_k = ww.as_kelvin();
    this->_warm_white_mired = mired;
    this->_red_wb_impurity_k = ww.add_mired(this->_red_wb_impurity_threshold_mired).as_kelvin();
  }

  void set_cold_white(float mired) {
    auto cw = color_space::ColorTemperature::from_mired(mired);
    this->_cold_white_k = cw.as_kelvin();
    this->_cold_white_mired = mired;
    this->_blue_wb_impurity_k = cw.sub_mired(this->_blue_wb_impurity_threshold_mired).as_kelvin();
  }

  void set_white_point(float mired) {
    this->_white_point_mired = mired;
  }

  void set_red_wb_impurity(float mired) {
    this->_red_wb_impurity_k = color_space::ColorTemperature::from_kelvin(this->_warm_white_k)
                                   .add_mired(this->_red_wb_impurity_threshold_mired)
                                   .as_kelvin();
  }

  void set_blue_wb_impurity(float mired) {
    this->_blue_wb_impurity_k = color_space::ColorTemperature::from_kelvin(this->_cold_white_k)
                                    .sub_mired(this->_blue_wb_impurity_threshold_mired)
                                    .as_kelvin();
  }

  void set_green_tint_duv_impurity(float duv) { this->_green_tint_duv_impurity = duv; }

  void set_purple_tint_duv_impurity(float duv) { this->_purple_tint_duv_impurity = duv; }

  void set_impurity_decay_gamma(float g) { this->_impurity_attn_decay_gamma = g; }

  void set_attenuation_function(CwWwAttenuationFunction function) {
    this->_attenuation_function = function;
  }

 private:
  float tint_impurity_attenuation_factor(color_space::xyY_Cie1931 t_xyY) {
    return color_space::Xy_Cie1931(t_xyY.x, t_xyY.y)
        .as_uv_cie1960()
        .tint_impurity(this->_green_tint_duv_impurity, this->_purple_tint_duv_impurity);
  }

  float wb_impurity_attenuation_factor(float k) {
    // Calculate in kelvin, otherwise the role off for warmer colors is too sudden
    if (k > this->_cold_white_k) {
      return clamp(((this->_blue_wb_impurity_k - k) / (k - this->_cold_white_k)), 0.0f, 1.0f);
    } else if (k < this->_warm_white_k) {
      return clamp(((k - this->_red_wb_impurity_k) / (this->_warm_white_k - k)), 0.0f, 1.0f);
    }

    return 1.0;
  }

 public:
  CwWwProjection project_XYZ_to_CwWw(color_space::XYZ_Cie1931 XYZ) {
    return this->project_XYZ_to_CwWw(XYZ, this->_attenuation_function);
  }

  CwWwProjection project_XYZ_to_CwWw(color_space::XYZ_Cie1931 XYZ,
                                      CwWwAttenuationFunction attenuation_function) {
    CwWwProjection result;
    auto t_xyY = XYZ.as_xyY_cie1931();

    if (!std::isfinite(t_xyY.x) || !std::isfinite(t_xyY.y) ||
        !std::isfinite(t_xyY.Y) || t_xyY.Y <= 0.0f) {
      return result;
    }

    // Reuse the same Duv validity and tint rejection used by the physical
    // CWWW transform. Values outside the valid CCT/Duv region remain invalid.
    auto tint_impurity_attn = this->tint_impurity_attenuation_factor(t_xyY);
    if (!std::isfinite(tint_impurity_attn) || tint_impurity_attn <= 0.0f) {
      return result;
    }

    auto k = t_xyY.cct_kelvin_approx();
    if (!std::isfinite(k) || k <= 0.0f) {
      return result;
    }

    auto wb_impurity_attn = this->wb_impurity_attenuation_factor(k);
    float impurity_attn = std::min(wb_impurity_attn, tint_impurity_attn);
    if (!std::isfinite(impurity_attn) || impurity_attn <= 0.0f) {
      return result;
    }

    auto attenuation = apply_cwww_impurity_attenuation(
        impurity_attn, this->_impurity_attn_decay_gamma, attenuation_function);
    if (!std::isfinite(attenuation)) {
      return result;
    }

    auto estimated_mired = 1000000.0f / k;
    result.valid = true;
    result.estimated_color_temperature_mired = estimated_mired;
    result.projected_color_temperature_mired =
        std::min(std::max(estimated_mired, this->_cold_white_mired), this->_warm_white_mired);
    result.impurity = impurity_attn;
    result.attenuation = attenuation;
    result.brightness_scale = attenuation * t_xyY.Y;
    return result;
  }

  color_space::CwWw XYZ_to_CwWw(color_space::XYZ_Cie1931 XYZ) {
    return this->XYZ_to_CwWw(XYZ, this->_attenuation_function);
  }

  color_space::CwWw XYZ_to_CwWw(color_space::XYZ_Cie1931 XYZ,
                                CwWwAttenuationFunction attenuation_function) {
    auto projection = this->project_XYZ_to_CwWw(XYZ, attenuation_function);
    if (!projection.valid || projection.brightness_scale == 0.0f) {
      return {0.0f, 0.0f};
    }

    float wp = this->white_point_mired();
    // Preserve the legacy physical-output behaviour by using the unbounded
    // estimated CCT here. Consumers that need an abstract, achievable colour
    // should use projected_color_temperature_mired instead.
    auto mired = projection.estimated_color_temperature_mired;
    auto brightness = projection.brightness_scale;

    auto cw = (1 - ((mired - wp) / (this->_warm_white_mired - wp))) * brightness;
    auto ww = (1 - ((wp - mired) / (wp - this->_cold_white_mired))) * brightness;
    auto cwww = color_space::CwWw(cw, ww);

    cwww = cwww.gamma_compress(this->_gamma);
    cwww = this->_int_cal.apply_calibration(cwww);

    return cwww;
  }

 protected:
  float &white_point_mired() {
    if (!this->_white_point_mired.has_value()) {
      this->_white_point_mired = (this->_warm_white_mired + this->_cold_white_mired) / 2;
    }
    return this->_white_point_mired.value();
  }
};

class CwWwProfile : public Component {
 protected:
  CwWwChromaTransform _chroma_transform;

 public:
  void set_gamma(float g) { this->_chroma_transform.set_gamma(g); }

  void set_max_cold_white_intensity(float i) { this->_chroma_transform.set_max_cold_white_intensity(i); }
  void set_max_warm_white_intensity(float i) { this->_chroma_transform.set_max_warm_white_intensity(i); }
  void set_max_combined_white_intensity(float i) { this->_chroma_transform.set_max_combined_white_intensity(i); }

  void set_min_cold_white_intensity(float i) { this->_chroma_transform.set_min_cold_white_intensity(i); }
  void set_min_warm_white_intensity(float i) { this->_chroma_transform.set_min_warm_white_intensity(i); }
  void set_min_combined_white_intensity(float i) { this->_chroma_transform.set_min_combined_white_intensity(i); }

  void set_green_tint_duv_impurity(float duv) { this->_chroma_transform.set_green_tint_duv_impurity(duv); }

  void set_purple_tint_duv_impurity(float duv) { this->_chroma_transform.set_purple_tint_duv_impurity(duv); }

  void set_white_point_cct(float mired) { this->_chroma_transform.set_white_point(mired); }

  void set_warm_white_cct(float mired) { this->_chroma_transform.set_warm_white(mired); }

  void set_cold_white_cct(float mired) { this->_chroma_transform.set_cold_white(mired); }

  void set_impurity_decay_gamma(float g) { this->_chroma_transform.set_impurity_decay_gamma(g); }

  void set_attenuation_function(CwWwAttenuationFunction function) {
    this->_chroma_transform.set_attenuation_function(function);
  }

  void set_red_wb_impurity(float mired) { this->_chroma_transform.set_red_wb_impurity(mired); }

  void set_blue_wb_impurity(float mired) { this->_chroma_transform.set_blue_wb_impurity(mired); }

  CwWwChromaTransform get_chroma_transform() { return this->_chroma_transform; }
};

}  // namespace xy_light
}  // namespace esphome
