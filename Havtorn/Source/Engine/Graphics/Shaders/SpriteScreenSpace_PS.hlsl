// Copyright 2022 Team Havtorn. All Rights Reserved.

// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "Includes/SpriteShaderStructs.hlsli"
#include "Includes/MathHelpers.hlsli"

PixelOutput main(GeometryToPixelScreenSpace input)
{
    PixelOutput output;
    float4 textureColor = spriteTexture.Sample(defaultSampler, input.UV.xy).rgba;
    output.Color.rgb = GammaToLinear(textureColor.rgb) * GammaToLinear(input.Color.rgb);
    output.Color.a = textureColor.a * input.Color.a;
    return output;
}