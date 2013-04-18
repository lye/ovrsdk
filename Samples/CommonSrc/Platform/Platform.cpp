
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

UInt64 PlatformCore::GetTicks() const
{
    return OVR::Timer::GetRawTicks();
}

bool PlatformCore::SetFullscreen(const Render::RendererParams&, int fullscreen)
{
    if (pRender)
        return pRender->SetFullscreen((Render::DisplayMode)fullscreen);
    return 0;
}

}}
