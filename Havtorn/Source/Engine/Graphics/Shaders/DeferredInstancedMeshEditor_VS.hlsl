// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "Includes/DeferredShaderStructs.hlsli"

VertexModelToPixelEditor main(StaticInstancedMeshEditorVertexInput input)
{
    VertexModelToPixelEditor returnValue;

    const float4 vertexObjectPos      = float4(input.Position.xyz, 1.0f);
    const float4 vertexWorldPos       = mul(input.Transform, vertexObjectPos);
    const float4 vertexViewPos        = mul(ToCameraSpace, vertexWorldPos);
    const float4 vertexProjectionPos  = mul(ToProjectionSpace, vertexViewPos);

    const float3x3 toWorldRotation = (float3x3) input.Transform;
    float3 vertexWorldNormal    = mul(toWorldRotation, input.Normal.xyz);
    float3 vertexWorldTangent   = mul(toWorldRotation, input.Tangent.xyz);
    float3 vertexWorldBinormal = mul(toWorldRotation, input.Bitangent.xyz);

    returnValue.Position      = vertexProjectionPos;
    returnValue.WorldPosition = vertexWorldPos;
    returnValue.Normal        = float4(vertexWorldNormal, 0);
    returnValue.Tangent       = float4(vertexWorldTangent, 0);
    returnValue.Bitangent      = float4(vertexWorldBinormal, 0);
    returnValue.UV            = input.UV;
    returnValue.Entity        = input.Entity;
    return returnValue;
}