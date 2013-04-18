
#ifndef OVR_FONT_H
#define OVR_FONT_H

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
