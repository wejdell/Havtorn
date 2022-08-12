// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Includes/DeferredSamplingFunctions.hlsli"

PixelOutput main(VertexToPixel input)
{
    PixelOutput output;

    const float3 normal = GBuffer_Normal(input.UV).xyz;
    output.Color.rgb = normal * 0.5f + 0.5f;
    output.Color.a = 1.0f;
    return output;
}