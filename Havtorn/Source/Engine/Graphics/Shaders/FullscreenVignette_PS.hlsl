// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Includes/FullscreenShaderStructs.hlsli"

PixelOutput main(VertexToPixel input)
{
    PixelOutput returnValue;
    float4 resource = fullscreenTexture1.Sample(defaultSampler, input.UV.xy).rgba;
    float4 vignetteTexture = fullscreenTexture2.Sample(defaultSampler, input.UV.xy).rgba;
     
    vignetteTexture.a *= VignetteStrength;
    resource = lerp(resource, vignetteTexture, vignetteTexture.a);
    
    returnValue.Color.rgb = resource.rgb;
    returnValue.Color.a = 1.0f;
    
    return returnValue;
}