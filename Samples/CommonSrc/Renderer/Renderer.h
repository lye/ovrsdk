/************************************************************************************

Filename    :   OVR_Renderer.h
Content     :   Platform renderer for simple scene graph
Created     :   September 6, 2012
Authors     :   Andrew Reisse

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus LLC license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#ifndef OVR_Renderer_h
#define OVR_Renderer_h

#include "Kernel/OVR_Math.h"
#include "Kernel/OVR_Array.h"
#include "Kernel/OVR_RefCount.h"
#include "Kernel/OVR_String.h"
#include "Kernel/OVR_File.h"

namespace OVR { namespace Render {

class Renderer;

struct Color
{
    unsigned char R,G,B,A;

    Color() {}
    // Constructs color by channel. Alpha is set to 0xFF (fully visible)
    // if not specified.
    Color(unsigned char r,unsigned char g,unsigned char b, unsigned char a = 0xFF)
        : R(r), G(g), B(b), A(a) { }
    
    // 0xAARRGGBB - Common HTML color Hex layout
    Color(unsigned c)
        : R((unsigned char)(c>>16)), G((unsigned char)(c>>8)),
          B((unsigned char)c), A((unsigned char)(c>>24)) { }

    bool operator==(const Color& b) const
    {
        return R == b.R && G == b.G && B == b.B && A == b.A;
    }

    void  GetRGBA(float *r, float *g, float *b, float* a) const
    {
        *r = R / 255.0f;
        *g = G / 255.0f;
        *b = B / 255.0f;
        *a = A / 255.0f;
    }
};


//-----------------------------------------------------------------------------------

enum PrimitiveType
{
    Prim_Triangles,
    Prim_Lines,
    Prim_TriangleStrip,
    Prim_Points,
    Prim_Unknown,
    Prim_Count
};

class Fill : public RefCountBase<Fill>
{
public:
    enum Flags
    {
        F_Solid = 1,
        F_Wireframe = 2,
    };

    virtual ~Fill() {}

    virtual void Set(PrimitiveType prim = Prim_Unknown) const = 0;
    virtual void Unset() const {}

    virtual void SetTexture(int i, class Texture* tex) { OVR_UNUSED2(i,tex); }
};

enum ShaderStage
{
    Shader_Vertex   = 0,
    Shader_Geometry = 1,
    Shader_Fragment = 2,
    Shader_Pixel    = 2,
    Shader_Count    = 3,
};

enum BuiltinShaders
{
    VShader_MV          = 0,
    VShader_MVP         = 1,
    VShader_PostProcess = 2,
    VShader_Count       = 3,

    FShader_Solid       = 0,
    FShader_Gouraud     = 1,
    FShader_Texture     = 2,
    FShader_AlphaTexture= 3,
    FShader_PostProcess = 4,
    FShader_LitGouraud  = 5,
    FShader_LitTexture  = 6,
    FShader_Count       = 7,
};


enum MapFlags
{
    Map_Discard        = 1,
    Map_Read           = 2, // do not use
    Map_Unsynchronized = 4, // like D3D11_MAP_NO_OVERWRITE
};

enum BufferUsage
{
    Buffer_Unknown  = 0,
    Buffer_Vertex   = 1,
    Buffer_Index    = 2,
    Buffer_Uniform  = 4,
    Buffer_Feedback = 8,
    Buffer_TypeMask = 0xff,
    Buffer_ReadOnly = 0x100, // Buffer must be created with Data().
};

enum TextureFormat
{
    Texture_RGBA            = 0x100,
    Texture_R               = 0x200,
    Texture_Depth           = 0x8000,
    Texture_TypeMask        = 0xff00,
    Texture_SamplesMask     = 0x00ff,
    Texture_RenderTarget    = 0x10000,
    Texture_GenMipmaps      = 0x20000,
};

enum SampleMode
{
    Sample_Linear       = 0,
    Sample_Nearest      = 1,
    Sample_Anisotropic  = 2,
    Sample_FilterMask   = 3,

    Sample_Repeat       = 0,
    Sample_Clamp        = 4,
    Sample_ClampBorder  = 8, // If unsupported Clamp is used instead.
    Sample_AddressMask  =12,

    Sample_Count        =13,
};

class Shader : public RefCountBase<Shader>
{
    friend class ShaderSet;

protected:
    ShaderStage Stage;

public:
    Shader(ShaderStage s) : Stage(s) {}
    virtual ~Shader() {}

    ShaderStage GetStage() const { return Stage; }

    virtual void Set(PrimitiveType prim) const = 0;
    virtual void SetUniformBuffer(class Buffer* buffers, int i = 0) { OVR_UNUSED2(buffers, i); }
    virtual bool UseTransposeMatrix() const { return 0; }

protected:
    virtual bool SetUniform(const char* name, int n, const float* v) { OVR_UNUSED3(name, n, v); return false; }
};


// A group of shaders, one per stage.
// Some renderers subclass this, so CreateShaderSet must be used.

class ShaderSet : public RefCountBase<ShaderSet>
{
    Ptr<Shader> Shaders[Shader_Count];

public:
    ShaderSet() { }
    ~ShaderSet() { }

    virtual void SetShader(Shader *s)
    {
        Shaders[s->GetStage()] = s;
    }
    virtual void UnsetShader(int stage)
    {
        Shaders[stage] = NULL;
    }
    Shader* GetShader(int stage) { return Shaders[stage]; }

    virtual void Set(PrimitiveType prim) const
    {
        for (int i = 0; i < Shader_Count; i++)
            if (Shaders[i])
                Shaders[i]->Set(prim);
    }

    // Set a uniform (other than the standard matrices). It is undefined whether the
    // uniforms from one shader occupy the same space as those in other shaders
    // (unless a buffer is used, then each buffer is independent).     
    virtual bool SetUniform(const char* name, int n, const float* v)
    {
        bool result = 0;
        for (int i = 0; i < Shader_Count; i++)
            if (Shaders[i])
                result |= Shaders[i]->SetUniform(name, n, v);

        return result;
    }
    bool SetUniform2f(const char* name, float x, float y)
    {
        const float v[] = {x,y};
        return SetUniform(name, 2, v);
    }
    bool SetUniform4f(const char* name, float x, float y, float z, float w = 1)
    {
        const float v[] = {x,y,z,w};
        return SetUniform(name, 4, v);
    }
    bool SetUniformv(const char* name, const Vector3f& v)
    {
        const float a[] = {v.x,v.y,v.z,1};
        return SetUniform(name, 4, a);
    }
    virtual bool SetUniform4x4f(const char* name, const Matrix4f& m)
    {
        return SetUniform(name, 16, &m.M[0][0]);
    }
};

class ShaderSetMatrixTranspose : public ShaderSet
{
public:
    virtual bool SetUniform4x4f(const char* name, const Matrix4f& m)
    {
        Matrix4f mt = m.Transposed();
        return SetUniform(name, 16, &mt.M[0][0]);
    }
};

class ShaderFill : public Fill
{
    Ptr<ShaderSet> Shaders;
    Ptr<Texture>   Textures[8];

public:
    ShaderFill(ShaderSet* sh) : Shaders(sh) {  }
    ShaderFill(ShaderSet& sh) : Shaders(sh) {  }
    void Set(PrimitiveType prim) const;
    ShaderSet* GetShaders() { return Shaders; }

    virtual void SetTexture(int i, class Texture* tex) { if (i < 8) Textures[i] = tex; }
};

/* Buffer for vertex or index data. Some renderers require separate buffers, so that
   is recommended. Some renderers cannot have high-performance buffers which are readable,
   so reading in Map should not be relied on.

   Constraints on buffers, such as ReadOnly, are not enforced by the api but may result in 
   rendering-system dependent undesirable behavior, such as terrible performance or unreported failure.

   Use of a buffer inconsistent with usage is also not checked by the api, but it may result in bad
   performance or even failure.

   Use the Data() function to set buffer data the first time, if possible (it may be faster).
*/

