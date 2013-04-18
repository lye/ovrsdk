/************************************************************************************

Filename    :   Render_Font_h
Content     :   Font data structure used by renderer
Created     :   September, 2012
Authors     :   Andrew Reisse

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus LLC license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#ifndef OVR_Render_Font_h
#define OVR_Render_Font_h

namespace OVR { namespace Render {

class Fill;

struct Font
{
    struct Char
    {
        short x, y;       // offset
        short advance;
        float u1, v1, u2, v2;
    };

    int            lineheight, ascent, descent;
    const Char*    chars;
    const short**  kerning;
    int            twidth, theight;
    const
    unsigned char* tex;
    mutable Fill*  fill;
};

}}

#endif
