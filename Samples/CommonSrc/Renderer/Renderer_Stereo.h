/************************************************************************************

Filename    :   Renderer_Stereo.h
Content     :   Sample stereo rendering configuration classes.
Created     :   October 22, 2012
Authors     :   Michael Antonov

Copyright   :   Copyright 2012 Oculus, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus Inc license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#ifndef OVR_Renderer_Stereo_h
#define OVR_Renderer_Stereo_h

#include "Renderer.h"
#include "OVR_Device.h"

namespace OVR { namespace Render {


//-----------------------------------------------------------------------------------
// ***** Stereo Enumerations

// StereoMode describes rendering modes that can be used by StereoConfig.
// These modes control whether stereo rendering is used or not (Stereo_None),
// and how it is implemented.
enum StereoMode
{
    Stereo_None                 = 0,
    Stereo_LeftRight_Multipass  = 1
};


// StereoEye specifies which eye we are rendering for; it is used to
// retrieve StereoRenderParams.
enum StereoEye
{
    StereoEye_Center,
    StereoEye_Left,
    StereoEye_Right,
    // StereoEye_LeftRight_Singlepass
};


//-----------------------------------------------------------------------------------

// StereoRenderParams describes Renderer configuration needed to render
// the scene for one eye. 
class StereoRenderParams
{
public:
    StereoEye                Eye;
    Viewport                 VP;       // Viewport that we are rendering to    
    float                    ProjectionCenterOffset; // For debugging
    const DistortionConfig*  pDistortion;

    Matrix4f                 ViewAdjust;       // Translation to be applied to view matrix.
    Matrix4f                 Projection;       // Projection matrix used with this eye.
    Matrix4f                 OrthoProjection;  // Orthographic projection used with this eye.

    void Init(StereoEye eye, const Viewport &vp, float vofs,
              const Matrix4f& proj, const Matrix4f& orthoProj,
              const DistortionConfig* distortion = 0)
    {
        Eye                    = eye;
        VP                     = vp;
        ViewAdjust             = Matrix4f::Translation(Vector3f(vofs,0,0));
        Projection             = proj;
        OrthoProjection        = orthoProj;
        pDistortion            = distortion;        
        ProjectionCenterOffset = 0;
    }

    // Apply the current state to renderer.
    void Apply(Renderer* renderer) const
    {
        renderer->SetViewport(VP);
        renderer->SetProjection(Projection);
        ApplyDistortion(renderer);
    }

    void Apply2D(Renderer* renderer) const
    {
        renderer->SetViewport(VP);
        renderer->SetProjection(OrthoProjection);
        ApplyDistortion(renderer);
    }

    bool ApplyDistortion(Renderer* renderer) const
    {
        if (pDistortion)        
        {
            DistortionConfig dc(*pDistortion);
            if (Eye == StereoEye_Right)
                dc.XCenterOffset = -dc.XCenterOffset;
            renderer->SetDistortionConfig(dc);
            return true;
        }
        return false;
    }
};


//-----------------------------------------------------------------------------------
// *****  StereoConfig

// StereoConfig maintains a scene stereo state and allow switching between different
// stereo rendering modes. To support rendering, StereoConfig keeps track of HMD
// variables such as screen size, eye-to-screen distance and distortion, and computes
// extra data such as FOV and distortion center offsets based on it. Rendering
// parameters are returned though StereoRenderParams for each eye.
//
// Beyond regular 3D projection, this class supports rendering a 2D orthographic
// surface for UI and text. The 2D surface will be defined as fitting within a 2D
// field of view (85 degrees by default) and used [-1,1] coordinate system with
// square pixels. The (0,0) coordinate corresponds to eye center location
// that is properly adjusted during rendering through SterepRenderParams::Adjust2D.
// Genreally speaking, text outside [-1,1] coordinate range will not be readable.

class StereoConfig
{
public:

    StereoConfig(StereoMode mode = Stereo_None,
                 const Viewport& fullViewport = Viewport(0,0, 1280,800));
 

    // *** Modifiable State Access

    // Sets a stereo rendering mode and updates internal cached
    // state (matrices, per-eye view) based on it.
    void        SetStereoMode(StereoMode mode)  { Mode = mode; DirtyFlag = true; }
    StereoMode  GetStereoMode() const           { return Mode; }

    // Sets HMD parameters; also initializes distortion coefficients.
    void        SetHMDInfo(const HMDInfo& hmd);
    const HMDInfo& GetHMDInfo() const           { return HMD; }

    // Query physical eye-to-screen distance in meters, which combines screen-to-lens and
    // and lens-to-eye pupil distances. Modifying this value adjusts FOV.
    float       GetEyeToScreenDistance() const  { return HMD.EyeToScreenDistance; }
    void        SetEyeToScreenDistance(float esd) { HMD.EyeToScreenDistance = esd; DirtyFlag = true; }

    // Interpupillary distance used for stereo, in meters. Default is 0.064m (64 mm).
    void        SetIPD(float ipd)               { InterpupillaryDistance = ipd; DirtyFlag = true; }
    float       GetIPD() const                  { return InterpupillaryDistance; }

    // Set full render target viewport; for HMD this includes both eyes. 
    void        SetFullViewport(const Viewport& vp);
    const Viewport& GetFullViewport() const     { return FullView; }

    // Aspect ratio defaults to ((w/h)*multiplier) computed per eye.
    // Aspect multiplier allows adjusting aspect ratio consistently for Stereo/NoStereo.
    void        SetAspectMultiplier(float m)    { AspectMultiplier = m; DirtyFlag = true; }
    float       GetAspectMultiplier() const     { return AspectMultiplier; }

    
    // For the distorted image to fill rendered viewport, input texture render target needs to be
    // scaled by DistortionScale before sampling. The scale factor is computed by fitting a point
    // on of specified radius from a distortion center, more easily specified as a coordinate.
    // SetDistortionFitPointVP sets the (x,y) coordinate of the point that scale will be "fit" to,
    // assuming [-1,1] coordinate range for full left-eye viewport. A fit point is a location
    // where source (pre-distortion) and target (post-distortion) image match each other.
    // For the right eye, the interpretation of 'u' will be inverted.  
    void       SetDistortionFitPointVP(float x, float y);
    // SetDistortionFitPointPixels sets the (x,y) coordinate of the point that scale will be "fit" to,
    // specified in pixeld for full left-eye texture.
    void       SetDistortionFitPointPixels(float x, float y);

    // Changes all distortion settings.
    // Note that setting HMDInfo also changes Distortion coefficients.
    void        SetDistortionConfig(const DistortionConfig& d) { Distortion = d; DirtyFlag = true; }
    
    // Modify distortion coefficients; useful for adjustment tweaking.
    void        SetDistortionK(int i, float k)  { Distortion.K[i] = k; DirtyFlag = true; }
    float       GetDistortionK(int i) const     { return Distortion.K[i]; }

    // Sets the fieldOfView that the 2D coordinate area stretches to.
    void        Set2DAreaFov(float fovRadians);


    // *** Computed State

    // Return current aspect ratio.
    float      GetAspect()                      { updateIfDirty(); return Aspect; }
    
    // Return computed vertical FOV in radians/degrees.
    float      GetYFOVRadians()                 { updateIfDirty(); return YFov; }
    float      GetYFOVDegrees()                 { return RadToDegree(GetYFOVRadians()); }

    // Query horizontal projection center offset as a distance away from the
    // one-eye [-1,1] unit viewport.
    // Positive return value should be used for left eye, negative for right eye. 
    float      GetProjectionCenterOffset()      { updateIfDirty(); return ProjectionCenterOffset; }

    // GetDistortionConfig isn't const because XCenterOffset bay need to be recomputed.  
    const DistortionConfig& GetDistortionConfig() { updateIfDirty(); return Distortion; }

    // Returns DistortionScale factor by which input texture size is increased to make
    // post-distortion result distortion fit the viewport.
    float      GetDistortionScale()            { updateIfDirty(); return Distortion.Scale; }

    // Returns the size of a pixel within 2D coordinate system.
    float      Get2DUnitPixel()                { updateIfDirty();  return (2.0f / (FovPixels * Distortion.Scale)); }

    // Returns full set of Stereo rendering parameters for the specified eye.
    const StereoRenderParams& GetEyeRenderParams(StereoEye eye);
   
private:    

    void updateIfDirty()   { if (DirtyFlag) updateComputedState(); }
    void updateComputedState();

    void updateDistortionOffsetAndScale();
    void updateProjectionOffset();
    void update2D();
    void updateEyeParams();


    // *** Modifiable State

    StereoMode         Mode;
    float              InterpupillaryDistance;
    float              AspectMultiplier; // Multiplied into aspect ratio.
    HMDInfo            HMD;
    DistortionConfig   Distortion;
    float              DistortionFitX, DistortionFitY; // In [-1,1] half-screen viewport units.
    Viewport           FullView;                       // Entire window viewport.

    float              Area2DFov; // FOV range mapping to [-1, 1] 2D area.
 
    // *** Computed State
 
    bool               DirtyFlag;   // Set when any if the modifiable state changed.
    float              YFov;        // Vertical FOV.
    float              Aspect;      // Aspect ratio: (w/h)*AspectMultiplier.
    float              ProjectionCenterOffset;
    StereoRenderParams EyeRenderParams[2];

  
    // ** 2D Rendering

    // Number of 2D pixels in the FOV. This defines [-1,1] coordinate range for 2D.  
    float              FovPixels;
    Matrix4f           OrthoCenter;
    float              OrthoPixelOffset;
};


}}  // OVR::Render

#endif
