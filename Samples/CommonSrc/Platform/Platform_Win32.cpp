/************************************************************************************

Filename    :   Platform_Win32.cpp
Content     :   Win32 implementation of Platform app infrastructure
Created     :   September 6, 2012
Authors     :   Andrew Reisse

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus LLC license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#include "Kernel/OVR_System.h"
#include "Kernel/OVR_Array.h"
#include "Kernel/OVR_String.h"

#include "../Platform/Platform_Win32.h"

// Renderers
#include "../Renderer/Renderer_GL.h"
#include "../Renderer/Renderer_D3D11.h"
#undef OVR_D3D_VERSION
#include "../Renderer/Renderer_D3D10.h"


namespace OVR { namespace Platform { namespace Win32 {


Platform::Platform(Application* app, HINSTANCE hinst)
  : PlatformBase(app), hWnd(NULL), hInstance(hinst), Quit(0), MMode(Mouse_Normal),
    Cursor(0), Modifiers(0), WindowTitle("App"),
    hXInputModule(0), pXInputGetState(0)
{
    hXInputModule = ::LoadLibraryA("Xinput9_1_0.dll");
    if (hXInputModule)
    {
        pXInputGetState = (PFn_XInputGetState)
            ::GetProcAddress(hXInputModule, "XInputGetState");        
    }
}
Platform::~Platform()
{
    if (hXInputModule)
        ::FreeLibrary(hXInputModule);
}

bool Platform::SetupWindow(int w, int h)
{
    WNDCLASS wc;
    memset(&wc, 0, sizeof(wc));
    wc.lpszClassName = L"OVRAppWindow";
    wc.style         = CS_OWNDC;
    wc.lpfnWndProc   = systemWindowProc;
    wc.cbWndExtra    = sizeof(Platform*);

    RegisterClass(&wc);

    Width = w;
    Height = h;
    RECT winSize;
    winSize.left = winSize.top = 0;
    winSize.right = Width;
    winSize.bottom = Height;
    AdjustWindowRect(&winSize, WS_OVERLAPPEDWINDOW, false);
    hWnd = CreateWindowA("OVRAppWindow", WindowTitle.ToCStr(), WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                        //  1950, 10,
                          winSize.right-winSize.left, winSize.bottom-winSize.top,
                          NULL, NULL, hInstance, (LPVOID)this);
    Modifiers = 0;

    LastPadPacketNo = 0;

    Cursor = LoadCursor(NULL, IDC_CROSS);    

    // Initialize Window center in screen coordinates
    POINT center = { Width / 2, Height / 2 };
    ::ClientToScreen(hWnd, &center);
    WindowCenter = center;
    
    if (MMode == Mouse_Relative)
    {
        ::SetCursorPos(WindowCenter.x, WindowCenter.y);
        ShowCursor(FALSE);
    }

    return (hWnd != NULL);
}

void Platform::DestroyWindow()
{
    // Release renderer.
    pRender.Clear();

    // Release window resources.
    ::DestroyWindow(hWnd);
    UnregisterClass(L"OVRAppWindow", hInstance);
    hWnd = 0;
    Width = Height = 0;

    //DestroyCursor(Cursor);
    Cursor = 0;    
}

void Platform::ShowWindow(bool visible)
{
    ::ShowWindow(hWnd, visible ? SW_SHOW : SW_HIDE);
}

void Platform::SetMouseMode(MouseMode mm)
{
    if (mm == MMode)
        return;

    if (hWnd)
    {
        if (mm == Mouse_Relative)
        {
            ShowCursor(FALSE);
            ::SetCursorPos(WindowCenter.x, WindowCenter.y);
        }
        else
        {
            if (MMode == Mouse_Relative)
                ShowCursor(TRUE);
        }
    }
    MMode = mm;
}

void Platform::GetWindowSize(int* w, int* h) const
{
    *w = Width;
    *h = Height;
}


void Platform::SetWindowTitle(const char* title)
{
    WindowTitle = title;
    if (hWnd)
        ::SetWindowTextA(hWnd, title);
}

static UByte KeyMap[][2] = 
{
    { VK_BACK,      Key_Backspace },
    { VK_TAB,       Key_Tab },
    { VK_CLEAR,     Key_Clear },
    { VK_RETURN,    Key_Return },
    { VK_SHIFT,     Key_Shift },
    { VK_CONTROL,   Key_Control },
    { VK_MENU,      Key_Alt },
    { VK_PAUSE,     Key_Pause },
    { VK_CAPITAL,   Key_CapsLock },
    { VK_ESCAPE,    Key_Escape },
    { VK_SPACE,     Key_Space },
    { VK_PRIOR,     Key_PageUp },
    { VK_NEXT,      Key_PageDown },
    { VK_END,       Key_End },
    { VK_HOME,      Key_Home },
    { VK_LEFT,      Key_Left },
    { VK_UP,        Key_Up },
    { VK_RIGHT,     Key_Right },
    { VK_DOWN,      Key_Down },
    { VK_INSERT,    Key_Insert },
    { VK_DELETE,    Key_Delete },
    { VK_HELP,      Key_Help },

    { VK_NUMLOCK,   Key_NumLock },
    { VK_SCROLL,    Key_ScrollLock },

    { VK_OEM_1,     Key_Semicolon },
    { VK_OEM_PLUS,  Key_Equal },
    { VK_OEM_COMMA, Key_Comma },
    { VK_OEM_MINUS, Key_Minus },
    { VK_OEM_PERIOD,Key_Period },
    { VK_OEM_2,     Key_Slash },
    { VK_OEM_3,     Key_Bar },
    { VK_OEM_4,     Key_BracketLeft },
    { VK_OEM_5,     Key_Backslash },
    { VK_OEM_6,     Key_BracketRight },
    { VK_OEM_7,     Key_Quote },

    { VK_OEM_AX,	Key_OEM_AX },   //  'AX' key on Japanese AX keyboard.
    { VK_OEM_102,   Key_OEM_102 },  //  "<>" or "\|" on RT 102-key keyboard.
    { VK_ICO_HELP,  Key_ICO_HELP },
    { VK_ICO_00,	Key_ICO_00 }
};


KeyCode MapVKToKeyCode(unsigned vk)
{
    unsigned key = Key_None;

    if ((vk >= '0') && (vk <= '9'))
    {
        key = vk - '0' + Key_Num0;
    }
    else if ((vk >= 'A') && (vk <= 'Z'))
    {
        key = vk - 'A' + Key_A;
    }
    else if ((vk >= VK_NUMPAD0) && (vk <= VK_DIVIDE))
    {
        key = vk - VK_NUMPAD0 + Key_KP_0;
    }
    else if ((vk >= VK_F1) && (vk <= VK_F15))
    {
        key = vk - VK_F1 + Key_F1;
    }
    else 
    {
        for (unsigned i = 0; i< (sizeof(KeyMap) / sizeof(KeyMap[1])); i++)
        {
            if (vk == KeyMap[i][0])
            {                
                key = KeyMap[i][1];
                break;
            }
        }
    }

    return (KeyCode)key;
}



LRESULT CALLBACK Platform::systemWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    Platform* self;  

    // WM_NCCREATE should be the first message to come it; use it to set class pointer.
    if (msg == WM_NCCREATE)
    {
        self = static_cast<Platform*>(((LPCREATESTRUCT)lp)->lpCreateParams);

        if (self)
        {
            SetWindowLongPtr(hwnd, 0, (LONG_PTR)self);
            self->hWnd = hwnd;
        }
    }
    else
    {
        self = (Platform*)(UPInt)GetWindowLongPtr(hwnd, 0);
    }
        
    return self ? self->WindowProc(msg, wp, lp) :
                  DefWindowProc(hwnd, msg, wp, lp);
}


LRESULT Platform::WindowProc(UINT msg, WPARAM wp, LPARAM lp)
{
    KeyCode keyCode;

    switch (msg)
    {
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        return 0;

    case WM_SETCURSOR:
        ::SetCursor(Cursor);
        return 0;

    case WM_MOUSEMOVE:
        if (MMode == Mouse_Relative)
        {
            POINT newPos = { LOWORD(lp), HIWORD(lp) };
            ::ClientToScreen(hWnd, &newPos);
            if ((newPos.x == WindowCenter.x) && (newPos.y == WindowCenter.y))
                break;
            ::SetCursorPos(WindowCenter.x, WindowCenter.y);

            LONG dx = newPos.x - WindowCenter.x;
            LONG dy = newPos.y - WindowCenter.y;
           
            pApp->OnMouseMove(dx, dy, Mod_MouseRelative);
        }
        else
        {
            pApp->OnMouseMove(LOWORD(lp), HIWORD(lp), 0);
        }
        break;

    case WM_MOVE:
        {
            RECT r;
            GetClientRect(hWnd, &r);
            WindowCenter.x = r.right/2;
            WindowCenter.y = r.bottom/2;
            ::ClientToScreen(hWnd, &WindowCenter);
        }
        break;

    case WM_KEYDOWN:        
        switch (wp)
        {
        case VK_CONTROL:        Modifiers |= Mod_Control; break;
        case VK_MENU:           Modifiers |= Mod_Alt; break;
        case VK_SHIFT:          Modifiers |= Mod_Shift; break;
        case VK_LWIN:
        case VK_RWIN:           Modifiers |= Mod_Meta; break;
        }
        if ((keyCode = MapVKToKeyCode((unsigned)wp)) != Key_None)
            pApp->OnKey(keyCode, 0, true, Modifiers);

        if (keyCode == Key_Escape && MMode == Mouse_Relative)
        {
            MMode = Mouse_RelativeEscaped;
            ShowCursor(TRUE);
        }
        break;

    case WM_KEYUP:
        if ((keyCode = MapVKToKeyCode((unsigned)wp)) != Key_None)
            pApp->OnKey(keyCode, 0, false, Modifiers);
        switch (wp)
        {
        case VK_CONTROL:        Modifiers &= ~Mod_Control; break;
        case VK_MENU:           Modifiers &= ~Mod_Alt; break;
        case VK_SHIFT:          Modifiers &= ~Mod_Shift; break;
        case VK_LWIN:
        case VK_RWIN:           Modifiers &= ~Mod_Meta; break;
        }
        break;

    case WM_LBUTTONDOWN:
        //App->OnMouseButton(0, 

        ::SetCapture(hWnd);

        if (MMode == Mouse_RelativeEscaped)
        {            
            ::SetCursorPos(WindowCenter.x, WindowCenter.y);
            ::ShowCursor(FALSE);
            MMode = Mouse_Relative;
        }
        break;

    case WM_LBUTTONUP:
        ReleaseCapture();
        break;

    case WM_SETFOCUS:
        // Do NOT restore the Relative mode here, since calling SetCursorPos
        // would screw up titlebar window dragging.
        // Let users click in the center instead to resume.        
        break;

    case WM_KILLFOCUS:
        if (MMode == Mouse_Relative)
        {            
            MMode = Mouse_RelativeEscaped;
            ShowCursor(TRUE);
        }
        break;

    case WM_SIZE:
        // Change window size as long as we're not being minimized. 
        if (wp != SIZE_MINIMIZED)
        {
            Width = LOWORD(lp);
            Height = HIWORD(lp);
            if (pRender)
                pRender->SetWindowSize(Width, Height);
            pApp->OnResize(Width,Height);
        }
        break;

    case WM_STYLECHANGING:
        // Resize the window. This is needed because the size includes any present system controls, and
        // windows does not adjust it when changing to fullscreen.
        {
            STYLESTRUCT* pss = (STYLESTRUCT*)lp;
            RECT winSize;
            winSize.left = winSize.top = 0;
            winSize.right = Width;
            winSize.bottom = Height;
            int w = winSize.right-winSize.left;
            int h = winSize.bottom-winSize.top;
            AdjustWindowRect(&winSize, pss->styleNew, false);
            ::SetWindowPos(hWnd, NULL, 0, 0, w, h, SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
        }
        break;

    case WM_QUIT:
    case WM_CLOSE:
        pApp->OnQuitRequest();
        return false;
    }

    return DefWindowProc(hWnd, msg, wp, lp);
}

static inline float GamepadStick(short in)
{
    float v;
    if (abs(in) < 9000)
        return 0;
    else if (in > 9000)
        v = (float) in - 9000;
    else
        v = (float) in + 9000;
    return v / (32767 - 9000);
}

static inline float GamepadTrigger(BYTE in)
{
    if (in < 30)
        return 0;
    else
        return float(in-30) / 225;
}

int Platform::Run()
{
    while (!Quit)
    {
        MSG msg;
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            // Gamepad
            if (pXInputGetState)
            {
                XINPUT_STATE xis;
                if (!pXInputGetState(0, &xis))
                {
                    if (xis.dwPacketNumber != LastPadPacketNo)
                    {
                        OVR::Platform::GamepadState pad;
                        pad.Buttons = xis.Gamepad.wButtons; // Currently matches Xinput
                        pad.LT = GamepadTrigger(xis.Gamepad.bLeftTrigger);
                        pad.RT = GamepadTrigger(xis.Gamepad.bRightTrigger);
                        pad.LX = GamepadStick(xis.Gamepad.sThumbLX);
                        pad.LY = GamepadStick(xis.Gamepad.sThumbLY);
                        pad.RX = GamepadStick(xis.Gamepad.sThumbRX);
                        pad.RY = GamepadStick(xis.Gamepad.sThumbRY);

                        pApp->OnGamepad(pad);
                        LastPadPacketNo = xis.dwPacketNumber;
                    }
                }
            }

            pApp->OnIdle();

            // Keep sleeping when we're minimized.
            if (IsIconic(hWnd))
            {
                Sleep(10);
            }
        }
    }

    return ExitCode;
}

// GL

Render::Renderer* Platform::SetupGraphics_GL(const Render::RendererParams& rp)
{
    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(pfd));

    pfd.nSize       = sizeof(pfd);
    pfd.nVersion    = 1;
    pfd.iPixelType  = PFD_TYPE_RGBA;
    pfd.dwFlags     = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    pfd.cColorBits  = 32;
    pfd.cDepthBits  = 16;

    HDC dc = GetDC(hWnd);
    int pf = ChoosePixelFormat(dc, &pfd);
    if (!pf)
    {
        ReleaseDC(hWnd, dc);
        return NULL;
    }
    if (!SetPixelFormat(dc, pf, &pfd))
    {
        ReleaseDC(hWnd, dc);
        return NULL;
    }
    HGLRC context = wglCreateContext(dc);
    if (!wglMakeCurrent(dc, context))
    {
        wglDeleteContext(context);
        ReleaseDC(hWnd, dc);
        return NULL;
    }

    ::ShowWindow(hWnd, SW_RESTORE);

    return new Renderer_GL_Win32(rp, hWnd, dc, context);
}

void Renderer_GL_Win32::Present()
{
    SwapBuffers(GdiDc);
}

void Renderer_GL_Win32::Shutdown()
{
    if (WglContext)
    {
        wglMakeCurrent(NULL,NULL);
        wglDeleteContext(WglContext);
        ReleaseDC(Window, GdiDc);
        WglContext = NULL;
        GdiDc = NULL;
        Window = NULL;
    }
}

Render::Renderer* Platform::SetupGraphics_D3D10(const Render::RendererParams& rp)
{
    ::ShowWindow(hWnd, SW_RESTORE);
    return new OVR::Render::D3D10::Renderer(rp, hWnd);
    //return NULL;
}

Render::Renderer* Platform::SetupGraphics_D3D11(const Render::RendererParams& rp)
{
   :: ShowWindow(hWnd, SW_RESTORE);
    return new OVR::Render::D3D11::Renderer(rp, hWnd);
}

Render::Renderer* Platform::SetupGraphics(const char* type, const Render::RendererParams& rp)
{
    if (type)
    {
        if (!_stricmp(type, "GL"))
            pRender = *SetupGraphics_GL(rp);
        else if (!_stricmp(type, "D3D11"))
            pRender = *SetupGraphics_D3D11(rp);
        else if (!_stricmp(type, "D3D10"))
            pRender = *SetupGraphics_D3D10(rp);
    }

    if (!pRender)
        pRender = *SetupGraphics_GL(rp);
    if (!pRender)
        pRender = *SetupGraphics_D3D11(rp);
    if (!pRender)
        pRender = *SetupGraphics_D3D10(rp);

    if (pRender)
        pRender->SetWindowSize(Width, Height);

    return pRender;
}


}}}


int WINAPI WinMain(HINSTANCE hinst, HINSTANCE prevInst, LPSTR inArgs, int show)
{
    using namespace OVR;
    using namespace OVR::Platform;

    OVR_UNUSED2(prevInst, show);
    
    // CreateApplication must be the first call since it does OVR::System::Initialize.
    Application*     app = Application::CreateApplication();
    Win32::Platform* platform = new Win32::Platform(app, hinst);
    // The platform attached to an app will be deleted by DestroyApplication.
    app->SetPlatform(platform);

    int exitCode = 0;

    // Nested scope for container destructors to shutdown before DestroyApplication.
    {
        Array<String>      args;
        Array<const char*> argv;
        argv.PushBack("app");

        const char* p = inArgs;
        const char* pstart = inArgs;
        while (*p)
        {
            if (*p == ' ')
            {
                args.PushBack(String(pstart, p - pstart));
                while (*p == ' ')
                    p++;
                pstart = p;
            }
            else
            {
                p++;
            }
        }
        if (p != pstart)
            args.PushBack(String(pstart, p - pstart));
        for (UPInt i = 0; i < args.GetSize(); i++)
            argv.PushBack(args[i].ToCStr());

        exitCode = app->OnStartup((int)argv.GetSize(), &argv[0]);
        if (!exitCode)
            exitCode = platform->Run();
    }

    // No OVR functions involving memory are allowed after this.
    Application::DestroyApplication(app);
    app = 0;

    OVR_DEBUG_STATEMENT(_CrtDumpMemoryLeaks());
    return exitCode;
}
