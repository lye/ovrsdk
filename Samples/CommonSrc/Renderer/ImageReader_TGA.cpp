

#include "Renderer.h"

namespace OVR { namespace Render {

Texture* LoadTextureTga(Renderer* ren, File* f)
{
    int desclen = f->ReadUByte();
    int palette = f->ReadUByte();
    OVR_UNUSED(palette);
    int imgtype = f->ReadUByte();
    f->ReadUInt16();
    int palCount = f->ReadUInt16();
    int palSize = f->ReadUByte();
    f->ReadUInt16();
    f->ReadUInt16();
    int width = f->ReadUInt16();
    int height = f->ReadUInt16();
    int bpp = f->ReadUByte();
    f->ReadUByte();
    int imgsize = width * height * 4;
    unsigned char* imgdata = (unsigned char*) OVR_ALLOC(imgsize);
    unsigned char buf[16];
    f->Read(imgdata, desclen);
    f->Read(imgdata, palCount * (palSize + 7) >> 3);
    int bpl = width * 4;

    switch (imgtype)
    {
    case 2:
        switch (bpp)
        {
        case 24:
            for (int y = 0; y < height; y++)
                for (int x = 0; x < width; x++)
                {
                    f->Read(buf, 3);
                    imgdata[y*bpl+x*4+0] = buf[2];
                    imgdata[y*bpl+x*4+1] = buf[1];
                    imgdata[y*bpl+x*4+2] = buf[0];
                    imgdata[y*bpl+x*4+3] = 255;
                }
            break;
        case 32:
            for (int y = 0; y < height; y++)
                for (int x = 0; x < width; x++)
                {
                    f->Read(buf, 4);
                    imgdata[y*bpl+x*4+0] = buf[3];
                    imgdata[y*bpl+x*4+1] = buf[2];
                    imgdata[y*bpl+x*4+2] = buf[1];
                    imgdata[y*bpl+x*4+3] = buf[0];
                }
            break;

        default:
            OVR_FREE(imgdata);
            return NULL;
        }
        break;

    default:
        OVR_FREE(imgdata);
        return NULL;
    }

    Texture* out = ren->CreateTexture(Texture_RGBA|Texture_GenMipmaps, width, height, imgdata);
    OVR_FREE(imgdata);
    return out;
}

}}
