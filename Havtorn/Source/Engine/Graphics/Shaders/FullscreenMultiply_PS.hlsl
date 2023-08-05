// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Includes/FullscreenShaderStructs.hlsli"

PixelOutput main(VertexToPixel input)
{
    PixelOutput returnValue;
    const float3 resource = fullscreenTexture1.Sample(defaultSampler, input.UV.xy).rgb;
    const float3 resource2 = fullscreenTexture2.Sample(defaultSampler, input.UV.xy).rgb;
	
	returnValue.Color.rgb = resource * resource2;
	
    returnValue.Color.a = 0.0f;
    return returnValue;
};