/************************************************************************************

Filename    :   WavPlayer_OSX.h
Content     :   A DDS file loader for cross-platform compressed texture support.
Created     :   March 5, 2013
Authors     :   Robotic Arm Software - Peter Hoff, Dan Goodman, Bryan Croteau

Copyright   :   Copyright 2013 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus LLC license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#include "Render_Device.h"

#ifdef OVR_DEFINE_NEW
#undef new
#endif

namespace OVR { namespace Render {

enum OVR_DDS_FORMAT
{
    OVR_DDS_FORMAT_UNKNOWN                     = 0,
    OVR_DDS_FORMAT_R32G32B32A32_TYPELESS       = 1,
    OVR_DDS_FORMAT_R32G32B32A32_FLOAT          = 2,
    OVR_DDS_FORMAT_R32G32B32A32_UINT           = 3,
    OVR_DDS_FORMAT_R32G32B32A32_SINT           = 4,
    OVR_DDS_FORMAT_R32G32B32_TYPELESS          = 5,
    OVR_DDS_FORMAT_R32G32B32_FLOAT             = 6,
    OVR_DDS_FORMAT_R32G32B32_UINT              = 7,
    OVR_DDS_FORMAT_R32G32B32_SINT              = 8,
    OVR_DDS_FORMAT_R16G16B16A16_TYPELESS       = 9,
    OVR_DDS_FORMAT_R16G16B16A16_FLOAT          = 10,
    OVR_DDS_FORMAT_R16G16B16A16_UNORM          = 11,
    OVR_DDS_FORMAT_R16G16B16A16_UINT           = 12,
    OVR_DDS_FORMAT_R16G16B16A16_SNORM          = 13,
    OVR_DDS_FORMAT_R16G16B16A16_SINT           = 14,
    OVR_DDS_FORMAT_R32G32_TYPELESS             = 15,
    OVR_DDS_FORMAT_R32G32_FLOAT                = 16,
    OVR_DDS_FORMAT_R32G32_UINT                 = 17,
    OVR_DDS_FORMAT_R32G32_SINT                 = 18,
    OVR_DDS_FORMAT_R32G8X24_TYPELESS           = 19,
    OVR_DDS_FORMAT_D32_FLOAT_S8X24_UINT        = 20,
    OVR_DDS_FORMAT_R32_FLOAT_X8X24_TYPELESS    = 21,
    OVR_DDS_FORMAT_X32_TYPELESS_G8X24_UINT     = 22,
    OVR_DDS_FORMAT_R10G10B10A2_TYPELESS        = 23,
    OVR_DDS_FORMAT_R10G10B10A2_UNORM           = 24,
    OVR_DDS_FORMAT_R10G10B10A2_UINT            = 25,
    OVR_DDS_FORMAT_R11G11B10_FLOAT             = 26,
    OVR_DDS_FORMAT_R8G8B8A8_TYPELESS           = 27,
    OVR_DDS_FORMAT_R8G8B8A8_UNORM              = 28,
    OVR_DDS_FORMAT_R8G8B8A8_UNORM_SRGB         = 29,
    OVR_DDS_FORMAT_R8G8B8A8_UINT               = 30,
    OVR_DDS_FORMAT_R8G8B8A8_SNORM              = 31,
    OVR_DDS_FORMAT_R8G8B8A8_SINT               = 32,
    OVR_DDS_FORMAT_R16G16_TYPELESS             = 33,
    OVR_DDS_FORMAT_R16G16_FLOAT                = 34,
    OVR_DDS_FORMAT_R16G16_UNORM                = 35,
    OVR_DDS_FORMAT_R16G16_UINT                 = 36,
    OVR_DDS_FORMAT_R16G16_SNORM                = 37,
    OVR_DDS_FORMAT_R16G16_SINT                 = 38,
    OVR_DDS_FORMAT_R32_TYPELESS                = 39,
    OVR_DDS_FORMAT_D32_FLOAT                   = 40,
    OVR_DDS_FORMAT_R32_FLOAT                   = 41,
    OVR_DDS_FORMAT_R32_UINT                    = 42,
    OVR_DDS_FORMAT_R32_SINT                    = 43,
    OVR_DDS_FORMAT_R24G8_TYPELESS              = 44,
    OVR_DDS_FORMAT_D24_UNORM_S8_UINT           = 45,
    OVR_DDS_FORMAT_R24_UNORM_X8_TYPELESS       = 46,
    OVR_DDS_FORMAT_X24_TYPELESS_G8_UINT        = 47,
    OVR_DDS_FORMAT_R8G8_TYPELESS               = 48,
    OVR_DDS_FORMAT_R8G8_UNORM                  = 49,
    OVR_DDS_FORMAT_R8G8_UINT                   = 50,
    OVR_DDS_FORMAT_R8G8_SNORM                  = 51,
    OVR_DDS_FORMAT_R8G8_SINT                   = 52,
    OVR_DDS_FORMAT_R16_TYPELESS                = 53,
    OVR_DDS_FORMAT_R16_FLOAT                   = 54,
    OVR_DDS_FORMAT_D16_UNORM                   = 55,
    OVR_DDS_FORMAT_R16_UNORM                   = 56,
    OVR_DDS_FORMAT_R16_UINT                    = 57,
    OVR_DDS_FORMAT_R16_SNORM                   = 58,
    OVR_DDS_FORMAT_R16_SINT                    = 59,
    OVR_DDS_FORMAT_R8_TYPELESS                 = 60,
    OVR_DDS_FORMAT_R8_UNORM                    = 61,
    OVR_DDS_FORMAT_R8_UINT                     = 62,
    OVR_DDS_FORMAT_R8_SNORM                    = 63,
    OVR_DDS_FORMAT_R8_SINT                     = 64,
    OVR_DDS_FORMAT_A8_UNORM                    = 65,
    OVR_DDS_FORMAT_R1_UNORM                    = 66,
    OVR_DDS_FORMAT_R9G9B9E5_SHAREDEXP          = 67,
    OVR_DDS_FORMAT_R8G8_B8G8_UNORM             = 68,
    OVR_DDS_FORMAT_G8R8_G8B8_UNORM             = 69,
    OVR_DDS_FORMAT_BC1_TYPELESS                = 70,
    OVR_DDS_FORMAT_BC1_UNORM                   = 71,
    OVR_DDS_FORMAT_BC1_UNORM_SRGB              = 72,
    OVR_DDS_FORMAT_BC2_TYPELESS                = 73,
    OVR_DDS_FORMAT_BC2_UNORM                   = 74,
    OVR_DDS_FORMAT_BC2_UNORM_SRGB              = 75,
    OVR_DDS_FORMAT_BC3_TYPELESS                = 76,
    OVR_DDS_FORMAT_BC3_UNORM                   = 77,
    OVR_DDS_FORMAT_BC3_UNORM_SRGB              = 78,
    OVR_DDS_FORMAT_BC4_TYPELESS                = 79,
    OVR_DDS_FORMAT_BC4_UNORM                   = 80,
    OVR_DDS_FORMAT_BC4_SNORM                   = 81,
    OVR_DDS_FORMAT_BC5_TYPELESS                = 82,
    OVR_DDS_FORMAT_BC5_UNORM                   = 83,
    OVR_DDS_FORMAT_BC5_SNORM                   = 84,
    OVR_DDS_FORMAT_B5G6R5_UNORM                = 85,
    OVR_DDS_FORMAT_B5G5R5A1_UNORM              = 86,
    OVR_DDS_FORMAT_B8G8R8A8_UNORM              = 87,
    OVR_DDS_FORMAT_B8G8R8X8_UNORM              = 88,
    OVR_DDS_FORMAT_R10G10B10_XR_BIAS_A2_UNORM  = 89,
    OVR_DDS_FORMAT_B8G8R8A8_TYPELESS           = 90,
    OVR_DDS_FORMAT_B8G8R8A8_UNORM_SRGB         = 91,
    OVR_DDS_FORMAT_B8G8R8X8_TYPELESS           = 92,
    OVR_DDS_FORMAT_B8G8R8X8_UNORM_SRGB         = 93,
    OVR_DDS_FORMAT_BC6H_TYPELESS               = 94,
    OVR_DDS_FORMAT_BC6H_UF16                   = 95,
    OVR_DDS_FORMAT_BC6H_SF16                   = 96,
    OVR_DDS_FORMAT_BC7_TYPELESS                = 97,
    OVR_DDS_FORMAT_BC7_UNORM                   = 98,
    OVR_DDS_FORMAT_BC7_UNORM_SRGB              = 99,
    OVR_DDS_FORMAT_AYUV                        = 100,
    OVR_DDS_FORMAT_Y410                        = 101,
    OVR_DDS_FORMAT_Y416                        = 102,
    OVR_DDS_FORMAT_NV12                        = 103,
    OVR_DDS_FORMAT_P010                        = 104,
    OVR_DDS_FORMAT_P016                        = 105,
    OVR_DDS_FORMAT_420_OPAQUE                  = 106,
    OVR_DDS_FORMAT_YUY2                        = 107,
    OVR_DDS_FORMAT_Y210                        = 108,
    OVR_DDS_FORMAT_Y216                        = 109,
    OVR_DDS_FORMAT_NV11                        = 110,
    OVR_DDS_FORMAT_AI44                        = 111,
    OVR_DDS_FORMAT_IA44                        = 112,
    OVR_DDS_FORMAT_P8                          = 113,
    OVR_DDS_FORMAT_A8P8                        = 114,
    OVR_DDS_FORMAT_B4G4R4A4_UNORM              = 115,
    OVR_DDS_FORMAT_FORCE_UINT                  = 0xffffffffUL
};

enum OVR_DDS_RESOURCE_DIMENSION
{
    OVR_DDS_RESOURCE_DIMENSION_UNKNOWN    = 0,
    OVR_DDS_RESOURCE_DIMENSION_BUFFER     = 1,
    OVR_DDS_RESOURCE_DIMENSION_TEXTURE1D  = 2,
    OVR_DDS_RESOURCE_DIMENSION_TEXTURE2D  = 3,
    OVR_DDS_RESOURCE_DIMENSION_TEXTURE3D  = 4
};

enum OVR_DDS_PF_FLAGS
{
    OVR_DDS_PF_ALPHAPIXELS = 0x1,
    OVR_DDS_PF_ALPHA = 0x2,
    OVR_DDS_PF_FOURCC = 0x4,
    OVR_DDS_PF_RGB = 0x40,
    OVR_DDS_PF_YUV = 0x200,
    OVR_DDS_PF_LUMINANCE = 0x20000
};

enum OVR_DDS_HEADER_FLAGS
{
    OVR_DDS_HEADER_CAPS = 0x1,
    OVR_DDS_HEADER_HEIGHT = 0x2,
    OVR_DDS_HEADER_WIDTH = 0x4,
    OVR_DDS_HEADER_PITCH = 0x8,
    OVR_DDS_HEADER_PIXELFORMAT = 0x1000,
    OVR_DDS_HEADER_MIPMAPCOUNT = 0x20000,
    OVR_DDS_HEADER_LINEARSIZE = 0x80000,
    OVR_DDS_HEADER_DEPTH = 0x800000
};

enum OVR_DDS_HEADER_CAPS_FLAGS
{
    OVR_DDS_CAPS_COMPLEX = 0x8,
    OVR_DDS_CAPS_MIPMAP = 0x400000,
    OVR_DDS_CAPS_TEXTURE = 0x1000
};

enum OVR_DDS_HEADER_CAPS2_FLAGS
{
    OVR_DDS_CAPS2_CUBEMAP = 0x200,
    OVR_DDS_CAPS2_CUBEMAP_POSITIVEX = 0x400,
    OVR_DDS_CAPS2_CUBEMAP_NEGATIVEX = 0x800,
    OVR_DDS_CAPS2_CUBEMAP_POSITIVEY = 0x1000,
    OVR_DDS_CAPS2_CUBEMAP_NEGATIVEY = 0x2000,
    OVR_DDS_CAPS2_CUBEMAP_POSITIVEZ = 0x4000,
    OVR_DDS_CAPS2_CUBEMAP_NEGATIVEZ = 0x8000,
    OVR_DDS_CAPS2_VOLUME = 0x200000,
};

enum OVR_DDS_CUBEMAP
{
    OVR_DDS_CUBEMAP_POSITIVEX = OVR_DDS_CAPS2_CUBEMAP | OVR_DDS_CAPS2_CUBEMAP_POSITIVEX,
    OVR_DDS_CUBEMAP_NEGATIVEX = OVR_DDS_CAPS2_CUBEMAP | OVR_DDS_CAPS2_CUBEMAP_NEGATIVEX,
    OVR_DDS_CUBEMAP_POSITIVEY = OVR_DDS_CAPS2_CUBEMAP | OVR_DDS_CAPS2_CUBEMAP_POSITIVEY,
    OVR_DDS_CUBEMAP_NEGATIVEY = OVR_DDS_CAPS2_CUBEMAP | OVR_DDS_CAPS2_CUBEMAP_NEGATIVEY,
    OVR_DDS_CUBEMAP_POSITIVEZ = OVR_DDS_CAPS2_CUBEMAP | OVR_DDS_CAPS2_CUBEMAP_POSITIVEZ,
    OVR_DDS_CUBEMAP_NEGATIVEZ = OVR_DDS_CAPS2_CUBEMAP | OVR_DDS_CAPS2_CUBEMAP_NEGATIVEZ,
    OVR_DDS_CUBEMAP_ALLFACES  = OVR_DDS_CAPS2_CUBEMAP_POSITIVEX | OVR_DDS_CAPS2_CUBEMAP_NEGATIVEX |
                                OVR_DDS_CAPS2_CUBEMAP_POSITIVEY | OVR_DDS_CAPS2_CUBEMAP_NEGATIVEY |
                                OVR_DDS_CAPS2_CUBEMAP_POSITIVEZ | OVR_DDS_CAPS2_CUBEMAP_NEGATIVEZ
};

struct OVR_DDS_PIXELFORMAT
{
    UInt32 Size;
    UInt32 Flags;
    UInt32 FourCC;
    UInt32 RGBBitCount;
    UInt32 RBitMask;
    UInt32 GBitMask;
    UInt32 BBitMask;
    UInt32 ABitMask;
};

struct OVR_DDS_HEADER
{
    UInt32				Size;
    UInt32				Flags;
    UInt32				Height;
    UInt32				Width;
    UInt32				PitchOrLinearSize;
    UInt32				Depth;
    UInt32				MipMapCount;
    UInt32				Reserved1[11];
    OVR_DDS_PIXELFORMAT ddspf;
    UInt32				Caps;
    UInt32				Caps2;
    UInt32				Caps3;
    UInt32				Caps4;
    UInt32				Reserved2;
};

struct OVR_DDS_HEADER_DXT10
{
    OVR_DDS_FORMAT			   DXGIFormat;
    OVR_DDS_RESOURCE_DIMENSION ResourceDimension;
    UInt32					   MiscFlag;
    UInt32					   ArraySize;
    UInt32					   Reserved;
};

struct OVR_SUBRESOURCE_DATA
{
    const void* Bytes;
    UInt32        Pitch;
    UInt32        SlicePitch;
};

static size_t BitsPerPixel(OVR_DDS_FORMAT format)
{
    switch(format)
    {
    case OVR_DDS_FORMAT_R32G32B32A32_TYPELESS:
    case OVR_DDS_FORMAT_R32G32B32A32_FLOAT:
    case OVR_DDS_FORMAT_R32G32B32A32_UINT:
    case OVR_DDS_FORMAT_R32G32B32A32_SINT:
        return 128u;

    case OVR_DDS_FORMAT_R32G32B32_TYPELESS:
    case OVR_DDS_FORMAT_R32G32B32_FLOAT:
    case OVR_DDS_FORMAT_R32G32B32_UINT:
    case OVR_DDS_FORMAT_R32G32B32_SINT:
        return 96u;

    case OVR_DDS_FORMAT_R16G16B16A16_TYPELESS:
    case OVR_DDS_FORMAT_R16G16B16A16_FLOAT:
    case OVR_DDS_FORMAT_R16G16B16A16_UNORM:
    case OVR_DDS_FORMAT_R16G16B16A16_UINT:
    case OVR_DDS_FORMAT_R16G16B16A16_SNORM:
    case OVR_DDS_FORMAT_R16G16B16A16_SINT:
    case OVR_DDS_FORMAT_R32G32_TYPELESS:
    case OVR_DDS_FORMAT_R32G32_FLOAT:
    case OVR_DDS_FORMAT_R32G32_UINT:
    case OVR_DDS_FORMAT_R32G32_SINT:
    case OVR_DDS_FORMAT_R32G8X24_TYPELESS:
    case OVR_DDS_FORMAT_D32_FLOAT_S8X24_UINT:
    case OVR_DDS_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    case OVR_DDS_FORMAT_X32_TYPELESS_G8X24_UINT:
        return 64u;

    case OVR_DDS_FORMAT_R10G10B10A2_TYPELESS:
    case OVR_DDS_FORMAT_R10G10B10A2_UNORM:
    case OVR_DDS_FORMAT_R10G10B10A2_UINT:
    case OVR_DDS_FORMAT_R11G11B10_FLOAT:
    case OVR_DDS_FORMAT_R8G8B8A8_TYPELESS:
    case OVR_DDS_FORMAT_R8G8B8A8_UNORM:
    case OVR_DDS_FORMAT_R8G8B8A8_UNORM_SRGB:
    case OVR_DDS_FORMAT_R8G8B8A8_UINT:
    case OVR_DDS_FORMAT_R8G8B8A8_SNORM:
    case OVR_DDS_FORMAT_R8G8B8A8_SINT:
    case OVR_DDS_FORMAT_R16G16_TYPELESS:
    case OVR_DDS_FORMAT_R16G16_FLOAT:
    case OVR_DDS_FORMAT_R16G16_UNORM:
    case OVR_DDS_FORMAT_R16G16_UINT:
    case OVR_DDS_FORMAT_R16G16_SNORM:
    case OVR_DDS_FORMAT_R16G16_SINT:
    case OVR_DDS_FORMAT_R32_TYPELESS:
    case OVR_DDS_FORMAT_D32_FLOAT:
    case OVR_DDS_FORMAT_R32_FLOAT:
    case OVR_DDS_FORMAT_R32_UINT:
    case OVR_DDS_FORMAT_R32_SINT:
    case OVR_DDS_FORMAT_R24G8_TYPELESS:
    case OVR_DDS_FORMAT_D24_UNORM_S8_UINT:
    case OVR_DDS_FORMAT_R24_UNORM_X8_TYPELESS:
    case OVR_DDS_FORMAT_X24_TYPELESS_G8_UINT:
    case OVR_DDS_FORMAT_R9G9B9E5_SHAREDEXP:
    case OVR_DDS_FORMAT_R8G8_B8G8_UNORM:
    case OVR_DDS_FORMAT_G8R8_G8B8_UNORM:
    case OVR_DDS_FORMAT_B8G8R8A8_UNORM:
    case OVR_DDS_FORMAT_B8G8R8X8_UNORM:
    case OVR_DDS_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
    case OVR_DDS_FORMAT_B8G8R8A8_TYPELESS:
    case OVR_DDS_FORMAT_B8G8R8A8_UNORM_SRGB:
    case OVR_DDS_FORMAT_B8G8R8X8_TYPELESS:
    case OVR_DDS_FORMAT_B8G8R8X8_UNORM_SRGB:
        return 32u;

    case OVR_DDS_FORMAT_R8G8_TYPELESS:
    case OVR_DDS_FORMAT_R8G8_UNORM:
    case OVR_DDS_FORMAT_R8G8_UINT:
    case OVR_DDS_FORMAT_R8G8_SNORM:
    case OVR_DDS_FORMAT_R8G8_SINT:
    case OVR_DDS_FORMAT_R16_TYPELESS:
    case OVR_DDS_FORMAT_R16_FLOAT:
    case OVR_DDS_FORMAT_D16_UNORM:
    case OVR_DDS_FORMAT_R16_UNORM:
    case OVR_DDS_FORMAT_R16_UINT:
    case OVR_DDS_FORMAT_R16_SNORM:
    case OVR_DDS_FORMAT_R16_SINT:
    case OVR_DDS_FORMAT_B5G6R5_UNORM:
    case OVR_DDS_FORMAT_B5G5R5A1_UNORM:
    case OVR_DDS_FORMAT_B4G4R4A4_UNORM:
        return 16u;

    case OVR_DDS_FORMAT_R8_TYPELESS:
    case OVR_DDS_FORMAT_R8_UNORM:
    case OVR_DDS_FORMAT_R8_UINT:
    case OVR_DDS_FORMAT_R8_SNORM:
    case OVR_DDS_FORMAT_R8_SINT:
    case OVR_DDS_FORMAT_A8_UNORM:
    case OVR_DDS_FORMAT_BC2_TYPELESS:
    case OVR_DDS_FORMAT_BC2_UNORM:
    case OVR_DDS_FORMAT_BC2_UNORM_SRGB:
    case OVR_DDS_FORMAT_BC3_TYPELESS:
    case OVR_DDS_FORMAT_BC3_UNORM:
    case OVR_DDS_FORMAT_BC3_UNORM_SRGB:
    case OVR_DDS_FORMAT_BC5_TYPELESS:
    case OVR_DDS_FORMAT_BC5_UNORM:
    case OVR_DDS_FORMAT_BC5_SNORM:
    case OVR_DDS_FORMAT_BC6H_TYPELESS:
    case OVR_DDS_FORMAT_BC6H_UF16:
    case OVR_DDS_FORMAT_BC6H_SF16:
    case OVR_DDS_FORMAT_BC7_TYPELESS:
    case OVR_DDS_FORMAT_BC7_UNORM:
    case OVR_DDS_FORMAT_BC7_UNORM_SRGB:
        return 8u;

    case OVR_DDS_FORMAT_R1_UNORM:
        return 1u;

    case OVR_DDS_FORMAT_BC1_TYPELESS:
    case OVR_DDS_FORMAT_BC1_UNORM:
    case OVR_DDS_FORMAT_BC1_UNORM_SRGB:
    case OVR_DDS_FORMAT_BC4_TYPELESS:
    case OVR_DDS_FORMAT_BC4_UNORM:
    case OVR_DDS_FORMAT_BC4_SNORM:
        return 4u;
    default:
        return 0u;
    }
}

static void GetSurfaceInfo(
    size_t width,
    size_t height,
    OVR_DDS_FORMAT format,
    size_t* outNumBytes,
    size_t* outRowBytes,
    size_t* outNumRows
)
{
    size_t numBytes = 0;
    size_t rowBytes = 0;
    size_t numRows = 0;

    bool bc = false;
    bool packed  = false;
    size_t bcnumBytesPerBlock = 0;
    switch(format)
    {
    case OVR_DDS_FORMAT_BC1_TYPELESS:
    case OVR_DDS_FORMAT_BC1_UNORM:
    case OVR_DDS_FORMAT_BC1_UNORM_SRGB:
    case OVR_DDS_FORMAT_BC4_TYPELESS:
    case OVR_DDS_FORMAT_BC4_UNORM:
    case OVR_DDS_FORMAT_BC4_SNORM:
        bc = true;
        bcnumBytesPerBlock = 8;
        break;

    case OVR_DDS_FORMAT_BC2_TYPELESS:
    case OVR_DDS_FORMAT_BC2_UNORM:
    case OVR_DDS_FORMAT_BC2_UNORM_SRGB:
    case OVR_DDS_FORMAT_BC3_TYPELESS:
    case OVR_DDS_FORMAT_BC3_UNORM:
    case OVR_DDS_FORMAT_BC3_UNORM_SRGB:
    case OVR_DDS_FORMAT_BC5_TYPELESS:
    case OVR_DDS_FORMAT_BC5_UNORM:
    case OVR_DDS_FORMAT_BC5_SNORM:
    case OVR_DDS_FORMAT_BC6H_TYPELESS:
    case OVR_DDS_FORMAT_BC6H_UF16:
    case OVR_DDS_FORMAT_BC6H_SF16:
    case OVR_DDS_FORMAT_BC7_TYPELESS:
    case OVR_DDS_FORMAT_BC7_UNORM:
    case OVR_DDS_FORMAT_BC7_UNORM_SRGB:
        bc = true;
        bcnumBytesPerBlock = 16;
        break;

    case OVR_DDS_FORMAT_R8G8_B8G8_UNORM:
    case OVR_DDS_FORMAT_G8R8_G8B8_UNORM:
        packed = true;
        break;
    }

    if(bc)
    {
        size_t numBlocksWide = 0;
        if(width > 0)
        {
            numBlocksWide = (width + 3) / 4;
            if(numBlocksWide < 1)
            {
                numBlocksWide = 1;
            }
        }
        size_t numBlocksHigh = 0;
        if(height > 0)
        {
            numBlocksHigh = (height + 3) / 4;
            if(numBlocksHigh < 1)
            {
                numBlocksHigh = 1;
            }
        }
        rowBytes = numBlocksWide * bcnumBytesPerBlock;
        numRows = numBlocksHigh;
    }
    else if(packed)
    {
        rowBytes = ((width + 1) >> 1) * 4;
        numRows = height;
    }
    else
    {
        size_t bpp = BitsPerPixel(format);
        rowBytes = (width * bpp + 7) / 8;
        numRows = height;
    }

    numBytes = rowBytes * numRows;
    if(outNumBytes)
    {
        *outNumBytes = numBytes;
    }
    if(outRowBytes)
    {
        *outRowBytes = rowBytes;
    }
    if(outNumRows)
    {
        *outNumRows = numRows;
    }
}

inline bool ISBITMASK(const OVR_DDS_PIXELFORMAT &ddpf, UInt32 r, UInt32 g, UInt32 b, UInt32 a)
{
    return (ddpf.RBitMask == r && ddpf.GBitMask == g && ddpf.BBitMask == b && ddpf.ABitMask == a);
}

#ifndef MAKEFOURCC
inline UInt32 MAKEFOURCC(char a, char b, char c, char d)
{
    return a | (UInt32(b) << 8) | (UInt32(c) << 16) | (UInt32(d) << 24);
}
#endif

static OVR_DDS_FORMAT GetDXGIFormat(const OVR_DDS_PIXELFORMAT& ddpf)
{
    if(ddpf.Flags & OVR_DDS_PF_RGB)
    {
        switch(ddpf.RGBBitCount)
        {
        case 32:
            if(ISBITMASK(ddpf, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
            {
                return OVR_DDS_FORMAT_R8G8B8A8_UNORM;
            }

            if(ISBITMASK(ddpf, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
            {
                return OVR_DDS_FORMAT_B8G8R8A8_UNORM;
            }

            if(ISBITMASK(ddpf, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000))
            {
                return OVR_DDS_FORMAT_B8G8R8X8_UNORM;
            }

            if(ISBITMASK(ddpf, 0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
            {
                return OVR_DDS_FORMAT_R10G10B10A2_UNORM;
            }

            if(ISBITMASK(ddpf, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
            {
                return OVR_DDS_FORMAT_R16G16_UNORM;
            }

            if(ISBITMASK(ddpf, 0xffffffff, 0x00000000, 0x00000000, 0x00000000))
            {
                return OVR_DDS_FORMAT_R32_FLOAT;
            }
            break;

        case 24:
            break;

        case 16:
            if(ISBITMASK(ddpf, 0x7c00, 0x03e0, 0x001f, 0x8000))
            {
                return OVR_DDS_FORMAT_B5G5R5A1_UNORM;
            }

            if(ISBITMASK(ddpf, 0xf800, 0x07e0, 0x001f, 0x0000))
            {
                return OVR_DDS_FORMAT_B5G6R5_UNORM;
            }

            if(ISBITMASK(ddpf, 0x0f00, 0x00f0, 0x000f, 0xf000))
            {
                return OVR_DDS_FORMAT_B4G4R4A4_UNORM;
            }
            break;
        }
    }
    else if(ddpf.Flags & OVR_DDS_PF_LUMINANCE)
    {
        if(8 == ddpf.RGBBitCount)
        {
            if(ISBITMASK(ddpf, 0x000000ff, 0x00000000, 0x00000000, 0x00000000))
            {
                return OVR_DDS_FORMAT_R8_UNORM;
            }
        }

        if(16 == ddpf.RGBBitCount)
        {
            if(ISBITMASK(ddpf, 0x0000ffff, 0x00000000, 0x00000000, 0x00000000))
            {
                return OVR_DDS_FORMAT_R16_UNORM;
            }
            if(ISBITMASK(ddpf, 0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
            {
                return OVR_DDS_FORMAT_R8G8_UNORM;
            }
        }
    }
    else if(ddpf.Flags & OVR_DDS_PF_ALPHA)
    {
        if(8 == ddpf.RGBBitCount)
        {
            return OVR_DDS_FORMAT_A8_UNORM;
        }
    }
    else if(ddpf.Flags & OVR_DDS_PF_FOURCC)
    {
        if(MAKEFOURCC('D', 'X', 'T', '1') == ddpf.FourCC)
        {
            return OVR_DDS_FORMAT_BC1_UNORM;
        }
        if(MAKEFOURCC('D', 'X', 'T', '3') == ddpf.FourCC)
        {
            return OVR_DDS_FORMAT_BC2_UNORM;
        }
        if(MAKEFOURCC('D', 'X', 'T', '5') == ddpf.FourCC)
        {
            return OVR_DDS_FORMAT_BC3_UNORM;
        }

        if(MAKEFOURCC('D', 'X', 'T', '2') == ddpf.FourCC)
        {
            return OVR_DDS_FORMAT_BC2_UNORM;
        }
        if(MAKEFOURCC('D', 'X', 'T', '4') == ddpf.FourCC)
        {
            return OVR_DDS_FORMAT_BC3_UNORM;
        }

        if(MAKEFOURCC('A', 'T', 'I', '1') == ddpf.FourCC)
        {
            return OVR_DDS_FORMAT_BC4_UNORM;
        }
        if(MAKEFOURCC('B', 'C', '4', 'U') == ddpf.FourCC)
        {
            return OVR_DDS_FORMAT_BC4_UNORM;
        }
        if(MAKEFOURCC('B', 'C', '4', 'S') == ddpf.FourCC)
        {
            return OVR_DDS_FORMAT_BC4_SNORM;
        }

        if(MAKEFOURCC('A', 'T', 'I', '2') == ddpf.FourCC)
        {
            return OVR_DDS_FORMAT_BC5_UNORM;
        }
        if(MAKEFOURCC('B', 'C', '5', 'U') == ddpf.FourCC)
        {
            return OVR_DDS_FORMAT_BC5_UNORM;
        }
        if(MAKEFOURCC('B', 'C', '5', 'S') == ddpf.FourCC)
        {
            return OVR_DDS_FORMAT_BC5_SNORM;
        }

        if(MAKEFOURCC('R', 'G', 'B', 'G') == ddpf.FourCC)
        {
            return OVR_DDS_FORMAT_R8G8_B8G8_UNORM;
        }
        if(MAKEFOURCC('G', 'R', 'G', 'B') == ddpf.FourCC)
        {
            return OVR_DDS_FORMAT_G8R8_G8B8_UNORM;
        }

        switch(ddpf.FourCC)
        {
        case 36:
            return OVR_DDS_FORMAT_R16G16B16A16_UNORM;

        case 110:
            return OVR_DDS_FORMAT_R16G16B16A16_SNORM;

        case 111:
            return OVR_DDS_FORMAT_R16_FLOAT;

        case 112:
            return OVR_DDS_FORMAT_R16G16_FLOAT;

        case 113:
            return OVR_DDS_FORMAT_R16G16B16A16_FLOAT;

        case 114:
            return OVR_DDS_FORMAT_R32_FLOAT;

        case 115:
            return OVR_DDS_FORMAT_R32G32_FLOAT;

        case 116:
            return OVR_DDS_FORMAT_R32G32B32A32_FLOAT;
        }
    }

    return OVR_DDS_FORMAT_UNKNOWN;
}

static void FillInitData(
    size_t width,
    size_t height,
    size_t depth,
    size_t mipCount,
    size_t arraySize,
    OVR_DDS_FORMAT format,
    size_t maxsize,
    const unsigned char* bitData,
    size_t& twidth,
    size_t& theight,
    size_t& tdepth,
    size_t& skipMip,
    OVR_SUBRESOURCE_DATA* initData,
	size_t& byteSize
)
{
    skipMip = 0;
    twidth = 0;
    theight = 0;
    tdepth = 0;

    size_t NumBytes = 0;
    size_t RowBytes = 0;
    size_t NumRows = 0;
    const unsigned char* pSrcBits = bitData;

    size_t index = 0;
    for(size_t j = 0; j < arraySize; j++)
    {
        size_t w = width;
        size_t h = height;
        size_t d = depth;
        for(size_t i = 0; i < mipCount; i++)
        {
            GetSurfaceInfo(w, h, format, &NumBytes, &RowBytes, &NumRows);

            if((mipCount <= 1) || !maxsize || (w <= maxsize && h <= maxsize && d <= maxsize))
            {
                if(!twidth)
                {
                    twidth = w;
                    theight = h;
                    tdepth = d;
                }

                initData[index].Bytes = (const void*)pSrcBits;
                initData[index].Pitch = static_cast<UInt32>(RowBytes);
                initData[index].SlicePitch = static_cast<UInt32>(NumBytes);
				byteSize += NumBytes * d;
                ++index;
            }
            else
            {
                ++skipMip;
            }

            pSrcBits += NumBytes * d;

            w = w >> 1;
            h = h >> 1;
            d = d >> 1;
            if(w == 0)
            {
                w = 1;
            }
            if(h == 0)
            {
                h = 1;
            }
            if(d == 0)
            {
                d = 1;
            }
        }
    }
}

#define _256Megabytes 268435456
#define _512Megabytes 536870912

Texture* LoadTextureDDS(RenderDevice* ren, File* f, UPInt gpuMemorySize, size_t& textureSize)
{
    OVR_DDS_HEADER		 header;
    OVR_DDS_HEADER_DXT10 ext;
    unsigned char		 filecode[4];

    f->Read(filecode, 4);
    if (strncmp((const char*)filecode, "DDS ", 4) != 0)
    {
        return NULL;
    }

    f->Read((unsigned char*)(&header), sizeof(header));

    if ((header.ddspf.Flags & OVR_DDS_PF_FOURCC) && (MAKEFOURCC('D', 'X', '1', '0') == header.ddspf.FourCC))
    {
        f->Read((unsigned char*)(&ext), sizeof(ext));
    }

    size_t width = header.Width;
    size_t height = header.Height;
    size_t depth = header.Depth;

    OVR_DDS_RESOURCE_DIMENSION resDim = OVR_DDS_RESOURCE_DIMENSION_UNKNOWN;
    size_t arraySize = 1;
    OVR_DDS_FORMAT format = OVR_DDS_FORMAT_UNKNOWN;
    int oformat;
    bool isCubeMap = false;

    size_t mipCount = header.MipMapCount;
    if (0 == mipCount)
    {
        mipCount = 1;
    }
    format = GetDXGIFormat(header.ddspf);
    if (header.Flags & OVR_DDS_CAPS2_VOLUME)
    {
        resDim = OVR_DDS_RESOURCE_DIMENSION_TEXTURE3D;
    }
    else
    {
        if (header.Caps2 & OVR_DDS_CAPS2_CUBEMAP)
        {
            arraySize = 6;
            isCubeMap = true;
        }

        depth = 1;
        resDim = OVR_DDS_RESOURCE_DIMENSION_TEXTURE2D;
    }
    switch (format)
    {
        case OVR_DDS_FORMAT_BC1_UNORM:  oformat = Texture_DXT1; break;
        case OVR_DDS_FORMAT_BC2_UNORM:  oformat = Texture_DXT3; break;
        case OVR_DDS_FORMAT_BC3_UNORM:  oformat = Texture_DXT5; break;
        default:
            return NULL;
    }

    OVR_SUBRESOURCE_DATA* initData = new OVR_SUBRESOURCE_DATA[mipCount * arraySize];
    size_t skipMip = 0;
    size_t twidth = 0;
    size_t theight = 0;
    size_t tdepth = 0;
    int            byteLen = f->BytesAvailable();
    unsigned char* bytes   = new unsigned char[byteLen];
    f->Read(bytes, byteLen);
	
    size_t maxsize = 0;
	if (gpuMemorySize <= _256Megabytes)
	{
		maxsize = 512;
	}
	else if (gpuMemorySize <= _512Megabytes)
	{
		maxsize = 1024;
	}
	textureSize = 0;
    FillInitData(width, height, depth, mipCount, arraySize,
                 format, maxsize, bytes, twidth, theight, tdepth,
                 skipMip, initData, textureSize);
#ifdef OVR_OS_WIN32
    Texture* out = ren->CreateTexture(resDim, (unsigned)twidth, (unsigned)theight,
                                      (unsigned)(mipCount - skipMip),
                                      (unsigned)arraySize, format, initData);
#else
    Texture* out = ren->CreateTexture(oformat, twidth, theight, initData->Bytes, mipCount-skipMip);
#endif
	delete [] initData;

    if(strstr(f->GetFilePath(), "_c."))
    {
        out->SetSampleMode(Sample_Clamp);
    }
    OVR_FREE(bytes);
    return out;
}

}}

#ifdef OVR_DEFINE_NEW
#define new OVR_DEFINE_NEW
#endif