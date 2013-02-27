#pragma once

#include "framebuffer.h"
#include "v3.h"

class EnvMap 
{
public:

   

    PPC *ppc; // camera used to render current frame

    FrameBuffer **envBufs;
    int ID;  //ID for the env map.

    EnvMap();

    void Load(int idNum);
};

