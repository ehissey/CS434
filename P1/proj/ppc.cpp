#include "ppc.h"

#include "m33.h"
#include "framebuffer.h"

#include <math.h>
#include <fstream>

PPC::PPC(float hfov, int _w, int _h) : w(_w), h(_h) {

  C = V3(0.0f, 0.0f, 0.0f);
  a = V3(1.0f, 0.0f, 0.0f);
  b = V3(0.0f, -1.0f, 0.0f);
  float hfovRAD = hfov / 180.0f * 3.1415926f;
  c = V3(-((float)w)/2.0f, ((float)h)/2.0f, -(float)w/(2*tanf(hfovRAD/2.0f)));
  SetPMat();
  zNear = 1.0f;
  zFar = 10000.0f;

}

bool PPC::Project(V3 P, V3 &projP) {

  V3 Q = pMat*(P-C);

  if (Q[2] <= 0.0f)
    return false;

  projP[0] = Q[0] / Q[2];
  projP[1] = Q[1] / Q[2];
  projP[2] = 1.0f / Q[2];

  return true;

}

void PPC::TranslateRight(float tstep) {

  C = C + a.Normalized()*tstep;

}

void PPC::TranslateLeft(float tstep) {

  C = C - a.Normalized()*tstep;

}

void PPC::TranslateUp(float tstep) {

  C = C - b.Normalized()*tstep;

}

void PPC::TranslateDown(float tstep) {

  C = C + b.Normalized()*tstep;

}

void PPC::Pan(float rstep) {

  a = a.RotateDirectionAboutArbitraryAxis(b.Normalized()*-1.0f, rstep);
  b = b.RotateDirectionAboutArbitraryAxis(b.Normalized()*-1.0f, rstep);
  c = c.RotateDirectionAboutArbitraryAxis(b.Normalized()*-1.0f, rstep);
  SetPMat();

}

void PPC::Tilt(float rstep) {

  a = a.RotateDirectionAboutArbitraryAxis(a.Normalized(), rstep);
  b = b.RotateDirectionAboutArbitraryAxis(a.Normalized(), rstep);
  c = c.RotateDirectionAboutArbitraryAxis(a.Normalized(), rstep);
  SetPMat();

}

void PPC::Roll(float rstep) {

  a = a.RotateDirectionAboutArbitraryAxis(GetVD(), rstep);
  b = b.RotateDirectionAboutArbitraryAxis(GetVD(), rstep);
  c = c.RotateDirectionAboutArbitraryAxis(GetVD(), rstep);
  SetPMat();

}


V3 PPC::GetVD() {

  return (a^b).Normalized();

}

float PPC::Getf() {

  return GetVD()*c;

}


//PPC ppc0; // camera to position and orient
//PPC ppc; // output of function: original camera positioned and oriented according to parameters
//ppc0.PositionAndOrient(newC, newVD, vinUP, ppc);

void PPC::PositionAndOrient(V3 newC, V3 newVD, V3 vinUP, PPC &ppc) {

  ppc.w = w;
  ppc.h = h;
  ppc.zFar = zFar;
  ppc.zNear = zNear;
  ppc.C = newC;
  ppc.a = (newVD ^ vinUP).Normalized() * a.Length();
  V3 newUp = (ppc.a ^ newVD).Normalized();
  ppc.b = newUp * -1.0f * b.Length();
  ppc.c = newVD*Getf() - ppc.b*(float)h/2.0f - ppc.a*(float)w/2.0f;

  ppc.SetPMat();

}

PPC PPC::Interpolate(PPC *ppc0, PPC *ppc1, float frac) {

  V3 newC = ppc0->C + (ppc1->C-ppc0->C)*frac;
  V3 anewb = ppc0->b + (ppc1->b-ppc0->b)*frac;
  V3 vinUP = anewb * -1.0f;
  V3 newVD = (ppc0->GetVD() + (ppc1->GetVD() - ppc0->GetVD())*frac).Normalized();
  PPC ret;
  PositionAndOrient(newC, newVD, vinUP, ret);
  return ret;

}


