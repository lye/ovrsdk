/************************************************************************************

Filename    :   Renderer_GL.h
Content     :   Renderer implementation header for OpenGL
Created     :   September 10, 2012
Authors     :   Andrew Reisse

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus LLC license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#ifndef OVR_Renderer_GL_h
#define OVR_Renderer_GL_h

#include "../Renderer/Renderer.h"

#ifdef WIN32
#include <Windows.h>
#endif

#include <GL/gl.h>

namespace OVR { namespace Render { namespace GL {


class Texture : public Render::Texture
{
public:
    GLuint TexId;
    int    Width, Height;

    Texture(int w, int h);
    ~Texture();

    virtual int GetWidth() const { return Width; }
    virtual int GetHeight() const { return Height; }

    virtual void SetSampleMode(int) {}

    // Does nothing until shaders are supported
    virtual void Set(int slot, ShaderStage stage = Shader_Fragment) const { OVR_UNUSED2(slot, stage); }
};


class Fill_Simple : public Render::Fill
{
    bool Wireframe;

public:
    Fill_Simple(bool wf = 0) : Wireframe(wf) {}

    virtual void Set(PrimitiveType prim = Prim_Unknown) const;
};

class Fill_Texture : public Render::Fill
{
    Texture* Tex;
public:
    Fill_Texture(Texture* t) : Tex(t) {}

    virtual void Set(PrimitiveType prim = Prim_Unknown) const;
    virtual void Unset() const;
    virtual void SetTexture(int i, Texture* tex) { OVR_UNUSED(i); Tex = tex; }
};


class Renderer : public Render::Renderer
{
    Fill_Simple DefaultFill;
    Matrix4f    Proj;

public:
    virtual void SetRealViewport(int x, int y, int w, int h);
    //virtual void SetScissor(int x, int y, int w, int h);

    virtual void Clear(float r = 0, float g = 0, float b = 0, float a = 1, float depth = 1);
    virtual void Rect(float left, float top, float right, float bottom) { OVR_UNUSED4(left,top,right,bottom); }

    virtual void BeginRendering();
    virtual void SetDepthMode(bool enable, bool write, CompareFunc func = Compare_Less);
    virtual void SetWorldUniforms(const Matrix4f& proj);

    virtual void Render(const Matrix4f& matrix, Model* model);
    virtual void Render(const Fill* fill, Render::Buffer* vertices, Render::Buffer* indices,
                        const Matrix4f& matrix, int offset, int count, PrimitiveType prim = Prim_Triangles) { OVR_UNUSED3(fill,vertices,indices); OVR_UNUSED4(matrix,offset,count,prim); }

    //virtual void RenderText(const struct Font* font, const char* str, float x, float y, const Matrix4x4& );

    virtual Texture* CreateTexture(int format, int width, int height, const void* data);

    virtual Fill *CreateSimpleFill(int flags = Fill::F_Solid) { return new Fill_Simple((flags & Fill::F_Wireframe) ? 1 : 0); }
    virtual Fill *CreateTextureFill(Render::Texture* tex, bool useAlpha = false) { OVR_UNUSED(useAlpha); return new Fill_Texture((Texture*)tex); }

    virtual Shader *LoadBuiltinShader(ShaderStage stage, int shader) { OVR_UNUSED2(stage,shader); return NULL; }
};

}}}

#endif
