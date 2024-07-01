// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "Includes/SpriteShaderStructs.hlsli"
#include "Includes/MathHelpers.hlsli"

struct GBufferOutput
{
    float4 Albedo           : SV_TARGET0;
    float4 Normal           : SV_TARGET1;
    float4 VertexNormal     : SV_TARGET2;
    float4 MetalRoughEmAO   : SV_TARGET3;
};

GBufferOutput main(GeometryToPixelWorldSpace input)
{    
    // NR: Experimental lighting of sprites
    // NR: Could probably use implementation of GBuffer_PS
    
    float4 albedo = spriteTexture.Sample(defaultSampler, input.UV.xy).rgba;
    albedo *= input.Color;
    clip(albedo.a - 0.25f);
    
    float3x3 tangentSpaceMatrix = float3x3(normalize(input.Tangent.xyz), normalize(input.Bitangent.xyz), normalize(input.Normal.xyz));
    float3 normal = mul(input.Normal.xyz, tangentSpaceMatrix);
    normal = normalize(normal);
    
    // TODO.NR: Add implementation here
    float metalness = 0.0f;
    float perceptualRoughness = 0.0f;
    float emissive = 0.0f;
    float ambientOcclusion = 0.0f;
    
    GBufferOutput output;
    output.Albedo = albedo;
    output.Normal = float4(normal.xyz, 1.0f); // Should this W be 1?
    output.VertexNormal = float4(input.Normal.xyz, 1.0f);
    output.MetalRoughEmAO = float4(metalness, perceptualRoughness, emissive, ambientOcclusion);
    
    return output;
}