class Buffer : public RefCountBase<Buffer>
{
public:
    virtual ~Buffer() {}

    virtual size_t GetSize() = 0;
    virtual void*  Map(size_t start, size_t size, int flags = 0) = 0;
    virtual bool   Unmap(void *m) = 0;

    // Allocates a buffer, optionally filling it with data.
    virtual bool   Data(int use, const void* buffer, size_t size) = 0;
};

class Texture : public RefCountBase<Texture>
{
public:
    virtual ~Texture() {}

    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;
    virtual int GetSamples() const { return 1; }

    virtual void SetSampleMode(int sm) = 0;
    virtual void Set(int slot, ShaderStage stage = Shader_Fragment) const = 0;
};



//-----------------------------------------------------------------------------------

class Node : public RefCountBase<Node>
{
    Vector3f     Pos;
    Quatf        Rot;

    mutable Matrix4f  Mat;
    mutable bool      MatCurrent;

public:
    Node() : Pos(Vector3f(0)), MatCurrent(1) { }
    virtual ~Node() { }

    enum NodeType
    {
        Node_NonDisplay,
        Node_Container,
        Node_Model
    };
    virtual NodeType GetType() const { return Node_NonDisplay; }

    virtual void ClearRenderer() { }

    const Vector3f&  GetPosition() const      { return Pos; }
    const Quatf&     GetOrientation() const   { return Rot; }
    void             SetPosition(Vector3f p)  { Pos = p; MatCurrent = 0; }
    void             SetOrientation(Quatf q)  { Rot = q; MatCurrent = 0; }

