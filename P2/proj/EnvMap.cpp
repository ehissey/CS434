#include <GL\glew.h>

#include "scene.h"
#include "EnvMap.h"
#include "v3.h"


EnvMap::EnvMap() 
{
    envBufs = new FrameBuffer *[6];

    envBufs[0] = Scene::openImg("envmap/z-.bmp");
    envBufs[2] = Scene::openImg("envmap/z+.BMP");
    envBufs[1] = Scene::openImg("envmap/x-.bmp");
    envBufs[3] = Scene::openImg("envmap/x+.BMP");
    envBufs[5] = Scene::openImg("envmap/y-.BMP");
    envBufs[4] = Scene::openImg("envmap/y+.BMP");

}

void EnvMap::Load(int idNum)
{
    int index = 0;

    ID = idNum;
    
    glBindTexture(GL_TEXTURE_CUBE_MAP_EXT, idNum);

    glTexParameterf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT, GL_RGBA8, 
        envBufs[index]->w, envBufs[index]->h, GL_RGBA, GL_UNSIGNED_BYTE, 
        envBufs[index]->pix);
    index++;

    
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT, GL_RGBA8, 
        envBufs[index]->w, envBufs[index]->h, GL_RGBA, GL_UNSIGNED_BYTE, 
        envBufs[index]->pix);
    index++;

    
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT, GL_RGBA8, 
        envBufs[index]->w, envBufs[index]->h, GL_RGBA, GL_UNSIGNED_BYTE, 
        envBufs[index]->pix);
    index++;

   
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT, GL_RGBA8, 
        envBufs[index]->w, envBufs[index]->h, GL_RGBA, GL_UNSIGNED_BYTE, 
        envBufs[index]->pix);
    index++;

    
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT, GL_RGBA8, 
        envBufs[index]->w, envBufs[index]->h, GL_RGBA, GL_UNSIGNED_BYTE, 
        envBufs[index]->pix);
    index++;

    //Load Y- image
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT, GL_RGBA8, 
        envBufs[index]->w, envBufs[index]->h, GL_RGBA, GL_UNSIGNED_BYTE, 
        envBufs[index]->pix);
    index++;
}