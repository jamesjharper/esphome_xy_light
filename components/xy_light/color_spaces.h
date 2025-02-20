#pragma once
#include "esphome/core/optional.h"
#include "esphome/core/helpers.h"

#include <math.h>
#include <vector>

namespace esphome {
namespace xy_light {
namespace color_space {

struct Uv_Cie1976;
struct Uv_Cie1960;
struct xyY_Cie1931;
struct Xy_Cie1931;
struct Cct;
struct RGB;

static float exp_gamma_compress(float linear, float gamma) {
  if (linear <= 0.0f)
    return 0.0f;
  if (gamma == 1.0f || gamma <= 0.0f)
    return linear;
  return powf(linear, 1.0f / gamma);
}

static float exp_gamma_decompress(float value, float gamma) {
  if (value <= 0.0f)
    return 0.0f;
  if (gamma == 1.0f || gamma <= 0.0f)
    return value;

  return powf(value, gamma);
}

static float srgb_gamma_compress(float linear, float gamma) {
    if (linear <= 0.0031308f) {
        return 12.92 * linear;
    } else {
        return 1.055 * powf(linear, 1.0f / gamma) - 0.055f;
    }
}
static float srgb_gamma_decompress(float sRGB, float gamma) {
    if (sRGB <= 0.04045f) {
        return sRGB / 12.92f;
    } else {
        return powf((sRGB + 0.055f) / 1.055f, gamma);
    }
}

static float clamp_output_value(float v) {
  if(isnan(v)) {
    return 0.0f;
  } else if(std::isinf(v)){
    return 1.0f;
  }
  return clamp(v, 0.0f, 1.0f);
}

struct RGB {
  float r;
  float g;
  float b;

  RGB() : r(0.0), g(0.0), b(0.0){};
  RGB(float r, float g, float b) : r(r), g(g), b(b){};

  float max() { return std::max(std::max(this->r, this->g), this->b); }

  

  RGB clamp_truncate() {
    return RGB(
      clamp_output_value(this->r), 
      clamp_output_value(this->g), 
      clamp_output_value(this->b)
    );
  }

  RGB clamp_normalize() {
    auto max = this->max();
    if (max > 1.0f) {
      return RGB(this->r / max, this->g / max, this->b / max);
    }
    return *this;
  }

  RGB adjust_brightness(float i) {
    return RGB(this->r * i, this->g * i, this->b * i);
  }
};

struct RGBIntensityCalibration {

  float r_int_output_cal = 1.0; 
  float g_int_output_cal = 1.0; 
  float b_int_output_cal = 1.0;

  float r_max_output_cal = 1.0;
  float g_max_output_cal = 1.0;
  float b_max_output_cal = 1.0;

  float r_min_output_cal = 0.0;
  float g_min_output_cal = 0.0;
  float b_min_output_cal = 0.0;

  float r_gamma = 1.0; // value of 1 indicates no gamma correction is applied
  float g_gamma = 1.0; // value of 1 indicates no gamma correction is applied
  float b_gamma = 1.0; // value of 1 indicates no gamma correction is applied

  RGB apply_calibration(RGB rgb) {
    this->adjust_for_weighted_outputs(rgb);
    this->adjust_for_colors_out_of_gamut(rgb);
  
    // Adjust gamma last as this is to correct for the output response curve
    this->adjust_for_gamma_correction(rgb);
    this->adjust_for_hardware_min_max_intensity(rgb);
    return rgb;
  }

  protected:

  void adjust_for_colors_out_of_gamut(RGB &rgb) {
    auto max = rgb.max();
    if (max > 1.0f) {
      rgb.r /= max;
      rgb.g /= max;
      rgb.b /= max;
    }
  }

  void adjust_for_weighted_outputs(RGB &rgb) {
    auto l = (rgb.r + rgb.g + rgb.b) / 3.0f;

    auto r_adj = rgb.r * this->r_int_output_cal;
    auto g_adj = rgb.g * this->g_int_output_cal;
    auto b_adj = rgb.b * this->b_int_output_cal;
    auto max = std::max(r_adj, std::max(g_adj, b_adj));

    rgb.r = (r_adj / max) * l;
    rgb.g = (g_adj / max) * l;
    rgb.b = (b_adj / max) * l;

  }

  void quantize_value(RGB &rgb, uint32_t bit_depth)  {
    const float max_duty = (uint32_t(1) << bit_depth) - 1;
    rgb.r = (float)((uint32_t)(std::max(rgb.r, 0.0f) * max_duty)) / max_duty;
    rgb.b = (float)((uint32_t)(std::max(rgb.g, 0.0f) * max_duty)) / max_duty;
    rgb.g = (float)((uint32_t)(std::max(rgb.b, 0.0f) * max_duty)) / max_duty;
  }

