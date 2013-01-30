#include "tmesh.h"
#include "m33.h"
#include <float.h>
#include <fstream>

// implementation of triangle mesh class; see header file for functionality description

void TMesh::SetCube(V3 center, float sl) {

    vertsN = 8;
    verts = new V3[vertsN];
    trisN = 12;
    tris = new unsigned int[trisN*3];

    verts[0] = center + V3(-sl/2.0f, +sl/2.0f, +sl/2.0f);
    verts[1] = center + V3(-sl/2.0f, -sl/2.0f, +sl/2.0f);
    verts[2] = center + V3(+sl/2.0f, -sl/2.0f, +sl/2.0f);
    verts[3] = center + V3(+sl/2.0f, +sl/2.0f, +sl/2.0f);
    verts[4] = center + V3(-sl/2.0f, +sl/2.0f, -sl/2.0f);
    verts[5] = center + V3(-sl/2.0f, -sl/2.0f, -sl/2.0f);
    verts[6] = center + V3(+sl/2.0f, -sl/2.0f, -sl/2.0f);
    verts[7] = center + V3(+sl/2.0f, +sl/2.0f, -sl/2.0f);

    int tri = 0;
    tris[3*tri+0] = 0;
    tris[3*tri+1] = 1;
    tris[3*tri+2] = 2;
    tri++;
    tris[3*tri+0] = 2;
    tris[3*tri+1] = 3;
    tris[3*tri+2] = 0;
    tri++;
    tris[3*tri+0] = 3;
    tris[3*tri+1] = 2;
    tris[3*tri+2] = 6;
    tri++;
    tris[3*tri+0] = 6;
    tris[3*tri+1] = 7;
    tris[3*tri+2] = 3;
    tri++;
    tris[3*tri+0] = 7;
    tris[3*tri+1] = 6;
    tris[3*tri+2] = 5;
    tri++;
    tris[3*tri+0] = 5;
    tris[3*tri+1] = 4;
    tris[3*tri+2] = 7;
    tri++;
    tris[3*tri+0] = 4;
    tris[3*tri+1] = 5;
    tris[3*tri+2] = 1;
    tri++;
    tris[3*tri+0] = 1;
    tris[3*tri+1] = 0;
    tris[3*tri+2] = 4;
    tri++;
    tris[3*tri+0] = 4;
    tris[3*tri+1] = 0;
    tris[3*tri+2] = 3;
    tri++;
    tris[3*tri+0] = 3;
    tris[3*tri+1] = 7;
    tris[3*tri+2] = 4;
    tri++;
    tris[3*tri+0] = 1;
    tris[3*tri+1] = 5;
    tris[3*tri+2] = 6;
    tri++;
    tris[3*tri+0] = 6;
    tris[3*tri+1] = 2;
    tris[3*tri+2] = 1;
    tri++;

    cols = new V3[vertsN];
    for (int i = 0; i < 4; i++) {
        cols[i] = V3(1.0f, 0.0f, 0.0f);
        cols[4+i] = V3(0.0f, 0.0f, 0.0f);
    }


}

