#pragma once

#include "v3.h"
#include "m33.h"

class FrameBuffer;

// planar pinhole camera class

class PPC {
public:
  V3 a, b, c, C; // pixel width vector, pixel height vector, vector from eye to top left corner of image, and eye
  int w, h; // camera resolution
  float zNear, zFar; // near and far planes
  M33 pMat; // projection matrix, computed only when the camera changes, and not for every projection
  void SetPMat();
  PPC() {};
  PPC(float hfov, int _w, int _h); // construct a camera with given horizontal field of view and resolution;
                                                      // camera is placed in default position, i.e. with eye at (0, 0, 0), looking down the -z axis, and up vector (0, 1, 0)
  bool Project(V3 P, V3 &projP); // project 3-D point

  void TranslateRight(float tstep); // view change, i.e. camera navigation
  void TranslateLeft(float tstep);
  void TranslateUp(float tstep);
  void TranslateDown(float tstep);
  void TranslateFB(float tstep);
  void Pan(float rstep);
  void Tilt(float rstep);
  void Roll(float rstep);

  V3 GetVD(); // returns view direction
  float Getf(); // returns focal length
  void PositionAndOrient(V3 newC, V3 newVD,
    V3 vinUP, PPC &ppc); // new eye is newC, new view direction is newVD, new up vector is in plane defined by vinUP and newVD
  PPC Interpolate(PPC *ppc0, PPC *ppc1, float frac); // interpolate in between two cameras (i.e. two views), with factor frac
  void Visualize(PPC *ppc0, FrameBuffer *fb, float visF, int vLN); // visualizae camera
                                                                                                                 // visF is the focal length used for visualization
  void RenderImageFrameGL(); // render 3-D image frame in HW
  void Save(char *fname); // save camera parameters in text file
  void Load(char *fname); // load ...
  V3 GetRay(float uf, float vf); // returns ray through image plane point with pixel coordinates (uf, vf)
  V3 GetPoint(V3 projP); // unprojects a projected point projP[2] is "depth"
  V3 GetPoint(float uf, float vf, float z); // returns point on ray through (uf, vf) at depth z
  void SetIntrinsicsHW(); // sets OpenGL Projection Matrix according to camera intrinsic parameters
  void SetExtrinsicsHW(); // sets OpenGL Modelview Matrix according to camera extrinsic parameters
};