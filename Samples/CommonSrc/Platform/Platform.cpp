
#include "Platform.h"
#include "../Renderer/Renderer.h"

namespace OVR { namespace Platform {

Render::Renderer* PlatformBase::SetupGraphics(const char* gtype)
{
    return SetupGraphics(gtype, Render::RendererParams());
}

}}
