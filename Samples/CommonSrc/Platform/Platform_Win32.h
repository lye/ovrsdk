/************************************************************************************

Filename    :   Platform_Win32.h
Content     :   Win32 implementation of Platform app infrastructure
Created     :   September 6, 2012
Authors     :   Andrew Reisse

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus LLC license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#ifndef OVR_Platform_Win32_h
#define OVR_Platform_Win32_h

#include "Platform.h"
#include <windows.h>

#include "../Renderer/Renderer_GL.h"

#include <xinput.h>

namespace OVR { namespace Render {
    class Renderer;
}}

namespace OVR { namespace Platform { namespace Win32 {

class Platform : public PlatformBase
{
    HWND        hWnd;
    HINSTANCE   hInstance;
    bool        Quit;
    int         ExitCode;
    int         Width, Height;

    MouseMode   MMode;    
    POINT       WindowCenter; // In desktop coordinates
    HCURSOR     Cursor;
    int         Modifiers;
    String      WindowTitle;

    // Dynamically ink to XInput to simplify projects.
    HMODULE     hXInputModule;
    typedef DWORD (WINAPI *PFn_XInputGetState)(DWORD dwUserIndex, XINPUT_STATE* pState);
    PFn_XInputGetState pXInputGetState;

    UInt32      LastPadPacketNo;

    // Win32 static function that delegates to WindowProc member function.
    static LRESULT CALLBACK systemWindowProc(HWND window, UINT msg, WPARAM wp, LPARAM lp);

    LRESULT     WindowProc(UINT msg, WPARAM wp, LPARAM lp);

    Render::Renderer* SetupGraphics_GL(const Render::RendererParams& rp);
    Render::Renderer* SetupGraphics_D3D10(const Render::RendererParams& rp);
    Render::Renderer* SetupGraphics_D3D11(const Render::RendererParams& rp);

public:
    Platform(Application* app, HINSTANCE hinst);
    ~Platform();

    bool      SetupWindow(int w, int h);
    void      DestroyWindow();
    void      ShowWindow(bool visible);
    void      Exit(int exitcode) { Quit = 1; ExitCode = exitcode; }

    Render::Renderer* SetupGraphics(const char* type = 0, const Render::RendererParams& rp = Render::RendererParams());

    void      SetMouseMode(MouseMode mm);
    void      GetWindowSize(int* w, int* h) const;

    void      SetWindowTitle(const char*title);

    UInt64    GetTicks() const { return GetTickCount() * 1000; }

    int  Run();
};

class Renderer_GL_Win32 : public Render::GL::Renderer
{
    HWND   Window;
    HGLRC  WglContext;
    HDC    GdiDc;

public:
    Renderer_GL_Win32(const Render::RendererParams& p, HWND win, HDC dc, HGLRC gl)
        : Window(win), WglContext(gl), GdiDc(dc) { OVR_UNUSED(p); }

    virtual void Shutdown();
    virtual void Present();
};

// Win32 key conversion helper.
KeyCode MapVKToKeyCode(unsigned vk);

}}}


// OVR_PLATFORM_APP_ARGS specifies the Application class to use for startup,
// providing it with startup arguments.
#define OVR_PLATFORM_APP_ARGS(AppClass, args)                                            \
    OVR::Platform::Application* OVR::Platform::Application::CreateApplication()          \
    { OVR::System::Init(OVR::Log::ConfigureDefaultLog(OVR::LogMask_All));                \
      return new AppClass args; }                                                        \
    void OVR::Platform::Application::DestroyApplication(OVR::Platform::Application* app) \
    { OVR::Platform::PlatformBase* platform = app->pPlatform;                            \
      delete app; delete platform; OVR::System::Destroy(); };

// OVR_PLATFORM_APP_ARGS specifies the Application startup class with no args.
#define OVR_PLATFORM_APP(AppClass) OVR_PLATFORM_APP_ARGS(AppClass, ())


#endif
