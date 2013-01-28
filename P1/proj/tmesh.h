#pragma once

#include "v3.h"
#include "ppc.h"
#include "framebuffer.h"


// triangle mesh class

class TMesh {
public:
  bool enabled; // is it enabled
  bool isReflective; // is it reflective
  bool msi; // does it want model space rasterization parameter interpolation for SW rendering
  bool renderWF; // should it be rendered in wireframe
  V3 *verts, *cols, *tcs, *normals; // vertex data: xyz, rgb, texture coordinates, normalx, normaly, normalz
  int vertsN; // number of vertices
  unsigned int *tris; // connectivity data; a triple per triangle; one element is an index in the verts arrays
  int trisN; // number of triangles; tris is of size 3*trisN
  FrameBuffer *tmap; // texture map for mesh (one per mesh)

  TMesh() : enabled(true), msi(true), verts(0), vertsN(0), 
    tris(0), trisN(0), cols(0), tcs(0), tmap(0),
    renderWF(false), normals(0), isReflective(false) {};

  void SetCube(V3 center, float sl); // construct cube
  void SetRectangle(float wf, float hf, FrameBuffer *_tmap = 0); // construct rectangle
  void Render(PPC *ppc, FrameBuffer *fb); // render in SW filled
  void RenderWF(PPC *ppc, FrameBuffer *fb); // render in SW wireframe
  void RenderPoints(PPC *ppc, FrameBuffer *fb, int pSize); // render vertices as points (and don't render triangles)
  void RotateAboutArbitraryAxis(V3 aO, V3 aDir, float theta); // rotate vertices about arbitrary axis
  V3 GetCenter(); // find centroid as average of all vertices
  void Load(char *fname); // load from bin file
  void Translate(V3 transv); // translate vertices
  void Scale(float scalef); // scale vertices (about origin)
  void ScaleAboutCenter(float scalef); // scale about centroid of mesh
  AABB GetAABB(); // get axis aligned bounding box of mesh
  void SetFromFB(FrameBuffer *fb, PPC *ppc); // construct triangle mesh from framebuffer (using depth, implicit connectivity)
  void RenderHW(); // render HW (issue geometry, per vertex parameters)
};

