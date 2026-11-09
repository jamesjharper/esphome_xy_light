// Run: c++ -std=c++17 -Wall -Wextra -pedantic tests/test_duv_approx.cpp -o /tmp/test_duv_approx && /tmp/test_duv_approx
#include <cassert>
#include <cmath>
#include <limits>

#include "../components/xy_light/duv_math.h"

using esphome::xy_light::color_space::detail::duv_approx_ohno;

int main() {
  // Undefined angle at the reference point must not produce NaN.
  assert(duv_approx_ohno(0.292f, 0.240f) == 0.0f);

  // Invalid inputs must not propagate NaNs/Infs to downstream light outputs.
  const float nan = std::numeric_limits<float>::quiet_NaN();
  const float inf = std::numeric_limits<float>::infinity();
  assert(std::isfinite(duv_approx_ohno(nan, 0.24f)));
  assert(std::isfinite(duv_approx_ohno(0.292f, inf)));

  // acos(du / radius) equals atan2(abs(dv), du) in each quadrant.
  for (float du : {-0.12f, -0.03f, 0.03f, 0.12f}) {
    for (float dv : {-0.08f, -0.02f, 0.02f, 0.08f}) {
      float u = 0.292f + du;
      float v = 0.240f + dv;
      double dx = static_cast<double>(u) - 0.292;
      double dy = static_cast<double>(v) - 0.240;
      double radius = std::sqrt(dx * dx + dy * dy);
      double a = std::acos(dx / radius);
      double a2 = a*a, a3 = a2*a, a4 = a3*a, a5 = a4*a, a6 = a5*a;
      double baseline = radius - ((-0.00616793 * a6) + (0.0893944 * a5)
          - (0.5179722 * a4) + (1.5317403 * a3) - (2.4243787 * a2)
          + (1.925865 * a) - 0.471106);
      assert(std::fabs(duv_approx_ohno(u, v) - baseline) < 1e-5);
    }
  }

  // Boundary axes and tiny, finite displacements should also remain finite.
  for (float u : {0.292f, 0.292001f, 0.291999f, 0.5f, 0.1f})
    for (float v : {0.240f, 0.240001f, 0.239999f})
      assert(std::isfinite(duv_approx_ohno(u, v)));
}
