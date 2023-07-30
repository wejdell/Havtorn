// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Includes/FullscreenShaderStructs.hlsli"

PixelOutput main(VertexToPixel input)
{
    PixelOutput returnValue;
    float4 resource = fullscreenTexture1.Sample(defaultSampler, input.UV.xy).rgba;
    float4 vignetteTexture = fullscreenTexture2.Sample(defaultSampler, input.UV.xy).rgba;
    
    //float2 uv = input.myUV.xy;
    //uv -= 0.5f;
    //uv *= 2.0f;
    //float vignetteStrength = 1.0f / 9.0f; // 1/4 to 1/16
    //float vignette = (uv.x * uv.x + uv.y * uv.y) * myVignetteStrength;
    
    //vignette = cos(vignette * (3.14159265f * 2.0f));
    
    //resource.x *= clamp(vignette + myVignetteColor.r, 0.0f, 1.0f);
    //resource.y *= clamp(vignette + myVignetteColor.g, 0.0f, 1.0f);
    //resource.z *= clamp(vignette + myVignetteColor.b, 0.0f, 1.0f);
    
    vignetteTexture.a *= VignetteStrength;
    
    //resource = lerp(vignetteTexture, resource, saturate(vignette));
    resource = lerp(resource, vignetteTexture, vignetteTexture.a);
    
    returnValue.Color.rgb = resource.rgb;
    returnValue.Color.a = /*resource.a*/1.0f;
    
    return returnValue;
}