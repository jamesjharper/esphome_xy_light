#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace esphome {
namespace xy_light {

enum class CwWwAttenuationFunction : uint8_t {
  // Legacy behaviour: strongly suppress CWWW output as chromaticity moves
  // away from a reproducible white.
  GAMMA_DECAY,

  // Gentler default: retain more luminance close to the Planckian locus while
  // preserving the same 0 -> 0 and 1 -> 1 endpoints.
  INVERSE_GAMMA,

  // No curve shaping; use the normalized impurity value directly.
  LINEAR,
};

inline float apply_cwww_impurity_attenuation(
    float impurity, float gamma, CwWwAttenuationFunction function) {
  impurity = std::clamp(impurity, 0.0f, 1.0f);
  if (impurity == 0.0f)
    return 0.0f;

  switch (function) {
    case CwWwAttenuationFunction::LINEAR:
      return impurity;

    case CwWwAttenuationFunction::GAMMA_DECAY:
      return std::pow(impurity, std::max(gamma, 1.0e-6f));

    case CwWwAttenuationFunction::INVERSE_GAMMA:
    default:
      return std::pow(impurity, 1.0f / std::max(gamma, 1.0f));
  }
}

}  // namespace xy_light
}  // namespace esphome
