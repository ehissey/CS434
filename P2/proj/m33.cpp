#include "m33.h"
#include <math.h>

V3& M33::operator[](int i) {

  return r[i];

}

M33::M33() {

  r[0] = V3(1.0f, 0.0f, 0.0f);
  r[1] = V3(0.0f, 1.0f, 0.0f);
  r[2] = V3(0.0f, 0.0f, 1.0f);

}

V3 M33::operator*(V3 v) {

  V3 ret(r[0]*v, r[1]*v, r[2]*v);
  return ret;

}

M33 M33::operator*(M33 m1) {

  M33 ret;
  for (int i = 0; i < 3; i++) {
    V3 col = m1.GetColumn(i);
    ret.SetColumn(i, (*this)*col);
  }
  return ret;

}

V3 M33::GetColumn(int i) {

  V3 ret(r[0][i], r[1][i], r[2][i]);
  return ret;

}

void M33::SetColumn(int i, V3 v) {

  r[0][i] = v[0];
  r[1][i] = v[1];
  r[2][i] = v[2];

}

ostream& operator<<(ostream &os, M33 m) {

  return os << m[0] << endl << m[1] << endl << m[2] << endl;

}

M33 M33::Invert() {

  M33 ret;
  V3 a = GetColumn(0), b = GetColumn(1), c = GetColumn(2);
  V3 _a = b ^ c; _a = _a / (a * _a);
  V3 _b = c ^ a; _b = _b / (b * _b);
  V3 _c = a ^ b; _c = _c / (c * _c);
  ret[0] = _a;
  ret[1] = _b;
  ret[2] = _c;

  return ret;

}

void M33::SetRotationY(float thetad) {

  float thetar = thetad*3.1415f/180.0f;
  (*this)[0] = V3(cosf(thetar), 0.0f, sinf(thetar));
  (*this)[1] = V3(0.0f, 1.0f, 0.0f);
  (*this)[2] = V3(-sinf(thetar), 0.0f, cosf(thetar));

}

void M33::Transpose() {

  M33 tmp(*this);
  r[0] = tmp.GetColumn(0);
  r[1] = tmp.GetColumn(1);
  r[2] = tmp.GetColumn(2);

}