    void             Move(Vector3f p)         { Pos += p; MatCurrent = 0; }
    void             Rotate(Quatf q)          { Rot = q * Rot; MatCurrent = 0; }


    // For testing only; causes Position an Orientation
    void  SetMatrix(const Matrix4f& m)
    {
        MatCurrent = true;
        Mat = m;        
    }


    const Matrix4f&  GetMatrix() const 
    {
        if (!MatCurrent)
        {
            Mat = Rot;
            Mat = Matrix4f::Translation(Pos) * Mat;
            MatCurrent = 1;
        }
        return Mat;
    }

    virtual void     Render(const Matrix4f& ltw, Renderer* ren) { OVR_UNUSED2(ltw, ren); }
};

struct Vertex
{
    Vector3f  Pos;
    Color     C;
    float     U, V;
    Vector3f  Norm;

    Vertex (const Vector3f& p, const Color& c = Color(64,0,0,255), 
            float u = 0, float v = 0, Vector3f n = Vector3f(1,0,0))
      : Pos(p), C(c), U(u), V(v), Norm(n) {}
    Vertex(float x, float y, float z, const Color& c = Color(64,0,0,255),
           float u = 0, float v = 0) : Pos(x,y,z), C(c), U(u), V(v) { }


    bool operator==(const Vertex& b) const
    {
        return Pos == b.Pos && C == b.C && U == b.U && V == b.V;
    }
};

// A vector with a dummy w component for alignment in uniform buffers (and for float colors).
// The w component is not used in any calculations.

struct Vector4f : public Vector3f
{
    float w;

    Vector4f() : w(1) {}
    Vector4f(const Vector3f& v) : Vector3f(v), w(1) {}
    Vector4f(float r, float g, float b, float a) : Vector3f(r,g,b), w(a) {}
};

// this is stored in a uniform buffer, don't change it without fixing all renderers

struct LightingParams
{
    Vector4f Ambient;
    Vector4f LightPos[8];
    Vector4f LightColor[8];
    float    LightCount;

    LightingParams() : LightCount(0) {}
};

//-----------------------------------------------------------------------------------

class Model : public Node
{
public:
    Array<Vertex>     Vertices;
    Array<UInt16>     Indices;
    PrimitiveType     Type;
    Ptr<class Fill>   Fill;
    bool              Visible;

    // Some renderers will create these if they didn't exist before rendering.
    // Currently they are not updated, so vertex data should not be changed after rendering.
    Ptr<Buffer>       VertexBuffer;
    Ptr<Buffer>       IndexBuffer;

    Model(PrimitiveType t = Prim_Triangles) : Type(t), Fill(NULL), Visible(true) { }
    ~Model() { }

    virtual NodeType GetType() const { return Node_Model; }

    virtual void Render(const Matrix4f& ltw, Renderer* ren);

    PrimitiveType GetPrimType() const { return Type; }

    void SetVisible(bool visible) { Visible = visible; }
    bool IsVisible() const        { return Visible; }

    void ClearRenderer()
    {
        VertexBuffer.Clear();
        IndexBuffer.Clear();
    }

    // Returns the index next added vertex will have.
    UInt16 GetNextVertexIndex() const
    {
        return (UInt16)Vertices.GetSize();
    }

    UInt16 AddVertex(const Vertex& v)
    {
        assert(!VertexBuffer && !IndexBuffer);
        UInt16 index = (UInt16)Vertices.GetSize();
        Vertices.PushBack(v);
        return index;
    }
    UInt16 AddVertex(const Vector3f& v, const Color& c, float u_ = 0, float v_ = 0)
    {
        return AddVertex(Vertex(v,c,u_,v_));
    }
    UInt16 AddVertex(float x, float y, float z, const Color& c, float u, float v)
    {
        return AddVertex(Vertex(Vector3f(x,y,z),c, u,v));
    }

