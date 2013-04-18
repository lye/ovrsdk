/************************************************************************************

Filename    :   Renderer_Stereo.cpp
Content     :   Stereo rendering configuration implementation
Created     :   October 22, 2012
Authors     :   Michael Antonov, Andrew Reisse

Copyright   :   Copyright 2012 Oculus, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus Inc license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#include "Renderer_Stereo.h"
#include "Kernel/OVR_Log.h"

namespace OVR { namespace Render {


// **** StereoConfig Implementation

StereoConfig::StereoConfig(StereoMode mode, const Viewport& vp)
    : Mode(mode),
      InterpupillaryDistance(0.064f), AspectMultiplier(1.0f),
      FullView(vp), DirtyFlag(true),
      YFov(0), Aspect(vp.w / float(vp.h)), ProjectionCenterOffset(0),
      OrthoPixelOffset(0)
{
    // And default distortion for it.
    Distortion.SetCoefficients(1.0f, 0.18f, 0.115f);
    Distortion.Scale = 1.0f; // Will be computed later.

    // Fit top of the image.
    DistortionFitX = 0.0f;
    DistortionFitY = 1.0f;

    // Initialize "fake" default HMD values for testing without HMD plugged in.
    HMD.HResolution            = 1280;
    HMD.VResolution            = 800;
    HMD.HScreenSize            = InterpupillaryDistance * 2;
    HMD.VScreenSize            = HMD.HScreenSize / (1280.0f / 800.0f);
    HMD.InterpupillaryDistance = InterpupillaryDistance;
    HMD.LensSeparationDistance = 0.064f;
    HMD.EyeToScreenDistance    = 0.047f;
    HMD.DistortionK0           = Distortion.K[0];
    HMD.DistortionK1           = Distortion.K[1];
    HMD.DistortionK2           = Distortion.K[2];

    Set2DAreaFov(DegreeToRad(85.0f));
}

void StereoConfig::SetFullViewport(const Viewport& vp)
{
    if (vp != FullView)
    { 
        FullView = vp;
        DirtyFlag = true;
    }
}

void StereoConfig::SetHMDInfo(const HMDInfo& hmd)
{
    HMD = hmd;
    Distortion.K[0] = hmd.DistortionK0;
    Distortion.K[1] = hmd.DistortionK1;
    Distortion.K[2] = hmd.DistortionK2;
    DirtyFlag = true;
}

void StereoConfig::SetDistortionFitPointVP(float x, float y)
{
    DistortionFitX = x;
    DistortionFitY = y;
    DirtyFlag = true;
}

void StereoConfig::SetDistortionFitPointPixels(float x, float y)
{
    DistortionFitX = (4 * x / float(FullView.w)) - 1.0f;
    DistortionFitY = (2 * y / float(FullView.h)) - 1.0f;
    DirtyFlag = true;
}

void StereoConfig::Set2DAreaFov(float fovRadians)
{
    Area2DFov = fovRadians;
    DirtyFlag = true;
}


const StereoRenderParams& StereoConfig::GetEyeRenderParams(StereoEye eye)
{
    static const UByte eyeParamIndices[3] = { 0, 0, 1 };

    updateIfDirty();
    OVR_ASSERT(eye < sizeof(eyeParamIndices));
    return EyeRenderParams[eyeParamIndices[eye]];
}


void StereoConfig::updateComputedState()
{
    // Need to compute all of the following:
    //   - Aspect Ratio
    //   - FOV
    //   - Projection offsets for 3D
    //   - Distortion XCenterOffset
    //   - Update 2D
    //   - Initialize EyeRenderParams

    // Compute aspect ratio. Stereo mode cuts width in half.
    Aspect = float(FullView.w) / float(FullView.h);
    Aspect *= (Mode == Stereo_None) ? 1.0f : 0.5f;
    Aspect *= AspectMultiplier; 

    updateDistortionOffsetAndScale();

    // Compute Vertical FOV based on distance, distortion, etc.
    // Distance from vertical center to render vertical edge perceived through the lens.
    // This will be larger then normal screen size due to magnification & distortion.
    float percievedHalfScreenDistance = (HMD.VScreenSize / 2) * Distortion.Scale;
    YFov = 2.0f * atan(percievedHalfScreenDistance/HMD.EyeToScreenDistance);
    
    updateProjectionOffset();
    update2D();
    updateEyeParams();

    DirtyFlag = false;
}

void StereoConfig::updateDistortionOffsetAndScale()
{
    // Distortion center shift is stored separately, since it isn't affected
    // by the eye distance.
    float lensOffset        = HMD.LensSeparationDistance * 0.5f;
    float lensShift         = HMD.HScreenSize * 0.25f - lensOffset;
    float lensViewportShift = 4.0f * lensShift / HMD.HScreenSize;
    Distortion.XCenterOffset= lensViewportShift;

    // Compute distortion scale from DistortionFitX & DistortionFitY.
    // Fit value of 0.0 means "no fit".
    if ((fabs(DistortionFitX) < 0.0001f) &&  (fabs(DistortionFitY) < 0.0001f))
    {
        Distortion.Scale = 1.0f;
    }
    else
    {
        // Convert fit value to distortion-centered coordinates before fit radius
        // calculation.
        float stereoAspect = 0.5f * float(FullView.w) / float(FullView.h);
        float dx           = DistortionFitX - Distortion.XCenterOffset;
        float dy           = DistortionFitY / stereoAspect;
        float fitRadius    = sqrt(dx * dx + dy * dy);
        Distortion.Scale   = Distortion.CalcScale(fitRadius);
    }
}

void StereoConfig::updateProjectionOffset()
{
    // Post-projection viewport coordinates range from (-1.0, 1.0), with the
    // center of the left viewport falling at (1/4) of horizontal screen size.
    // We need to shift this projection center to match with the eye center
    // corrected by IPD. We compute this shift in physical units (meters) to
    // correct for different screen sizes and then rescale to viewport coordinates.
    float viewCenter         = HMD.HScreenSize * 0.25f;
    float eyeProjectionShift = viewCenter - InterpupillaryDistance*0.5f;
    ProjectionCenterOffset   = 4.0f * eyeProjectionShift / HMD.HScreenSize;

    /*
    // TBD.
    // This more advanced logic attempts to correct IPD with distortion function.
    // Generally works (produces same image regardles of FOV for different IPDs),
    // however IPD==0 doesn't meet at the center as it should.
    // Might be a problem with meters to distortion function units conversions (or back).
    
    float lensToIPD_m                    = (HMD.LensSeparationDistance - InterpupillaryDistance)/2;
    float lensToIPD_du                   = (4.0f/HMD.HScreenSize) * lensToIPD_m; // distortion units
    float preDistortLensToIPD_du         = Distortion.CalcScaleInverse(lensToIPD_du);
    float preScalePreDistortLensToIPD_du = preDistortLensToIPD_du / Distortion.Scale;

    float preScalePreDistortLensToIPD_m  = preScalePreDistortLensToIPD_du / (4.0f/HMD.HScreenSize);    
    float lensToViewCenter_m             = HMD.HScreenSize * 0.25f - HMD.LensSeparationDistance * 0.5f;
    float viewCenterToIPDPoint_m         = lensToViewCenter_m + preScalePreDistortLensToIPD_m;

    ProjectionCenterOffset = 4.0f * viewCenterToIPDPoint_m / HMD.HScreenSize;
    */
}

