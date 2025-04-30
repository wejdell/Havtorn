// Copyright 2025 Team Havtorn. All Rights Reserved.

struct VertexInput
{
    float3 Position     : POSITION;
    float3 Normal       : NORMAL;
    float3 Tangent      : TANGENT;
    float3 Bitangent    : BINORMAL;
    float2 UV           : UV;
    float4 BoneIDs      : BONEID;
    float4 BoneWeights  : BONEWEIGHT;
    uint2 AnimationData : INSTANCEANIMATIONDATA;
    float4x4 Transform  : INSTANCETRANSFORM;
};

struct VertexToPixel
{
    float4 Position     : SV_POSITION;
    float4 Normal       : NORMAL;
    float4 Tangent      : TANGENT;
    float4 Bitangent    : BINORMAL;
    float2 UV           : UV;
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
}

cbuffer BoneBuffer : register(b2)
{
    matrix Bones[64];
};

VertexToPixel main(VertexInput input)
{
    VertexToPixel returnValue;

    float4 weights = input.BoneWeights;
    uint4 boneIndices = uint4((uint) input.BoneIDs.x, (uint) input.BoneIDs.y, (uint) input.BoneIDs.z, (uint) input.BoneIDs.w);
    
    float4 skinnedPos = 0;
    const float4 pos = float4(input.Position.xyz, 1.0f);
    
    skinnedPos += weights.x * mul(Bones[boneIndices.x], pos);
    skinnedPos += weights.y * mul(Bones[boneIndices.y], pos);
    skinnedPos += weights.z * mul(Bones[boneIndices.z], pos);
    skinnedPos += weights.w * mul(Bones[boneIndices.w], pos);

    const float4 vertexWorldPos = mul(input.Transform, skinnedPos);
    const float4 vertexViewPos = mul(ToCameraSpace, vertexWorldPos);
    const float4 vertexProjectionPos = mul(ToProjectionSpace, vertexViewPos);

    const float3x3 toWorldRotation = (float3x3) ToWorld;
    float3 vertexWorldNormal = mul(toWorldRotation, input.Normal.xyz);
    float3 vertexWorldTangent = mul(toWorldRotation, input.Tangent.xyz);
    float3 vertexWorldBitangent = mul(toWorldRotation, input.Bitangent.xyz);

    returnValue.Position = vertexProjectionPos;
    returnValue.Normal = float4(vertexWorldNormal, 0);
    returnValue.Tangent = float4(vertexWorldTangent, 0);
    returnValue.Bitangent = float4(vertexWorldBitangent, 0);
    returnValue.UV = input.UV;
    return returnValue;
}