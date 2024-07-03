// Copyright 2023 Team Havtorn. All Rights Reserved.

struct VertexInput
{
    unsigned int Index : SV_VertexID;
};

struct InstancedVertexInput
{
    //unsigned int Index : SV_VertexID;
    float4x4 Transform : INSTANCETRANSFORM;
    float4 UVRect : INSTANCEUVRECT;
    float4 Color : INSTANCECOLOR;
};

struct VertexToGeometry
{
    float4 Color : COLOR;
    float4 UVRect : UVRECT;
    float2 Position : POSITION;
    float2 Size : SIZE;
    float Rotation : ROTATION;
};

struct InstancedVertexToGeometry
{
    float4x4 Transform : INSTANCETRANSFORM;
    float4 UVRect : INSTANCEUVRECT;
    float4 Color : INSTANCECOLOR;
};

struct GeometryToPixelScreenSpace
{
    float4 Position     : SV_POSITION;
    float4 Color        : COLOR;
    float2 UV           : UV;
};

struct GeometryToPixelWorldSpace
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float4 Normal : NORMAL;
    float4 Tangent : TANGENT;
    float4 Bitangent : BINORMAL;
    float2 UV : UV;
};

struct PixelOutput
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