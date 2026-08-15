// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include <CoreTypes.h>

namespace Havtorn
{
    constexpr U32 AppendAlignedElementCode = 0xffffffff;

#ifdef HV_RENDER_BACKEND_DIRECTX11
	enum class ETopologies : U8
	{
		TriangleList = 4, //D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
		LineList = 2, //D3D11_PRIMITIVE_TOPOLOGY_LINELIST
		PointList = 1 //D3D11_PRIMITIVE_TOPOLOGY_POINTLIST
	};
#endif

	enum class EShaderType : U8
	{
		Vertex,
		Compute,
		Geometry,
		Pixel
	};

	enum class ERenderComparisonFunction : U8
	{
		Never = 1,
		Less = 2,
		Equal = 3,
		LessOrEqual = 4,
		Greater = 5,
		NotEqual = 6,
		GreaterOrEqual = 7,
		Always = 8
	};

    enum class ERenderInputClassification : U8
    {
        InputPerVertexData = 0,
        InputPerInstanceData = 1
    };

    enum class ERenderResourceFormat : U8
    {
        Unknown = 0,
        R32G32B32A32_Typeless= 1,
        R32G32B32A32_Float = 2,
        R32G32B32A32_UnsignedInt = 3,
        R32G32B32A32_SignedInt = 4,

        R32G32B32_Typeless = 5,
        R32G32B32_Float = 6,
        R32G32B32_UnsignedInt = 7,
        R32G32B32_SignedInt = 8,

        R16G16B16A16_Typeless = 9,
        R16G16B16A16_Float = 10,
        R16G16B16A16_UnsignedNormalizedInt = 11,
        R16G16B16A16_UnsignedInt = 12,
        R16G16B16A16_SignedNormalizedInt = 13,
        R16G16B16A16_SignedInt = 14,

        R32G32_Typeless = 15,
        R32G32_Float = 16,
        R32G32_UnsignedInt = 17,
        R32G32_SignedInt = 18,

        R32G8X24_Typeless = 19,
        D32_Float_S8X24_UnsignedInt = 20,
        R32_Float_X8X24_Typeless = 21,
        X32_Typeless_G8X24_UnsignedInt = 22,

        R10G10B10A2_Typeless = 23,
        R10G10B10A2_UnsignedNormalizedInt = 24,
        R10G10B10A2_UnsignedInt = 25,

        R11G11B10_Float = 26,

        R8G8B8A8_Typeless = 27,
        R8G8B8A8_UnsignedNormalizedInt = 28,
        R8G8B8A8_UnsignedNormalizedInt_SRGB = 29,
        R8G8B8A8_UnsignedInt = 30,
        R8G8B8A8_SignedNormalizedInt = 31,
        R8G8B8A8_SignedInt = 32,

        R16G16_Typeless = 33,
        R16G16_Float = 34,
        R16G16_UnsignedNormalizedInt = 35,
        R16G16_UnsignedInt = 36,
        R16G16_SignedNormalizedInt = 37,
        R16G16_SignedInt = 38,

        R32_Typeless = 39,
        D32_Float = 40,
        R32_Float = 41,
        R32_UnsignedInt = 42,
        R32_SignedInt = 43,

        R24G8_Typeless = 44,
        D24_UnsignedNormalizedInt_S8_UnsignedInt = 45,
        R24_UnsignedNormalizedInt_X8_Typeless = 46,
        X24_Typeless_G8_UnsignedInt = 47,

        R8G8_Typeless = 48,
        R8G8_UnsignedNormalizedInt = 49,
        R8G8_UnsignedInt = 50,
        R8G8_SignedNormalizedInt = 51,
        R8G8_SignedInt = 52,

        R16_Typeless = 53,
        R16_Float = 54,
        D16_UnsignedNormalizedInt = 55,
        R16_UnsignedNormalizedInt = 56,
        R16_UnsignedInt = 57,
        R16_SignedNormalizedInt = 58,
        R16_SignedInt = 59,

        R8_Typeless = 60,
        R8_UnsignedNormalizedInt = 61,
        R8_UnsignedInt = 62,
        R8_SignedNormalizedInt = 63,
        R8_SignedInt = 64,
        A8_UnsignedNormalizedInt = 65,

        R1_UnsignedNormalizedInt = 66,
        R9G9B9E5_SharedExponent = 67,
        R8G8_B8G8_UnsignedNormalizedInt = 68,
        G8R8_G8B8_UnsignedNormalizedInt = 69,

        BC1_Typeless = 70,
        BC1_UnsignedNormalizedInt = 71,
        BC1_UnsignedNormalizedInt_SRGB = 72,
        BC2_Typeless = 73,
        BC2_UnsignedNormalizedInt = 74,
        BC2_UnsignedNormalizedInt_SRGB = 75,
        BC3_Typeless = 76,
        BC3_UnsignedNormalizedInt = 77,
        BC3_UnsignedNormalizedInt_SRGB = 78,
        BC4_Typeless = 79,
        BC4_UnsignedNormalizedInt = 80,
        BC4_SignedNormalizedInt = 81,
        BC5_Typeless = 82,
        BC5_UnsignedNormalizedInt = 83,
        BC5_SignedNormalizedInt = 84,

        B5G6R5_UnsignedNormalizedInt = 85,
        B5G5R5A1_UnsignedNormalizedInt = 86,
        B8G8R8A8_UnsignedNormalizedInt = 87,
        B8G8R8X8_UnsignedNormalizedInt = 88,

        R10G10B10_XR_Biased_A2_UnsignedNormalizedInt = 89,

        B8G8R8A8_Typeless = 90,
        B8G8R8A8_UnsignedNormalizedInt_SRGB = 91,
        B8G8R8X8_Typeless = 92,
        B8G8R8X8_UnsignedNormalizedInt_SRGB = 93,

        BC6H_Typeless = 94,
        BC6H_UF16 = 95,
        BC6H_SF16 = 96,
        BC7_Typeless = 97,
        BC7_UnsignedNormalizedInt = 98,
        BC7_UnsignedNormalizedInt_SRGB = 99,

        AYUV = 100,
        Y410 = 101,
        Y416 = 102,
        NV12 = 103,
        P010 = 104,
        P016 = 105,
        YUV_420_OPAQUE = 106,
        YUY2 = 107,
        Y210 = 108,
        Y216 = 109,
        NV11 = 110,
        AI44 = 111,
        IA44 = 112,
        P8 = 113,
        A8P8 = 114,
        B4G4R4A4_UnsignedNormalizedInt = 115,

        P208 = 130,
        V208 = 131,
        V408 = 132,

        SamplerFeedbackMinMipOpaque = 189,
        SamplerFeedbackMipRegionUsedOpaque = 190,

        A4B4G4R4_UnsignedNormalizedInt = 191,

        Force_UnsignedInt = 0xff
    };
}