    void AddLine(UInt16 a, UInt16 b)
    {
        Indices.PushBack(a);
        Indices.PushBack(b);
    }

    UInt16 AddVertex(float x, float y, float z, const Color& c,
                     float u, float v, float nx, float ny, float nz)
    {
        return AddVertex(Vertex(Vector3f(x,y,z),c, u,v, Vector3f(nx,ny,nz)));
    }

    void AddLine(const Vertex& a, const Vertex& b)
    {
        AddLine(AddVertex(a), AddVertex(b));
    }

    void AddTriangle(UInt16 a, UInt16 b, UInt16 c)
    {
        Indices.PushBack(a);
        Indices.PushBack(b);
        Indices.PushBack(c);
    }


    // Uses texture coordinates for uniform world scaling (must use a repeat sampler).
    void  AddSolidColorBox(float x1, float y1, float z1,
                           float x2, float y2, float z2,
                           Color c);


    static Model* CreateAxisFaceColorBox(float x1, float x2, Color xcolor,
                                         float y1, float y2, Color ycolor,
                                         float z1, float z2, Color zcolor);
   


    // Uses texture coordinates for exactly covering each surface once.
    static Model* CreateBox(Color c, Vector3f origin, Vector3f size);
    static Model* CreateCylinder(Color c, Vector3f origin, float height, float radius, int sides = 20);

    // Grid having halfx,halfy lines in each direction from the origin
    static Model* CreateGrid(Vector3f origin, Vector3f stepx, Vector3f stepy,
                             int halfx, int halfy, int nmajor = 5,
                             Color minor = Color(64,64,64,192), Color major = Color(128,128,128,192));
};

class Container : public Node
{
public:
    Array<Ptr<Node> > Nodes;

    ~Container()
    {

    }

    void ClearRenderer()
    {
        for (UPInt i=0; i< Nodes.GetSize(); i++)
            Nodes[i]->ClearRenderer();
    }

    virtual NodeType GetType() const { return Node_Container; }

    virtual void Render(const Matrix4f& ltw, Renderer* ren);

    void Add(Node *n) { Nodes.PushBack(n); }
    void Add(Model *n, class Fill *f) { n->Fill = f; Nodes.PushBack(n); }
};

class Scene
{
public:
    Container      World;

    Vector4f        LightPos[8];
    LightingParams  Lighting;
    Ptr<Buffer>     LightingBuffer;

public:
    void Render(Renderer* ren, const Matrix4f& view);

    void SetAmbient(Vector4f color)
    {
        Lighting.Ambient = color;
    }
    void AddLight(Vector3f pos, Vector4f color)
    {
        int n = (int)Lighting.LightCount;
        OVR_ASSERT(n < 8);
        LightPos[n] = pos;
        Lighting.LightColor[n] = color;
        Lighting.LightCount++;
    }

    void ClearRenderer()
    {
        LightingBuffer.Clear();
        World.ClearRenderer();
    }
};

class SceneView : public Node
{
public:
    Matrix4f GetViewMatrix() const;
};

enum RenderCaps
{
    Cap_VertexBuffer = 1,
};

// Post-processing type to apply to scene after rendering
enum PostProcessType
{
    PostProcess_None,
    PostProcess_Distortion
};


// DistortionConfig Provides controls for the distortion shader.
//  - K[0] - K[3] are coefficients for the distortion function.
//  - XCenterOffset is the offset of lens distortion center from the 
//    center of one-eye screen half. [-1, 1] Range.
//  - Scale is a factor of how much larger will the input image be,
//    with a factor of 1.0f being no scaling. An inverse of this 
//    value is applied to sampled UV coordinates (1/Scale).

class DistortionConfig
{
public:
    DistortionConfig(float k0 = 1.0f, float k1 = 0.0f, float k2 = 0.0f, float k3 = 0.0f)
        : XCenterOffset(0), YCenterOffset(0), Scale(1.0f)
    { SetCoefficients(k0, k1, k2, k3); }

    void SetCoefficients(float k0, float k1 = 0.0f, float k2 = 0.0f, float k3 = 0.0f)
    { K[0] = k0; K[1] = k1;  K[2] = k2; K[3] = k3; }

