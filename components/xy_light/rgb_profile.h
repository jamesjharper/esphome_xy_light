#pragma once
#include "esphome/core/optional.h"
#include "esphome/core/component.h"

#include "esphome/components/xy_light/color_spaces.h"
#include "esphome/components/xy_light/matrices.h"

namespace esphome {
namespace xy_light {

class RgbChromaTransform {
  // Great learning resources can be found here
  // https://patapom.com/blog/Colorimetry/ColorTransforms/

  // Original inspiration for implementation
  // https://gist.github.com/dbr/24cfd1033c2d59f263e3#file-rgb_to_xyz_matrix-py-L181
  color_space::Cie2dColorSpace _r, _g, _b, _w;
  float _r_output_cal, _g_output_cal, _b_output_cal, _gamma;
  optional<matrices::Vec3> _wp_scale, _wp_scale_inv;
  optional<matrices::Matrix3x3> _XYZ2RGB_d, _XYZ2RGB_inv_d;
  optional<matrices::Matrix3x3> _XYZ2RGB, _RGB2XYZ;

 public:
  RgbChromaTransform()
      : _r(color_space::Xy_Cie1931(0.7347f, 0.2653f)),
        _g(color_space::Xy_Cie1931(0.0000f, 1.0000f)),
        _b(color_space::Xy_Cie1931(0.0001f, -0.0770f)),
        _w(color_space::Xy_Cie1931(0.32168f, 0.33767f)),
        _r_output_cal(1.0f),
        _g_output_cal(1.0f),
        _b_output_cal(1.0f),
        _gamma(1.0f) {}

  void set_typical_led() {
    this->_r = color_space::Xy_Cie1931(0.7f, 0.3f);
    this->_g = color_space::Xy_Cie1931(0.3f, 0.6f);
    this->_b = color_space::Xy_Cie1931(0.15f, 0.06f);
    this->_w = color_space::Cie2dColorSpace::Illuminant_d65();
    this->_gamma = 1.0f;
  }

  void set_sRGB() {
    this->_r = color_space::Xy_Cie1931(0.6400f, 0.3300f);
    this->_g = color_space::Xy_Cie1931(0.3000f, 0.6000f);
    this->_b = color_space::Xy_Cie1931(0.1500f, 0.0600f);
    this->_w = color_space::Cie2dColorSpace::Illuminant_d65();
    this->_gamma = 2.2f;
  }

  void set_AdobeRGB_D55() {
    this->_r = color_space::Xy_Cie1931(0.6400f, 0.3300f);
    this->_g = color_space::Xy_Cie1931(0.2100f, 0.7100f);
    this->_b = color_space::Xy_Cie1931(0.1500f, 0.0600f);
    this->_w = color_space::Cie2dColorSpace::Illuminant_d55();
    this->_gamma = 2.2f;
  }

  void set_AdobeRGB_D65() {
    this->_r = color_space::Xy_Cie1931(0.6400f, 0.3300f);
    this->_g = color_space::Xy_Cie1931(0.2100f, 0.7100f);
    this->_b = color_space::Xy_Cie1931(0.1500f, 0.0600f);
    this->_w = color_space::Cie2dColorSpace::Illuminant_d65();
    this->_gamma = 2.2f;
  }

  void set_ProPhoto() {
    this->_r = color_space::Xy_Cie1931(0.7347f, 0.2653f);
    this->_g = color_space::Xy_Cie1931(0.1596f, 0.8404f);
    this->_b = color_space::Xy_Cie1931(0.0366f, 0.0001f);
    this->_w = color_space::Cie2dColorSpace::Illuminant_d50();
    this->_gamma = 1.8f;
  }

  void set_ACES_AP0() {
    this->_r = color_space::Xy_Cie1931(0.7347f, 0.2653f);
    this->_g = color_space::Xy_Cie1931(0.0000f, 1.0000f);
    this->_b = color_space::Xy_Cie1931(0.0001f, -0.0770f);  // Not a typo
    this->_w = color_space::Xy_Cie1931(0.32168f, 0.33767f);
    this->_gamma = 1.0f;
  }

  void set_gamma(float g) { this->_gamma = g; }

  void set_red(color_space::Cie2dColorSpace r) {
    this->_r = r;
    this->reset_chroma();
  }

