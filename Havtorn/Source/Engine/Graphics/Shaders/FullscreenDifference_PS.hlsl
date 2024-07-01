// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Includes/FullscreenShaderStructs.hlsli"

PixelOutput main(VertexToPixel input) 
{
	PixelOutput returnValue;
	const float3 resource1 = fullscreenTexture1.Sample(defaultSampler, input.UV.xy).rgb;
    const float3 resource2 = fullscreenTexture2.Sample(defaultSampler, input.UV.xy).rgb;
    returnValue.Color.rgb = saturate(resource1 - resource2);
	returnValue.Color.a = 1.0f;
	return returnValue;
};