    // Computes scale that should be applied to the input render texture
    // before distortion to fit the result in the same screen size.
    // The 'fitRadius' parameter specifies the distance away from distortion center at
    // which the input and output coordinates will match, assuming [-1,1] range.
    float  CalcScale(float fitRadius) const
    {
        float s = fitRadius;
        // This should match distortion equation used in shader.
        float ssq   = s * s;
        float scale = s * (K[0] + K[1] * ssq + K[2] * ssq * ssq + K[3] * ssq * ssq * ssq);
        return scale / fitRadius;
    }

    float  CalcScaleEQ(float s) const
    {
        // This should match distortion equation used in shader.
        float ssq   = s * s;
        float scale = s * (K[0] + K[1] * ssq + K[2] * ssq * ssq + K[3] * ssq * ssq * ssq);
        return scale;
    }

    // 
    float CalcScaleInverse(float scale)
    {    
        OVR_ASSERT((scale <= 10.0f));

        float s, d;
        float delta = scale * 0.25f;

        s = scale * 0.5f;
        d = fabs(scale - CalcScaleEQ(s));

        for (int i = 0; i < 20; i++)
        {
            float sUp   = s + delta;
            float sDown = s - delta;
            float dUp   = fabs(scale - CalcScaleEQ(sUp));
            float dDown = fabs(scale - CalcScaleEQ(sDown));

            if (dUp < d)
            {
                s = sUp;
                d = dUp;
            }
            else if (dDown < d)
            {
                s = sDown;
                d = dDown;
            }
            else
            {
                delta *= 0.5f;
            }
        }

        return s;
    }


    float   K[4];
    float   XCenterOffset, YCenterOffset;
    float   Scale;
};



struct Viewport
{
    int x, y;
    int w, h;

    Viewport() {}
    Viewport(int x1, int y1, int w1, int h1) : x(x1), y(y1), w(w1), h(h1) { }

    bool operator == (const Viewport& vp) const
    { return (x == vp.x) && (y == vp.y) && (w == vp.w) && (h == vp.h); }
    bool operator != (const Viewport& vp) const
    { return !operator == (vp); }
};

enum DisplayMode
{
    Display_Window = 0,
    Display_Fullscreen = 1,
    Display_FakeFullscreen
};

struct RendererParams
{
    int  Multisample;
    int  Fullscreen;

    // Windows
    String MonitorName; // Monitor name for fullscreen mode

    RendererParams(int ms = 1) : Multisample(ms), Fullscreen(0) {}
};



//-----------------------------------------------------------------------------------
// ***** Renderer

class Renderer : public RefCountBase<Renderer>
{
    friend class StereoGeomShaders;
protected:
    int            WindowWidth, WindowHeight;
    RendererParams Params;
    Viewport       VP;

    Matrix4f        Proj;
    Ptr<Buffer>     pTextVertexBuffer;


    // For rendering with lens warping
    PostProcessType CurPostProcess;
    Ptr<Texture>    pSceneColorTex;
    int             SceneColorTexW;
    int             SceneColorTexH;
    Ptr<ShaderSet>  pPostProcessShader;
    Ptr<Buffer>     pFullScreenVertexBuffer;
    float           SceneRenderScale;    
    DistortionConfig Distortion;
    Color           DistortionClearColor;

    void FinishScene1();

public:
    enum CompareFunc
    {
        Compare_Always  = 0,
        Compare_Less    = 1,
        Compare_Greater = 2,
        Compare_Count
    };
    Renderer();
    virtual ~Renderer() { Shutdown(); }

    virtual void Init() {}
    virtual void Shutdown() {}
    virtual bool SetParams(const RendererParams&) { return 0; }

    const RendererParams& GetParams() const { return Params; }

    virtual void SetViewport(const Viewport& vp);
    void         SetViewport(int x, int y, int w, int h) { SetViewport(Viewport(x,y,w,h)); }
    //virtual void SetScissor(int x, int y, int w, int h) = 0;

    // Set viewport ignoring any adjustments used for the stereo mode.
    virtual void SetRealViewport(const Viewport& vp) { SetMultipleViewports(1, &vp); }
    virtual void SetMultipleViewports(int n, const Viewport* vps) { OVR_UNUSED2(n, vps); }

    virtual void Clear(float r = 0, float g = 0, float b = 0, float a = 1, float depth = 1) = 0;
    virtual void Rect(float left, float top, float right, float bottom) = 0;

