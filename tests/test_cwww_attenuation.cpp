// Run:
// c++ -std=c++17 -Wall -Wextra -pedantic tests/test_cwww_attenuation.cpp -o /tmp/test_cwww_attenuation && /tmp/test_cwww_attenuation
#include <cassert>
#include <cmath>

#include "../components/xy_light/cwww_attenuation.h"

using esphome::xy_light::CwWwAttenuationPolicy;
using esphome::xy_light::apply_cwww_impurity_attenuation;

int main() {
  // Both policies preserve the safety endpoints.
  assert(apply_cwww_impurity_attenuation(0.0f, 3.0f,
      CwWwAttenuationPolicy::COLOR_FIDELITY) == 0.0f);
  assert(apply_cwww_impurity_attenuation(0.0f, 3.0f,
      CwWwAttenuationPolicy::EFFECT_VISIBILITY) == 0.0f);
  assert(apply_cwww_impurity_attenuation(1.0f, 3.0f,
      CwWwAttenuationPolicy::COLOR_FIDELITY) == 1.0f);
  assert(apply_cwww_impurity_attenuation(1.0f, 3.0f,
      CwWwAttenuationPolicy::EFFECT_VISIBILITY) == 1.0f);

  // Existing fidelity behaviour is unchanged.
  const float fidelity = apply_cwww_impurity_attenuation(
      0.5f, 3.0f, CwWwAttenuationPolicy::COLOR_FIDELITY);
  assert(std::fabs(fidelity - 0.125f) < 1e-6f);

  // Effect mode retains substantially more luminance without bypassing
  // impurity attenuation entirely.
  const float effect = apply_cwww_impurity_attenuation(
      0.5f, 3.0f, CwWwAttenuationPolicy::EFFECT_VISIBILITY);
  assert(effect > fidelity);
  assert(effect < 1.0f);

  // Near the invalid boundary it still approaches zero continuously.
  const float near_zero = apply_cwww_impurity_attenuation(
      0.000001f, 3.0f, CwWwAttenuationPolicy::EFFECT_VISIBILITY);
  assert(near_zero > 0.0f);
  assert(near_zero < 0.02f);

  // Inputs outside the normalized impurity range are bounded.
  assert(apply_cwww_impurity_attenuation(-1.0f, 3.0f,
      CwWwAttenuationPolicy::EFFECT_VISIBILITY) == 0.0f);
  assert(apply_cwww_impurity_attenuation(2.0f, 3.0f,
      CwWwAttenuationPolicy::EFFECT_VISIBILITY) == 1.0f);
}
