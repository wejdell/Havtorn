// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "Includes/FullscreenShaderStructs.hlsli"

PixelOutput main(VertexToPixel input)
{
    PixelOutput returnValue;
    
    const float4 resource = worldPositionTexture.Sample(defaultSampler, input.UV.xy).rgba;
    returnValue.Color.rgb = resource.rgb;
    returnValue.Color.rgb = abs(resource.rgb); // Bias here
    returnValue.Color.rgb *= 1; // Bias here
    returnValue.Color.a = 1;
    return returnValue;
};