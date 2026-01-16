// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "Includes/DeferredSamplingFunctions.hlsli"

struct GBufferOutputEditor
{
    float4 Albedo             : SV_TARGET0;
    float4 Normal             : SV_TARGET1;
    float4 VertexNormal       : SV_TARGET2;
    float4 MetalRoughEmAO     : SV_TARGET3;
    float4 WorldPosition      : SV_TARGET4;
    uint2 Entity              : SV_TARGET5;
};

GBufferOutputEditor main(VertexModelToPixelEditor input)
{
    VertexToPixel vertToPixel;
    vertToPixel.Position  = input.Position;
    vertToPixel.UV        = input.UV;   
    
    float albedoA = DetermineProperty(MaterialProperties[ALBEDO_A], vertToPixel.UV);
    clip(albedoA - 0.5f);
    
    float albedoR = DetermineProperty(MaterialProperties[ALBEDO_R], vertToPixel.UV);
    float albedoG = DetermineProperty(MaterialProperties[ALBEDO_G], vertToPixel.UV);
    float albedoB = DetermineProperty(MaterialProperties[ALBEDO_B], vertToPixel.UV);
    float3 albedo = float3(albedoR, albedoG, albedoB);
    
    float normalX = DetermineProperty(MaterialProperties[NORMAL_X], vertToPixel.UV);
    float normalY = DetermineProperty(MaterialProperties[NORMAL_Y], vertToPixel.UV);
    float3 normal = float3(normalX, normalY, 0.0f);
    
    if (RecreateNormalZ)
        RecreateZ(normal);
    else
        normal.z = DetermineProperty(MaterialProperties[NORMAL_Z], vertToPixel.UV);
    normal = normalize(normal);
    
    float ambientOcclusion = DetermineProperty(MaterialProperties[AMBIENT_OCCLUSION], vertToPixel.UV);
    float metalness = DetermineProperty(MaterialProperties[METALNESS], vertToPixel.UV);
    float perceptualRoughness = DetermineProperty(MaterialProperties[ROUGHNESS], vertToPixel.UV);
    float emissive = DetermineProperty(MaterialProperties[EMISSIVE], vertToPixel.UV);
    
    float3x3 tangentSpaceMatrix = float3x3(normalize(input.Tangent.xyz), normalize(input.Bitangent.xyz), normalize(input.Normal.xyz));
    normal = mul(normal.xyz, tangentSpaceMatrix);
    normal = normalize(normal);
    
    GBufferOutputEditor output;    
    output.Albedo = float4(albedo.xyz, 1.0f);
    output.Normal = float4(normal.xyz, 1.0f);
    output.VertexNormal = float4(input.Normal.xyz, 1.0f);
    output.MetalRoughEmAO = float4(metalness, perceptualRoughness, emissive, ambientOcclusion);
    output.WorldPosition = PixelShader_WorldPosition(vertToPixel.UV);
    output.Entity = input.Entity;
    
    return output;
}