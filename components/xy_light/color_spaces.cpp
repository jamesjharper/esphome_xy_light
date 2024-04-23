#include <cmath>
#include <vector>

#include "esphome/core/log.h"
#include "esphome/components/xy_light/color_spaces.h"

using namespace esphome::xy_light::color_space;

HSL RGB::as_hsl() {
  float max = std::max(std::max(this->r, this->g), this->b);
  float min = std::min(std::min(this->r, this->g), this->b);

  float h = (max + min) / 2;
  float s = h;
  float l = h;

  if (max == min) {
    h = s = 0;  // achromatic
  } else {
    float d = max - min;
    s = (l > 0.5) ? d / (2 - max - min) : d / (max + min);

    if (max == this->r) {
      h = (this->g - this->b) / d + (this->g < this->b ? 6 : 0);
    } else if (max == this->g) {
      h = (this->b - this->r) / d + 2;
    } else if (max == b) {
      h = (this->r - g) / d + 4;
    }

    h /= 6;
  }

  return HSL(h, s, l);
}

RGB RGB::adjust_brightness_saturation(float l, float s) {
  auto hsl = this->as_hsl();
  hsl.l *= l;
  hsl.s *= s;
  return hsl.as_rgb();
}

RGB HSL::as_rgb() {
  float r = this->l;
  float g = this->l;
  float b = this->l;

  if (0 != s) {
    float q = this->l < 0.5 ? this->l * (1 + this->s) : this->l + this->s - this->l * this->s;
    float p = 2 * this->l - q;
    r = hue2rgb(p, q, h + 1. / 3);
    g = hue2rgb(p, q, h);
    b = hue2rgb(p, q, h - 1. / 3);
  }

  return RGB(r, g, b);
}

float HSL::hue2rgb(float p, float q, float t) {
  if (t < 0)
    t += 1;
  if (t > 1)
    t -= 1;
  if (t < 1. / 6)
    return p + (q - p) * 6 * t;
  if (t < 1. / 2)
    return q;
  if (t < 2. / 3)
    return p + (q - p) * (2. / 3 - t) * 6;

  return p;
}

xyY_Cie1931 XYZ_Cie1931::as_xyY_cie1931() {
  float inv_sum = this->X + this->Y + this->Z;
  inv_sum = inv_sum > 1e-8f ? 1.0f / inv_sum : 0.0f;
  return xyY_Cie1931(this->X * inv_sum, this->Y * inv_sum, this->Y);
}

float xyY_Cie1931::cct_kelvin_approx() { return Xy_Cie1931(this->x, this->y).cct_kelvin_approx(); }

float xyY_Cie1931::cct_mired_approx() { return Xy_Cie1931(this->x, this->y).cct_mired_approx(); }

XYZ_Cie1931 xyY_Cie1931::as_XYZ_cie1931() {
  float Y_y = this->y > 1e-8f ? this->Y / this->y : 0.0f;
  return XYZ_Cie1931(this->x * Y_y, this->Y, (1.0f - this->x - this->y) * Y_y);
}

xyY_Cie1931 Xy_Cie1931::as_xyY_cie1931(float Y) { return xyY_Cie1931(this->x, this->y, Y); }

XYZ_Cie1931 Xy_Cie1931::as_XYZ_cie1931(float Y) { return xyY_Cie1931(this->x, this->y, Y).as_XYZ_cie1931(); }

float Xy_Cie1931::cct_kelvin_approx() {
  // Note: the McCamy approximation method is unstable under 1800k
  double n = (this->x - 0.3320) / (this->y - 0.1858);
  double n2 = n * n;
  double n3 = n * n2;

  // McCamy
  // https://www.researchgate.net/publication/304006255_Accurate_method_for_computing_correlated_color_temperature
  return (-449.0 * n3) + (3525.0 * n2) + (-6823.3 * n) + 5520.33;
}

float Xy_Cie1931::cct_mired_approx() { return 1000000.0f / this->cct_kelvin_approx(); }

Uv_Cie1960 Xy_Cie1931::as_uv_cie1960() {
  auto denom = (-2.0f * this->x) + (12.0f * this->y) + 3.0f;
  return Uv_Cie1960((4.0f * this->x) / denom, (6.0f * this->y) / denom);
}

Uv_Cie1976 Xy_Cie1931::as_uv_cie1976() {
  auto denom = 3.0f * this->x - (12.0f * this->y) + 3.0f;
  return Uv_Cie1976((4.0f * this->x) / denom, (9.0f * this->y) / denom);
}

Xy_Cie1931 Uv_Cie1960::as_xy_cie1931() {
  auto denom = 2.0f * this->u - (8.0f * this->v) + 4.0f;
  return Xy_Cie1931((3.0f * this->u) / denom, (2.0f * this->v) / denom);
}

Uv_Cie1976 Uv_Cie1960::as_uv_cie1976() { return Uv_Cie1976(this->u, this->v * 1.5); }

float Uv_Cie1960::cct_kelvin_approx() { return this->as_xy_cie1931().cct_kelvin_approx(); }

float Uv_Cie1960::cct_mired_approx() { return this->as_xy_cie1931().cct_mired_approx(); }