// SW rendering filled
void TMesh::Render(PPC *ppc, FrameBuffer *fb) {

    if (trisN == 0 && vertsN != 0) {
        RenderPoints(ppc, fb, 3);
        return;
    }

    V3 *projVerts = new V3[vertsN];
    for (int vi = 0; vi < vertsN; vi++) {
        projVerts[vi] = V3(FLT_MAX, FLT_MAX, FLT_MAX);
        ppc->Project(verts[vi], projVerts[vi]);
    }

    for (int tri = 0; tri < trisN; tri++) {
        if (tri && (tri%10000 == 0))
            cerr << tri << "\r         ";
        unsigned int vinds[3];
        vinds[0] = tris[3*tri+0];
        vinds[1] = tris[3*tri+1];
        vinds[2] = tris[3*tri+2];

        // rasterization setup
        // AABB setup
        AABB aabb(projVerts[vinds[0]]);
        aabb.AddPoint(projVerts[vinds[1]]);
        aabb.AddPoint(projVerts[vinds[2]]);
        if (!aabb.ClipWithScreen(fb->w, fb->h))
            continue;

        int top = (int) (.5f+aabb.corners[0][1]);
        int left = (int) (.5f+aabb.corners[0][0]);
        int bottom = (int) (-.5f+aabb.corners[1][1]);
        int right = (int) (-.5f+aabb.corners[1][0]);

        // edge equation setup
        V3 eeqs[3]; // eeqs[ei][0] is A of first edge, etc
        for (int ei = 0; ei < 3; ei++) {
            int _ei = (ei+1)%3;
            eeqs[ei][0] = projVerts[vinds[_ei]][1]-projVerts[vinds[ei]][1];
            eeqs[ei][1] = projVerts[vinds[ei]][0]-projVerts[vinds[_ei]][0];
            eeqs[ei][2] = projVerts[vinds[ei]][1]*(-eeqs[ei][1]) -
                projVerts[vinds[ei]][0]*eeqs[ei][0];
            int ei2 = (ei+2)%3;
            V3 v3(projVerts[vinds[ei2]][0], projVerts[vinds[ei2]][1], 1.0f);
            if (v3*eeqs[ei] < 0)
                eeqs[ei] = eeqs[ei]*-1.0f;
        }

        // screen space interpolation setup
        M33 ssii;
        ssii[0] = projVerts[vinds[0]]; ssii[0][2] = 1.0f;
        ssii[1] = projVerts[vinds[1]]; ssii[1][2] = 1.0f;
        ssii[2] = projVerts[vinds[2]]; ssii[2][2] = 1.0f;
        ssii = ssii.Invert();

        // color interpolation
        M33 tcols;
        // R0 G0 B0
        // R1 G1 B1
        // R2 G2 B2
        tcols[0] = cols[vinds[0]];
        tcols[1] = cols[vinds[1]];
        tcols[2] = cols[vinds[2]];
        M33 colABCs = ssii * tcols;
        colABCs.Transpose();

        // zbuffer interpolation
        V3 tzs(projVerts[vinds[0]][2], projVerts[vinds[1]][2], 
            projVerts[vinds[2]][2]);
        V3 zABC = ssii*tzs;

        // setup model space interpolation
        M33 camM;
        camM.SetColumn(0, ppc->a);
        camM.SetColumn(1, ppc->b);
        camM.SetColumn(2, ppc->c);
        M33 triM;
        triM.SetColumn(0, verts[vinds[0]] - ppc->C);
        triM.SetColumn(1, verts[vinds[1]] - ppc->C);
        triM.SetColumn(2, verts[vinds[2]] - ppc->C);
        M33 Q = triM.Invert() * camM;
        V3 DEF = Q[0] + Q[1] + Q[2];
        V3 ABCr = V3(
            tcols.GetColumn(0) * Q.GetColumn(0),
            tcols.GetColumn(0) * Q.GetColumn(1),
            tcols.GetColumn(0) * Q.GetColumn(2));
        V3 ABCg = V3(
            tcols.GetColumn(1) * Q.GetColumn(0),
            tcols.GetColumn(1) * Q.GetColumn(1),
            tcols.GetColumn(1) * Q.GetColumn(2));
        V3 ABCb = V3(
            tcols.GetColumn(2) * Q.GetColumn(0),
            tcols.GetColumn(2) * Q.GetColumn(1),
            tcols.GetColumn(2) * Q.GetColumn(2));

        V3 ABCs, ABCt;
        V3 ABCssis, ABCssit;
        if (tmap) {
            // texture coordinate model space interpolation
            V3 sv(tcs[vinds[0]][0], tcs[vinds[1]][0],  tcs[vinds[2]][0]);
            V3 tv(tcs[vinds[0]][1], tcs[vinds[1]][1],  tcs[vinds[2]][1]);
            ABCs = V3(
                sv * Q.GetColumn(0),
                sv * Q.GetColumn(1),
                sv * Q.GetColumn(2));
            ABCt = V3(
                tv * Q.GetColumn(0),
                tv * Q.GetColumn(1),
                tv * Q.GetColumn(2));
            // screen space interpolation of texture coordinates
            ABCssis = ssii*sv;
            ABCssit = ssii*tv;
        }


        // rasterization
        for (int v = top; v <= bottom; v++) {
            for (int u = left; u <= right; u++) {
                V3 pixv(.5f+(float)u, .5f+(float)v, 1.0f);
                if (pixv*eeqs[0] < 0.0f ||
                    pixv*eeqs[1] < 0.0f ||
                    pixv*eeqs[2] < 0.0f)
                    continue;
                float currz = zABC*pixv;
                if (!fb->CloserThenSet(V3(pixv[0], pixv[1], currz)))
                    continue;
                V3 currCol;
                if (!msi)
                    currCol = colABCs*pixv; // screen space interpolation
                else { // model space interpolation
                    currCol[0] = (ABCr*pixv) / (DEF*pixv);
                    currCol[1] = (ABCg*pixv) / (DEF*pixv);
                    currCol[2] = (ABCb*pixv) / (DEF*pixv);
                }
                if (tmap) {
                    float currs, currt;
                    if (msi) {
                        currs = (ABCs*pixv) / (DEF*pixv);
                        currt = (ABCt*pixv) / (DEF*pixv);
                    }
                    else {
                        currs = ABCssis*pixv;
                        currt = ABCssit*pixv;
                    }
                    currCol = tmap->LookUp(currs, currt);
                }
                fb->Set(u, v, currCol.GetColor());
            }
        }

    }

    if (trisN > 10000)
        cerr << endl;
    delete []projVerts;

}


