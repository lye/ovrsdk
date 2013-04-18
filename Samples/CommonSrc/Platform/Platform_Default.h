/************************************************************************************

Filename    :   Platform_Default.h
Content     :   Default Platform class and RenderDevice selection file
Created     :   October 4, 2012
Authors     :   

Copyright   :   Copyright 2012 Oculus, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus Inc license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

// This should select proper header file for the platform/compiler.
#include <Kernel/OVR_Types.h>

#if defined(OVR_OS_WIN32)
  #include "Win32_Platform.h"

  #include "../Render/Render_D3D11_Device.h"
  #undef OVR_D3D_VERSION  
  #include "../Render/Render_D3D10_Device.h"
//  #include "../Render/Render_GL_Win32_Device.h"

// Modify this list or pass a smaller set to select a specific render device,
// while avoiding linking extra classes.
  #define OVR_DEFAULT_RENDER_DEVICE_SET                                                    \
        SetupGraphicsDeviceSet("D3D11", &OVR::Render::D3D11::RenderDevice::CreateDevice,       \
        SetupGraphicsDeviceSet("D3D10", &OVR::Render::D3D10::RenderDevice::CreateDevice) )

#elif defined(OVR_OS_MAC) && !defined(OVR_MAC_X11)
  #include "MacOS_Platform.h"

  #define OVR_DEFAULT_RENDER_DEVICE_SET                                         \
    SetupGraphicsDeviceSet("GL", &OVR::Render::GL::MacOS::RenderDevice::CreateDevice)

#else

  #include "X11_Platform.h"

  #define OVR_DEFAULT_RENDER_DEVICE_SET                                         \
    SetupGraphicsDeviceSet("GL", &OVR::Render::GL::X11::RenderDevice::CreateDevice)

#endif
