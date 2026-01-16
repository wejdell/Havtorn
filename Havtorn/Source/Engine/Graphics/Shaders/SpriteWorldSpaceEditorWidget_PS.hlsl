// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "Includes/SpriteShaderStructs.hlsli"
#include "Includes/MathHelpers.hlsli"

Texture2D depthTexture : register(t21);

float4 PixelShader_WorldPosition(float2 uv)
{
    // Depth sampling
    float z = depthTexture.Sample(defaultSampler, uv).r;
    float x = uv.x * 2.0f - 1;
    float y = (1 - uv.y) * 2.0f - 1;
    const float4 projectedPos = float4(x, y, z, 1.0f);
    float4 viewSpacePos = mul(ToCameraFromProjection, projectedPos);
    viewSpacePos /= viewSpacePos.w;
    float4 worldPos = mul(ToWorldFromCamera, viewSpacePos);

    worldPos.a = 1.0f;
    return worldPos;
}

struct PixelOutputEditorWidget
{
    float4 Color  : SV_TARGET0;
    float4 WorldPosition : SV_TARGET1;
    uint2 Entity  : SV_TARGET2;
};

PixelOutputEditorWidget main(EditorGeometryToPixelWorldSpace input)
{   
    float4 albedo = spriteTexture.Sample(defaultSampler, input.UV.xy).rgba;
    albedo *= input.Color;
    clip(albedo.a - 0.25f);
   
    PixelOutputEditorWidget output;
    output.Color = albedo;
    output.Entity = input.Entity;
    output.WorldPosition = PixelShader_WorldPosition(input.UV);
    
    return output;
}