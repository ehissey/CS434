#include "framebuffer.h"
#include "v3.h"
#include <iostream>
#include "scene.h"
#include <math.h>


// makes an OpenGL window that supports SW, HW rendering, that can be displayed on screen
//        and that receives UI events, i.e. keyboard, mouse, etc.
FrameBuffer::FrameBuffer(int u0, int v0, 
  int _w, int _h) : Fl_Gl_Window(u0, v0, _w, _h, 0) {

  w = _w;
  h = _h;
  pix = new unsigned int[w*h];
  zb  = new float[w*h];
  Set(0xFFFFFFFF);
  isHW = false;

}

// set all z values in SW ZB to z0
void FrameBuffer::SetZB(float z0) {

  for (int i = 0; i < w*h; i++)
    zb[i] = z0;

}


// rendering callback; see header file comment
void FrameBuffer::draw() {

  if (isHW && scene) {
    // right now fixed / programmable pipeline is done by commenting/uncommenting the lines below; feel free to add a flag
//    scene->RenderHW(); // fixed pipeline
    scene->RenderGPU(); // programmable pipeline
  }
  else {
    // SW window, just transfer computed pixels from pix to HW for display
    glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, pix);
  }

}

// set all SW pixels to color
void FrameBuffer::Set(unsigned int color) {

  for (int i = 0; i < w*h; i++)
    pix[i] = color;

}

// set one pixel to color
void FrameBuffer::Set(int u, int v, unsigned int color) {

  pix[w*(h-1-v)+u] = color;

}

// return color at u, v
unsigned int FrameBuffer::Get(int u, int v) {

  return pix[w*(h-1-v)+u];

}

// set SW framebuffer to checker
void FrameBuffer::SetAsChecker(int cs, unsigned int cols[2]) {

  for (int v = 0; v < h; v++) {
    for (int u = 0; u < w; u++) {
      int cu = u/cs;
      int cv = v/cs;
      int i = (cu+cv)%2;
      Set(u, v, cols[i]);
    }
  }

}


// project 3-D segment, draw projected segment
void FrameBuffer::DrawSegment3D(V3 P0, V3 P1, V3 c0, 
  V3 c1, PPC *ppc) {

  V3 pP0, pP1;

  if (!ppc->Project(P0, pP0))
    return;

  if (!ppc->Project(P1, pP1))
    return;

  DrawSegment(pP0, pP1, c0, c1);

}

// draw 2-D segment
void FrameBuffer::DrawSegment(V3 P0, V3 P1, V3 c0, V3 c1) {

  int pixN;
  float du = fabsf(P0[0]-P1[0]);
  float dv = fabsf(P0[1]-P1[1]);
  if (du < dv) {
    pixN = 1+(int)dv;
  }
  else {
    pixN = 1+(int)du;
  }

  V3 currP = P0;
  V3 dP = (P1-P0)/(float)pixN;
  V3 currC = c0;
  V3 dC = (c1-c0)/(float)pixN;
  for (int si = 0; si < pixN+1; si++) {
    // if the current pixel has already seen a sample closer than current sample, continue
    if (CloserThenSet(currP)) {
      SetGuarded(currP, currC.GetColor());
    }
    currP = currP + dP;
    currC = currC + dC;
  }

}


// set pixel to color
void FrameBuffer::Set(V3 P, unsigned int color) {

  int u = (int) P[0];
  int v = (int) P[1];
  Set(u, v, color);

}


// set pixel but check image boundaries
void FrameBuffer::SetGuarded(V3 P, unsigned int color) {

  int u = (int) P[0];
  int v = (int) P[1];
  if (u < 0 || u > w-1 || v < 0 || v > h-1)
    return;
  Set(u, v, color);

}

// some image processing
void FrameBuffer::Brighten(float scf) {

  for (int v = 0; v < h; v++) {
    for (int u = 0; u < w; u++) {
      V3 col;
      col.SetFromColor(Get(u, v));
      col = col * scf;
      Set(u, v, col.GetColor());
    }
  }

}

