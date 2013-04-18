/************************************************************************************

Filename    :   Render_XmlSceneLoader.h
Content     :   Imports and exports XML files
Created     :   January 21, 2013
Authors     :   Robotic Arm Software - Peter Hoff, Dan Goodman, Bryan Croteau

Copyright   :   Copyright 2013 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus LLC license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#ifndef INC_Render_XMLSceneLoader_h
#define INC_Render_XMLSceneLoader_h

#include "Render_Device.h"
#include <Kernel/OVR_SysFile.h>
using namespace OVR;
using namespace OVR::Render;

#ifdef OVR_DEFINE_NEW
#undef new
#endif

#include "../../../3rdParty/TinyXml/tinyxml2.h"

namespace OVR { namespace Render {

using namespace tinyxml2;

class XmlHandler
{
public:
    XmlHandler();
    ~XmlHandler();

    bool ReadFile(const char* fileName, OVR::Render::RenderDevice* pRender,
                  OVR::Render::Scene* pScene,
		          OVR::Array<Ptr<CollisionModel> >* pColisions,
                  OVR::Array<Ptr<CollisionModel> >* pGroundCollisions,
				  size_t& totalTextureMemoryUsage);

protected:
    void ParseVectorString(const char* str, OVR::Array<OVR::Vector3f> *array,
		                   bool is2element = false);

private:
    tinyxml2::XMLDocument* pXmlDocument;
    char                   filePath[250];
    int                    textureCount;
    OVR::Array<Ptr<Texture> > Textures;
    int                    modelCount;
    OVR::Array<Ptr<Model> > Models;
    int                    collisionModelCount;
    int                    groundCollisionModelCount;
};

}} // OVR::Render

#ifdef OVR_DEFINE_NEW
#define new OVR_DEFINE_NEW
#endif

#endif // INC_Render_XMLSceneLoader_h
