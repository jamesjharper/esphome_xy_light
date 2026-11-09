#pragma once

#include <cmath>

namespace esphome {
namespace xy_light {
namespace color_space {
namespace detail {

// Ohno's Duv approximation needs the unsigned angle from the reference
// (u=0.292, v=0.240). atan2(|dv|, du) is equivalent to acos(du / radius)
// for a nonzero radius, without division by zero or acos domain drift.
inline float duv_approx_ohno(float u, float v) {
  if (!std::isfinite(u) || !std::isfinite(v))
    return 0.0f;  // Undefined chromaticity: use a finite, neutral fallback.

  // Subtract float reference constants before widening so a float input
  // exactly at the reference point produces an exact zero radius.
  const double du = static_cast<double>(u - 0.292f);
  const double dv = static_cast<double>(v - 0.240f);
  const double radius = std::sqrt(du * du + dv * dv);
  if (radius == 0.0)
    return 0.0f;  // Angle is undefined exactly at the reference point.

  const double a = std::atan2(std::abs(dv), du);
  const double a2 = a * a;
  const double a3 = a * a2;
  const double a4 = a * a3;
  const double a5 = a * a4;
  const double a6 = a * a5;

  const double lbb = (-0.00616793 * a6) + (0.0893944 * a5)
                   - (0.5179722 * a4) + (1.5317403 * a3)
                   - (2.4243787 * a2) + (1.925865 * a)
                   - 0.471106;
  return static_cast<float>(radius - lbb);
}

}  // namespace detail
}  // namespace color_space
}  // namespace xy_light
}  // namespace esphome