void FrameBuffer::FindEdges(FrameBuffer *edgeMap) {

  // traversing the entire image
  for (int v = 1; v < h-1; v++) {
    for (int u = 1; u < w-1; u++) {
      // compute convolution at current pixel
      V3 conv(0.0f, 0.0f, 0.0f);
      conv = Getv(u, v)*4.0f - Getv(u-1, v)
        - Getv(u+1, v) - Getv(u, v-1) - Getv(u, v+1);
      // take absolute value of each channel
      // write in edge map
    }
  }

}


// get color in vector format
V3 FrameBuffer::Getv(int u, int v) {

  V3 ret;
  ret.SetFromColor(pix[(h-1-v)*w+u]);
  return ret;

}

// function called automatically on event within window (callback)
int FrameBuffer::handle(int event)  
{
  switch(event) 
  {   
	case FL_KEYBOARD: 
	{
		KeyboardHandle();
		return 0;	
	}

	case FL_MOUSEWHEEL: 
	{
		MouseWheelHandle();
		return 0;
	}

	default:
		break;
  }

  return 0;
}

void FrameBuffer::MouseWheelHandle()
{
	int num = Fl::event_dy();
	float tstep = 3.0f;
	if(num > 0)
	{
		scene->ppc->TranslateFB(-tstep);
		scene->Render();		
	}
	else if(num < 0)
	{
		scene->ppc->TranslateFB(tstep);
		scene->Render();		
	}

}

void FrameBuffer::KeyboardHandle() {

  // keys are used for 6 dof navigation (3 trans and pan, tilt, roll of camera)
  float tstep = 3.0f;
  float rstep = 3.0f;
  int key = Fl::event_key();



  switch (key) 
  {
    case 'a': 
	{
      scene->ppc->TranslateLeft(tstep);
      scene->Render();
	  break;
    }
    case 'd': 
	{
      scene->ppc->TranslateRight(tstep);
      scene->Render();
	  break;
    }
    case 'w': 
	{
      scene->ppc->TranslateUp(tstep);
      scene->Render();
      break;
    }
    case 's': 
	{
      scene->ppc->TranslateDown(tstep);
      scene->Render();
      break;
    }
    case 'e': 
	{
      scene->ppc->TranslateFB(tstep);
      scene->Render();
      break;
    }
    case 'q': 
	{
      scene->ppc->TranslateFB(-tstep);
      scene->Render();
      break;
    }
    case FL_Left: 
	{
      scene->ppc->Pan(rstep);
      scene->Render();
      break;
    }
    case FL_Right: 
	{
      scene->ppc->Pan(-rstep);
      scene->Render();
      break;
    }
    case FL_Up: 
	{
      scene->ppc->Tilt(rstep);
      scene->Render();
      break;
    }
    case FL_Down: 
	{
      scene->ppc->Tilt(-rstep);
      scene->Render();
      break;
    }
    case '3': 
	{
      scene->ppc->Roll(rstep);
      scene->Render();
      break;
    }
    case '1': 
	{
      scene->ppc->Roll(-rstep);
      scene->Render();
      break;
    }
    default:
    cerr << "INFO: do not understand keypress" << endl;
  }
}


// SW zbuffering
bool FrameBuffer::CloserThenSet(V3 P) {

  int u = (int) P[0];
  int v = (int) P[1];
  if (u < 0 || u > w-1 || v < 0 || v > h-1)
    return false;
  int uv = (h-1-v)*w+u;
  if (zb[uv] > P[2])
    return false;

  zb[uv] = P[2];
  return true;

}


// lookup image at normalized coordinates (i.e. coords between 0 and 1)
V3 FrameBuffer::LookUp(float s, float t) {

  int u = (int) (s*(float)w);
  int v = (int) (t*(float)h);
  if (u < 0 || u > w-1 || v < 0 || v > h-1)
    return V3(0.0f, 1.0f, 1.0f);

  return Getv(u, v);

}

// copying from given framebuffer
void FrameBuffer::CopyFrom(FrameBuffer *fb) {

  for (int uv = 0; uv < w*h; uv++) {
    zb[uv] = fb->zb[uv];
    pix[uv] = fb->pix[uv];
  }

}