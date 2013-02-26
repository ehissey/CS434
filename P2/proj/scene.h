#pragma once


#include "gui.h"
#include "framebuffer.h"
#include "ppc.h"
#include "tmesh.h"
#include "CGInterface.h"

class Scene {
public:

    // interfaces for GPU programming
    CGInterface *cgi;
    ShaderOneInterface *soi;


    PPC *ppc; // camera used to render current frame

    TMesh *tms; // geometry
    int tmsN; // number of triangle meshes
    GUI *gui; // user interface
    FrameBuffer *fb, *hwfb; // SW and HW framebuffers
    PPC * dImgCam; //camera used to move ppc

    int u, v, w, h;

    Scene();
    void DBG();

    void Render(); // SW & HW
    void RenderHW(); // Fixed pipeline
    void RenderGPU(); // Programmable pipeline
    void FrameSetupHW(PPC * cam); // clearing buffers and setting view for HW rendering

    void FrameSetup(); // clearing buffers for SW rendering

    void SaveView0();
    void LoadView0();
    void GoToView0();
    void SaveView1();
    void LoadView1();
    void GoToView1();
    void SaveView2();
    void LoadView2();
    void GoToView2();
    void GoToView(PPC *nppc);
    void GetTransportMatrix();
};

extern Scene *scene;

