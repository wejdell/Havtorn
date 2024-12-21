// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "Includes/FullscreenShaderStructs.hlsli"

PixelOutput main(VertexToPixel input)
{
    PixelOutput returnValue;
    
    const uint2 resource = fullscreenTexture6.Load(int3(input.UV.xy * Resolution, 0));
    returnValue.Color.rgb = float3(resource.x, resource.y, (resource.x + resource.y) * 0.5f);
    returnValue.Color.rg /= 1000000000; // Bias here
    returnValue.Color.a = 1;
    return returnValue;
};