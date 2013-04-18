
#include "Platform.h"
#include <Kernel/OVR_Std.h>
#include <Kernel/OVR_Timer.h>
#include "../Render/Render_Device.h"

namespace OVR { namespace Platform {


const SetupGraphicsDeviceSet* SetupGraphicsDeviceSet::PickSetupDevice(const char* typeArg) const
{
    // Search for graphics creation object that matches type arg.
    if (typeArg)
    {
        for (const SetupGraphicsDeviceSet* p = this; p != 0; p = p->pNext)
        {
            if (!OVR_stricmp(p->pTypeArg, typeArg))
                return p;
        }
    }
    return this;
}

//-------------------------------------------------------------------------------------

PlatformCore::PlatformCore(Application *app)
{
    pApp = app;
    pApp->SetPlatformCore(this);    
    StartupTicks = OVR::Timer::GetTicks();
}

double PlatformCore::GetAppTime() const
{
    return (OVR::Timer::GetTicks() - StartupTicks) * (1.0 / (double)OVR::Timer::MksPerSecond);
}

bool PlatformCore::SetFullscreen(const Render::RendererParams&, int fullscreen)
{
    if (pRender)
        return pRender->SetFullscreen((Render::DisplayMode)fullscreen);
    return 0;
}

}}