  void adjust_for_hardware_min_max_intensity(RGB &rgb) {
    const float epsilon = 1.401298E-45f;
    rgb.r = rgb.r <= epsilon? 0.0f : (rgb.r * (1.0f - r_min_output_cal) * r_max_output_cal) + r_min_output_cal;
    rgb.b = rgb.b <= epsilon? 0.0f : (rgb.b * (1.0f - b_min_output_cal) * b_max_output_cal) + b_min_output_cal;
    rgb.g = rgb.g <= epsilon? 0.0f : (rgb.g * (1.0f - g_min_output_cal) * g_max_output_cal) + g_min_output_cal;
  }

  void adjust_for_gamma_correction(RGB &rgb) {
    rgb.r = exp_gamma_compress(rgb.r, r_gamma);
    rgb.g = exp_gamma_compress(rgb.g, g_gamma);
    rgb.b = exp_gamma_compress(rgb.b, b_gamma);
    
  }
};

struct CwWw {
  float cw;
  float ww;

  CwWw() : cw(0.0), ww(0.0){};
  CwWw(float cw, float ww) : cw(cw), ww(ww){};

  float max() { return std::max(this->cw, this->ww); }

  CwWw gamma_compress(float gamma) {
    return CwWw(exp_gamma_compress(this->cw, gamma), exp_gamma_compress(this->ww, gamma));
  }

  CwWw gamma_decompress(float gamma) {
    return CwWw(exp_gamma_decompress(this->cw, gamma), exp_gamma_decompress(this->ww, gamma));
  }

  CwWw clamp_truncate() {
    return CwWw(
      clamp_output_value(this->cw), 
      clamp_output_value(this->ww)
    );
  }
};

struct CwWwIntensityCalibration {
  float max_cw = 1.0f;
  float max_ww = 1.0f;
  float max_combined = 1.0f;

  float min_cw = 0.0f; 
  float min_ww = 0.0f; 
  float min_combined = 0.0f;

  CwWw apply_calibration(CwWw in) {

    const float epsilon = 1.401298E-45f;
    auto cw = in.cw;
    auto ww = in.ww;

    auto max = in.max();
    if (max > 1.0f) {
      cw /= max;
      ww /= max;
    }

    auto total = in.cw + in.ww;
    auto cw_lv = in.cw / total;
    auto ww_lv = in.ww / total;

    auto max_cw_lv = 1.0f;
    auto max_ww_lv = 1.0f;

    auto min_cw_lv = 1.0f;
    auto min_ww_lv = 1.0f;

    if (cw_lv <= epsilon) {
      // Zero means zero
      max_cw_lv = 0.0f;
      min_cw_lv = 0.0f;
    } else if (cw_lv <= 0.5f) {
      auto ratio = cw_lv * 2.0f;
      max_cw_lv = (max_cw * (1.0f - ratio)) + (max_combined * ratio);
      min_cw_lv = (min_cw * (1.0f - ratio)) + (min_combined * ratio);
    } else {
      auto ratio =  (cw_lv - 0.5f) * 2.0f;
      max_cw_lv = (max_cw * ratio) + (max_combined * (1.0f -  ratio));
      min_cw_lv = (min_cw * ratio) + (min_combined * (1.0f -  ratio));
    }
    
    if (ww_lv <= epsilon) {
      // Zero means zero
      min_ww_lv = 0.0f;
      min_ww_lv = 0.0f;
    } else if (ww_lv <= 0.5f) {
      auto ratio = ww_lv * 2.0f;
      max_ww_lv = (max_ww * (1.0f - ratio)) + (max_combined * ratio);
      min_ww_lv = (min_ww * (1.0f - ratio)) + (min_combined * ratio);
    } else {
      auto ratio =  (ww_lv - 0.5f) * 2.0f;
      max_ww_lv = (max_ww * ratio) + (max_combined * (1.0f -  ratio));
      min_ww_lv = (min_ww * ratio) + (min_combined * (1.0f -  ratio));
    }
    auto cw_adj = (cw * (1.0f - min_cw_lv) * max_cw_lv) + min_cw_lv;
    auto ww_adj = (ww * (1.0f - min_ww_lv) * max_ww_lv) + min_ww_lv;

    return CwWw(cw_adj, ww_adj);
  }
};

struct XYZ_Cie1931 {
  float X;
  float Y;
  float Z;

  XYZ_Cie1931() : X(0.0), Y(0.0), Z(0.0){};
  XYZ_Cie1931(float X, float Y, float Z) : X(X), Y(Y), Z(Z){};

  xyY_Cie1931 as_xyY_cie1931();
  Xy_Cie1931 as_xy_cie1931();
};

struct xyY_Cie1931 {
  float x;
  float y;
  float Y;

  xyY_Cie1931() : x(0.0), y(0.0), Y(0.0){};
  xyY_Cie1931(float x, float y, float Y) : x(x), y(y), Y(Y){};

