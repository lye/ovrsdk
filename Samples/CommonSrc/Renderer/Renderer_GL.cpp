/************************************************************************************

Filename    :   Renderer_GL.cpp
Content     :   Renderer implementation for OpenGL
Created     :   September 10, 2012
Authors     :   Andrew Reisse

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus LLC license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#include "../Renderer/Renderer_GL.h"

namespace OVR { namespace Render { namespace GL {

void Renderer::BeginRendering()
{
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glLineWidth(3.0f);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Renderer::SetDepthMode(bool enable, bool write, CompareFunc func)
{
    if (enable)
    {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(write);
        switch (func)
        {
        case Compare_Always:  glDepthFunc(GL_ALWAYS); break;
        case Compare_Less:    glDepthFunc(GL_LESS); break;
        case Compare_Greater: glDepthFunc(GL_GREATER); break;
        default: assert(0);
        }
    }
    else
        glDisable(GL_DEPTH_TEST);
}

void Renderer::SetRealViewport(int x, int y, int w, int h)
{
    glViewport(x, WindowHeight-y-h, w, h);

    glEnable(GL_SCISSOR_TEST);
    glScissor(x, WindowHeight-y-h, w, h);
}

void Renderer::Clear(float r, float g, float b, float a, float depth)
{
    glClearColor(r,g,b,a);
    glClearDepth(depth);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Renderer::SetWorldUniforms(const Matrix4f& proj)
{
    Proj = proj;
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(&proj.M[0][0]);
    glMatrixMode(GL_MODELVIEW);
}


void Renderer::Render(const Matrix4f& matrix, Model* model)
{
    glLoadMatrixf(&matrix.M[0][0]);

    GLenum prim;
    switch (model->GetPrimType())
    {
    case Prim_Triangles:
        prim = GL_TRIANGLES;
        break;
    case Prim_Lines:
        prim = GL_LINES;
        break;
    default:
        assert(0);
        return;
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &model->Vertices[0].Pos.x);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), &model->Vertices[0].C.R);
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &model->Vertices[0].U);

    if (model->Fill)
        model->Fill->Set();
    else
        DefaultFill.Set();

    glDrawElements(prim, (GLsizei)model->Indices.GetSize(), GL_UNSIGNED_SHORT, &model->Indices[0]);

    if (model->Fill)
        model->Fill->Unset();

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void Fill_Simple::Set(PrimitiveType) const
{
    glPolygonMode(GL_FRONT_AND_BACK, Wireframe ? GL_LINE : GL_FILL);
}

Texture::Texture(int w, int h) : Width(w), Height(h)
{
    glGenTextures(1, &TexId);
}

Texture::~Texture()
{
    if (TexId)
        glDeleteTextures(1, &TexId);
}

Texture* Renderer::CreateTexture(int format, int width, int height, const void* data)
{
    GLenum   glformat, gltype = GL_UNSIGNED_BYTE;
    switch(format)
    {
    case Texture_RGBA: glformat = GL_RGBA; break;
    case Texture_R:    glformat = GL_ALPHA; break; // XXX
    default:
        return NULL;
    }
    Texture* NewTex = new Texture(width, height);
    glBindTexture(GL_TEXTURE_2D, NewTex->TexId);
    glTexImage2D(GL_TEXTURE_2D, 0, glformat, width, height, 0, glformat, gltype, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    return NewTex;
}

void Fill_Texture::Set(PrimitiveType) const
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, Tex->TexId);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void Fill_Texture::Unset() const
{
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

/*
void Renderer::RenderText(const Font* font, const char* str, float x, float y, float size, Color c) const
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    Matrix4x4 ortho = Matrix4x4::Ortho2D(WindowWidth, WindowHeight);
    glLoadMatrixf(&ortho.M[0][0]);


    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}
*/

}}}