void StereoConfig::update2D()
{
    // Orthographic projection fakes a screen at a distance of 0.8m from the
    // eye, where hmd screen projection surface is at 0.05m distance.
    // This introduces an extra off-center pixel projection shift based on eye distance.
    // This offCenterShift is the pixel offset of the other camera's center
    // in your reference camera based on surface distance.
    float eyeDistanceScreenPixels = (HMD.HResolution / HMD.HScreenSize) * InterpupillaryDistance;
    float offCenterShiftPixels    = (HMD.EyeToScreenDistance / 0.8f) * eyeDistanceScreenPixels;
    float leftPixelCenter         = (HMD.HResolution / 2) - eyeDistanceScreenPixels / 2;
    float rightPixelCenter        = eyeDistanceScreenPixels / 2;
    float pixelDifference         = leftPixelCenter - rightPixelCenter;

    
    // This computes the number of pixels that fit within specified 2D FOV (assuming
    // distortion scaling will be done).
    float percievedHalfScreenDistance = tan(Area2DFov * 0.5f) * HMD.EyeToScreenDistance;
    float vfovSize = 2.0f * percievedHalfScreenDistance / Distortion.Scale;
    FovPixels = HMD.VResolution * vfovSize / HMD.VScreenSize;
    
    // Create orthographic matrix.   
    Matrix4f& m      = OrthoCenter;
    m.SetIdentity();
    m.M[0][0] = FovPixels / (FullView.w * 0.5f);
    m.M[1][1] = -FovPixels / FullView.h;
    m.M[0][3] = 0;
    m.M[1][3] = 0;
    m.M[2][2] = 0;

    float orthoPixelOffset = (pixelDifference + offCenterShiftPixels/Distortion.Scale) * 0.5f;
    OrthoPixelOffset = orthoPixelOffset * 2.0f / FovPixels;
}

void StereoConfig::updateEyeParams()
{
    // Projection matrix for the center eye, which the left/right matrices are based on.
    Matrix4f projCenter = Matrix4f::PerspectiveRH(YFov, Aspect, 0.3f, 1000.0f);
   
    switch(Mode)
    {
    case Stereo_None:
        {
            EyeRenderParams[0].Init(StereoEye_Center, FullView, 0, projCenter, OrthoCenter);
        }
        break;

    case Stereo_LeftRight_Multipass:
        {
            Matrix4f projLeft  = Matrix4f::Translation(ProjectionCenterOffset, 0, 0) * projCenter,
                     projRight = Matrix4f::Translation(-ProjectionCenterOffset, 0, 0) * projCenter;

            EyeRenderParams[0].Init(StereoEye_Left,
                Viewport(FullView.x, FullView.y, FullView.w/2, FullView.h),
                         +InterpupillaryDistance * 0.5f,  // World view shift.                       
                         projLeft, OrthoCenter * Matrix4f::Translation(OrthoPixelOffset, 0, 0),
                         &Distortion);
            EyeRenderParams[1].Init(StereoEye_Right,
                Viewport(FullView.x + FullView.w/2, FullView.y, FullView.w/2, FullView.h),
                         -InterpupillaryDistance * 0.5f,                         
                         projRight, OrthoCenter * Matrix4f::Translation(-OrthoPixelOffset, 0, 0),
                         &Distortion);

            EyeRenderParams[0].ProjectionCenterOffset = ProjectionCenterOffset;
            EyeRenderParams[1].ProjectionCenterOffset = -ProjectionCenterOffset;
        }
        break;
    }

}



}}  // OVR::Render