  void set_green(color_space::Cie2dColorSpace g) {
    this->_g = g;
    this->reset_chroma();
  }

  void set_blue(color_space::Cie2dColorSpace b) {
    this->_b = b;
    this->reset_chroma();
  }

  void set_white_point(color_space::Cie2dColorSpace w) {
    this->_w = w;
    this->reset_scale_vec();
  }

  void set_weighted_red_intensity(float i) { this->_r_output_cal = i; }

  void set_weighted_green_intensity(float i) { this->_g_output_cal = i; }

  void set_weighted_blue_intensity(float i) { this->_b_output_cal = i; }

  color_space::XYZ_Cie1931 RGB_to_XYZ(color_space::RGB rgb) {
    auto rgb_decomp = rgb.gamma_decompress(this->_gamma);
    auto XYZ = RGB_2_Cie1931XYZ_transform_matrix() * matrices::Vec3(rgb_decomp.r, rgb_decomp.g, rgb_decomp.b);
    return color_space::XYZ_Cie1931(XYZ.x, XYZ.y, XYZ.z);
  }

  color_space::RGB XYZ_to_RGB(color_space::XYZ_Cie1931 XYZ) {
    auto rgb_xyz = Cie1931XYZ_2_rgb_transform_matrix() * matrices::Vec3(XYZ.X, XYZ.Y, XYZ.Z);
    auto rgb = color_space::RGB(rgb_xyz.x, rgb_xyz.y, rgb_xyz.z).gamma_compress(this->_gamma);

    this->adjust_for_weighted_outputs(rgb);
    this->adjust_for_colors_out_of_gamut(rgb);
    return rgb;
  }

  matrices::Matrix3x3 &RGB_2_Cie1931XYZ_transform_matrix() {
    if (!this->_RGB2XYZ.has_value()) {
      auto wp_scale = this->wp_scale_vector();
      auto XYZ2RGB_d = this->XYZ2RGB_d();
      this->_RGB2XYZ =
          matrices::Matrix3x3(XYZ2RGB_d(0, 0) * wp_scale.x, XYZ2RGB_d(1, 0) * wp_scale.y, XYZ2RGB_d(2, 0) * wp_scale.z,
                              XYZ2RGB_d(0, 1) * wp_scale.x, XYZ2RGB_d(1, 1) * wp_scale.y, XYZ2RGB_d(2, 1) * wp_scale.z,
                              XYZ2RGB_d(0, 2) * wp_scale.x, XYZ2RGB_d(1, 2) * wp_scale.y, XYZ2RGB_d(2, 2) * wp_scale.z);
    }
    return this->_RGB2XYZ.value();
  }

  matrices::Matrix3x3 &Cie1931XYZ_2_rgb_transform_matrix() {
    if (!this->_XYZ2RGB.has_value()) {
      auto wp_scale_inv = this->wp_scale_vector_inv();
      auto XYZ2RGB_inv_d = this->XYZ2RGB_inv_d();
      this->_XYZ2RGB = matrices::Matrix3x3(XYZ2RGB_inv_d(0, 0) * wp_scale_inv.x, XYZ2RGB_inv_d(1, 0) * wp_scale_inv.x,
                                           XYZ2RGB_inv_d(2, 0) * wp_scale_inv.x, XYZ2RGB_inv_d(0, 1) * wp_scale_inv.y,
                                           XYZ2RGB_inv_d(1, 1) * wp_scale_inv.y, XYZ2RGB_inv_d(2, 1) * wp_scale_inv.y,
                                           XYZ2RGB_inv_d(0, 2) * wp_scale_inv.z, XYZ2RGB_inv_d(1, 2) * wp_scale_inv.z,
                                           XYZ2RGB_inv_d(2, 2) * wp_scale_inv.z);
    }
    return this->_XYZ2RGB.value();
  }

 protected:
  void adjust_for_colors_out_of_gamut(color_space::RGB &rgb) {
    // Currently only support a basic clipping strategy
    auto max = rgb.max();
    if (max > 1.0f) {
      rgb.r /= max;
      rgb.g /= max;
      rgb.b /= max;
    }
  }

  void adjust_for_weighted_outputs(color_space::RGB &rgb) {
    rgb.r *= this->_r_output_cal;
    rgb.g *= this->_g_output_cal;
    rgb.b *= this->_b_output_cal;
  }