// render SW wireframe
void TMesh::RenderWF(PPC *ppc, FrameBuffer *fb) {

    V3 *projVerts = new V3[vertsN];
    for (int vi = 0; vi < vertsN; vi++) {
        projVerts[vi] = V3(FLT_MAX, FLT_MAX, FLT_MAX);
        ppc->Project(verts[vi], projVerts[vi]);
    }

    for (int tri = 0; tri < trisN; tri++) {
        unsigned int vinds[3];
        vinds[0] = tris[3*tri+0];
        vinds[1] = tris[3*tri+1];
        vinds[2] = tris[3*tri+2];
        for (int ei = 0; ei < 3; ei++) {
            fb->DrawSegment(projVerts[vinds[ei]],
                projVerts[vinds[(ei+1)%3]],
                cols[vinds[ei]], cols[vinds[(ei+1)%3]]);
        }
    }

    delete []projVerts;

}


void TMesh::RotateAboutArbitraryAxis(V3 aO, 
    V3 aDir, float theta) {

        for (int vi = 0; vi < vertsN; vi++) {
            verts[vi] = verts[vi].RotatePointAboutArbitraryAxis(aO, aDir, theta);
            if (normals) {
                normals[vi] = normals[vi].
                    RotateDirectionAboutArbitraryAxis(aDir, theta);
            }
        }


}

V3 TMesh::GetCenter() {

    V3 ret(0.0f, 0.0f, 0.0f);
    for (int vi = 0; vi < vertsN; vi++) {
        ret = ret + verts[vi];
    }
    return ret/(float)vertsN;

}

void TMesh::Load(char *fname) {

    ifstream ifs(fname, ios::binary);
    if (ifs.fail()) {
        cerr << "INFO: cannot open file: " << fname << endl;
        return;
    }

    ifs.read((char*)&vertsN, sizeof(int));
    char yn;
    ifs.read(&yn, 1); // always xyz
    if (yn != 'y') {
        cerr << "INTERNAL ERROR: there should always be vertex xyz data" << endl;
        return;
    }
    if (verts)
        delete verts;
    verts = new V3[vertsN];

    ifs.read(&yn, 1); // cols 3 floats
    if (cols)
        delete cols;
    cols = 0;
    if (yn == 'y') {
        cols = new V3[vertsN];
    }

    ifs.read(&yn, 1); // normals 3 floats
    if (normals)
        delete normals;
    normals = 0;
    if (yn == 'y') {
        normals = new V3[vertsN];
    }

    ifs.read(&yn, 1); // texture coordinates 2 floats
    float *tcs = 0; // don't have texture coordinates for now
    if (tcs)
        delete tcs;
    tcs = 0;
    if (yn == 'y') {
        tcs = new float[vertsN*2];
    }

    ifs.read((char*)verts, vertsN*3*sizeof(float)); // load verts

    if (cols) {
        ifs.read((char*)cols, vertsN*3*sizeof(float)); // load cols
    }

    if (normals)
        ifs.read((char*)normals, vertsN*3*sizeof(float)); // load normals

    if (tcs)
        ifs.read((char*)tcs, vertsN*2*sizeof(float)); // load texture coordinates

    ifs.read((char*)&trisN, sizeof(int));
    if (tris)
        delete tris;
    tris = new unsigned int[trisN*3];
    ifs.read((char*)tris, trisN*3*sizeof(unsigned int)); // read tiangles

    ifs.close();

    cerr << "INFO: loaded " << vertsN << " verts, " << trisN << " tris from " << endl << "      " << fname << endl;
    cerr << "      xyz " << ((cols) ? "rgb " : "") << ((normals) ? "nxnynz " : "") << ((tcs) ? "tcstct " : "") << endl;

}

void TMesh::Translate(V3 transv) {

    for (int vi = 0; vi < vertsN; vi++) {
        verts[vi] = verts[vi] + transv;
    }

}

void TMesh::ScaleAboutCenter(float scalef) {

    V3 center = GetCenter();
    Translate(center*-1.0f);

    Scale(scalef);

    Translate(center);

}

void TMesh::Scale(float scalef) {

    for (int vi = 0; vi < vertsN; vi++) {
        verts[vi] = verts[vi] * scalef;
    }

}

AABB TMesh::GetAABB() {

    AABB ret(verts[0]);
    for (int vi = 0; vi < vertsN; vi++) {
        ret.AddPoint(verts[vi]);
    }
    return ret;

}

