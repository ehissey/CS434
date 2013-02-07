#include "scene.h"
#include "m33.h"
#include "ppc.h"
#include <float.h>
#include "CImg.h"
#include "v3.h"
#include <iostream>
#include<math.h>

using namespace std;
using namespace cimg_library;

Scene *scene;

Scene::Scene() {



    // create interface between CPU and GPU
    cgi = new CGInterface();
    soi = new ShaderOneInterface();

    // create user interface
    gui = new GUI();
    gui->show();

    hasMovedCamera = 0;

    // create SW framebuffer
    int u0 = 20;
    int v0 = 50;
    int sci = 2;
    int w = sci*240;
    int h = sci*180;


    fb = new FrameBuffer(u0, v0, w, h);
    depthImage = new FrameBuffer(0, 0, fb->w, fb->h);
    depthID = 77;

    fb->label("SW Framebuffer");

    // create HW framebuffer
    hwfb = new FrameBuffer(u0+fb->w+20, v0, w, h);

    hwfb->label("HW Framebuffer");
    hwfb->isHW = true;
    hwfb->show();

    // position UI window
    gui->uiw->position(2*fb->w+u0 + 2*20, v0);

    // create camera for rendering scene
    float hfov = 45.0f;
    ppc = new PPC(hfov, w, h);


    obtainedDImg = 0;
    // load, scale and position geometry, i.e. triangle meshes
    tmsN = 4;
    tms = new TMesh[tmsN];
    V3 center(0.0f, 0.0f, -175.0f);
    V3 side(200.0f, 0.0f, -175.0f);
    tms[0].Load("geometry/bunny.bin");
    tms[1].Load("geometry/bunny.bin");

    AABB aabb = tms[0].GetAABB();

    float size0 = (aabb.corners[1]-aabb.corners[0]).Length();

    V3 tcenter = tms[0].GetCenter();
    tms[0].Translate(tcenter*-1.0f+center);
    tms[1].Translate(tcenter*-1.0f+side);

    dImgCam = new PPC(hfov, w, h);


    float size1 = 170.0f;
    tms[0].ScaleAboutCenter(size1/size0);
    tms[0].renderWF = false;
    tms[0].shaderIsEnabled = 1;
    tms[1].ScaleAboutCenter(size1/size0);
    tms[1].renderWF = false;
    tms[1].shaderIsEnabled = 0;
    tms[2].SetFloor();
    tms[2].shaderIsEnabled = 0;

    TMesh floorTM = tms[2];

    //Set floor parameters
    quad0 = V3(-floorTM.wf/2, floorTM.down, -floorTM.hf/2);
    quad1 = V3(-floorTM.wf/2, floorTM.down, floorTM.hf/2);
    quad2 = V3(floorTM.wf/2, floorTM.down, floorTM.hf/2);
    quad3 = V3(floorTM.wf/2, floorTM.down, -floorTM.hf/2);

    // render scene
    Render();

}


// SW per frame setup
void Scene::FrameSetup() {

    fb->Set(0xFF7F0000); // clear color buffer
    fb->SetZB(0.0f); // clear z buffer



}

// render frame
void Scene::Render() {



    if (hwfb) {
        // ask to redraw HW framebuffer; will get FrameBuffer::draw called (callback), which will call either
        //        Scene::RenderHW for fixed pipeline HW rendering, or Scene::RenderGPU for GPU HW rendering;
        //        this is needed since with FLTK one can only make GL calls from FrameBuffer::draw
        hwfb->redraw();
    }
    return;
    // SW rendering
    FrameSetup();

    // render all geometry
    for (int tmi = 0; tmi < tmsN; tmi++) 
    {
        if (!tms[tmi].enabled)
            continue;
        if (tms[tmi].renderWF) 
        {
            tms[tmi].RenderWF(ppc, fb); // wireframe
        }
        else {
            tms[tmi].Render(ppc, fb); // regular filled rendering
        }
    }


    fb->redraw(); // this calls FrameBuffer::draw wich posts pixels with glDrawPixels;

}

// function linked to the DBG GUI button for testing new features
void Scene::DBG() {

    cerr << "INFO: DBG" << endl;

}

// GUI view saving, loading and interpolation
void Scene::SaveView0() {

    ppc->Save("mydbg/view0.txt");

}

