#pragma once

#include "v3.h"

// 3x3 matrix class

class M33 {
public:
  V3 r[3];
  M33();
  V3& operator[](int i);
  V3 operator*(V3 v);
  M33 operator*(M33 m1);
  V3 GetColumn(int i);
  void SetColumn(int i, V3 v);
  friend ostream& operator<<(ostream &os, M33 m);
  M33 Invert();
  void SetRotationY(float thetad);
  void Transpose();
};