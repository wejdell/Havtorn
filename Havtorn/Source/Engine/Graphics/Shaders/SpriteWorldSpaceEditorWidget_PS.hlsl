// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "Includes/SpriteShaderStructs.hlsli"
#include "Includes/MathHelpers.hlsli"

struct PixelOutputEditorWidget
{
    float4 Color  : SV_TARGET0;
    uint2 Entity  : SV_TARGET1;
};

PixelOutputEditorWidget main(EditorGeometryToPixelWorldSpace input)
{   
    float4 albedo = spriteTexture.Sample(defaultSampler, input.UV.xy).rgba;
    albedo *= input.Color;
    clip(albedo.a - 0.25f);
   
    PixelOutputEditorWidget output;
    output.Color = albedo;
    output.Entity = input.Entity;
    
    return output;
}