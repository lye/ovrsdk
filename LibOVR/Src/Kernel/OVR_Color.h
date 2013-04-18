/************************************************************************************

PublicHeader:   OVR.h
Filename    :   OVR_Color.h
Content     :   Contains color struct.
Created     :   February 7, 2013
Notes       : 

Copyright   :   Copyright 2013 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/
#ifndef OVR_Color_h
#define OVR_Color_h

#include "OVR_Types.h"

namespace OVR {

struct ColorRGB
{
    UByte R,G,B;

    ColorRGB()
     :  R(0), G(0), B(0)  
    {}

    // Constructs color by channel.
    ColorRGB(UByte r, UByte g, UByte b)
     :  R(r), G(g), B(b) 
    { }
    
    bool operator==(const ColorRGB& b) const
    {
        return R == b.R && G == b.G && B == b.B;
    }

    void  GetRGB(float *r, float *g, float *b) const
    {
        *r = R / 255.0f;
        *g = G / 255.0f;
        *b = B / 255.0f;
    }
};

}

#endif