V3 PPC::GetPoint(float uf, float vf, float z) {

  float f = Getf();
  float scf = z/f;
  V3 pointIP = C + a*uf + b*vf + c;
  V3 ret = C + (pointIP-C)*scf;
  return ret;

}


void PPC::RenderImageFrameGL() {

  float bf = 0.0f;
  glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
  float zMed = (zNear + zFar) / 2.0f;
  V3 V;
  glBegin(GL_QUADS);
  V = GetPoint(bf+0.0f, bf+0.0f, zMed);
  glVertex3fv((float*)&V);
  V = GetPoint(bf+0.0f, (float)h-bf, zMed);
  glVertex3fv((float*)&V);
  V = GetPoint((float)w-bf, (float)h-bf, zMed);
  glVertex3fv((float*)&V);
  V = GetPoint((float)w-bf, bf+0.0f, zMed);
  glVertex3fv((float*)&V);
  glEnd();
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

}


void PPC::Visualize(PPC *ppc0, FrameBuffer *fb, float visF,
  int vLN) {

  V3 P0, P1, c0(0.0f, 0.0f, 0.0f), c1(0.0f, 0.0f, 0.0f);

  float scf = visF / Getf();

  P0 = C + c*scf;
  P1 = P0 + a*(float)w*scf;
  fb->DrawSegment3D(P0, P1, c0, c1, ppc0);  

  P0 = P1;
  P1 = P0 + b*(float)h*scf;
  fb->DrawSegment3D(P0, P1, c0, c1, ppc0);  

  P0 = P1;
  P1 = P0 - a*(float)w*scf;
  fb->DrawSegment3D(P0, P1, c0, c1, ppc0);  

  P0 = P1;
  P1 = C+c*scf;
  fb->DrawSegment3D(P0, P1, c0, c1, ppc0);  

  P0 = C;
  P1 = C+c*scf;
  fb->DrawSegment3D(P0, P1, V3(1.0f, 0.0f, 0.0f), c1, ppc0);  

  int du = w/vLN;
  for (int i = 0; i < vLN; i++) {
    
    P0 = C + c*scf + a*(float)(du*i)*scf;
    P1 = C + c*scf + a*(float)(du*i)*scf + b*(float)h*scf;
    fb->DrawSegment3D(P0, P1, c0, c1, ppc0);  

  }

}

void PPC::TranslateFB(float tstep) {

  C = C + GetVD()*tstep;

}

void PPC::Save(char *fname) {

  ofstream ofs(fname);
  ofs << a << endl;
  ofs << b << endl;
  ofs << c << endl;
  ofs << C << endl;
  ofs << "// a b c C" << endl;
  ofs.close();

}


void PPC::Load(char *fname) {

  ifstream ifs(fname);
  ifs >> a ;
  ifs >> b ;
  ifs >> c ;
  ifs >> C ;
  ifs.close();
  
  SetPMat();

}

V3 PPC::GetRay(float uf, float vf) {

  return c + a*uf + b*vf;

}

V3 PPC::GetPoint(V3 projP) {

  // projP[0] is "u"
  // projP[1] is "v"
  // projP[2] is "1/w"

  return C + GetRay(projP[0], projP[1])/projP[2];

}

void PPC::SetPMat() {

  M33 cam;
  cam.SetColumn(0, a);
  cam.SetColumn(1, b);
  cam.SetColumn(2, c);
  pMat = cam.Invert();

}

void PPC::SetIntrinsicsHW() {

  glViewport(0, 0, w, h);
  float wf = a.Length()*(float)w;
  float hf = b.Length()*(float)h;
  float scf = zNear / Getf();
  float left = -wf/2.0f*scf;
  float right = +wf/2.0f*scf;
  float top= hf/2.0f*scf;
  float bottom = -hf/2.0f*scf;
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glFrustum(left, right, bottom, top, zNear, zFar);
  glMatrixMode(GL_MODELVIEW);

}


void PPC::SetExtrinsicsHW() {

  V3 LAP = C + GetVD();
  V3 up = (b*-1.0f).Normalized();
  glLoadIdentity();
  gluLookAt(C[0], C[1], C[2], LAP[0], LAP[1], LAP[2], 
    up[0], up[1], up[2]);

}