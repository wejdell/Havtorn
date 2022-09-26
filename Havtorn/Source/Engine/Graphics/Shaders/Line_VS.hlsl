// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Includes/LineShaderStructs.hlsli"

cbuffer FrameBuffer : register(b0)
{
    float4x4 ToCameraSpace;
    float4x4 ToWorldFromCamera;
    float4x4 ToProjectionSpace;
    float4x4 ToCameraFromProjection;
    float4 CameraPosition;
}
cbuffer ObjectBuffer : register(b1)
{
    float4x4 ToWorld;
    //Add color here later
}

LineVertexToPixel main(LineVertexInput input)
{
    float4 vertexObjectPos      = input.Position.xyzw;
    float4 vertexWorldPos       = mul(ToWorld, vertexObjectPos);
    float4 vertexViewPos        = mul(ToCameraSpace, vertexWorldPos);
    float4 vertexProjectionPos  = mul(ToProjectionSpace, vertexViewPos);

    LineVertexToPixel returnValue;
    returnValue.Position = vertexProjectionPos;
    returnValue.Color = float4(0.5, 0.9, 0.7, 1);
    return returnValue;
}