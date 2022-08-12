// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Includes/DecalShaderStructs.hlsli"

VertexToPixel main(VertexInput input)
{
    VertexToPixel returnValue;
    
    float4 vertexObjectPos = float4(input.Position.x, input.Position.y, input.Position.z, 1.0f);
    float4 vertexWorldPos = mul(toWorld, vertexObjectPos);
    float4 vertexViewPos = mul(ToCameraSpace, vertexWorldPos);
    float4 vertexProjectionPos = mul(ToProjectionSpace, vertexViewPos);
    
    float3x3 toWorldRotation = (float3x3) toWorld;
    float3 vertexWorldNormal = mul(toWorldRotation, input.Normal);
    float3 vertexWorldTangent = mul(toWorldRotation, input.Tangent);
    float3 vertexWorldBitangent = mul(toWorldRotation, input.Bitangent);
    
    returnValue.Position = vertexProjectionPos;
    returnValue.ClipSpacePosition = vertexProjectionPos.xyw;
    returnValue.Normal = float4(vertexWorldNormal, 0);
    returnValue.Tangent = float4(vertexWorldTangent, 0);
    returnValue.Bitangent = float4(vertexWorldBitangent, 0);

    returnValue.UV = input.UV;
     
    return returnValue;
}