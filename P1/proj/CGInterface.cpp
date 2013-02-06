//#define GEOM_SHADER

#include "CGInterface.h"
#include "v3.h"
#include "scene.h"

#include <iostream>

using namespace std;

CGInterface::CGInterface() : needInit(true) {};

void CGInterface::PerSessionInit() {

    glEnable(GL_DEPTH_TEST);

    CGprofile latestVertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
    CGprofile latestGeometryProfile = cgGLGetLatestProfile(CG_GL_GEOMETRY);
    CGprofile latestPixelProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);

    if (latestGeometryProfile == CG_PROFILE_UNKNOWN) {
        cerr << "ERROR: geometry profile is not available" << endl;
#ifdef GEOM_SHADER
        exit(0);
#endif
    }

    cgGLSetOptimalOptions(latestGeometryProfile);
    CGerror Error = cgGetError();
    if (Error) {
        cerr << "CG ERROR: " << cgGetErrorString(Error) << endl;
    }

    cout << "Info: Latest GP Profile Supported: " << cgGetProfileString(latestGeometryProfile) << endl;

    geometryCGprofile = latestGeometryProfile;

    cout << "Info: Latest VP Profile Supported: " << cgGetProfileString(latestVertexProfile) << endl;
    cout << "Info: Latest FP Profile Supported: " << cgGetProfileString(latestPixelProfile) << endl;

    vertexCGprofile = latestVertexProfile;
    pixelCGprofile = latestPixelProfile;
    cgContext = cgCreateContext();  

    needInit = false;

}

bool ShaderOneInterface::PerSessionInit(CGInterface *cgi) {

#ifdef GEOM_SHADER
    geometryProgram = cgCreateProgramFromFile(cgi->cgContext, CG_SOURCE, 
        "CG/shaderOne.cg", cgi->geometryCGprofile, "GeometryMain", NULL);
    if (geometryProgram == NULL)  {
        CGerror Error = cgGetError();
        cerr << "Shader One Geometry Program COMPILE ERROR: " << cgGetErrorString(Error) << endl;
        cerr << cgGetLastListing(cgi->cgContext) << endl << endl;
        return false;
    }
#endif

    vertexProgram = cgCreateProgramFromFile(cgi->cgContext, CG_SOURCE, 
        "CG/shaderOne.cg", cgi->vertexCGprofile, "VertexMain", NULL);
    if (vertexProgram == NULL) {
        CGerror Error = cgGetError();
        cerr << "Shader One Geometry Program COMPILE ERROR: " << cgGetErrorString(Error) << endl;
        cerr << cgGetLastListing(cgi->cgContext) << endl << endl;
        return false;
    }

    fragmentProgram = cgCreateProgramFromFile(cgi->cgContext, CG_SOURCE, 
        "CG/shaderOne.cg", cgi->pixelCGprofile, "FragmentMain", NULL);
    if (fragmentProgram == NULL)  {
        CGerror Error = cgGetError();
        cerr << "Shader One Fragment Program COMPILE ERROR: " << cgGetErrorString(Error) << endl;
        cerr << cgGetLastListing(cgi->cgContext) << endl << endl;
        return false;
    }

    // load programs
#ifdef GEOM_SHADER
    cgGLLoadProgram(geometryProgram);
#endif
    cgGLLoadProgram(vertexProgram);
    cgGLLoadProgram(fragmentProgram);

    // build some parameters by name such that we can set them later...
    vertexModelViewProj = cgGetNamedParameter(vertexProgram, "modelViewProj" );
    geometryModelViewProj = cgGetNamedParameter(geometryProgram, "modelViewProj" );

    eyePix = cgGetNamedParameter(fragmentProgram, "eyeCam");
    renderingBG = cgGetNamedParameter(fragmentProgram, "bgRendering");
    renderingFloor =cgGetNamedParameter(fragmentProgram, "floorRendering");
    cMapPix = cgGetNamedParameter(fragmentProgram, "cMap");
    floorPix = cgGetNamedParameter(fragmentProgram, "floor");

    v0 = cgGetNamedParameter(fragmentProgram, "quad0");
    v1 = cgGetNamedParameter(fragmentProgram, "quad1");
    v2 = cgGetNamedParameter(fragmentProgram, "quad2");
    v3 = cgGetNamedParameter(fragmentProgram, "quad3");



    return true;

}

void ShaderOneInterface::PerFrameInit() {

    //set parameters
    cgGLSetStateMatrixParameter(vertexModelViewProj, 
        CG_GL_MODELVIEW_PROJECTION_MATRIX, 
        CG_GL_MATRIX_IDENTITY);

#ifdef GEOMETRY_SUPPORT
    cgGLSetStateMatrixParameter(geometryModelViewProj, 
        CG_GL_MODELVIEW_PROJECTION_MATRIX, 
        CG_GL_MATRIX_IDENTITY);
#endif
    cgGLSetParameter3fv(eyePix, (float*)&(scene->ppc->C));
    
    cgGLSetTextureParameter(cMapPix, scene->eMap->ID);
    cgGLEnableTextureParameter(cMapPix);
    
    cgGLSetTextureParameter(floorPix, scene->tms[2].floorID);
    cgGLEnableTextureParameter(floorPix);
    
    
    cgGLSetParameter1f(renderingBG, scene->isRenderingBG);

    cgGLSetParameter1f(renderingFloor, scene->isRenderingFloor);

    cgGLSetParameter3fv(v0, (float *)&(scene->quad0));
    cgGLSetParameter3fv(v1, (float *)&(scene->quad1));
    cgGLSetParameter3fv(v2, (float *)&(scene->quad2));
    cgGLSetParameter3fv(v3, (float *)&(scene->quad3));




}

void ShaderOneInterface::PerFrameDisable() {
}


void ShaderOneInterface::BindPrograms() {

#ifdef GEOM_SHADER
    cgGLBindProgram( geometryProgram);
#endif
    cgGLBindProgram( vertexProgram);
    cgGLBindProgram( fragmentProgram);

}

void CGInterface::DisableProfiles() {

    cgGLDisableProfile(vertexCGprofile);
#ifdef GEOM_SHADER
    cgGLDisableProfile(geometryCGprofile);
#endif
    cgGLDisableProfile(pixelCGprofile);

}

void CGInterface::EnableProfiles() {

    cgGLEnableProfile(vertexCGprofile);
#ifdef GEOM_SHADER
    cgGLEnableProfile(geometryCGprofile);
#endif
    cgGLEnableProfile(pixelCGprofile);

}

