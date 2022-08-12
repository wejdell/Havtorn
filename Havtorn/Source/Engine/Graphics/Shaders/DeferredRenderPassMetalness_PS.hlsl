// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Includes/DeferredSamplingFunctions.hlsli"

PixelOutput main(VertexToPixel input)
{
    PixelOutput output;

    const float metalness = GBuffer_Metalness(input.UV);
    output.Color.rgb = metalness;
    output.Color.a = 1.0f;
    return output;
}