void TMesh::SetFromFB(FrameBuffer *fb, PPC *ppc) {

    vertsN = fb->w*fb->h;
    verts = new V3[vertsN];
    cols = new V3[vertsN];
    int vi = 0;
    for (int v = 0; v < fb->h; v++) {
        for (int u = 0; u < fb->w; u++) {
            int uv = (fb->h-1-v)*fb->w+u;
            if (fb->zb[uv] == 0.0f)
                continue;
            V3 projP(.5f+(float)u, .5f+(float)v, fb->zb[uv]);
            verts[vi] = ppc->GetPoint(projP);
            cols[vi] = fb->Getv(u, v);
            vi++;
        }
    }

    vertsN = vi;

    trisN = 0;



}

void TMesh::RenderPoints(PPC *ppc, FrameBuffer *fb, int pSize) {

    for (int vi = 0; vi < vertsN; vi++) {
        V3 projV;
        if (!ppc->Project(verts[vi], projV))
            continue;
        for (int i = 0; i < pSize; i++) {
            for (int j = 0; j < pSize; j++) {
                V3 currP = projV - 
                    V3((float)pSize/2, (float)pSize/2, 0.0f) +
                    V3((float)i, (float)j, 0.0f);
                if (!fb->CloserThenSet(currP))
                    continue;
                fb->Set(currP, cols[vi].GetColor());
            }
        }
    }

}

void TMesh::SetRectangle(float wf, float hf, FrameBuffer *_tmap) {


    vertsN = 4;
    verts = new V3[vertsN];
    cols = new V3[vertsN];
    tcs = new V3[vertsN];

    trisN = 2;
    tris = new unsigned int[trisN*3];

    verts[0] = V3(-wf/2.0f, hf/2.0f, 0.0f);
    verts[1] = V3(-wf/2.0f, -hf/2.0f, 0.0f);
    verts[2] = V3(wf/2.0f, -hf/2.0f, 0.0f);
    verts[3] = V3(wf/2.0f, hf/2.0f, 0.0f);

    cols[0] = cols[1] = cols[2] = cols[3] = V3(0.0f, 0.0f, 0.0f);

    tris[0] = 0;
    tris[1] = 1;
    tris[2] = 2;

    tris[3] = 2;
    tris[4] = 3;
    tris[5] = 0;

    if (_tmap) {
        tcs[0] = V3(0.0f, 0.0f, 1.0f);
        tcs[1] = V3(0.0f, 1.0f, 1.0f);
        tcs[2] = V3(1.0f, 1.0f, 1.0f);
        tcs[3] = V3(1.0f, 0.0f, 1.0f);
        tmap = _tmap;
    }



}

void TMesh::SetFloor() {

    vertsN = 4; 
    verts = new V3[vertsN];
    cols = new V3[vertsN];
    tcs = new V3[vertsN];

    float wf, hf, down;

    wf = 1000.0f;
    hf = 1000.0f;
    down = -10.0f;

    trisN = 2;
    tris = new unsigned int[trisN*3];

    verts[0] = V3(-wf/2.0f, down, hf/2.0f);
    verts[1] = V3(wf/2.0f, down, hf/2.0f);
    verts[2] = V3(wf/2.0f, down, -hf/2.0f);
    verts[3] = V3(-wf/2.0f, down, -hf/2.0f);

    cols[0] = cols[1] = cols[2] = cols[3] = V3(0.0f, 0.0f, 0.0f);

    tris[0] = 0;
    tris[1] = 1;
    tris[2] = 2;

    tris[3] = 2;
    tris[4] = 3;
    tris[5] = 0;

    float mult = 100.0f;

    float * texCoords = new float [2*vertsN];
    texCoords[0] = mult * 0.0f; texCoords[1] = mult * 0.0f;
    texCoords[2] = mult * 0.0f; texCoords[3] = mult * 1.0f;
    texCoords[3] = mult * 1.0f; texCoords[4] = mult * 1.0f;
    texCoords[5] = mult * 1.0f; texCoords[6] = mult * 0.0f;

    floorIsTextured = 1;
}

void TMesh::RenderHW() {

    if (renderWF) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    // specify vertices
    glEnableClientState(GL_VERTEX_ARRAY); // "there's going to be an array of vertices"
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, verts); // "here's the pointer to the array of vertices"
    glColorPointer(3, GL_FLOAT, 0, cols);
    glNormalPointer(GL_FLOAT, 0, normals);

    // sepcify additional vertex attributes (e.g. texture coordinates, etc)

    // actually draw mesh by providing connectivity data
    glDrawElements(GL_TRIANGLES, 3*trisN, GL_UNSIGNED_INT, tris);

    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);



    if (renderWF) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }


}