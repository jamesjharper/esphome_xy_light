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
  if (gamma <= 0.0f)
    return linear;

  return powf(linear, gamma);
}

static float exp_gamma_decompress(float value, float gamma) {
  if (value <= 0.0f)
    return 0.0f;
  if (gamma <= 0.0f)
    return value;

  return powf(value, 1.0f / gamma);
}

static float srgb_gamma_compress(float linear, float gamma) {
    if (linear <= 0.0031308) {
        return 12.92 * linear;
    } else {
        return 1.055 * pow(linear, 1.0f / gamma) - 0.055f;
    }
}
static float srgb_gamma_decompress(float sRGB, float gamma) {
    if (sRGB <= 0.04045) {
        return sRGB / 12.92f;
    } else {
        return pow((sRGB + 0.055f) / 1.055f, gamma);
    }
}

struct RGB {
  float r;
  float g;
  float b;

  RGB() : r(0.0), g(0.0), b(0.0){};
  RGB(float r, float g, float b) : r(r), g(g), b(b){};

  float max() { return std::max(std::max(this->r, this->g), this->b); }

  RGB clamp_truncate() {
    return RGB(clamp(this->r, 0.0f, 1.0f), clamp(this->g, 0.0f, 1.0f), clamp(this->b, 0.0f, 1.0f));
  }

  RGB clamp_normalize() {
    auto max = this->max();
    if (max > 1.0f) {
      return RGB(this->r / max, this->g / max, this->b / max);
    }
    return *this;
  }
};

struct CwWw {
  float cw;
  float ww;

  CwWw() : cw(0.0), ww(0.0){};
  CwWw(float cw, float ww) : cw(cw), ww(ww){};

  CwWw gamma_compress(float gamma) {
    return CwWw(exp_gamma_compress(this->cw, gamma), exp_gamma_compress(this->ww, gamma));
  }

  CwWw gamma_decompress(float gamma) {
    return CwWw(exp_gamma_decompress(this->cw, gamma), exp_gamma_decompress(this->ww, gamma));
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