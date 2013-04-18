/************************************************************************************

Filename    :   OculusWorldDemo.cpp
Content     :   First-person view test application for Oculus Rift
Created     :   October 4, 2012
Authors     :   Michael Antonov, Andrew Reisse

Copyright   :   Copyright 2012 Oculus, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus Inc license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#include "OVR.h"

#include "../CommonSrc/Platform/Platform_Default.h"
#include "../CommonSrc/Renderer/Renderer.h"
#include "../CommonSrc/Renderer/Renderer_Stereo.h"
#include "../CommonSrc/Renderer/FontEmbed_DejaVu48.h"

#include <Kernel/OVR_SysFile.h>

using namespace OVR;
using namespace OVR::Platform;
using namespace OVR::Render;


//-------------------------------------------------------------------------------------
// ***** OculusWorldDemo Description

// This app renders a simple flat-shaded room allowing the user to move along the 
// floor and look around with an HMD, mouse and keyboard. The following keys work:
//
//  'W', 'S', 'A', 'D' - Move forward, back; strafe left/right.
//  F1 - No stereo, no distortion.
//  F2 - Stereo, no distortion.
//  F3 - Stereo and distortion.
//  F8 - Toggle MSAA.
//  F9 - Set FullScreen mode on the HMD; necessary for previwing content with Rift.
//
// Important Oculus-specific logic can be found at following locations:
//
//  OculusWorldDemoApp::OnStartup - This function will initialize OVR::DeviceManager and HMD,
//                                  creating SensorDevice and attaching it to SensorFusion.
//                                  This needs to be done before obtaining sensor data.
//
//  OculusWorldDemoApp::OnIdle    - Here we poll SensorFusion for orientation, apply it
//                                  to the scene and handle movement.
//                                  Stereo rendering is also done here, by delegating to
//                                  to Render function for each eye.
// 

//-------------------------------------------------------------------------------------
// The RHS coordinate system is defines as follows (as seen in perspective view):
//  Y - Up
//  Z - Back
//  X - Right
const Vector3f UpVector(0.0f, 1.0f, 0.0f);
const Vector3f ForwardVector(0.0f, 0.0f, -1.0f);
const Vector3f RightVector(1.0f, 0.0f, 0.0f);

// We start out looking in the positive Z (180 degree rotation).
const float    YawInitial = 3.141592f;
const float    Sensitivity = 1.0f;
const float    MoveSpeed   = 3.0f; // m/s


//-------------------------------------------------------------------------------------
// ***** OculusWorldDemo Application class

// An instance of this class is created on application startup (main/WinMain).
// It then works as follows:
//  - Graphics and HMD setup is done OculusWorldDemoApp::OnStartup(). This function
//    also creates the room model from Slab declarations.
//  - Per-frame processing is done in OnIdle(). This function processes
//    sensor and movement input and then renders the frame.
//  - Additional input processing is done in OnMouse, OnKey and OnGamepad.

class OculusWorldDemoApp : public Application, public MessageHandler
{
public:
    OculusWorldDemoApp();
    ~OculusWorldDemoApp();

    virtual int  OnStartup(int argc, const char** argv);
    virtual void OnIdle();

    virtual void OnMouseMove(int x, int y, int modifiers);
    virtual void OnKey(KeyCode key, int chr, bool down, int modifiers);
    virtual void OnGamepad(const GamepadState& pad);
    virtual void OnResize(int width, int height);

    virtual void OnMessage(const Message& msg);

    void         Render(const StereoRenderParams& stereo);

    // Sets temporarily displayed message for adjustments
    void         SetAdjustMessage(const char* format, ...);

    // Stereo setting adjustment functions.
    // Called with deltaTime when relevant key is held.
    void         AdjustFov(float dt);
    void         AdjustAspect(float dt);
    void         AdjustIPD(float dt)
    {        
        SConfig.SetIPD(SConfig.GetIPD() + 0.0025f * dt);
        SetAdjustMessage("EyeDistance: %6.4f", SConfig.GetIPD());
    }
    
    void         AdjustDistortion(float dt, int kIndex, const char* label);
    void         AdjustDistortionK0(float dt) { AdjustDistortion(dt, 0, "K0"); }
    void         AdjustDistortionK1(float dt) { AdjustDistortion(dt, 1, "K1"); }
    void         AdjustDistortionK2(float dt) { AdjustDistortion(dt, 2, "K2"); }
    void         AdjustDistortionK3(float dt) { AdjustDistortion(dt, 3, "K3"); }

    // Adds room model to scene.
    void         PopulateScene();

protected:
    Renderer*                   pRender;
    RendererParams              RenderParams;
    int                         Width, Height;
    
    // *** Oculus HMD Variables
    Ptr<DeviceManager>          pManager;
    Ptr<SensorDevice>           pSensor;
    Ptr<HMDDevice>              pHMD;
    SensorFusion                SFusion;
    HMDInfo                     HMDInfo;

    Ptr<LatencyTestDevice>      pLatencyTester;
    LatencyTestUtil             LatencyUtil;

    double                      LastUpdate;

    int                         FPS;
    int                         FrameCounter;
    double                      NextFPSUpdate;

    // Position and look. The following apply:
    Vector3f                    EyePos;
    float                       EyeYaw;         // Rotation around Y, CCW positive when looking at RHS (X,Z) plane.
    float                       EyePitch;       // Pitch. If sensor is plugged in, only read from sensor.
    float                       EyeRoll;        // Roll, only accessible from Sensor.
    float                       LastSensorYaw;  // Stores previous Yaw value from to support computing delta.

    // Movement state; different bits may be set based on the state of keys.
    UByte                       MoveForward;
    UByte                       MoveBack;
    UByte                       MoveLeft;
    UByte                       MoveRight;
    Vector3f                    GamepadMove, GamepadRotate;

    Matrix4f                    View;
    Scene                       GridScene;
    Scene                       Scene;    
    Ptr<ShaderFill>             LitSolid, LitTextures[4];

      // Stereo view parameters.
    StereoConfig                SConfig;
    PostProcessType             PostProcess;
 
    String                      AdjustMessage;
    double                      AdjustMessageTimeout;

    // Saved distortion state.
    float                       SavedK0, SavedK1, SavedK2, SavedK3;
    float                       SavedESD, SavedAspect, SavedEyeDistance;

    // Allows toggling color around distortion.
    Color                       DistortionClearColor;

    // Stereo settings adjustment state.
    typedef void (OculusWorldDemoApp::*AdjustFuncType)(float);
    bool                        ShiftDown;
    AdjustFuncType              pAdjustFunc;    
    float                       AdjustDirection;

    enum SceneRenderMode
    {
        Scene_World,
        Scene_Grid,
        Scene_Both,

    };
    SceneRenderMode             SceneMode;


    enum TextScreen
    {
        Text_None,
        Text_Orientation,
        Text_Config,
        Text_Help,
        Text_Count
    };
    TextScreen                  TextScreen;

    Model* OculusWorldDemoApp::CreateModel(Vector3f pos, struct SlabModel* sm);
};

//-------------------------------------------------------------------------------------

OculusWorldDemoApp::OculusWorldDemoApp()
    : pRender(0),
      LastUpdate(0),
      // Initial location
      EyePos(0.0f, 1.6f, -5.0f),
      EyeYaw(YawInitial), EyePitch(0), EyeRoll(0),
      LastSensorYaw(0),
      SConfig(),
      PostProcess(PostProcess_None),   
      DistortionClearColor(0,0,0),

      ShiftDown(false),
      pAdjustFunc(0),
      AdjustDirection(1.0f),
      SceneMode(Scene_World),
      TextScreen(Text_None)
{
    Width  = 1280;
    Height = 800;
   
    FPS          = 0;
    FrameCounter = 0;
    NextFPSUpdate= 0;
    
    MoveForward   = MoveBack = MoveLeft = MoveRight = 0;
    GamepadMove   = Vector3f(0);
    GamepadRotate = Vector3f(0);

    AdjustMessageTimeout = 0;
}

OculusWorldDemoApp::~OculusWorldDemoApp()
{
	RemoveHandlerFromDevices();

    if (DejaVu.fill)
        DejaVu.fill->Release();
    pSensor.Clear();
    pHMD.Clear();
}

int OculusWorldDemoApp::OnStartup(int argc, const char** argv)
{
    OVR::HMDInfo hmd;

    // Report relative mouse motion in OnMouseMove
    pPlatform->SetMouseMode(Mouse_Relative);

    // *** Oculus HMD & Sensor Initialization

    // Create DeviceManager and first available HMDDevice from it.
    // Sensor object is created from the HMD, to ensure that it is on the
    // correct device.

    pManager = *DeviceManager::Create();

	// We'll handle it's messages in this case.
	pManager->SetMessageHandler(this);

    pHMD     = *pManager->EnumerateDevices<HMDDevice>().CreateDevice();    
    if (pHMD)
    {
        pSensor = *pHMD->GetSensor();

        // This will initialize HMDInfo with information about configured IPD,
        // screen size and other variables needed for correct projection.
        // We pass HMD DisplayDeviceName into the renderer to select the
        // correct monitor in full-screen mode.
        if (pHMD->GetDeviceInfo(&hmd))
        {            
            RenderParams.MonitorName = hmd.DisplayDeviceName;
            SConfig.SetHMDInfo(hmd);
        }
    }
    else
    {
        // If we didn't detect an HMD, try to create the sensor directly.
        // This is useful for debugging sensor interaction; it is not needed in
        // a shipping app.
        pSensor = *pManager->EnumerateDevices<SensorDevice>().CreateDevice();
	}

    // Create the Latency Tester device and assign it to the LatencyTesterUtil object.
    pLatencyTester = *pManager->EnumerateDevices<LatencyTestDevice>().CreateDevice();
    if (pLatencyTester)
    {
        LatencyUtil.SetDevice(pLatencyTester);
    }

	// Make the user aware which devices are present.
	if (pHMD == NULL && pSensor == NULL)
	{
		SetAdjustMessage("---------------------------------\nNO HMD DETECTED\nNO SENSOR DETECTED\n---------------------------------");
	}
	else if (pHMD == NULL)
	{
		SetAdjustMessage("----------------------------\nNO HMD DETECTED\n----------------------------");
	}
	else if (pSensor == NULL)
	{
		SetAdjustMessage("---------------------------------\nNO SENSOR DETECTED\n---------------------------------");
	}


    if (hmd.HResolution > 0)
    {
        Width  = hmd.HResolution;
        Height = hmd.VResolution;
    }

    if (!pPlatform->SetupWindow(Width,Height))
        return 1;

    String Title = "Oculus World Demo";
    if (hmd.ProductName[0])
    {
        Title += " : ";
        Title += hmd.ProductName;
    }
    pPlatform->SetWindowTitle(Title);

    if (pSensor)
    {
        // We need to attach sensor to SensorFusion object for it to receive 
        // body frame messages and update orientation. SFusion.GetOrientation() 
        // is used in OnIdle() to orient the view.
        SFusion.AttachToSensor(pSensor);

        SFusion.SetDelegateMessageHandler(this);
    }

    
    // *** Initialize Rendering

    const char* graphics = "d3d11";

    // Select renderer based on command line arguments.
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-r") && i < argc-1)
            graphics = argv[i+1];
        else if (!strcmp(argv[i], "-fs"))
            RenderParams.Fullscreen = true;
    }

    // Enable multi-sampling by default.
    RenderParams.Multisample = 4;
    pRender = pPlatform->SetupGraphics(graphics, RenderParams);
  


    // *** Configure Stereo settings.

    SConfig.SetFullViewport(Viewport(0,0, Width, Height));
    SConfig.SetStereoMode(Stereo_LeftRight_Multipass);

    // Configure proper Distortion Fit.
    // For 7" screen, fit to touch left side of the view, leaving a bit of invisible
    // screen on the top (saves on rendering cost).
    // For smaller screens (5.5"), fit to the top.
    if (hmd.HScreenSize > 0.140f) // 7"
        SConfig.SetDistortionFitPointVP(-1.0f, 0.0f);
    else
        SConfig.SetDistortionFitPointVP(0.0f, 1.0f);

    pRender->SetSceneRenderScale(SConfig.GetDistortionScale());

    SConfig.Set2DAreaFov(DegreeToRad(85.0f));


    // *** Populate Room Scene

    // This creates lights and models.
    PopulateScene();


    LastUpdate = pPlatform->GetAppTime();
    return 0;
}

void OculusWorldDemoApp::OnMessage(const Message& msg)
{
	if (msg.Type == Message_DeviceAdded && msg.pDevice == pManager)
	{
		LogText("DeviceManager reported device added.\n");
	}
	else if (msg.Type == Message_DeviceRemoved && msg.pDevice == pManager)
	{
		LogText("DeviceManager reported device removed.\n");
	}
	else if (msg.Type == Message_DeviceAdded && msg.pDevice == pSensor)
	{
		LogText("Sensor reported device added.\n");
	}
	else if (msg.Type == Message_DeviceRemoved && msg.pDevice == pSensor)
	{
		LogText("Sensor reported device removed.\n");
	}
}

void OculusWorldDemoApp::OnResize(int width, int height)
{
    Width  = width;
    Height = height;
    SConfig.SetFullViewport(Viewport(0,0, Width, Height));
}

void OculusWorldDemoApp::OnGamepad(const GamepadState& pad)
{
    GamepadMove   = Vector3f(pad.LX * pad.LX * (pad.LX > 0 ? 1 : -1),
                             0,
                             pad.LY * pad.LY * (pad.LY > 0 ? -1 : 1));
    GamepadRotate = Vector3f(2 * pad.RX, -2 * pad.RY, 0);
}

void OculusWorldDemoApp::OnMouseMove(int x, int y, int modifiers)
{
    if (modifiers & Mod_MouseRelative)
    {
        // Get Delta
        int dx = x, dy = y;
 
        const float maxPitch = ((3.1415f/2)*0.98f);

        // Apply to rotation. Subtract for right body frame rotation,
        // since yaw rotation is positive CCW when looking down on XZ plane.
        EyeYaw   -= (Sensitivity * dx)/ 360.0f;

        if (!pSensor)
        {
            EyePitch -= (Sensitivity * dy)/ 360.0f;
            
            if (EyePitch > maxPitch)
                EyePitch = maxPitch;
            if (EyePitch < -maxPitch)
                EyePitch = -maxPitch;
        }
    }
}


void OculusWorldDemoApp::OnKey(KeyCode key, int chr, bool down, int modifiers)
{
    OVR_UNUSED(chr);

    switch (key)
    {
    case Key_Q:
        if (down && (modifiers & Mod_Control))
            pPlatform->Exit(0);
        break;

    // Handle player movement keys.
    // We just update movement state here, while the actual translation is done in OnIdle()
    // based on time.
    case Key_W:     MoveForward = down ? (MoveForward | 1) : (MoveForward & ~1); break;
    case Key_S:     MoveBack    = down ? (MoveBack    | 1) : (MoveBack    & ~1); break;
    case Key_A:     MoveLeft    = down ? (MoveLeft    | 1) : (MoveLeft    & ~1); break;
    case Key_D:     MoveRight   = down ? (MoveRight   | 1) : (MoveRight   & ~1); break;
    case Key_Up:    MoveForward = down ? (MoveForward | 2) : (MoveForward & ~2); break;
    case Key_Down:  MoveBack    = down ? (MoveBack    | 2) : (MoveBack    & ~2); break;

    case Key_B:
        if (down)
        {            
            if (SConfig.GetDistortionScale() == 1.0f)
            {
                if (SConfig.GetHMDInfo().HScreenSize > 0.140f) // 7"
                    SConfig.SetDistortionFitPointVP(-1.0f, 0.0f);
                else
                    SConfig.SetDistortionFitPointVP(0.0f, 1.0f);
            }
            else
            {
                // No fitting; scale == 1.0.
                SConfig.SetDistortionFitPointVP(0, 0);
            }
        }
        break;


    // Support toggling background color doe distortion so that we can see
    // effect on the periphery.
    case Key_V:
        if (down)
        {
            if (DistortionClearColor.B == 0)
                DistortionClearColor = Color(0,128,255);
            else
                DistortionClearColor = Color(0,0,0);

            pRender->SetDistortionClearColor(DistortionClearColor);
        }
        break;
    
    case Key_F1:
        SConfig.SetStereoMode(Stereo_None);
        PostProcess = PostProcess_None;
        SetAdjustMessage("StereoMode: None");
        break;
    case Key_F2:
        SConfig.SetStereoMode(Stereo_LeftRight_Multipass);
        PostProcess = PostProcess_None;
        SetAdjustMessage("StereoMode: Stereo + No Distortion");
        break;
        
    case Key_F3:
        SConfig.SetStereoMode(Stereo_LeftRight_Multipass);
        PostProcess = PostProcess_Distortion;
        SetAdjustMessage("StereoMode: Stereo + Distortion");
        break;

    case Key_R:
        SFusion.Reset();
        SetAdjustMessage("Sensor Fusion Reset");
        break;

    case Key_Space:
        if (!down)
            TextScreen = (enum TextScreen)((TextScreen + 1) % Text_Count);
        break;

    case Key_F8:
        if (!down)
        {
            RenderParams.Multisample = RenderParams.Multisample > 1 ? 1 : 4;
            pRender->SetParams(RenderParams);
            if (RenderParams.Multisample > 1)
                SetAdjustMessage("Multisampling On");
            else
                SetAdjustMessage("Multisampling Off");
        }
        break;
    case Key_F9:
        if (!down)
        {
            pRender->SetFullscreen(pRender->IsFullscreen() ? Display_Window : Display_Fullscreen);
            // If using an HMD, enable post-process (for distortion) and stereo.
            if (RenderParams.MonitorName.GetLength() && pRender->IsFullscreen())
            {
                SConfig.SetStereoMode(Stereo_LeftRight_Multipass);                
                PostProcess = PostProcess_Distortion;
            }
        }
        break;

    case Key_F11:
        if (!down)
        {
            pPlatform->SetMouseMode(Mouse_Normal);
            pRender->SetFullscreen(pRender->IsFullscreen() ? Display_Window : Display_FakeFullscreen);
            pPlatform->SetMouseMode(Mouse_Relative); // Avoid mode world rotation jump.
            // If using an HMD, enable post-process (for distortion) and stereo.
            if (RenderParams.MonitorName.GetLength() && pRender->IsFullscreen())
            {
                SConfig.SetStereoMode(Stereo_LeftRight_Multipass);                
                PostProcess = PostProcess_Distortion;
            }
        }
        break;

    case Key_Escape:
        if (!down)
            pRender->SetFullscreen(Display_Window);
        break;
    
    // Stereo adjustments.    
    case Key_BracketLeft:  pAdjustFunc = down ? &OculusWorldDemoApp::AdjustFov    : 0;  AdjustDirection = 1;  break;
    case Key_BracketRight: pAdjustFunc = down ? &OculusWorldDemoApp::AdjustFov    : 0;  AdjustDirection = -1; break;
    case Key_Equal:    
    case Key_Insert:       pAdjustFunc = down ? &OculusWorldDemoApp::AdjustIPD    : 0;  AdjustDirection = 1;  break;
    case Key_Minus:
    case Key_Delete:       pAdjustFunc = down ? &OculusWorldDemoApp::AdjustIPD    : 0;  AdjustDirection = -1; break;
    case Key_PageUp:       pAdjustFunc = down ? &OculusWorldDemoApp::AdjustAspect : 0;  AdjustDirection = 1;  break;
    case Key_PageDown:     pAdjustFunc = down ? &OculusWorldDemoApp::AdjustAspect : 0;  AdjustDirection = -1; break;

    // Distortion correction adjustments
    case Key_H:            pAdjustFunc = down ? &OculusWorldDemoApp::AdjustDistortionK0 : NULL; AdjustDirection = -1;  break;
    case Key_Y:            pAdjustFunc = down ? &OculusWorldDemoApp::AdjustDistortionK0 : NULL; AdjustDirection = 1;   break;
    case Key_J:            pAdjustFunc = down ? &OculusWorldDemoApp::AdjustDistortionK1 : NULL; AdjustDirection = -1;  break;
    case Key_U:            pAdjustFunc = down ? &OculusWorldDemoApp::AdjustDistortionK1 : NULL; AdjustDirection = 1;   break;
    case Key_K:            pAdjustFunc = down ? &OculusWorldDemoApp::AdjustDistortionK2 : NULL; AdjustDirection = -1;  break;
    case Key_I:            pAdjustFunc = down ? &OculusWorldDemoApp::AdjustDistortionK2 : NULL; AdjustDirection = 1;   break;
    case Key_L:            pAdjustFunc = down ? &OculusWorldDemoApp::AdjustDistortionK3 : NULL; AdjustDirection = -1;  break;
    case Key_O:            pAdjustFunc = down ? &OculusWorldDemoApp::AdjustDistortionK3 : NULL; AdjustDirection = 1;   break;

    case Key_Tab:
        if (down)
        {
            float t0      = SConfig.GetDistortionK(0),
                  t1      = SConfig.GetDistortionK(1),
                  t2      = SConfig.GetDistortionK(2),
                  t3      = SConfig.GetDistortionK(3);
            float tESD    = SConfig.GetEyeToScreenDistance(),
                  taspect = SConfig.GetAspectMultiplier(),
                  tipd    = SConfig.GetIPD();

            if (SavedK0 > 0.0f)
            {
                SConfig.SetDistortionK(0, SavedK0);
                SConfig.SetDistortionK(1, SavedK1);
                SConfig.SetDistortionK(2, SavedK2);
                SConfig.SetDistortionK(3, SavedK3);
                SConfig.SetEyeToScreenDistance(SavedESD);
                SConfig.SetAspectMultiplier(SavedAspect);
                SConfig.SetIPD(SavedEyeDistance);

                SetAdjustMessage("Restored:\n"
                                 "ESD:\t120 %.3f\t350 Eye:\t490 %.3f\n"
                                 "K0: \t120 %.4f\t350 K2: \t490 %.4f\n"
                                 "K1: \t120 %.4f\t350 K3: \t490 %.4f",
                                  SavedESD, SavedEyeDistance,
                                  SavedK0, SavedK2,
                                  SavedK1, SavedK3);
            }
            else
            {
                SetAdjustMessage("Setting Saved");
            }

            SavedK0 = t0;
            SavedK1 = t1;
            SavedK2 = t2;
            SavedK3 = t3;
            SavedESD = tESD;
            SavedAspect = taspect;
            SavedEyeDistance = tipd;
        }        
        break;

 
    case Key_G:
        if (down)
        {
            if (SceneMode == Scene_World)
            {
                SceneMode = Scene_Grid;
                SetAdjustMessage("Grid Only");
            }
            else if (SceneMode == Scene_Grid)
            {
                SceneMode = Scene_Both;
                SetAdjustMessage("Grid Overlay");
            }
            else if (SceneMode == Scene_Both)
            {
                SceneMode = Scene_World;
                SetAdjustMessage("Grid Off");
            }
            break;
        }

    // Holding down Shift key accelerates adjustment velocity.
    case Key_Shift:
        ShiftDown = down;
        break;
    }
}


void OculusWorldDemoApp::OnIdle()
{
    double curtime = pPlatform->GetAppTime();
    float  dt      = float(curtime - LastUpdate);
    LastUpdate     = curtime;

    // If one of Stereo setting adjustment keys is pressed, adjust related state.
    if (pAdjustFunc)
    {
        (this->*pAdjustFunc)(dt * AdjustDirection * (ShiftDown ? 5.0f : 1.0f));
    }

    // Handle Sensor motion.
    // We extract Yaw, Pitch, Roll instead of directly using the orientation
    // to allow "additional" yaw manipulation with mouse/controller.
    if (pSensor)
    {        
        Quatf    hmdOrient = SFusion.GetOrientation();

        // Have to place this as close as possible to where the HMD orientation is read.
        LatencyUtil.ProcessInputs();

        float    yaw = 0.0f;
        hmdOrient.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&yaw, &EyePitch, &EyeRoll);

        EyeYaw += (yaw - LastSensorYaw);
        LastSensorYaw = yaw;
        
        // NOTE: We can get a matrix from orientation as follows:
        // Matrix4f hmdMat(hmdOrient);

        // Test logic - assign quaternion result directly to view:
        // Quatf hmdOrient = SFusion.GetOrientation();
        // View = Matrix4f(hmdOrient.Inverted()) * Matrix4f::Translation(-EyePos);
    }    


    if (curtime >= NextFPSUpdate)
    {
        NextFPSUpdate = curtime + 1.0;
        FPS = FrameCounter;
        FrameCounter = 0;
    }
    FrameCounter++;

    
    EyeYaw -= GamepadRotate.x * dt;

    if (!pSensor)
    {
        EyePitch -= GamepadRotate.y * dt;

        const float maxPitch = ((3.1415f/2)*0.98f);
        if (EyePitch > maxPitch)
            EyePitch = maxPitch;
        if (EyePitch < -maxPitch)
            EyePitch = -maxPitch;
    }
    

    // Handle keyboard movement.
    // This translates EyePos based on Yaw vector direction and keys pressed.
    // Note that Pitch and Roll do not affect movement (they only affect view).
    if (MoveForward || MoveBack || MoveLeft || MoveRight)
    {
        Vector3f localMoveVector(0,0,0);
        Matrix4f yawRotate = Matrix4f::RotationY(EyeYaw);

        if (MoveForward)
            localMoveVector = ForwardVector;
        else if (MoveBack)
            localMoveVector = -ForwardVector;

        if (MoveRight)
            localMoveVector += RightVector;
        else if (MoveLeft)
            localMoveVector -= RightVector;

        // Normalize vector so we don't move faster diagonally.
        localMoveVector.Normalize();
        Vector3f orientationVector = yawRotate.Transform(localMoveVector);
        orientationVector *= MoveSpeed * dt * (ShiftDown ? 3.0f : 1.0f);

        EyePos += orientationVector;
    }

    else if (GamepadMove.LengthSq() > 0)
    {
        Matrix4f yawRotate = Matrix4f::RotationY(EyeYaw);
        Vector3f orientationVector = yawRotate.Transform(GamepadMove);
        orientationVector *= MoveSpeed * dt;
        EyePos += orientationVector;
    }


    // Rotate and position View Camera, using YawPitchRoll in BodyFrame coordinates.
    // 
    Matrix4f rollPitchYaw = Matrix4f::RotationY(EyeYaw) * Matrix4f::RotationX(EyePitch) *
                            Matrix4f::RotationZ(EyeRoll);
    Vector3f up      = rollPitchYaw.Transform(UpVector);
    Vector3f forward = rollPitchYaw.Transform(ForwardVector);

    
    // Minimal head modelling; should be moved as an option to SensorFusion.
    float headBaseToEyeHeight     = 0.15f;  // Vertical height of eye from base of head
    float headBaseToEyeProtrusion = 0.09f;  // Distance forward of eye from base of head

    Vector3f eyeCenterInHeadFrame(0.0f, headBaseToEyeHeight, -headBaseToEyeProtrusion);
    Vector3f shiftedEyePos = EyePos + rollPitchYaw.Transform(eyeCenterInHeadFrame);
    shiftedEyePos.y -= eyeCenterInHeadFrame.y; // Bring the head back down to original height
    View = Matrix4f::LookAtRH(shiftedEyePos, shiftedEyePos + forward, up); 

    //  Transformation without head modelling.    
    // View = Matrix4f::LookAtRH(EyePos, EyePos + forward, up);    

    // This is an alternative to LookAtRH:
    // Here we transpose the rotation matrix to get its inverse.
    //  View = (Matrix4f::RotationY(EyeYaw) * Matrix4f::RotationX(EyePitch) *
    //                                        Matrix4f::RotationZ(EyeRoll)).Transposed() * 
    //         Matrix4f::Translation(-EyePos);
   

    switch(SConfig.GetStereoMode())
    {
    case Stereo_None:
        Render(SConfig.GetEyeRenderParams(StereoEye_Center));
        break;

    case Stereo_LeftRight_Multipass:
    //case Stereo_LeftDouble_Multipass:
        Render(SConfig.GetEyeRenderParams(StereoEye_Left));
        Render(SConfig.GetEyeRenderParams(StereoEye_Right));
        break;
    }
     
    pRender->Present();
    // Force GPU to flush the scene, resulting in the lowest possible latency.
    pRender->ForceFlushGPU();
}



static const char* HelpText =
    "F1\t100 NoStereo   \t450 Ins/Del \t650 ViewOffset\n"
    "F2\t100 Stereo     \t450 [ ]     \t650 FOV\n"
    "F3\t100 StereoHMD  \t450 H-Y J-U \t650 Distortion 1,3\n"
    "F4\t100 MonoHMD    \t450 K-I L-O \t650 Distortion 5,7\n"
    "F8\t100 MSAA       \t450 Shift   \t650 Adjust Faster\n"
    "F9\t100 FullScreen \t450 F11     \t650 Fast FullScreen\n"
    "R \t100 Reset SensorFusion"    
    ;


enum DrawTextCenterType
{
    DrawText_NoCenter= 0,
    DrawText_VCenter = 0x1,
    DrawText_HCenter = 0x2,
    DrawText_Center  = DrawText_VCenter | DrawText_HCenter
};

static void DrawTextBox(Renderer* prender, float x, float y,
                        float textSize, const char* text,
                        DrawTextCenterType centerType = DrawText_NoCenter)
{
    float ssize[2] = {0.0f, 0.0f};

    prender->MeasureText(&DejaVu, text, textSize, ssize);

    // Treat 0 a VCenter.
    if (centerType & DrawText_HCenter)
        x = -ssize[0]/2;    
    if (centerType & DrawText_VCenter)
        y = -ssize[1]/2;

    prender->FillRect(x-0.02f, y-0.02f, x+ssize[0]+0.02f, y+ssize[1]+0.02f, Color(40,40,100,210));
    prender->RenderText(&DejaVu, text, x, y, textSize, Color(255,255,0,210));
}

void OculusWorldDemoApp::Render(const StereoRenderParams& stereo)
{
    pRender->BeginScene(PostProcess);

    // *** 3D - Configures Viewport/Projection and Render
    stereo.Apply(pRender);
    pRender->Clear();

    pRender->SetDepthMode(true, true);
    if (SceneMode != Scene_Grid)
        Scene.Render(pRender, stereo.ViewAdjust * View);


    // *** 2D Text & Grid - Configure Orthographic rendering.    
    
    // Render UI in 2D orthographic coordinate system that maps [-1,1] range
    // to a readable FOV area centered at your eye and properly adjusted.
    stereo.Apply2D(pRender);
    pRender->SetDepthMode(false, false);

    float unitPixel = SConfig.Get2DUnitPixel();
    float textHeight= unitPixel * 22; 

    if (SceneMode != Scene_World)
    {   // Draw grid two pixels thick.
        GridScene.Render(pRender, Matrix4f());
        GridScene.Render(pRender, Matrix4f::Translation(unitPixel,unitPixel,0));
    }


    if (AdjustMessageTimeout > pPlatform->GetAppTime())
    {
        DrawTextBox(pRender,0,0.5f, textHeight, AdjustMessage.ToCStr(), DrawText_HCenter);
    }    

    switch(TextScreen)
    {
    case Text_Orientation:
        {
            char buf[256];
            OVR_sprintf(buf, sizeof(buf),
                        " Yaw:%4.0f  Pitch:%4.0f  Roll:%4.0f \n"
                        " FPS: %d  Frame: %d",
                        RadToDegree(EyeYaw), RadToDegree(EyePitch), RadToDegree(EyeRoll),
                        FPS, FrameCounter);
            DrawTextBox(pRender, 0, 0.05f, textHeight, buf, DrawText_HCenter);
        }
        break;

    case Text_Config:
        {
            char   textBuff[2048];
              
            OVR_sprintf(textBuff, sizeof(textBuff),
                        "Fov\t300 %9.4f\n"
                        "EyeDistance\t300 %9.4f\n"
                        "DistortionK0\t300 %9.4f\n"
                        "DistortionK1\t300 %9.4f\n"
                        "DistortionK2\t300 %9.4f\n"
                        "DistortionK3\t300 %9.4f\n"
                        "TexScale\t300 %9.4f",
                        SConfig.GetYFOVDegrees(),
                        SConfig.GetIPD(),
                        SConfig.GetDistortionK(0),
                        SConfig.GetDistortionK(1),
                        SConfig.GetDistortionK(2),
                        SConfig.GetDistortionK(3),
                        SConfig.GetDistortionScale());
            
            DrawTextBox(pRender, 0, 0, textHeight, textBuff, DrawText_Center);
        }
        break;

    case Text_Help:
        DrawTextBox(pRender, 0, 0, textHeight, HelpText, DrawText_Center);
    }


    // Display colored quad if we're doing a latency test.
    ColorRGB colorToDisplay;
    if (LatencyUtil.DisplayScreenColor(colorToDisplay))
    {
        Color rectCol(colorToDisplay.R, colorToDisplay.G, colorToDisplay.B);
        pRender->FillRect(-0.4f, -0.4f, 0.4f, 0.4f, rectCol);
    }

    pRender->FinishScene();
}

// Sets temporarily displayed message for adjustments
void OculusWorldDemoApp::SetAdjustMessage(const char* format, ...)
{
    char textBuff[2048];
    va_list argList;
    va_start(argList, format);
    OVR_vsprintf(textBuff, sizeof(textBuff), format, argList);
    va_end(argList);

    // Message will time out in 4 seconds.
    AdjustMessage = textBuff;
    AdjustMessageTimeout = pPlatform->GetAppTime() + 4.0f;    
}

// ***** View Control Adjustments

void OculusWorldDemoApp::AdjustFov(float dt)
{
    float esd = SConfig.GetEyeToScreenDistance() + 0.01f * dt;
    SConfig.SetEyeToScreenDistance(esd);
    SetAdjustMessage("ESD:%6.3f  FOV: %6.3f", esd, SConfig.GetYFOVDegrees());
}

void OculusWorldDemoApp::AdjustAspect(float dt)
{
    float rawAspect = SConfig.GetAspect() / SConfig.GetAspectMultiplier();
    float newAspect = SConfig.GetAspect() + 0.01f * dt;
    SConfig.SetAspectMultiplier(newAspect/rawAspect);
    SetAdjustMessage("Aspect: %6.3f", newAspect);
}

void OculusWorldDemoApp::AdjustDistortion(float dt, int kIndex, const char* label)
{
    SConfig.SetDistortionK(kIndex, SConfig.GetDistortionK(kIndex) + 0.03f * dt);   
    SetAdjustMessage("%s: %6.4f", label, SConfig.GetDistortionK(kIndex));
}


//-------------------------------------------------------------------------------------
// ***** Room Model

// This model is hard-coded out of axis-aligned solid-colored slabs.
// Room unit dimensions are in meters. Player starts in the middle.
//
// TBD:
//  - Replace with high-detail textured mesh custom-designed for Rift.
//  - Add floor boundary collision detection.

enum BuiltinTexture
{
    Tex_None,
    Tex_Checker,
    Tex_Block,
    Tex_Panel,
    Tex_Count
};

struct Slab
{
    float x1, y1, z1;
    float x2, y2, z2;
    Color c;
};

struct SlabModel
{
    int   Count;
    Slab* pSlabs;
    BuiltinTexture tex;
};

Slab FloorSlabs[] =
{
    // Floor
    { -10.0f,  -0.1f,  -20.0f,  10.0f,  0.0f, 20.1f,  Color(128,128,128) }
};

SlabModel Floor = {sizeof(FloorSlabs)/sizeof(Slab), FloorSlabs, Tex_Checker};

Slab CeilingSlabs[] =
{
    { -10.0f,  4.0f,  -20.0f,  10.0f,  4.1f, 20.1f,  Color(128,128,128) }
};

SlabModel Ceiling = {sizeof(FloorSlabs)/sizeof(Slab), CeilingSlabs, Tex_Panel};

Slab RoomSlabs[] =
{
    // Left Wall
    { -10.1f,   0.0f,  -20.0f, -10.0f,  4.0f, 20.0f,  Color(128,128,128) },
    // Back Wall
    { -10.0f,  -0.1f,  -20.1f,  10.0f,  4.0f, -20.0f, Color(128,128,128) },

    // Right Wall
    {  10.0f,  -0.1f,  -20.0f,  10.1f,  4.0f, 20.0f,  Color(128,128,128) },
};

SlabModel Room = {sizeof(RoomSlabs)/sizeof(Slab), RoomSlabs, Tex_Block};

Slab FixtureSlabs[] =
{
    // Right side shelf
    {   9.5f,   0.75f,  3.0f,  10.1f,  2.5f,   3.1f,  Color(128,128,128) }, // Verticals
    {   9.5f,   0.95f,  3.7f,  10.1f,  2.75f,  3.8f,  Color(128,128,128) },
    {   9.5f,   1.20f,  2.5f,  10.1f,  1.30f,  3.8f,  Color(128,128,128) }, // Horizontals
    {   9.5f,   2.00f,  3.0f,  10.1f,  2.10f,  4.2f,  Color(128,128,128) },

    // Right railing    
    {   5.0f,   1.1f,   20.0f,  10.0f,  1.2f,  20.1f, Color(128,128,128) },
    // Bars
    {   9.0f,   1.1f,   20.0f,   9.1f,  0.0f,  20.1f, Color(128,128,128) },
    {   8.0f,   1.1f,   20.0f,   8.1f,  0.0f,  20.1f, Color(128,128,128) },
    {   7.0f,   1.1f,   20.0f,   7.1f,  0.0f,  20.1f, Color(128,128,128) },
    {   6.0f,   1.1f,   20.0f,   6.1f,  0.0f,  20.1f, Color(128,128,128) },
    {   5.0f,   1.1f,   20.0f,   5.1f,  0.0f,  20.1f, Color(128,128,128) },

    // Left railing    
    {  -10.0f,   1.1f, 20.0f,   -5.0f,   1.2f, 20.1f, Color(128,128,128) },
    // Bars
    {  -9.0f,   1.1f,   20.0f,  -9.1f,  0.0f,  20.1f, Color(128,128,128) },
    {  -8.0f,   1.1f,   20.0f,  -8.1f,  0.0f,  20.1f, Color(128,128,128) },
    {  -7.0f,   1.1f,   20.0f,  -7.1f,  0.0f,  20.1f, Color(128,128,128) },
    {  -6.0f,   1.1f,   20.0f,  -6.1f,  0.0f,  20.1f, Color(128,128,128) },
    {  -5.0f,   1.1f,   20.0f,  -5.1f,  0.0f,  20.1f, Color(128,128,128) },

    // Bottom Floor 2
    { -15.0f,  -6.1f,   18.0f,  15.0f, -6.0f, 30.0f,  Color(128,128,128) },
};

SlabModel Fixtures = {sizeof(FixtureSlabs)/sizeof(Slab), FixtureSlabs};

Slab FurnitureSlabs[] =
{
    // Table
    {  -1.8f, 0.7f, 1.0f,  0.0f,      0.8f, 0.0f,      Color(128,128,88) },
    {  -1.8f, 0.7f, 0.0f, -1.8f+0.1f, 0.0f, 0.0f+0.1f, Color(128,128,88) }, // Leg 1
    {  -1.8f, 0.7f, 1.0f, -1.8f+0.1f, 0.0f, 1.0f-0.1f, Color(128,128,88) }, // Leg 2
    {   0.0f, 0.7f, 1.0f,  0.0f-0.1f, 0.0f, 1.0f-0.1f, Color(128,128,88) }, // Leg 2
    {   0.0f, 0.7f, 0.0f,  0.0f-0.1f, 0.0f, 0.0f+0.1f, Color(128,128,88) }, // Leg 2

    // Chair
    {  -1.4f, 0.5f, -1.1f, -0.8f,       0.55f, -0.5f,       Color(88,88,128) }, // Set
    {  -1.4f, 1.0f, -1.1f, -1.4f+0.06f, 0.0f,  -1.1f+0.06f, Color(88,88,128) }, // Leg 1
    {  -1.4f, 0.5f, -0.5f, -1.4f+0.06f, 0.0f,  -0.5f-0.06f, Color(88,88,128) }, // Leg 2
    {  -0.8f, 0.5f, -0.5f, -0.8f-0.06f, 0.0f,  -0.5f-0.06f, Color(88,88,128) }, // Leg 2
    {  -0.8f, 1.0f, -1.1f, -0.8f-0.06f, 0.0f,  -1.1f+0.06f, Color(88,88,128) }, // Leg 2
    {  -1.4f, 0.97f,-1.05f,-0.8f,       0.92f, -1.10f,      Color(88,88,128) }, // Back high bar
};

SlabModel Furniture = {sizeof(FurnitureSlabs)/sizeof(Slab), FurnitureSlabs};

Slab PostsSlabs[] = 
{
    // Posts
    {  0,  0.0f, 0.0f,   0.1f, 1.3f, 0.1f, Color(128,128,128) },
    {  0,  0.0f, 0.4f,   0.1f, 1.3f, 0.5f, Color(128,128,128) },
    {  0,  0.0f, 0.8f,   0.1f, 1.3f, 0.9f, Color(128,128,128) },
    {  0,  0.0f, 1.2f,   0.1f, 1.3f, 1.3f, Color(128,128,128) },
    {  0,  0.0f, 1.6f,   0.1f, 1.3f, 1.7f, Color(128,128,128) },
    {  0,  0.0f, 2.0f,   0.1f, 1.3f, 2.1f, Color(128,128,128) },
    {  0,  0.0f, 2.4f,   0.1f, 1.3f, 2.5f, Color(128,128,128) },
    {  0,  0.0f, 2.8f,   0.1f, 1.3f, 2.9f, Color(128,128,128) },
    {  0,  0.0f, 3.2f,   0.1f, 1.3f, 3.3f, Color(128,128,128) },
    {  0,  0.0f, 3.6f,   0.1f, 1.3f, 3.7f, Color(128,128,128) },
};

SlabModel Posts = {sizeof(PostsSlabs)/sizeof(Slab), PostsSlabs};


// Helper function to create a model out of Slab arrays.
Model* OculusWorldDemoApp::CreateModel(Vector3f pos, SlabModel* sm)
{
    Model* m = new Model(Prim_Triangles);
    m->SetPosition(pos);

    for(int i=0; i< sm->Count; i++)
    {
        Slab &s = sm->pSlabs[i];
        m->AddSolidColorBox(s.x1, s.y1, s.z1, s.x2, s.y2, s.z2, s.c);
    }

    if (sm->tex > 0)
        m->Fill = LitTextures[sm->tex];
    else
        m->Fill = LitSolid;
    return m;
}

// Adds room model to the scene.
void OculusWorldDemoApp::PopulateScene()
{
    Ptr<Texture> BuiltinTextures[Tex_Count];

    // Create floor checkerboard texture.
    {
        Color checker[256*256];
        for (int j = 0; j < 256; j++)
            for (int i = 0; i < 256; i++)
                checker[j*256+i] = (((i/4 >> 5) ^ (j/4 >> 5)) & 1) ?
                                  Color(180,180,180,255) : Color(80,80,80,255);
        BuiltinTextures[Tex_Checker] = *pRender->CreateTexture(Texture_RGBA|Texture_GenMipmaps, 256, 256, checker);
        BuiltinTextures[Tex_Checker]->SetSampleMode(Sample_Anisotropic|Sample_Repeat);
    }

    // Ceiling panel texture.
    {
        Color panel[256*256];
        for (int j = 0; j < 256; j++)
            for (int i = 0; i < 256; i++)
                panel[j*256+i] = (i/4 == 0 || j/4 == 0) ?
                    Color(80,80,80,255) : Color(180,180,180,255);
        BuiltinTextures[Tex_Panel] = *pRender->CreateTexture(Texture_RGBA|Texture_GenMipmaps, 256, 256, panel);
        BuiltinTextures[Tex_Panel]->SetSampleMode(Sample_Anisotropic|Sample_Repeat);
    }

    // Wall brick textures.
    {
        Color block[256*256];
        for (int j = 0; j < 256; j++)
            for (int i = 0; i < 256; i++)
                block[j*256+i] = ((j/4 & 15) == 0) || ((i/4 & 15) == 0) && ((((i/4 & 31) == 0) ^ ((j/4 >> 4) & 1)) == 0) ?
                                Color(60,60,60,255) : Color(180,180,180,255);
        BuiltinTextures[Tex_Block] = *pRender->CreateTexture(Texture_RGBA|Texture_GenMipmaps, 256, 256, block);
        BuiltinTextures[Tex_Block]->SetSampleMode(Sample_Anisotropic|Sample_Repeat);
    }

    LitSolid = *new ShaderFill(*pRender->CreateShaderSet());
    LitSolid->GetShaders()->SetShader(pRender->LoadBuiltinShader(Shader_Vertex, VShader_MVP)); 
    LitSolid->GetShaders()->SetShader(pRender->LoadBuiltinShader(Shader_Fragment, FShader_LitGouraud)); 

    for (int i = 1; i < Tex_Count; i++)
    {
        LitTextures[i] = *new ShaderFill(*pRender->CreateShaderSet());
        LitTextures[i]->GetShaders()->SetShader(pRender->LoadBuiltinShader(Shader_Vertex, VShader_MVP)); 
        LitTextures[i]->GetShaders()->SetShader(pRender->LoadBuiltinShader(Shader_Fragment, FShader_LitTexture)); 
        LitTextures[i]->SetTexture(0, BuiltinTextures[i]);
    }

    Scene.World.Add(Ptr<Model>(*CreateModel(Vector3f(0,0,0),  &Room)));
    Scene.World.Add(Ptr<Model>(*CreateModel(Vector3f(0,0,0),  &Floor)));
    Scene.World.Add(Ptr<Model>(*CreateModel(Vector3f(0,0,0),  &Ceiling)));
    Scene.World.Add(Ptr<Model>(*CreateModel(Vector3f(0,0,0),  &Fixtures)));
    Scene.World.Add(Ptr<Model>(*CreateModel(Vector3f(0,0,0),  &Furniture)));
    Scene.World.Add(Ptr<Model>(*CreateModel(Vector3f(0,0,4),  &Furniture)));
    Scene.World.Add(Ptr<Model>(*CreateModel(Vector3f(-3,0,3), &Posts)));
    
    Ptr<File> ImageFile = *new SysFile("image.tga");
    Ptr<Texture> ImageTex;
    if (ImageFile->IsValid())
        ImageTex = *LoadTextureTga(pRender, ImageFile);
    if (ImageTex)
    {
        ImageTex->SetSampleMode(Sample_Anisotropic|Sample_Repeat);
        Ptr<Model> m = *new Model(Prim_Triangles);
        m->SetPosition(Vector3f(-9.98f, 1.0f, 0.0f));
        m->AddVertex(0, -0.2f,  1.0f, Color(255,255,255,255), 0, 0, 0,0,-1);
        m->AddVertex(0,  1.8f,  1.0f, Color(255,255,255,255), 0, 1, 0,0,-1);
        m->AddVertex(0,  1.8f, -1.0f, Color(255,255,255,255), 1, 1, 0,0,-1);
        m->AddVertex(0, -0.2f, -1.0f, Color(255,255,255,255), 1, 0, 0,0,-1);
        m->AddTriangle(0,1,2);
        m->AddTriangle(2,3,0);

        Ptr<ShaderFill> fill = *new ShaderFill(*pRender->CreateShaderSet());
        fill->GetShaders()->SetShader(pRender->LoadBuiltinShader(Shader_Vertex, VShader_MVP)); 
        fill->GetShaders()->SetShader(pRender->LoadBuiltinShader(Shader_Fragment, FShader_LitTexture)); 
        fill->SetTexture(0, ImageTex);
        m->Fill = fill;

        Scene.World.Add(m);
    }

    Scene.SetAmbient(Vector4f(0.65f,0.65f,0.65f,1));
    Scene.AddLight(Vector3f(-2,4,-2), Vector4f(8,8,8,1));
    Scene.AddLight(Vector3f(3,4,-3),  Vector4f(2,1,1,1));
    Scene.AddLight(Vector3f(-4,3,25), Vector4f(3,6,3,1));


    // Distortion debug grid (brought up by 'G' key). 
    Ptr<Model> gridModel = *Model::CreateGrid(Vector3f(0,0,0), Vector3f(1.0f/10, 0,0), Vector3f(0,1.0f/10,0),
                                              10, 10, 5, 
                                              Color(0, 255, 0, 255), Color(255, 50, 50, 255) );
    GridScene.World.Add(gridModel);
}


//-------------------------------------------------------------------------------------

OVR_PLATFORM_APP(OculusWorldDemoApp);
