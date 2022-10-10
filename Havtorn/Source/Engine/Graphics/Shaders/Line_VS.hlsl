// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "Includes/LineShaderStructs.hlsli"

LineVertexToPixel main(LineVertexInput input)
{
    float4 vertexObjectPos      = input.Position.xyzw;
    float4 vertexWorldPos       = mul(ToWorld, vertexObjectPos);
    float4 vertexViewPos        = mul(ToCameraSpace, vertexWorldPos);
    float4 vertexProjectionPos  = mul(ToProjectionSpace, vertexViewPos);

    LineVertexToPixel returnValue;
    returnValue.Position = vertexProjectionPos;
    returnValue.Color = Color;
    return returnValue;
}