void Scene::LoadView0() {

    ppc->Load("mydbg/view0.txt");
    Render();

}

void Scene::GoToView0() {

    PPC nppc;
    nppc.Load("mydbg/view0.txt");
    GoToView(&nppc);

}

void Scene::SaveView1() {

    ppc->Save("mydbg/view1.txt");

}

void Scene::LoadView1() {

    ppc->Load("mydbg/view1.txt");
    Render();

}

void Scene::GoToView1() {

    PPC nppc;
    nppc.Load("mydbg/view1.txt");
    GoToView(&nppc);

}


void Scene::GoToView(PPC *nppc) {

    PPC oppc(*ppc);
    int n = 100;
    for (int i = 0; i < n; i++) {
        float frac = (float) i / (float) (n-1);
        *ppc = ppc->Interpolate(&oppc, nppc, frac);
        Render();
        Fl::check();
    }



}

////////// HW rendering 

// there are two ways of rendering in HW
//1. fixed pipeline, i.e. "basic" OpenGL, no shaders, Scene::RenderHW
// 2. programmable pipeline, i.e. using GPU shaders, Scene::RenderGPU

// called by Scene::RenderHW and by Scene::RenderGPU, which is itself called by the draw callback of the HW framebuffer
void Scene::FrameSetupHW(PPC * cam) {

    // OpenGL setup
    glEnable(GL_DEPTH_TEST);
    //  glEnable(GL_CULL_FACE);

    // frame setup
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | 
        GL_DEPTH_BUFFER_BIT);

    // sets HW view, i.e. OpenGL matrices according to our camera, PPC::ppc
    // set intrinsics
    cam->SetIntrinsicsHW();
    // set extrinsics
    cam->SetExtrinsicsHW();

}


// fixed HW pipeline
void Scene::RenderHW() 
{

    /// OpenGL frame setup
    FrameSetupHW(ppc);

    // render geometry: issue geometry for rendering
    for (int tmi = 0; tmi < 1; tmi++) {
        if (!tms[tmi].enabled)
            continue;
        tms[tmi].RenderHW();
    }

}


// gpu HW pipeline
void Scene::RenderGPU() 
{
    if(!hasMovedCamera)
    {
        ppc->PositionAndOrient(tms[0].GetCenter(), (tms[1].GetCenter()- tms[0].GetCenter()).Normalized(), V3(0.0f, 1.0f, 0.0f), * dImgCam);
        hasMovedCamera = 1;
    }
    if(!obtainedDImg)
    {
        RenderDImg();
    }

    // per session initialization, i.e. once per run
    if (cgi->needInit) 
    {
        cgi->PerSessionInit();
        soi->PerSessionInit(cgi);

        eMap = new EnvMap();

        eMap->Load(7);

        //Load the floor
        glBindTexture(GL_TEXTURE_2D, tms[2].floorID);

        glTexParameterf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        FrameBuffer * floorBuf = openImg("floor/checker.bmp");

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, floorBuf->w, floorBuf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, floorBuf->pix);

    }

    FrameSetupHW(ppc);

    if(eMap)
    {
        isRenderingBG = 1.0f;
        soi->PerFrameInit();
        soi->BindPrograms();
        cgi->EnableProfiles();
        ppc->RenderImageFrameGL();
        cgi->DisableProfiles();
        isRenderingBG = 0.0f;


    }


    // per frame parameter setting and enabling shaders
    soi->PerFrameInit();
    soi->BindPrograms();
    cgi->EnableProfiles();

    // issue geometry to be rendered with the shaders enabled above
    for(int i = 0; i < tmsN; i++)
    {
        if(tms[i].shaderIsEnabled)
        {
            cgi->EnableProfiles();
        }
        else
        {
            cgi->DisableProfiles();
        }
        tms[i].RenderHW();
    }

    // disable GPU rendering
    soi->PerFrameDisable();
    cgi->DisableProfiles();
}

