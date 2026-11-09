// Run:
// c++ -std=c++17 -Wall -Wextra -pedantic tests/test_cwww_attenuation.cpp -o /tmp/test_cwww_attenuation && /tmp/test_cwww_attenuation
#include <cassert>
#include <cmath>

#include "../components/xy_light/cwww_attenuation.h"

using esphome::xy_light::CwWwAttenuationFunction;
using esphome::xy_light::apply_cwww_impurity_attenuation;

int main() {
  // All functions preserve the safety endpoints.
  for (auto function : {
           CwWwAttenuationFunction::GAMMA_DECAY,
           CwWwAttenuationFunction::INVERSE_GAMMA,
           CwWwAttenuationFunction::LINEAR,
       }) {
    assert(apply_cwww_impurity_attenuation(0.0f, 3.0f, function) == 0.0f);
    assert(apply_cwww_impurity_attenuation(1.0f, 3.0f, function) == 1.0f);
  }

  // Legacy gamma-decay behaviour remains available.
  const float decay = apply_cwww_impurity_attenuation(
      0.5f, 3.0f, CwWwAttenuationFunction::GAMMA_DECAY);
  assert(std::fabs(decay - 0.125f) < 1e-6f);

  // Inverse gamma retains substantially more luminance away from the locus.
  const float inverse = apply_cwww_impurity_attenuation(
      0.5f, 3.0f, CwWwAttenuationFunction::INVERSE_GAMMA);
  assert(std::fabs(inverse - std::pow(0.5f, 1.0f / 3.0f)) < 1e-6f);
  assert(inverse > 0.5f);

  // Linear leaves the normalized attenuation unchanged.
  const float linear = apply_cwww_impurity_attenuation(
      0.5f, 3.0f, CwWwAttenuationFunction::LINEAR);
  assert(std::fabs(linear - 0.5f) < 1e-6f);

  // Near the invalid boundary inverse gamma still approaches zero continuously.
  const float near_zero = apply_cwww_impurity_attenuation(
      0.000001f, 3.0f, CwWwAttenuationFunction::INVERSE_GAMMA);
  assert(near_zero > 0.0f);
  assert(near_zero < 0.02f);

  // Inputs outside the normalized impurity range are bounded.
  assert(apply_cwww_impurity_attenuation(
             -1.0f, 3.0f, CwWwAttenuationFunction::INVERSE_GAMMA) == 0.0f);
  assert(apply_cwww_impurity_attenuation(
             2.0f, 3.0f, CwWwAttenuationFunction::INVERSE_GAMMA) == 1.0f);

  // Gamma below 1 must not make inverse-gamma more aggressive than linear.
  const float inverse_low_gamma = apply_cwww_impurity_attenuation(
      0.25f, 0.5f, CwWwAttenuationFunction::INVERSE_GAMMA);
  assert(std::fabs(inverse_low_gamma - 0.25f) < 1e-6f);
}
