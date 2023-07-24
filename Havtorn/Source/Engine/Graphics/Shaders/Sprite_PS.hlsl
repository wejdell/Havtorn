// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "Includes/SpriteShaderStructs.hlsli"

PixelOutput main(GeometryToPixel input)
{
    PixelOutput output;
    float4 textureColor = spriteTexture.Sample(defaultSampler, input.UV.xy).rgba;
    output.Color.rgba = textureColor.rgba * input.Color.rgba;
    return output;
}