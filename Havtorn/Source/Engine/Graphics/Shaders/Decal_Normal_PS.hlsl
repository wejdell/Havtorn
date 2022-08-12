// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Includes/DecalShaderStructs.hlsli"

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

float4 main(VertexToPixel input) : SV_TARGET1
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
    
    float4 objectPosition = mul(toObjectSpace, worldPosFromDepth);
    clip(0.5f - abs(objectPosition.xyz));
    float2 decalUV = objectPosition.xy + 0.5f;
    decalUV.y *= -1.0f;

    float3 normal = PixelShader_Normal(decalUV).xyz;
    float3x3 tangentSpaceMatrix = float3x3(normalize(input.Tangent.xyz), normalize(input.Bitangent.xyz), normalize(input.Normal.xyz));
    normal = mul(normal.xyz, tangentSpaceMatrix);
    normal = normalize(normal);

    float alpha = colorTexture.Sample(defaultSampler, decalUV).a;
    
    return float4(normal.x, normal.y, normal.z, alpha);
}