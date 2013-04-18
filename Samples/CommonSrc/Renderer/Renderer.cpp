/************************************************************************************

Filename    :   OVR_Renderer.cpp
Content     :   Platform renderer for simple scene graph - implementation
Created     :   September 6, 2012
Authors     :   Andrew Reisse

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus LLC license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#include "../Renderer/Renderer.h"
#include "../Renderer/Font.h"

#include "Kernel/OVR_Log.h"

namespace OVR { namespace Render {

void Model::Render(const Matrix4f& ltw, Renderer* ren)
{
    if (Visible)
    {
        Matrix4f m = ltw * GetMatrix();
        ren->Render(m, this);
    }
}

void Container::Render(const Matrix4f& ltw, Renderer* ren)
{
    Matrix4f m = ltw * GetMatrix();
    for (unsigned i = 0; i < Nodes.GetSize(); i++)
        Nodes[i]->Render(m, ren);
}

Matrix4f SceneView::GetViewMatrix() const
{
    Matrix4f view = Matrix4f(GetOrientation().Conj()) * Matrix4f::Translation(GetPosition());
    return view;
}

void Scene::Render(Renderer* ren, const Matrix4f& view)
{
    for (int i = 0; i < Lighting.LightCount; i++)
    {
        Lighting.LightPos[i] = view.Transform(LightPos[i]);
    }
    if (!LightingBuffer)
        LightingBuffer = *ren->CreateBuffer();

    LightingBuffer->Data(Buffer_Uniform, &Lighting, sizeof(Lighting));
    ren->SetCommonUniformBuffer(1, LightingBuffer);

    World.Render(view, ren);
}



UInt16 CubeIndices[] =
{    
    0,1,3,
    3,1,2,

    5,4,6,
    6,4,7,

    8,9,11,
    11,9,10,

    13,12,14,
    14,12,15,

    16,17,19,
    19,17,18,

    21,20,22,
    22,20,23
};

// Colors are specified for planes perpendicular to the axis
// For example, "xColor" is the color of the y-z plane
Model* Model::CreateAxisFaceColorBox(float x1, float x2, Color xcolor,
                                     float y1, float y2, Color ycolor,
                                     float z1, float z2, Color zcolor)
{
    float t;

    if (x1 > x2)  { t = x1; x1 = x2; x2 = t; }
    if (y1 > y2)  { t = y1; y1 = y2; y2 = t; }
    if (z1 > z2)  { t = z1; z1 = z2; z2 = t; }

    Model* box = new Model();

    UInt16 startIndex = 0;
    // Cube
    startIndex =
    box->AddVertex(Vector3f(x1, y2, z1), ycolor );
    box->AddVertex(Vector3f(x2, y2, z1), ycolor );
    box->AddVertex(Vector3f(x2, y2, z2), ycolor );
    box->AddVertex(Vector3f(x1, y2, z2), ycolor );

    box->AddVertex(Vector3f(x1, y1, z1), ycolor );
    box->AddVertex(Vector3f(x2, y1, z1), ycolor );
    box->AddVertex(Vector3f(x2, y1, z2), ycolor );
    box->AddVertex(Vector3f(x1, y1, z2), ycolor );

    box->AddVertex(Vector3f(x1, y1, z2), xcolor );
    box->AddVertex(Vector3f(x1, y1, z1), xcolor );
    box->AddVertex(Vector3f(x1, y2, z1), xcolor );
    box->AddVertex(Vector3f(x1, y2, z2), xcolor );

    box->AddVertex(Vector3f(x2, y1, z2), xcolor );
    box->AddVertex(Vector3f(x2, y1, z1), xcolor );
    box->AddVertex(Vector3f(x2, y2, z1), xcolor );
    box->AddVertex(Vector3f(x2, y2, z2), xcolor );

    box->AddVertex(Vector3f(x1, y1, z1), zcolor );
    box->AddVertex(Vector3f(x2, y1, z1), zcolor );
    box->AddVertex(Vector3f(x2, y2, z1), zcolor );
    box->AddVertex(Vector3f(x1, y2, z1), zcolor );

    box->AddVertex(Vector3f(x1, y1, z2), zcolor );
    box->AddVertex(Vector3f(x2, y1, z2), zcolor );
    box->AddVertex(Vector3f(x2, y2, z2), zcolor );
    box->AddVertex(Vector3f(x1, y2, z2), zcolor );


    enum {
      //  CubeVertexCount = sizeof(CubeVertices)/sizeof(CubeVertices[0]),
        CubeIndexCount  = sizeof(CubeIndices)/sizeof(CubeIndices[0])
    };

    // Renumber indices
    for(int i=0; i < CubeIndexCount/3; i++)
    {
        box->AddTriangle(CubeIndices[i*3] + startIndex,
                         CubeIndices[i*3+1] + startIndex,
                         CubeIndices[i*3+2] + startIndex);
    }

    return box;
}

void Model::AddSolidColorBox(float x1, float y1, float z1,
                             float x2, float y2, float z2,
                             Color c)
{
    float t;

    if (x1 > x2)  { t = x1; x1 = x2; x2 = t; }
    if (y1 > y2)  { t = y1; y1 = y2; y2 = t; }
    if (z1 > z2)  { t = z1; z1 = z2; z2 = t; }

    // Cube vertices and their normals.
    Vector3f CubeVertices[][3] = {
        Vector3f( x1, y2, z1 ), Vector3f(z1,x1), Vector3f( 0.0f, 1.0f, 0.0f ),
        Vector3f( x2, y2, z1 ), Vector3f(z1,x2), Vector3f( 0.0f, 1.0f, 0.0f ),
        Vector3f( x2, y2, z2 ), Vector3f(z2,x2), Vector3f( 0.0f, 1.0f, 0.0f ),
        Vector3f( x1, y2, z2 ), Vector3f(z2,x1), Vector3f( 0.0f, 1.0f, 0.0f ),

        Vector3f( x1, y1, z1 ), Vector3f(z1,x1), Vector3f( 0.0f, -1.0f, 0.0f ),
        Vector3f( x2, y1, z1 ), Vector3f(z1,x2), Vector3f( 0.0f, -1.0f, 0.0f ),
        Vector3f( x2, y1, z2 ), Vector3f(z2,x2), Vector3f( 0.0f, -1.0f, 0.0f ),
        Vector3f( x1, y1, z2 ), Vector3f(z2,x1), Vector3f( 0.0f, -1.0f, 0.0f ),

        Vector3f( x1, y1, z2 ), Vector3f(z2,y1), Vector3f( -1.0f, 0.0f, 0.0f ),
        Vector3f( x1, y1, z1 ), Vector3f(z1,y1), Vector3f( -1.0f, 0.0f, 0.0f ),
        Vector3f( x1, y2, z1 ), Vector3f(z1,y2), Vector3f( -1.0f, 0.0f, 0.0f ),
        Vector3f( x1, y2, z2 ), Vector3f(z2,y2), Vector3f( -1.0f, 0.0f, 0.0f ),

        Vector3f( x2, y1, z2 ), Vector3f(z2,y1), Vector3f( 1.0f, 0.0f, 0.0f ),
        Vector3f( x2, y1, z1 ), Vector3f(z1,y1), Vector3f( 1.0f, 0.0f, 0.0f ),
        Vector3f( x2, y2, z1 ), Vector3f(z1,y2), Vector3f( 1.0f, 0.0f, 0.0f ),
        Vector3f( x2, y2, z2 ), Vector3f(z2,y2), Vector3f( 1.0f, 0.0f, 0.0f ),

        Vector3f( x1, y1, z1 ), Vector3f(x1,y1), Vector3f( 0.0f, 0.0f, -1.0f ),
        Vector3f( x2, y1, z1 ), Vector3f(x2,y1), Vector3f( 0.0f, 0.0f, -1.0f ),
        Vector3f( x2, y2, z1 ), Vector3f(x2,y2), Vector3f( 0.0f, 0.0f, -1.0f ),
        Vector3f( x1, y2, z1 ), Vector3f(x1,y2), Vector3f( 0.0f, 0.0f, -1.0f ),

        Vector3f( x1, y1, z2 ), Vector3f(x1,y1), Vector3f( 0.0f, 0.0f, 1.0f ),
        Vector3f( x2, y1, z2 ), Vector3f(x2,y1), Vector3f( 0.0f, 0.0f, 1.0f ),
        Vector3f( x2, y2, z2 ), Vector3f(x2,y2), Vector3f( 0.0f, 0.0f, 1.0f ),
        Vector3f( x1, y2, z2 ), Vector3f(x1,y2), Vector3f( 0.0f, 0.0f, 1.0f )
    };


    UInt16 startIndex = GetNextVertexIndex();

    enum {
        CubeVertexCount = sizeof(CubeVertices)/sizeof(CubeVertices[0]),
        CubeIndexCount  = sizeof(CubeIndices)/sizeof(CubeIndices[0])
    };

    for(int v=0; v < CubeVertexCount; v++)
    {
        AddVertex(Vertex(CubeVertices[v][0], c, CubeVertices[v][1].x, CubeVertices[v][1].y, CubeVertices[v][2]));
    }

    // Renumber indices
    for(int i=0; i < CubeIndexCount/3; i++)
    {
        AddTriangle(CubeIndices[i*3] + startIndex,
                    CubeIndices[i*3+1] + startIndex,
                    CubeIndices[i*3+2] + startIndex);
    }    
}



Model* Model::CreateBox(Color c, Vector3f origin, Vector3f size)
{
    Model *box = new Model();
    Vector3f s = size * 0.5f;

    box->AddVertex(-s.x,  s.y, -s.z,  c, 0, 1, 0,0,-1);
    box->AddVertex( s.x,  s.y, -s.z,  c, 1, 1, 0,0,-1);
    box->AddVertex( s.x, -s.y, -s.z,  c, 1, 0, 0,0,-1);
    box->AddVertex(-s.x, -s.y, -s.z,  c, 0, 0, 0,0,-1);
    box->AddTriangle(2, 1, 0);
    box->AddTriangle(0, 3, 2);

    box->AddVertex( s.x,  s.y,  s.z,  c, 1, 1, 0,0,1);
    box->AddVertex(-s.x,  s.y,  s.z,  c, 0, 1, 0,0,1);
    box->AddVertex(-s.x, -s.y,  s.z,  c, 0, 0, 0,0,1);
    box->AddVertex( s.x, -s.y,  s.z,  c, 1, 0, 0,0,1);
    box->AddTriangle(6, 5, 4);
    box->AddTriangle(4, 7, 6);
 
    box->AddVertex(-s.x,  s.y, -s.z,  c, 1, 0, -1,0,0);
    box->AddVertex(-s.x,  s.y,  s.z,  c, 1, 1, -1,0,0);
    box->AddVertex(-s.x, -s.y,  s.z,  c, 0, 1, -1,0,0);
    box->AddVertex(-s.x, -s.y, -s.z,  c, 0, 0, -1,0,0);
    box->AddTriangle(10, 11, 8);
    box->AddTriangle(8, 9, 10);

    box->AddVertex( s.x,  s.y, -s.z,  c, 1, 0, 1,0,0);
    box->AddVertex( s.x, -s.y, -s.z,  c, 0, 0, 1,0,0);
    box->AddVertex( s.x, -s.y,  s.z,  c, 0, 1, 1,0,0);
    box->AddVertex( s.x,  s.y,  s.z,  c, 1, 1, 1,0,0);
    box->AddTriangle(14, 15, 12);
    box->AddTriangle(12, 13, 14);

    box->AddVertex(-s.x, -s.y,  s.z,  c, 0, 1, 0,-1,0);
    box->AddVertex( s.x, -s.y,  s.z,  c, 1, 1, 0,-1,0);
    box->AddVertex( s.x, -s.y, -s.z,  c, 1, 0, 0,-1,0);
    box->AddVertex(-s.x, -s.y, -s.z,  c, 0, 0, 0,-1,0);
    box->AddTriangle(18, 19, 16);
    box->AddTriangle(16, 17, 18);

    box->AddVertex(-s.x,  s.y, -s.z,  c, 0, 0, 0,1,0);
    box->AddVertex( s.x,  s.y, -s.z,  c, 1, 0, 0,1,0);
    box->AddVertex( s.x,  s.y,  s.z,  c, 1, 1, 0,1,0);
    box->AddVertex(-s.x,  s.y,  s.z,  c, 0, 1, 0,1,0);
    box->AddTriangle(20, 21, 22);
    box->AddTriangle(22, 23, 20);
    
    box->SetPosition(origin);
    return box;
}

Model* Model::CreateCylinder(Color c, Vector3f origin, float height, float radius, int sides)
{
    Model *cyl = new Model();
    float halfht = height*0.5f;
    for (UInt16 i = 0; i < sides; i++)
    {
        float x = cosf(3.1415926536f * 2.0f * i / float(sides));
        float y = sinf(3.1415926536f * 2.0f * i / float(sides));
        cyl->AddVertex(radius * x, radius * y, -halfht, c, x+1, y, 0,0,-1);
        cyl->AddVertex(radius * x, radius * y,  halfht, c, x, y,   0,0,1);

        UInt16 j = 0;
        if (i < sides-1)
        {
            j = i+1;
            cyl->AddTriangle(0,     i*4,   i*4+4);
            cyl->AddTriangle(i*4+5, i*4+1, 1);
        }

        float nx = cosf(3.1415926536f * (0.5f + 2.0f * i / float(sides)));
        float ny = sinf(3.1415926536f * (0.5f + 2.0f * i / float(sides)));
        cyl->AddVertex(radius * x, radius * y, -halfht, c, x+1, y, nx, ny, 0);
        cyl->AddVertex(radius * x, radius * y,  halfht, c, x, y,   nx, ny, 0);

        cyl->AddTriangle(j*4+2, i*4+3, i*4+3);
        cyl->AddTriangle(i*4+3, j*4+2, j*4+2);
    }

    cyl->SetPosition(origin);
    return cyl;
}

Model* Model::CreateGrid(Vector3f origin, Vector3f stepx, Vector3f stepy,
                         int halfx, int halfy, int nmajor, Color minor, Color major)
{
    Model* grid = new Model(Prim_Lines);
    float  halfxf = (float)halfx;
    float  halfyf = (float)halfy;

    for (int jn = 0; jn <= halfy; jn++)
    {
        float j = (float)jn;

        grid->AddLine(grid->AddVertex((stepx * -halfxf) + (stepy *  j), (jn % nmajor) ? minor : major, 0, 0.5f),
                      grid->AddVertex((stepx *  halfxf) + (stepy *  j), (jn % nmajor) ? minor : major, 1, 0.5f));

        if (j)
            grid->AddLine(grid->AddVertex((stepx * -halfxf) + (stepy * -j), (jn % nmajor) ? minor : major, 0, 0.5f),
                          grid->AddVertex((stepx *  halfxf) + (stepy * -j), (jn % nmajor) ? minor : major, 1, 0.5f));
    }

    for (int in = 0; in <= halfx; in++)
    {
        float i = (float)in;

        grid->AddLine(grid->AddVertex((stepx *  i) + (stepy * -halfyf), (in % nmajor) ? minor : major, 0, 0.5f),
                      grid->AddVertex((stepx *  i) + (stepy *  halfyf), (in % nmajor) ? minor : major, 1, 0.5f));

        if (i)
            grid->AddLine(grid->AddVertex((stepx * -i) + (stepy * -halfyf), (in % nmajor) ? minor : major, 0, 0.5f),
                          grid->AddVertex((stepx * -i) + (stepy *  halfyf), (in % nmajor) ? minor : major, 1, 0.5f));
    }

    grid->SetPosition(origin);
    return grid;
}


//-------------------------------------------------------------------------------------


void ShaderFill::Set(PrimitiveType prim) const
{
    Shaders->Set(prim);
    for (int i = 0; i < 8; i++)
        if (Textures[i])
            Textures[i]->Set(i);
}



//-------------------------------------------------------------------------------------
// ***** Rendering


Renderer::Renderer()
    : CurPostProcess(PostProcess_None),
      SceneColorTexW(0), SceneColorTexH(0),
      SceneRenderScale(1),
      
      Distortion(1.0f, 0.18f, 0.115f),            
      DistortionClearColor(0,0,0)
{
}

Fill* Renderer::CreateTextureFill(Render::Texture* t, bool useAlpha)
{
    ShaderSet* shaders = CreateShaderSet();
    shaders->SetShader(LoadBuiltinShader(Shader_Vertex, VShader_MVP));
    shaders->SetShader(LoadBuiltinShader(Shader_Fragment, useAlpha ? FShader_AlphaTexture : FShader_Texture));
    Fill* f = new ShaderFill(*shaders);
    f->SetTexture(0, t);
    return f;
}

float Renderer::MeasureText(const Font* font, const char* str, float size, float* strsize)
{
    UPInt length = strlen(str);
    float w  = 0;
    float xp = 0;
    float yp = 0;
    
    for (UPInt i = 0; i < length; i++)
    {
        if (str[i] == '\n')
        {
            yp += font->lineheight;
            if (xp > w)
                w = xp;
            xp = 0;
            continue;
        }

        // Tab followed by a numbers sets position to specified offset.
        if (str[i] == '\t')
        {
            char *p = 0;
            float tabPixels = (float)OVR_strtoq(str+i+1, &p, 10);
            i += p-(str+i+1);
            xp = tabPixels;
        }
        else
        {
            const Font::Char* ch = &font->chars[str[i]];
            xp += ch->advance;
        }
    }
    
    if (xp > w)
        w = xp;

    if (strsize)
    {
        strsize[0] = (size/font->lineheight) * w;
        strsize[1] = (size/font->lineheight) * (yp + font->lineheight);
    }
    return (size/font->lineheight) * w;
}

void Renderer::RenderText(const Font* font, const char* str,
                          float x, float y, float size, Color c)
{
    if (!pTextVertexBuffer)
    {
        pTextVertexBuffer = *CreateBuffer();
        if (!pTextVertexBuffer)
            return;
    }

    if (!font->fill)
    {
        font->fill = CreateTextureFill(Ptr<Texture>(
            *CreateTexture(Texture_R, font->twidth, font->theight, font->tex)), true);
    }

    UPInt length = strlen(str);

    pTextVertexBuffer->Data(Buffer_Vertex, NULL, length * 6 * sizeof(Vertex));
    Vertex* vertices = (Vertex*)pTextVertexBuffer->Map(0, length * 6 * sizeof(Vertex), Map_Discard);
    if (!vertices)
        return;

    Matrix4f m = Matrix4f(size / font->lineheight, 0, 0,0,
                          0, size / font->lineheight, 0,0,
                          0, 0, 0, 0,
                          x, y, 0, 1).Transposed();

    float xp = 0, yp = (float)font->ascent;
    int   ivertex = 0;

    for (UPInt i = 0; i < length; i++)
    {
        if (str[i] == '\n')
        {
            yp += font->lineheight;
            xp = 0;
            continue;
        }
        // Tab followed by a numbers sets position to specified offset.
        if (str[i] == '\t')
        {
            char *p =  0;
            float tabPixels = (float)OVR_strtoq(str+i+1, &p, 10);
            i += p-(str+i+1);            
            xp = tabPixels;
            continue;
        }

        const Font::Char* ch = &font->chars[str[i]];
        Vertex* chv = &vertices[ivertex];
        for (int j = 0; j < 6; j++)
            chv[j].C = c;
        float x = xp + ch->x;
        float y = yp - ch->y;
        float cx = font->twidth * (ch->u2-ch->u1);
        float cy = font->theight * (ch->v2-ch->v1);
        chv[0] = Vertex(Vector3f(x, y, 0), c, ch->u1, ch->v1);
        chv[1] = Vertex(Vector3f(x+cx, y, 0), c, ch->u2, ch->v1);
        chv[2] = Vertex(Vector3f(x+cx, cy+y, 0), c, ch->u2, ch->v2);
        chv[3] = Vertex(Vector3f(x, y, 0), c, ch->u1, ch->v1);
        chv[4] = Vertex(Vector3f(x+cx, cy+y, 0), c, ch->u2, ch->v2);
        chv[5] = Vertex(Vector3f(x, y+cy, 0), c, ch->u1, ch->v2);
        ivertex+=6;

        xp += ch->advance;
    }

    pTextVertexBuffer->Unmap(vertices);

    Render(font->fill, pTextVertexBuffer, NULL, m, 0, ivertex, Prim_Triangles);
}

void Renderer::FillRect(float left, float top, float right, float bottom, Color c)
{
    if (!pTextVertexBuffer)
    {
        pTextVertexBuffer = *CreateBuffer();
        if (!pTextVertexBuffer)
            return;
    }

    // Get!!
    Fill* fill = CreateSimpleFill();

    pTextVertexBuffer->Data(Buffer_Vertex, NULL, 6 * sizeof(Vertex));
    Vertex* vertices = (Vertex*)pTextVertexBuffer->Map(0, 6 * sizeof(Vertex), Map_Discard);
    if (!vertices)
        return;

    vertices[0] = Vertex(Vector3f(left,  top, 0.0f),    c);
    vertices[1] = Vertex(Vector3f(right, top, 0),    c);
    vertices[2] = Vertex(Vector3f(left,  bottom, 0), c);
    vertices[3] = Vertex(Vector3f(left,  bottom, 0), c);
    vertices[4] = Vertex(Vector3f(right, top, 0),    c);
    vertices[5] = Vertex(Vector3f(right, bottom, 0), c);

    pTextVertexBuffer->Unmap(vertices);

    Render(fill, pTextVertexBuffer, NULL, Matrix4f(), 0, 6, Prim_Triangles);
}

/*
void Renderer::GetStereoViewports(const Viewport& in, Viewport* out) const
{
    out[0] = out[1] = in;
    if (StereoBufferMode == Stereo_SplitH)
    {
        out[0].w = out[1].w = in.w / 2;
        out[1].x += WindowWidth / 2;
    }
}
*/

