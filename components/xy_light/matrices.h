#pragma once
#include <math.h>

namespace esphome {
namespace xy_light {
namespace matrices {

class Vec3 {
 public:
  float x, y, z;

  Vec3() : x(0), y(0), z(0) {}
  Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
  float dot(const Vec3 &b) const { return x * b.x + y * b.y + z * b.z; }
  float dot(float x, float y, float z) const { return this->x * x + this->y * y + this->z * z; }

  Vec3 operator*(const Vec3 &v) const { return Vec3(x * v.x, y * v.y, z * v.z); }
};

static Vec3 operator*(float a, const Vec3 &b) { return Vec3(a * b.x, a * b.y, a * b.z); }

class Matrix3x3 {
 public:
  float m[3][3];

  Matrix3x3() {}

  Matrix3x3(Vec3 r1, Vec3 r2, Vec3 r3) : m{{r1.x, r1.y, r1.z}, {r2.x, r2.y, r2.z}, {r3.x, r3.y, r3.z}} {}

  Matrix3x3(float r1c1, float r1c2, float r1c3, float r2c1, float r2c2, float r2c3, float r3c1, float r3c2, float r3c3)
      : m{{r1c1, r1c2, r1c3}, {r2c1, r2c2, r2c3}, {r3c1, r3c2, r3c3}} {}

  Matrix3x3 &invert() {
    *this = inverse();
    return *this;
  }
  Matrix3x3 inverse() const;
  double determinant() const;

  float &operator()(int row, int column);
  Matrix3x3 operator*(const Matrix3x3 &b) const;
};

Vec3 operator*(const Vec3 &a, const Matrix3x3 &b);
Vec3 operator*(const Matrix3x3 &a, const Vec3 &b);

};  // namespace matrices
};  // namespace xy_light
};  // namespace esphome