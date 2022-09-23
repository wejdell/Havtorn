// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Includes/LineShaderStructs.hlsli"

cbuffer FrameBuffer : register(b0)
{
    float4x4 ToCamera;
    float4x4 ToProjection;
}
cbuffer ObjectBuffer : register(b1)
{
    float4x4 ToWorld;
}

LineVertexToPixel main(LineVertexInput input)
{
    float4 vertexObjectPos      = input.Position.xyzw;
    float4 vertexWorldPos       = mul(ToWorld, vertexObjectPos);
    float4 vertexViewPos        = mul(ToCamera, vertexWorldPos);
    float4 vertexProjectionPos  = mul(ToProjection, vertexViewPos);

    LineVertexToPixel returnValue;
    returnValue.Position = vertexProjectionPos;
    returnValue.Color = input.Color;
    return returnValue;
}