void Renderer::SetSceneRenderScale(float ss)
{
    SceneRenderScale = ss;
    pSceneColorTex = NULL;
}

void Renderer::SetViewport(const Viewport& vp)
{
    VP = vp;

    if (CurPostProcess == PostProcess_Distortion)
    {
        Viewport svp = vp;
        svp.w = (int)ceil(SceneRenderScale * vp.w);
        svp.h = (int)ceil(SceneRenderScale * vp.h);
        svp.x = (int)ceil(SceneRenderScale * vp.x);
        svp.y = (int)ceil(SceneRenderScale * vp.y);
        SetRealViewport(svp);
    }
    else
        SetRealViewport(vp);
}


bool Renderer::initPostProcessSupport(PostProcessType pptype)
{
    if (pptype != PostProcess_Distortion)
        return true;

    int texw = (int)ceil(SceneRenderScale * WindowWidth),
        texh = (int)ceil(SceneRenderScale * WindowHeight);

    // If pSceneColorTex is already created and is of correct size, we are done.
    // It's important to check width/height in case window size changed.
    if (pSceneColorTex && (texw == SceneColorTexW) && (texh == SceneColorTexH))
        return true;

    pSceneColorTex = *CreateTexture(Texture_RGBA|Texture_RenderTarget|Params.Multisample,
                                    texw, texh, NULL);
    if (!pSceneColorTex)
        return false;
    SceneColorTexW = texw;
    SceneColorTexH = texh;
    pSceneColorTex->SetSampleMode(Sample_ClampBorder|Sample_Linear);

    if (!pPostProcessShader)
    {
        Shader *vs   = LoadBuiltinShader(Shader_Vertex, VShader_PostProcess);
        Shader *ppfs = LoadBuiltinShader(Shader_Fragment, FShader_PostProcess);
        pPostProcessShader = *CreateShaderSet();
        pPostProcessShader->SetShader(vs);
        pPostProcessShader->SetShader(ppfs);
    }

    if (!pFullScreenVertexBuffer)
    {
        pFullScreenVertexBuffer = *CreateBuffer();
        const Render::Vertex QuadVertices[] =
        {
            Vertex(Vector3f(0,1,0), Color(1,1,1,1), 0,0),
            Vertex(Vector3f(1,1,0), Color(1,1,1,1), 1,0),
            Vertex(Vector3f(0,0,0), Color(1,1,1,1), 0,1),
            Vertex(Vector3f(1,0,0), Color(1,1,1,1), 1,1)
        };
        pFullScreenVertexBuffer->Data(Buffer_Vertex, QuadVertices, sizeof(QuadVertices));
    }
    return true;
}

