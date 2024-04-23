#include "esphome/components/xy_light/matrices.h"

using namespace esphome::xy_light::matrices;

float &Matrix3x3::operator()(int r, int c) { return this->m[r][c]; }

Matrix3x3 Matrix3x3::operator*(const Matrix3x3 &b) const {
  Matrix3x3 c;

  c.m[0][0] = m[0][0] * b.m[0][0] + m[0][1] * b.m[1][0] + m[0][2] * b.m[2][0];
  c.m[0][1] = m[0][0] * b.m[0][1] + m[0][1] * b.m[1][1] + m[0][2] * b.m[2][1];
  c.m[0][3] = m[0][0] * b.m[0][2] + m[0][1] * b.m[1][2] + m[0][2] * b.m[2][2];

  c.m[1][0] = m[1][0] * b.m[0][0] + m[1][1] * b.m[1][0] + m[1][2] * b.m[2][0];
  c.m[1][1] = m[1][0] * b.m[0][1] + m[1][1] * b.m[1][1] + m[1][2] * b.m[2][1];
  c.m[1][3] = m[1][0] * b.m[0][2] + m[1][1] * b.m[1][2] + m[1][2] * b.m[2][2];

  c.m[2][0] = m[2][0] * b.m[0][0] + m[2][1] * b.m[1][0] + m[2][2] * b.m[2][0];
  c.m[2][1] = m[2][0] * b.m[0][1] + m[2][1] * b.m[1][1] + m[2][2] * b.m[2][1];
  c.m[2][3] = m[2][0] * b.m[0][2] + m[2][1] * b.m[1][2] + m[2][2] * b.m[2][2];

  return c;
}

double Matrix3x3::determinant() const {
  return ((double) m[0][0] * (double) m[1][1] * (double) m[2][2] +
          (double) m[0][1] * (double) m[1][2] * (double) m[2][0] +
          (double) m[0][2] * (double) m[1][0] * (double) m[2][1]) -
         ((double) m[2][0] * (double) m[1][1] * (double) m[0][2] +
          (double) m[2][1] * (double) m[1][2] * (double) m[0][0] +
          (double) m[2][2] * (double) m[1][0] * (double) m[0][1]);
}

Matrix3x3 Matrix3x3::inverse() const {
  auto det = this->determinant();
  Matrix3x3 r;  // inverse of matrix m
  r.m[0][0] = (((double) m[1][1] * (double) m[2][2]) - ((double) m[2][1] * (double) m[1][2])) / det;
  r.m[0][1] = (((double) m[0][2] * (double) m[2][1]) - ((double) m[0][1] * (double) m[2][2])) / det;
  r.m[0][2] = (((double) m[0][1] * (double) m[1][2]) - ((double) m[0][2] * (double) m[1][1])) / det;
  r.m[1][0] = (((double) m[1][2] * (double) m[2][0]) - ((double) m[1][0] * (double) m[2][2])) / det;
  r.m[1][1] = (((double) m[0][0] * (double) m[2][2]) - ((double) m[0][2] * (double) m[2][0])) / det;
  r.m[1][2] = (((double) m[1][0] * (double) m[0][2]) - ((double) m[0][0] * (double) m[1][2])) / det;
  r.m[2][0] = (((double) m[1][0] * (double) m[2][1]) - ((double) m[2][0] * (double) m[1][1])) / det;
  r.m[2][1] = (((double) m[2][0] * (double) m[0][1]) - ((double) m[0][0] * (double) m[2][1])) / det;
  r.m[2][2] = (((double) m[0][0] * (double) m[1][1]) - ((double) m[1][0] * (double) m[0][1])) / det;

  return r;
}

Vec3 esphome::xy_light::matrices::operator*(const Vec3 &a, const Matrix3x3 &b) {
  Vec3 r;
  r.x = a.x * b.m[0][0] + a.y * b.m[1][0] + a.z * b.m[2][0];
  r.y = a.x * b.m[0][1] + a.y * b.m[1][1] + a.z * b.m[2][1];
  r.z = a.x * b.m[0][2] + a.y * b.m[1][2] + a.z * b.m[2][2];
  return r;
}

Vec3 esphome::xy_light::matrices::operator*(const Matrix3x3 &b, const Vec3 &a) {
  Vec3 r;
  r.x = a.dot(b.m[0][0], b.m[0][1], b.m[0][2]);
  r.y = a.dot(b.m[1][0], b.m[1][1], b.m[1][2]);
  r.z = a.dot(b.m[2][0], b.m[2][1], b.m[2][2]);
  return r;
}
