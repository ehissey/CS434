#include "v3.h"
#include "m33.h"
#include <math.h>


// access to elements
float& V3::operator[](int i) {

  return xyz[i];

}

V3::V3(float x, float y, float z) {

  xyz[0] = x;
  xyz[1] = y;
  xyz[2] = z;

}

float V3::operator*(V3 v) {

  return 
    v[0]*xyz[0] + 
    v[1]*xyz[1] + 
    v[2]*xyz[2];

}


V3 V3::operator+(V3 v) {

  V3 ret(xyz[0]+v[0], xyz[1]+v[1], xyz[2]+v[2]);
  return ret;

}

V3 V3::operator-(V3 v) {

  V3 ret(xyz[0]-v[0], xyz[1]-v[1], xyz[2]-v[2]);
  return ret;

}

ostream& operator<<(ostream &ostr, V3 v) {

  ostr << v[0] << " " << v[1] << " " << v[2] << " ";
  return ostr;

}

istream& operator>>(istream &istr, V3 &v) {

  istr >> v[0] >> v[1] >> v[2];
  return istr;

}


V3 V3::operator*(float scf) {

  V3 ret = *this;
  ret[0] *= scf;
  ret[1] *= scf;
  ret[2] *= scf;
  return ret;

}


V3 V3::operator/(float scf) {

  V3 ret = *this;
  ret[0] /= scf;
  ret[1] /= scf;
  ret[2] /= scf;
  return ret;

}

void V3::SetFromColor(unsigned int color) {

  unsigned char *rgb = (unsigned char*)(&color);
  xyz[0] = ((float)rgb[0])/255.0f;
  xyz[1] = ((float)rgb[1])/255.0f;
  xyz[2] = ((float)rgb[2])/255.0f;

}

unsigned int V3::GetColor() {

  unsigned int rgb[3];
  float _xyz[3];
  for (int i = 0; i < 3; i++) {
    if (xyz[i] < 0.0f)
      _xyz[i] = 0.0f;
    else if (xyz[i] > 1.0f)
      _xyz[i] = 1.0f;
    else
      _xyz[i] = xyz[i];
  }
  rgb[0] = (int) (_xyz[0]*255.0f);
  rgb[1] = (int) (_xyz[1]*255.0f);
  rgb[2] = (int) (_xyz[2]*255.0f);
  return 0xFF000000 + rgb[0] + rgb[1]*256 + rgb[2]*256*256;

}

// cross product
V3 V3::operator^(V3 op1) {

  V3 ret;
  ret.xyz[0] = xyz[1]*op1.xyz[2]-xyz[2]*op1.xyz[1];
  ret.xyz[1] = xyz[2]*op1.xyz[0]-xyz[0]*op1.xyz[2];
  ret.xyz[2] = xyz[0]*op1.xyz[1]-xyz[1]*op1.xyz[0];

  return ret;

}

// axis origin is aO
// axis direction is aD
// thetad degrees
V3 V3::RotatePointAboutArbitraryAxis(V3 aO, V3 aD, 
                                     float thetad) {

  V3 xaxis(1.0f, 0.0f, 0.0f);
  V3 yaxis(0.0f, 1.0f, 0.0f);

  float adx = fabsf(xaxis*aD);
  float ady = fabsf(yaxis*aD);
  V3 aux;
  if (adx < ady) {
    aux = xaxis;
  }
  else {
    aux = yaxis;
  }

  M33 lm;
  lm[0] = (aux^aD).Normalized();
  lm[1] = aD.Normalized();
  lm[2] = (lm[0] ^ lm[1]).Normalized();

  M33 ilm = lm.Invert();

  M33 rotY; rotY.SetRotationY(thetad);

  V3 pt(*this);
  V3 lpt = lm*(pt-aO);
  V3 rlpt = rotY*lpt;
  V3 ret = aO + ilm*rlpt;
  return ret;

}


// axis direction is aD
// thetad degrees
V3 V3::RotateDirectionAboutArbitraryAxis(V3 aD,  float thetad) {

  V3 xaxis(1.0f, 0.0f, 0.0f);
  V3 yaxis(0.0f, 1.0f, 0.0f);

  float adx = fabsf(xaxis*aD);
  float ady = fabsf(yaxis*aD);
  V3 aux;
  if (adx < ady) {
    aux = xaxis;
  }
  else {
    aux = yaxis;
  }

  M33 lm;
  lm[0] = (aux^aD).Normalized();
  lm[1] = aD.Normalized();
  lm[2] = (lm[0] ^ lm[1]).Normalized();

  M33 ilm = lm.Invert();

  M33 rotY; rotY.SetRotationY(thetad);

  V3 pt(*this);
  V3 lpt = lm*pt;
  V3 rlpt = rotY*lpt;
  V3 ret = ilm*rlpt;
  return ret;

}

V3 V3::Normalized() {

  V3 ret = *this;
  ret = ret / ret.Length();
  return ret;

}

float V3::Length() {

  float ret = sqrtf((*this)*(*this));
  return ret;

}



AABB::AABB(V3 P) {

  corners[0] = corners[1] = P;

}

void AABB::AddPoint(V3 P) {

  for (int i = 0; i < 3; i++) {
    if (P[i] < corners[0][i]) {
      corners[0][i] = P[i];
    }
    if (P[i] > corners[1][i]) {
      corners[1][i] = P[i];
    }
  }

}


bool AABB::ClipWithScreen(int w, int h) {

  if (
    corners[0][0] >= (float)w ||
    corners[1][0] <= 0.0f ||
    corners[0][1] >= (float)h ||
    corners[1][1] <= 0.0f)
    return false;

  if (corners[0][0] < 0.0f)
    corners[0][0] = 0.0f;
  if (corners[1][0] > (float)w)
    corners[1][0] = (float)w;
  if (corners[0][1] < 0.0f)
    corners[0][1] = 0.0f;
  if (corners[1][1] > (float)h)
    corners[1][1] = (float)h;

  return true;

}

V3 V3::Reflect(V3 r) {

  V3 &n = *this;
  V3 rn = n*(r*n);
  V3 rt = r-rn;
  V3 ret = r - (rt*2.0f);
  return ret;

}
