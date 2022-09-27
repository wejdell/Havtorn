// Copyright 2022 Team Havtorn. All Rights Reserved.

struct LineVertexInput
{
    float4 Position   : POSITION;
};

struct LineVertexToPixel
{
    float4 Position   : SV_POSITION;
    float4 Color      : COLOR;
};

struct LinePixelOutput
{
    float4 Color : SV_TARGET;
};

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
    float4 Color;
}