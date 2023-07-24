// Copyright 2023 Team Havtorn. All Rights Reserved.

struct VertexInput
{
    unsigned int Index : SV_VertexID;
};

struct VertexToGeometry
{
    float4 Color : COLOR;
    float4 UVRect : UVRECT;
    float2 Position : POSITION;
    float2 Size : SIZE;
    float Rotation : ROTATION;
};

struct GeometryToPixel
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 UV : UV;
};

struct PixelOutput
{
    float4 Color : SV_TARGET;
};

cbuffer ObjectBuffer : register(b0)
{
    float4 Color;
    float4 UVRect;
    float2 Position;
    float2 Size;
    float Rotation;
    float3 Padding;
};

Texture2D spriteTexture : register(t0);
SamplerState defaultSampler : register(s0);