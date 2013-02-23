#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <GL/glut.h>

#include "v3.h"
#include "ppc.h"


// framebuffer + window class
// supports SW, HW fixed and HW programmable pipeline rendering


class FrameBuffer : public Fl_Gl_Window {
public:
  unsigned int *pix; // SW color buffer
  float *zb; // SW zbuffer
  int w, h; // image resolution
  bool isHW; // flag indicating whether it is to be rendered in HW
  FrameBuffer(int u0, int v0, int _w, int _h); // constructor, top left coords and resolution

  // SW drawing
  void Set(unsigned int color); // set all pixels to color
  void SetZB(float z0); // set all z values to z0
  unsigned int Get(int u, int v); // get color at (u, v)
  V3 Getv(int u, int v); // get into vector color format
  void Set(int u, int v, unsigned int color); // set color at (u, v)
  void Set(V3 P, unsigned int color); // set color at (P[0], P[1])
  bool CloserThenSet(V3 P);  // SW zbuffer projected point P
  void SetGuarded(V3 P, unsigned int color); // set color if inside frame
  void SetAsChecker(int cs, unsigned int cols[2]); // set as checker with checker size cs and colors cs[0] and cs[1]
  void DrawSegment(V3 P0, V3 P1, V3 c0, V3 c1); // draw segment between projected point P0 and P1, with colors c0 and c1
  void DrawSegment3D(V3 P0, V3 P1, V3 c0, V3 c1, PPC *ppc); // project 3-D segment with ppc and then draw

  // some image processing
  void Brighten(float scf);
  void FindEdges();
  void FindEdges(FrameBuffer *edgeMap);

  // function that is always called back by system and never called directly by programmer
  // programmer triggers framebuffer update by calling FrameBuffer::redraw(), which makes
  //            system call draw
  void draw();

  // function called back when event occurs (mouse, keyboard, etc)
  int handle(int event);
  void KeyboardHandle();
  void MouseWheelHandle();

  // returns color at normalized coords s and t (e.g. texture coordinates s, and t)
  V3 LookUp(float s, float t);

  // copies SW pixels
  void CopyFrom(FrameBuffer *fb);

};