    inline void Clear(const Color &c, float depth = 1)
    {
        float r, g, b, a;
        c.GetRGBA(&r, &g, &b, &a);
        Clear(r, g, b, a, depth);
    }

    virtual bool IsFullscreen() const { return Params.Fullscreen != Display_Window; }
    virtual void Present() = 0;
    // Waits for rendering to complete; important for reducing latency.
    virtual void ForceFlushGPU() { }

    // Resources
    virtual Buffer*  CreateBuffer() { return NULL; }
    virtual Texture* CreateTexture(int format, int width, int height, const void* data)
    { OVR_UNUSED4(format,width,height,data); return NULL; }
    virtual bool     GetSamplePositions(Render::Texture*, Vector3f* pos) { pos[0] = Vector3f(0); return 1; }

    virtual ShaderSet* CreateShaderSet() { return new ShaderSetMatrixTranspose; }
    virtual Shader* LoadBuiltinShader(ShaderStage stage, int shader) = 0;

    // Rendering

    // Begin drawing directly to the currently selected render target, no post-processing.
    virtual void BeginRendering() {}
    // Begin drawing the primary scene. This will have post-processing applied (if enabled)
    // during FinishScene.
    virtual void BeginScene(PostProcessType pp = PostProcess_None); //StereoDisplay disp = Stereo_Center);
    // Postprocess the scene and return to the screen render target.
    virtual void FinishScene();

    // Texture must have been created with Texture_RenderTarget. Use NULL for the default render target.
    // NULL depth buffer means use an internal, temporary one.
    virtual void SetRenderTarget(Texture* color, Texture* depth = NULL, Texture* stencil = NULL)
    { OVR_UNUSED3(color, depth, stencil); }
    virtual void SetDepthMode(bool enable, bool write, CompareFunc func = Compare_Less) = 0;
    virtual void SetProjection(const Matrix4f& proj);
    virtual void SetWorldUniforms(const Matrix4f& proj) = 0;

    // The index 0 is reserved for non-buffer uniforms, and so cannot be used with this function.
    virtual void SetCommonUniformBuffer(int i, Buffer* buffer) { OVR_UNUSED2(i, buffer); }

    //    virtual void SetLighting(const Lighting& lit) {}
    virtual void SetExtraShaders(ShaderSet* s) { OVR_UNUSED(s); }
    virtual Matrix4f GetProjection() const { return Proj; }

    // This is a View matrix only, it will be combined with the projection matrix from SetProjection
    virtual void Render(const Matrix4f& matrix, Model* model) = 0;
    // offset is in bytes; indices can be null.
    virtual void Render(const Fill* fill, Buffer* vertices, Buffer* indices,
                        const Matrix4f& matrix, int offset, int count, PrimitiveType prim = Prim_Triangles) = 0;

    // Returns width of text in same units as drawing. If strsize is not null, stores width and height.
    float        MeasureText(const struct Font* font, const char* str, float size, float* strsize = NULL);
    virtual void RenderText(const struct Font* font, const char* str, float x, float y, float size, Color c);

    virtual void FillRect(float left, float top, float right, float bottom, Color c);

    virtual Fill *CreateSimpleFill(int flags = Fill::F_Solid) = 0;
    Fill *        CreateTextureFill(Texture* tex, bool useAlpha = false);

    // PostProcess distortion
    void          SetSceneRenderScale(float ss);

    void          SetDistortionConfig(const DistortionConfig& config)
    {
        Distortion = config;
    }

    // Sets the color that is applied around distortion.
    void          SetDistortionClearColor(Color clearColor)
    {
        DistortionClearColor = clearColor;
    }

    virtual bool SetFullscreen(DisplayMode fullscreen) { OVR_UNUSED(fullscreen); return false; }
    // Don't call this, use App/Platform instead
    virtual void SetWindowSize(int w, int h) { WindowWidth = w; WindowHeight = h; }

protected:
    // Stereo & post-processing
    virtual bool  initPostProcessSupport(PostProcessType pptype);
    
    virtual Shader* CreateStereoShader(PrimitiveType prim, Shader* vs)
    { OVR_UNUSED2(prim, vs); return NULL; }
};

int GetNumMipLevels(int w, int h);

// Filter an rgba image with a 2x2 box filter, for mipmaps.
// Image size must be a power of 2.
void FilterRgba2x2(const UByte* src, int w, int h, UByte* dest);

Texture* LoadTextureTga(Renderer* ren, File* f);

}}

#endif
