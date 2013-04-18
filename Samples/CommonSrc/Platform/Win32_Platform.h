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

#include <xinput.h>

namespace OVR { namespace Render {
    class RenderDevice;
}}

namespace OVR { namespace Platform { namespace Win32 {

class PlatformCore : public Platform::PlatformCore
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

public:
    PlatformCore(Application* app, HINSTANCE hinst);
    ~PlatformCore();

    bool      SetupWindow(int w, int h);
    void      DestroyWindow();
    void      ShowWindow(bool visible);
    void      Exit(int exitcode) { Quit = 1; ExitCode = exitcode; }

    RenderDevice* SetupGraphics(const SetupGraphicsDeviceSet& setupGraphicsDesc,
                                const char* type,
                                const Render::RendererParams& rp);

    void      SetMouseMode(MouseMode mm);
    void      GetWindowSize(int* w, int* h) const;

    void      SetWindowTitle(const char*title);
	void	  PlayMusicFile(const char *fileName);
    int       GetScreenCount();
    String    GetScreenName(int screen);

    UInt64    GetTicks() const { return GetTickCount() * 1000; }

    int  Run();
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
    { OVR::Platform::PlatformCore* platform = app->pPlatform;                            \
      delete app; delete platform; OVR::System::Destroy(); };

// OVR_PLATFORM_APP_ARGS specifies the Application startup class with no args.
#define OVR_PLATFORM_APP(AppClass) OVR_PLATFORM_APP_ARGS(AppClass, ())


#endif