void Scene::RenderDImg() 
{


    

    FrameSetupHW(dImgCam);

   /* dImgCam->zNear = 1;
    dImgCam->zFar = 10000;

    V3 VDP = dImgCam->GetVD() * dImgCam->zNear;
    V3 VDPP = dImgCam->GetVD() * dImgCam->zFar;

    float wp = 2*(dImgCam->zNear)*tan(dImgCam->hfov/2);
    float wpp = 2*(dImgCam->zFar)*tan(dImgCam->hfov/2);
    float hp = (wp*dImgCam->h)/dImgCam->w;
    float hpp = (wpp*dImgCam->h)/dImgCam->w;

    //V3 near0, near1, near2, near3;
    //V3 far0, far1, far2, far3;

    verts[0] = dImgCam->C + VDP - (dImgCam->a * (wp/2) + dImgCam->b * (hp/2));
    verts[1]= verts[0] + dImgCam->b * hp;
    verts[2] = verts[1] + dImgCam->b * wp;
    verts[3] = verts[0] + dImgCam->a * wp;

    verts[4] = dImgCam->C + VDPP - (dImgCam->a * (wpp/2) + dImgCam->b * (hpp/2));
    verts[5] = verts[4] + dImgCam->b * hpp;
    verts[6] = verts[5] + dImgCam->b * wpp;
    verts[7] = verts[4] + dImgCam->a * wpp;
    
    int tri = 0;
    tris[3*tri+0] = 0;
    tris[3*tri+1] = 1;
    tris[3*tri+2] = 2;
    tri++;
    tris[3*tri+0] = 2;
    tris[3*tri+1] = 3;
    tris[3*tri+2] = 0;
    tri++;

    tris[3*tri+0] = 4;
    tris[3*tri+1] = 7;
    tris[3*tri+2] = 6;
    tri++;
    tris[3*tri+0] = 6;
    tris[3*tri+1] = 5;
    tris[3*tri+2] = 4;
    tri++;

    tris[3*tri+0] = 5;
    tris[3*tri+1] = 1;
    tris[3*tri+2] = 0;
    tri++;
    tris[3*tri+0] = 0;
    tris[3*tri+1] = 4;
    tris[3*tri+2] = 5;
    tri++;

    tris[3*tri+0] = 1;
    tris[3*tri+1] = 5;
    tris[3*tri+2] = 6;
    tri++;
    tris[3*tri+0] = 6;
    tris[3*tri+1] = 2;
    tris[3*tri+2] = 1;
    tri++;

    tris[3*tri+0] = 6;
    tris[3*tri+1] = 7;
    tris[3*tri+2] = 3;
    tri++;
    tris[3*tri+0] = 3;
    tris[3*tri+1] = 2;
    tris[3*tri+2] = 6;
    tri++;

    tris[3*tri+0] = 4;
    tris[3*tri+1] = 0;
    tris[3*tri+2] = 3;
    tri++;
    tris[3*tri+0] = 3;
    tris[3*tri+1] = 7;
    tris[3*tri+2] = 4;
    tri++;

    V3 * cols = new V3[vertsN];
    for (int i = 0; i < 4; i++) 
    {
        cols[i] = V3(1.0f, 0.0f, 0.0f);
        cols[4+i] = V3(0.0f, 0.0f, 0.0f);

    }
    */

    tms[3].SetFrustum(dImgCam);

    tms[3].renderWF = true;

    tms[3].RenderHW();
    // issue geometry to be rendered with the shaders enabled above
    tms[1].RenderHW();

    glReadPixels(0, 0, depthImage->w, depthImage->h, GL_RGBA, GL_UNSIGNED_BYTE, depthImage->pix);
    glReadPixels(0, 0, depthImage->w, depthImage->h, GL_DEPTH_COMPONENT, GL_FLOAT, depthImage->zb);

    glBindTexture(GL_TEXTURE_2D, depthID);

    glTexParameterf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, depthImage->w, depthImage->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, depthImage->pix);


}

FrameBuffer * Scene::openImg(string fileName)
{
    CImg<unsigned char> src(fileName.c_str());


    int r, g, b;

    int width = src.width();
    int height = src.height();

    FrameBuffer * fb;

    fb = new FrameBuffer(0, 0, width, height);

    for(int h = 0; h < height; h++)
    {
        for(int w = 0; w < width; w++)
        {
            r = src.atXY(w, h, 0);
            g = src.atXY(w, h, 1);
            b = src.atXY(w, h, 2);

            V3 colorVal((float)r/255, (float)g/255, (float)b/255);

            fb->Set(w, h, colorVal.GetColor());
        }
    }

    return fb;
}

void Scene::CaptureDepthImage() {



}