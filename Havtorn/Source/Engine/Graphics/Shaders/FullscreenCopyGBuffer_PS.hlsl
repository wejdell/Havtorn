// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Includes/FullscreenShaderStructs.hlsli"

GBufferOutput main(VertexToPixel input)
{
    GBufferOutput gbuffer;
    const float3 albedo       = fullscreenTexture2.Sample(defaultSampler, input.UV.xy).rgb;
    const float3 normal       = fullscreenTexture3.Sample(defaultSampler, input.UV.xy).rgb;
    const float3 vertexNormal = fullscreenTexture4.Sample(defaultSampler, input.UV.xy).rgb;
    const float4 material     = fullscreenTexture5.Sample(defaultSampler, input.UV.xy).rgba;
    gbuffer.Albedo = albedo;
    gbuffer.Normal = normal;
    gbuffer.VertexNormal = vertexNormal;
    gbuffer.MetalRoughAOEm = material;
    return gbuffer;
}