#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace esphome {
namespace xy_light {

enum class CwWwAttenuationPolicy : uint8_t {
  // Existing behaviour: strongly suppress CWWW output as the requested
  // chromaticity moves away from a reproducible white.
  COLOR_FIDELITY,

  // Effect fallback behaviour: keep the same impurity/validity signal, but
  // use a gentler response curve so motion/brightness remains visible on
  // CWWW-only hardware. An impurity of zero still maps to zero.
  EFFECT_VISIBILITY,
};

inline float apply_cwww_impurity_attenuation(float impurity, float decay_gamma,
                                             CwWwAttenuationPolicy policy) {
  impurity = std::min(std::max(impurity, 0.0f), 1.0f);
  if (impurity == 0.0f)
    return 0.0f;

  const float gamma = std::max(decay_gamma, 1.0e-6f);
  switch (policy) {
    case CwWwAttenuationPolicy::EFFECT_VISIBILITY:
      // Preserve the same endpoints as COLOR_FIDELITY while bending the
      // curve in the opposite direction. With the current default gamma=3:
      //   fidelity: impurity^3
      //   effects:  impurity^(1/3)
      // This keeps invalid/fully impure colours dark, but avoids making
      // otherwise valid saturated colours disappear purely because CWWW
      // cannot reproduce their chroma.
      return std::pow(impurity, 1.0f / std::max(gamma, 1.0f));

    case CwWwAttenuationPolicy::COLOR_FIDELITY:
    default:
      return std::pow(impurity, gamma);
  }
}

}  // namespace xy_light
}  // namespace esphome