float Uv_Cie1960::duv_approx() {
  // Yoshi Ohno 2011(?)
  // https://cormusa.org/wp-content/uploads/2018/04/CORM_2011_Calculation_of_CCT_and_Duv_and_Practical_Conversion_Formulae.pdf

  auto vd2 = (this->v - 0.24f) * (this->v - 0.24f);
  auto ud2 = (this->u - 0.292f) * (this->u - 0.292f);
  auto Lfp = sqrt(ud2 + vd2);

  auto a = acos((this->u - 0.292) / Lfp);

  auto a2 = a * a;
  auto a3 = a * a2;
  auto a4 = a * a3;
  auto a5 = a * a4;
  auto a6 = a * a5;

  auto k6 = -0.00616793f;
  auto k5 = 0.0893944f;
  auto k4 = -0.5179722f;
  auto k3 = 1.5317403f;
  auto k2 = -2.4243787f;
  auto k1 = 1.925865f;
  auto k0 = -0.471106f;

  auto Lbb = (k6 * a6) + (k5 * a5) + (k4 * a4) + (k3 * a3) + (k2 * a2) + (k1 * a) + k0;
  return Lfp - Lbb;
}

float Uv_Cie1960::tint_impurity(float green_tint_duv_impurity, float purple_tint_duv_impurity) {
  // Filter for values known not to have a real VU value as the
  // the Delta UV calculation will result in erroneous small delta uv values
  if (!this->has_duv()) {
    return 0.0f;
  }

  auto duv = this->duv_approx();
  return duv > 0 ? clamp((green_tint_duv_impurity - duv) / green_tint_duv_impurity, 0.0f, 1.0f)
                 : clamp((purple_tint_duv_impurity + duv) / purple_tint_duv_impurity, 0.0f, 1.0f);
}

bool Uv_Cie1960::has_duv() {
  // XY values which lay outside beyond the intersection of iso-temperatures line (for example Green hues)
  // do not have a undefined delta UV, and can result in incorrect delta UV, values when approximated.
  // We can filter for these values using by testing for values which lay within a known polygon of valid values.
  // This method is fairly crude and could probably be improved, but it works.
  auto &vert = *this->duv_quadrants();

  int nvert = vert.size();

  // take the last item for first iteration to complete the "loop"
  int j = nvert - 1;

  bool c = false;
  auto u = this->u;
  auto v = this->v;

  for (int i = 0; i < nvert; j = i++) {
    if (((vert[i].v >= v) != (vert[j].v >= v)) &&
        (u <= (vert[j].u - vert[i].u) * (v - vert[i].v) / (vert[j].v - vert[i].v) + vert[i].u)) {
      c = !c;
    }
  }

  return c;
}

std::vector<Cct> Uv_Cie1960::quadrants() {
  return {
      Cct::from_kelvin(1000),  Cct::from_kelvin(1600), Cct::from_kelvin(2000),  Cct::from_kelvin(2500),
      Cct::from_kelvin(3000),  Cct::from_kelvin(4000), Cct::from_kelvin(5000),  Cct::from_kelvin(6000),
      Cct::from_kelvin(7000),  Cct::from_kelvin(8000), Cct::from_kelvin(10000), Cct::from_kelvin(15000),
      Cct::from_kelvin(20000),
  };
}

const std::vector<Uv_Cie1960> *Uv_Cie1960::duv_quadrants() {
  static std::vector<Uv_Cie1960> duv_area;
  if (!duv_area.empty())
    return &duv_area;

  float duv_area_limit = 0.09f;
  std::vector<Uv_Cie1960> green_duv;
  std::vector<Uv_Cie1960> purple_duv;

  auto bins = this->quadrants();
  for (auto cct : bins) {
    auto max_green_tint = cct.delta_uv_cie1960(duv_area_limit);
    auto max_purple_tint = cct.delta_uv_cie1960(-duv_area_limit);

    green_duv.push_back(max_green_tint);
    purple_duv.push_back(max_purple_tint);
  }
  duv_area = green_duv;
  duv_area.insert(duv_area.end(), purple_duv.rbegin(), purple_duv.rend());
  return &duv_area;
}

Uv_Cie1960 Cct::delta_uv_cie1960(float delta_uv) {
  // https://pdfslide.net/documents/practical-use-and-calculation-of-cct-and-duv.html?page=5
  if (this->calc_theta) {
    auto t0_uv = this->uv;
    auto t1_uv = Cct::from_kelvin(this->kelvin + 0.1f).uv;

    auto du = (double) (t0_uv.u - t1_uv.u);
    auto dv = (double) (t0_uv.v - t1_uv.v);

    auto du2 = du * du;
    auto dv2 = dv * dv;
    auto l = sqrt(du2 + dv2);

    this->cos_t = du / l;
    this->sin_t = dv / l;
    this->calc_theta = false;
  }

  return Uv_Cie1960(this->uv.u - (delta_uv * this->sin_t), this->uv.v + (delta_uv * this->cos_t));
}

Uv_Cie1960 Uv_Cie1976::as_uv_cie1960() { return Uv_Cie1960(this->u, this->v / 1.5); }

Xy_Cie1931 Uv_Cie1976::as_xy_cie1931() {
  auto denom = (-6.0f * this->u) + (16.0f * this->v) + 12.0f;
  return Xy_Cie1931((9.0f * this->u) / denom, (4.0f * this->v) / denom);
}

Cct Cct::from_mireds(float m) { return Cct::from_kelvin(1000000.0f / m); }

Cct Cct::from_kelvin(float k) {
  // Krystek, M. (1985). An algorithm to calculate correlated colour
  //        temperature. Color Research & Application, 10(1), 38–40.
  //        doi:10.1002/col.5080100109
  double t = k;
  double t2 = t * t;
  auto u = (0.860117757 + (0.000154118254 * t) + (0.000000128641212 * t2)) /
           (1.0 + (0.000842420235 * t) + (0.000000708145163 * t2));

  auto v = (0.317398726 + (0.0000422806245 * t) + (0.0000000420481691 * t2)) /
           (1.0 - (0.0000289741816 * t) + (0.000000161456053 * t2));

  return Cct(k, Uv_Cie1960(float(u), float(v)));
}