  float cct_kelvin_approx();
  float cct_mired_approx();

  XYZ_Cie1931 as_XYZ_cie1931();
  Xy_Cie1931 as_xy_cie1931();
};

struct Xy_Cie1931 {
  float x;
  float y;

  Xy_Cie1931() : x(0.0), y(0.0){};
  Xy_Cie1931(float x, float y) : x(x), y(y){};

  Uv_Cie1960 as_uv_cie1960();
  Uv_Cie1976 as_uv_cie1976();
  xyY_Cie1931 as_xyY_cie1931(float Y);
  XYZ_Cie1931 as_XYZ_cie1931(float Y);

  float cct_kelvin_approx();
  float cct_mired_approx();
};

struct Uv_Cie1960 {
  float u;
  float v;

  Uv_Cie1960() : u(0.0), v(0.0){};
  Uv_Cie1960(float u, float v) : u(u), v(v){};

  Xy_Cie1931 as_xy_cie1931();
  Uv_Cie1976 as_uv_cie1976();

  float cct_kelvin_approx();
  float cct_mired_approx();

  float duv_approx();
  float tint_impurity(float green_tint_duv_impurity, float purple_tint_duv_impurity);

 private:
  bool has_duv();
  std::vector<Cct> quadrants();
  const std::vector<Uv_Cie1960> *duv_quadrants();
};

struct Uv_Cie1976 {
  float u;
  float v;

  Uv_Cie1976() : u(0.0), v(0.0){};
  Uv_Cie1976(float u, float v) : u(u), v(v){};

  Xy_Cie1931 as_xy_cie1931();
  Uv_Cie1960 as_uv_cie1960();
};

class Cct {
  float sin_t;
  float cos_t;
  bool calc_theta = true;

 public:
  Uv_Cie1960 uv;

  float kelvin;

  Cct(){};
  Cct(float kelvin, Uv_Cie1960 xy) : uv(xy), kelvin(kelvin){};

  static Cct from_mireds(float m);

  static Cct from_kelvin(float k);

  Uv_Cie1960 delta_uv_cie1960(float delta_uv);
};

class Cie2dColorSpace {
  enum { CIE_1931, CIE_1976 };

  optional<Xy_Cie1931> xy;
  optional<Uv_Cie1976> uvt;

 public:
  static Cie2dColorSpace Illuminant_a() {
    return Cie2dColorSpace(Xy_Cie1931(0.44757f, 0.40745f));  // Incandescent, tungsten
  }

  static Cie2dColorSpace Illuminant_d50() {
    return Cie2dColorSpace(Xy_Cie1931(0.34567f, 0.35850f));  // Daylight, Horizon
  }

  static Cie2dColorSpace Illuminant_d55() {
    return Cie2dColorSpace(Xy_Cie1931(0.33242f, 0.34743f));  // Mid-Morning, Mid-Afternoon
  }

  static Cie2dColorSpace Illuminant_d65() {
    return Cie2dColorSpace(Xy_Cie1931(0.31271f, 0.32902f));  // Daylight, Noon, Overcast (sRGB reference illuminant)
  }

  static Cie2dColorSpace Illuminant_e() {
    return Cie2dColorSpace(Xy_Cie1931(1 / 3.0f, 1 / 3.0f));  // Reference
  }

  Cie2dColorSpace(const Xy_Cie1931 &xy) : xy(xy){};

  Cie2dColorSpace(const Uv_Cie1976 &uvt) : uvt(uvt){};

  Xy_Cie1931 as_xy_cie1931() {
    if (this->xy.has_value())
      return (*this->xy);
    this->xy = (*this->uvt).as_xy_cie1931();
    return (*this->xy);
  }

  Uv_Cie1976 as_uv_cie1976() {
    if (this->uvt.has_value())
      return (*this->uvt);
    this->uvt = (*this->xy).as_uv_cie1976();
    return (*this->uvt);
  }
};

class ColorTemperature {
  float _mired;

  ColorTemperature(float m) : _mired(m) {}

 public:
  static ColorTemperature from_kelvin(float k) { return ColorTemperature(1e6 / k); }

  static ColorTemperature from_mired(float mired) { return ColorTemperature(mired); }

  float as_kelvin() { return 1e6 / this->_mired; }

  float as_mired() { return this->_mired; }

  ColorTemperature add_mired(float mired) { return ColorTemperature(this->_mired + mired); }

  ColorTemperature sub_mired(float mired) { return ColorTemperature(this->_mired - mired); }

  ColorTemperature add_kelvin(float k) { return ColorTemperature::from_kelvin(this->as_kelvin() + k); }

  ColorTemperature sub_kelvin(float k) { return ColorTemperature::from_kelvin(this->as_kelvin() - k); }
};

}  // namespace color_space
}  // namespace xy_light
}  // namespace esphome