// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "Includes/DecalShaderStructs.hlsli"

struct GBufferOutput
{
    float4 Albedo : SV_TARGET0;
    float4 Normal : SV_TARGET1;
    float4 Material : SV_TARGET3;
};

float4 PixelShader_Normal(float2 uv)
{
    float3 normal;
    normal.xy = normalTexture.Sample(defaultSampler, uv).ag;
    // Recreate z
    normal.z = 0.0f;
    normal = (normal * 2.0f) - 1.0f;
    normal.z = sqrt(1 - saturate((normal.x * normal.x) + (normal.y * normal.y)));
    normal = normalize(normal);
    
    float4 output;
    output.xyz = normal.xyz;
    output.a = 1.0f;
    return output;
}

GBufferOutput main(VertexToPixel input)
{	
    float3 clipSpace = input.ClipSpacePosition;
    clipSpace.y *= -1.0f;
    float2 screenSpaceUV = (clipSpace.xy / clipSpace.z) * 0.5f + 0.5f;
    
    float z = depthTexture.Sample(defaultSampler, screenSpaceUV).r;
    float x = screenSpaceUV.x * 2.0f - 1;
    float y = (1 - screenSpaceUV.y) * 2.0f - 1;
    float4 projectedPos = float4(x, y, z, 1.0f);
    float4 viewSpacePos = mul(ToCameraFromProjection, projectedPos);
    viewSpacePos /= viewSpacePos.w;
    float4 worldPosFromDepth = mul(ToWorldFromCamera, viewSpacePos);
    
    float4 objectPosition = mul(input.ObjectTransform, worldPosFromDepth);
    clip(0.5f - abs(objectPosition.xyz));
    float2 decalUV = objectPosition.xy + 0.5f;
    decalUV.y *= -1.0f;
    
    GBufferOutput returnValue;
    returnValue.Albedo = colorTexture.Sample(defaultSampler, decalUV).rgba;
	
    float3 normal = PixelShader_Normal(decalUV).xyz;
    float3x3 tangentSpaceMatrix = float3x3(normalize(input.Tangent.xyz), normalize(input.Bitangent.xyz), normalize(input.Normal.xyz));
    normal = mul(normal.xyz, tangentSpaceMatrix);
    normal = normalize(normal);
    
    returnValue.Normal = float4(normal.x, normal.y, normal.z, returnValue.Albedo.a);

    float3 material = materialTexture.Sample(defaultSampler, decalUV).rgb;
    float metalness = material.r;
    float perceptualRoughness = material.g;
    float emissive = material.b;
    float ambientOcclusion = normalTexture.Sample(defaultSampler, decalUV).b;
	
    returnValue.Material = float4(metalness, perceptualRoughness, emissive, returnValue.Albedo.a /*ambientOcclusion*/);
    
    return returnValue;
}