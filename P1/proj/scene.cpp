#include "scene.h"
#include "m33.h"
#include "ppc.h"
#include <float.h>
#include "CImg.h"
#include "v3.h"
#include <iostream>

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

    // create SW framebuffer
    int u0 = 20;
    int v0 = 50;
    int sci = 2;
    int w = sci*240;
    int h = sci*180;
    fb = new FrameBuffer(u0, v0, w, h);
    fb->label("SW Framebuffer");
    //fb->show();

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
    tmsN = 3;
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
void Scene::FrameSetupHW() {

    // OpenGL setup
    glEnable(GL_DEPTH_TEST);
    //  glEnable(GL_CULL_FACE);

    // frame setup
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | 
        GL_DEPTH_BUFFER_BIT);

    // sets HW view, i.e. OpenGL matrices according to our camera, PPC::ppc
    // set intrinsics
    ppc->SetIntrinsicsHW();
    // set extrinsics
    ppc->SetExtrinsicsHW();

}


// fixed HW pipeline
void Scene::RenderHW() {

    /// OpenGL frame setup
    FrameSetupHW();

    // render geometry: issue geometry for rendering
    for (int tmi = 0; tmi < tmsN; tmi++) {
        if (!tms[tmi].enabled)
            continue;
        tms[tmi].RenderHW();
    }

}


// gpu HW pipeline
void Scene::RenderGPU() {
    if(!obtainedDImg)
    {
        RenderDImg();
    }
        //dImgCam->PositionAndOrient(tms[0].GetCenter(), (tms[1].GetCenter()- tms[0].GetCenter()).Normalized(), V3(0.0f, 1.0f, 0.0f), * ppc);

        // per session initialization, i.e. once per run
        if (cgi->needInit) 
        {
            cgi->PerSessionInit();
            soi->PerSessionInit(cgi);

            eMap = new EnvMap();

            eMap->Load(7);

            glBindTexture(GL_TEXTURE_2D, tms[2].floorID);

            glTexParameterf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            FrameBuffer * floorBuf = openImg("floor/checker.bmp");

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, floorBuf->w, floorBuf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, floorBuf->pix);

        }

        FrameSetupHW();

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
    ppc->PositionAndOrient(tms[0].GetCenter(), (tms[1].GetCenter()- tms[0].GetCenter()).Normalized(), V3(0.0f, 1.0f, 0.0f), * dImgCam);

    FrameSetupHW();

    //obtainedDImg = 1;

    

    // issue geometry to be rendered with the shaders enabled above
    tms[1].RenderHW();



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