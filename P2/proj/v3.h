#pragma once

#include <iostream>
#include <ostream>

using namespace std;

// 3-D vector class

class V3 {
public:
  float xyz[3];
  V3() {};
  V3(float x, float y, float z);
  float& operator[](int i);
  float operator*(V3 v);
  V3 operator*(float scf);
  V3 operator/(float scf);
  V3 operator+(V3 v);
  V3 operator-(V3 v);
  friend ostream& operator<<(ostream &ostr, V3 v);
  friend istream& operator>>(istream &istr, V3 &v);
  void SetFromColor(unsigned int color);
  unsigned int GetColor();
  V3 operator^(V3 op1);
  V3 RotatePointAboutArbitraryAxis(V3 aO, V3 aD, float thetad);
  V3 RotateDirectionAboutArbitraryAxis(V3 aD,  float thetad);
  V3 Normalized();
  float Length();
  V3 Reflect(V3 r);
};

// axis aligned bounding box class
class AABB {
public:
  V3 corners[2];
  AABB(V3 P);
  void AddPoint(V3 P);
  bool ClipWithScreen(int w, int h);
};