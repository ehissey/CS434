#pragma once


#include "gui.h"
#include "framebuffer.h"
#include "ppc.h"
#include "tmesh.h"

#include "EnvMap.h"


#include "CGInterface.h"


class Scene {
public:

    // interfaces for GPU programming
    CGInterface *cgi;
    ShaderOneInterface *soi;

    int depthID;
    int hasMovedCamera;

    PPC *ppc; // camera used to render current frame
    PPC * dImgCam; //camera used to move ppc
    TMesh *tms; // geometry
    int tmsN; // number of triangle meshes
    GUI *gui; // user interface
    FrameBuffer *fb, *hwfb; // SW and HW framebuffers
    EnvMap * eMap;

    float isRenderingBG;
    float isRenderingFloor;

    int obtainedDImg;

    V3 quad0, quad1, quad2, quad3;

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
    void GoToView(PPC *nppc);
    static FrameBuffer * openImg(string filename);
    void RenderDImg();
    // depth image
    FrameBuffer *depthImage;
    PPC *diPPC;
    TMesh *boundingFrustum;
    void CaptureDepthImage();
};

extern Scene *scene;

