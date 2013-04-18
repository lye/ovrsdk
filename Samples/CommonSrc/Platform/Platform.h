/************************************************************************************

Filename    :   Platform.h
Content     :   Platform-independent app and rendering framework for Oculus samples
Created     :   September 6, 2012
Authors     :   Andrew Reisse

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus LLC license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#ifndef OVR_Platform_h
#define OVR_Platform_h

#include "OVR.h"

#include "Kernel/OVR_KeyCodes.h"

namespace OVR { namespace Render {
    class Renderer;
    struct RendererParams;
}}

namespace OVR { namespace Platform {

class PlatformBase;
class Application;


// MouseMode configures mouse input behavior of the app. Three states are
// currently supported:
//   Normal          - Reports absolute coordinates with cursor shown.
//   Relative        - Reports relative delta coordinates with cursor hidden
//                     until 'Esc' key is pressed or window loses focus.
//   RelativeEscaped - Relative input is desired, but has been escaped until
//                     mouse is clicked in the window, which will return the state
//                     to relative. Absolute coordinates are reported.

enum MouseMode
{
    Mouse_Normal,
    Mouse_Relative,        // Cursor hidden, mouse grab, OnMouseMove reports relative deltas.
    Mouse_RelativeEscaped, // Clicking in window will return to Relative state.
};


enum Modifiers
{
    Mod_Shift       = 0x001,
    Mod_Control     = 0x002,
    Mod_Meta        = 0x004,
    Mod_Alt         = 0x008,

    // Set for input Mouse_Relative mode, indicating that x,y are relative deltas.
    Mod_MouseRelative = 0x100,
};

enum GamepadButtons
{
    Gamepad_A       = 0x1000,
    Gamepad_B       = 0x2000,
    Gamepad_X       = 0x4000,
    Gamepad_Y       = 0x8000,
    Gamepad_Up      = 0x0001,
    Gamepad_Down    = 0x0002,
    Gamepad_Left    = 0x0004,
    Gamepad_Right   = 0x0008,
    Gamepad_Start   = 0x0010,
    Gamepad_Back    = 0x0020,
    Gamepad_LStick  = 0x0040,
    Gamepad_RStick  = 0x0080,
    Gamepad_L1      = 0x0100,
    Gamepad_R1      = 0x0200,
};

struct GamepadState
{
    UInt32  Buttons;
    float   LX, LY, RX, RY;
    float   LT, RT;

    GamepadState() : Buttons(0), LX(0), LY(0), RX(0), RY(0), LT(0), RT(0) {}

    bool operator==(const GamepadState& b) const
    {
        return Buttons == b.Buttons && LX == b.LX && LY == b.LY && RX == b.RX && RY == b.RY && LT == b.LT && RT == b.RT;
    }
    bool operator!=(const GamepadState& b) const
    {
        return !(*this == b);
    }
};

//-------------------------------------------------------------------------------------

// PlatformBase implements system window/viewport setup functionality and
// maintains a renderer. This class is separated from Application because it can have
// derived platform-specific implementations.

class PlatformBase : public NewOverrideBase
{
protected:
    Application*          pApp;
    Ptr<Render::Renderer> pRender;
    // Subclass should initialize this by calling GetTicks() during SetupWindow.
    UInt64                StartupTicks; 

public:
    inline PlatformBase(Application *app);
    virtual ~PlatformBase() { }
    Application*      GetApp() { return pApp; }

    virtual bool      SetupWindow(int w, int h) = 0;
    // Destroys window and also releases renderer.
    virtual void      DestroyWindow() = 0;
    virtual void      Exit(int exitcode) = 0;

    virtual void      ShowWindow(bool visible) = 0;

    virtual Render::Renderer* SetupGraphics(const char* gtype, const Render::RendererParams& rp) = 0;
    Render::Renderer* SetupGraphics(const char* gtype = 0);

    virtual void      SetMouseMode(MouseMode mm) { OVR_UNUSED(mm); }

    virtual void      GetWindowSize(int* w, int* h) const = 0;

    virtual void      SetWindowTitle(const char*title) = 0;
    // Time
    

    // An arbitrary counter in us.
    virtual UInt64  GetTicks() const = 0;
    double          GetAppTime() const
    {
        return (GetTicks() - StartupTicks) * 0.000001;
    }
};

//-------------------------------------------------------------------------------------
// PlatformApp is a base application class from which end-user application
// classes derive.

class Application : public NewOverrideBase
{
protected:
    class PlatformBase* pPlatform;

public:
    virtual ~Application() { }

    virtual int  OnStartup(int argc, const char** argv) = 0;
    virtual void OnQuitRequest() { pPlatform->Exit(0); }

    virtual void OnIdle() {}

    virtual void OnKey(KeyCode key, int chr, bool down, int modifiers)
    { OVR_UNUSED4(key, chr, down, modifiers); }
    virtual void OnMouseMove(int x, int y, int modifiers)
    { OVR_UNUSED3(x, y, modifiers); }
    virtual void OnGamepad(const GamepadState& pad)
    { OVR_UNUSED(pad); }

    virtual void OnResize(int width, int height)
    { OVR_UNUSED2(width, height); }

    void         SetPlatform(PlatformBase* p) { pPlatform = p; }
    PlatformBase* GetPlatform() const         { return pPlatform; }


    // Static functions defined by OVR_PLATFORM_APP and used to initialize and
    // shut down the application class.
    static Application* CreateApplication();
    static void         DestroyApplication(Application* app);
};

inline PlatformBase::PlatformBase(Application *app)
{
    pApp = app;
    pApp->SetPlatform(this);    
}

}}

#endif