  void reset_chroma() {
    this->_XYZ2RGB_d.reset();
    this->_XYZ2RGB_inv_d.reset();
    this->reset_scale_vec();
  }

  void reset_transform_matrix() {
    this->_XYZ2RGB.reset();
    this->_RGB2XYZ.reset();
  }

  void reset_scale_vec() {
    this->_wp_scale.reset();
    this->_wp_scale_inv.reset();
    this->reset_transform_matrix();
  }

  matrices::Vec3 &wp_scale_vector() {
    if (!this->_wp_scale.has_value()) {
      auto w_XYZ = this->_w.as_xy_cie1931().as_XYZ_cie1931(1.0f);
      this->_wp_scale = matrices::Vec3(w_XYZ.X, w_XYZ.Y, w_XYZ.Z) * this->XYZ2RGB_inv_d();
    }
    return this->_wp_scale.value();
  }

  matrices::Vec3 &wp_scale_vector_inv() {
    if (!this->_wp_scale_inv.has_value()) {
      auto wp_scale = this->wp_scale_vector();
      this->_wp_scale_inv = matrices::Vec3(1.0f / wp_scale.x, 1.0f / wp_scale.y, 1.0f / wp_scale.z);
    }
    return this->_wp_scale_inv.value();
  }

  matrices::Matrix3x3 &XYZ2RGB_d() {
    if (!this->_XYZ2RGB_d.has_value()) {
      auto r = _r.as_xy_cie1931();
      auto g = _g.as_xy_cie1931();
      auto b = _b.as_xy_cie1931();

      // For each primary, get the "missing" z (where x+y+z = 1, so z = 1-x-y)
      this->_XYZ2RGB_d =
          matrices::Matrix3x3(r.x, r.y, 1.0f - r.x - r.y, g.x, g.y, 1.0f - g.x - g.y, b.x, b.y, 1.0f - b.x - b.y);
    }
    return this->_XYZ2RGB_d.value();
  }

  matrices::Matrix3x3 &XYZ2RGB_inv_d() {
    if (!this->_XYZ2RGB_inv_d.has_value()) {
      this->_XYZ2RGB_inv_d = this->XYZ2RGB_d().inverse();
    }
    return this->_XYZ2RGB_inv_d.value();
  }
};

class RgbProfile : public Component {
 protected:
  RgbChromaTransform _chroma_transform;

 public:
  void set_gamma(float g) { this->_chroma_transform.set_gamma(g); }

  void set_red_xy(float x, float y) { this->_chroma_transform.set_red(color_space::Xy_Cie1931(x, y)); }

  void set_green_xy(float x, float y) { this->_chroma_transform.set_green(color_space::Xy_Cie1931(x, y)); }

  void set_blue_xy(float x, float y) { this->_chroma_transform.set_blue(color_space::Xy_Cie1931(x, y)); }

  void set_white_point_xy(float x, float y) { this->_chroma_transform.set_white_point(color_space::Xy_Cie1931(x, y)); }

  void set_weighted_red_intensity(float i) { this->_chroma_transform.set_weighted_red_intensity(i); }

  void set_weighted_green_intensity(float i) { this->_chroma_transform.set_weighted_green_intensity(i); }

  void set_weighted_blue_intensity(float i) { this->_chroma_transform.set_weighted_blue_intensity(i); }

  void set_white_point_cct(float mireds) {
    this->_chroma_transform.set_white_point(color_space::Cct::from_mireds(mireds).uv.as_xy_cie1931());
  }

  void use_typical_led() { this->_chroma_transform.set_typical_led(); }

  void use_sRGB() { this->_chroma_transform.set_sRGB(); }

  void set_AdobeRGB_D55() { this->_chroma_transform.set_AdobeRGB_D55(); }

  void use_AdobeRGB_D65() { this->_chroma_transform.set_AdobeRGB_D65(); }

  void use_ProPhoto() { this->_chroma_transform.set_ProPhoto(); }

  void use_ACES_AP0() { this->_chroma_transform.set_ACES_AP0(); }

  RgbChromaTransform get_chroma_transform() { return this->_chroma_transform; }
};

}  // namespace xy_light
}  // namespace esphome