void Renderer::SetProjection(const Matrix4f& proj)
{
    Proj = proj;
    SetWorldUniforms(proj);
}

void Renderer::BeginScene(PostProcessType pptype)
{
    BeginRendering();

    if ((pptype != PostProcess_None) && initPostProcessSupport(pptype))
        CurPostProcess = pptype;
    else
        CurPostProcess = PostProcess_None;

    if (CurPostProcess == PostProcess_Distortion)
    {
        SetRenderTarget(pSceneColorTex);
        SetViewport(VP);
    }
    else
    {
        SetRenderTarget(0);
    }

    SetWorldUniforms(Proj);
    SetExtraShaders(NULL);
}

void Renderer::FinishScene()
{
    SetExtraShaders(0);
    if (CurPostProcess == PostProcess_None)
        return;

    SetRenderTarget(0);
    SetRealViewport(VP);
    FinishScene1();

    CurPostProcess = PostProcess_None;
}



void Renderer::FinishScene1()
{
    float r, g, b, a;
    DistortionClearColor.GetRGBA(&r, &g, &b, &a);    
    Clear(r, g, b, a);

    float w = float(VP.w) / float(WindowWidth),
          h = float(VP.h) / float(WindowHeight),
          x = float(VP.x) / float(WindowWidth),
          y = float(VP.y) / float(WindowHeight);

    float as = float(VP.w) / float(VP.h);

    // We are using 1/4 of DistortionCenter offset value here, since it is
    // relative to [-1,1] range that gets mapped to [0, 0.5].
    pPostProcessShader->SetUniform2f("LensCenter",
                                     x + (w + Distortion.XCenterOffset * 0.5f)*0.5f, y + h*0.5f);
    pPostProcessShader->SetUniform2f("ScreenCenter", x + w*0.5f, y + h*0.5f);

    // MA: This is more correct but we would need higher-res texture vertically; we should adopt this
    // once we have asymmetric input texture scale.
    float scaleFactor = 1.0f / Distortion.Scale;

    pPostProcessShader->SetUniform2f("Scale",   (w/2) * scaleFactor, (h/2) * scaleFactor * as);
    pPostProcessShader->SetUniform2f("ScaleIn", (2/w),               (2/h) / as);

    pPostProcessShader->SetUniform4f("HmdWarpParam",
                                     Distortion.K[0], Distortion.K[1], Distortion.K[2], Distortion.K[3]);
    Matrix4f texm(w, 0, 0, x,
                  0, h, 0, y,
                  0, 0, 0, 0,   
                  0, 0, 0, 1);
    pPostProcessShader->SetUniform4x4f("Texm", texm);

    Matrix4f view(2, 0, 0,-1,
                  0, 2, 0,-1,
                  0, 0, 0, 0,
                  0, 0, 0, 1);

    ShaderFill fill(pPostProcessShader);
    fill.SetTexture(0, pSceneColorTex);
    Render(&fill, pFullScreenVertexBuffer, NULL, view, 0, 4, Prim_TriangleStrip);
}


int GetNumMipLevels(int w, int h)
{
    int n = 1;
    while (w > 1 || h > 1)
    {
        w >>= 1;
        h >>= 1;
        n++;
    }
    return n;
}

void FilterRgba2x2(const UByte* src, int w, int h, UByte* dest)
{
    for (int j = 0; j < h; j+=2)
    {
        const UByte* psrc = src+(w*j*4);
        UByte*       pdest = dest+((w>>1)*(j>>1)*4);

        for (int i = 0; i < w>>1; i++, psrc+=8, pdest+=4)
        {
            pdest[0] = (((int)psrc[0]) + psrc[4] + psrc[w*4+0] + psrc[w*4+4]) >> 2;
            pdest[1] = (((int)psrc[1]) + psrc[5] + psrc[w*4+1] + psrc[w*4+5]) >> 2;
            pdest[2] = (((int)psrc[2]) + psrc[6] + psrc[w*4+2] + psrc[w*4+6]) >> 2;
            pdest[3] = (((int)psrc[3]) + psrc[7] + psrc[w*4+3] + psrc[w*4+7]) >> 2;
        }
    }
}

}}
