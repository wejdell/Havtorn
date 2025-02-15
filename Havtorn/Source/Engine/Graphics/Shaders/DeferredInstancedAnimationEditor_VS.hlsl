// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "Includes/DeferredShaderStructs.hlsli"

static matrix IdentityMatrix =
{
    { 1, 0, 0, 0 },
    { 0, 1, 0, 0 },
    { 0, 0, 1, 0 },
    { 0, 0, 0, 1 }
};

#define ANIMATION_TEXTURE_WIDTH 256

matrix DecodeMatrixFromTextureRows(float3x4 rows)
{
    matrix returnMatrix = IdentityMatrix;
    returnMatrix._11_12_13 = rows._11_12_13;
    returnMatrix._21_22_23 = rows._21_22_23;
    returnMatrix._31_32_33 = rows._31_32_33;
    returnMatrix._41_42_43 = rows._14_24_34;
    return returnMatrix;
}

// Read a matrix (3 texture reads) from a texture containing
// animation data.
matrix LoadBoneMatrix(uint2 animationData, float boneIndex)
{
    matrix boneMatrix = IdentityMatrix;
  // If this texture were 1D, what would be the offset?
    uint baseIndex = animationData.x + (animationData.y * 56);
  // We use 4 * bone because each bone is 4 texels to form a float4x4.
    baseIndex += (4 * boneIndex);
    //baseIndex += (boneIndex);
  // Now turn that into 2D coords
    uint baseU = baseIndex % ANIMATION_TEXTURE_WIDTH;
    uint baseV = baseIndex / ANIMATION_TEXTURE_WIDTH;
  // Note that we assume the width of the texture
  // is an even multiple of the number of texels per bone;
  // otherwise we'd have to recalculate the V component per lookup.
    float4 mat1 = MeshAnimationsTexture.Load(uint3(baseU, baseV, 0));
    float4 mat2 = MeshAnimationsTexture.Load(uint3(baseU + 1, baseV, 0));
    float4 mat3 = MeshAnimationsTexture.Load(uint3(baseU + 2, baseV, 0));
  // Only load 3 of the 4 values, and decode the matrix from them.
    boneMatrix = DecodeMatrixFromTextureRows(float3x4(mat1, mat2, mat3));
    return boneMatrix;
}

VertexModelToPixelEditor main(SkeletalMeshInstancedEditorVertexInput input)
{
    VertexModelToPixelEditor returnValue;

    float4 weights = input.BoneWeights;
    uint4 boneIndices = uint4((uint) input.BoneIDs.x, (uint) input.BoneIDs.y, (uint) input.BoneIDs.z, (uint) input.BoneIDs.w);
    
    float4 skinnedPos = 0;
    const float4 pos = float4(input.Position.xyz, 1.0f);
    
    skinnedPos += weights.x * mul(LoadBoneMatrix(input.AnimationData, boneIndices.x), pos);
    
    // NR: Branching may be cheaper than texture fetching
    if (weights.y > 0)
    {
        skinnedPos += weights.y * mul(LoadBoneMatrix(input.AnimationData, boneIndices.y), pos);
        if (weights.z > 0)
        {
            skinnedPos += weights.z * mul(LoadBoneMatrix(input.AnimationData, boneIndices.z), pos);
            if (weights.w > 0)
            {
                skinnedPos += weights.w * mul(LoadBoneMatrix(input.AnimationData, boneIndices.w), pos);
            }
        }
    }
    
    //skinnedPos = pos;
    input.Position.x = skinnedPos.x;
    input.Position.y = skinnedPos.y;
    input.Position.z = skinnedPos.z;

    const float4 vertexObjectPos = float4(input.Position.xyz, 1.0f);
    const float4 vertexWorldPos = mul(input.Transform, vertexObjectPos);
    const float4 vertexViewPos = mul(ToCameraSpace, vertexWorldPos);
    const float4 vertexProjectionPos = mul(ToProjectionSpace, vertexViewPos);

    const float3x3 toWorldRotation = (float3x3)input.Transform;
    float3 vertexWorldNormal = mul(toWorldRotation, input.Normal.xyz);
    float3 vertexWorldTangent = mul(toWorldRotation, input.Tangent.xyz);
    float3 vertexWorldBinormal = mul(toWorldRotation, input.Bitangent.xyz);

    returnValue.Position = vertexProjectionPos;
    returnValue.WorldPosition = vertexWorldPos;
    returnValue.Normal = float4(vertexWorldNormal, 0);
    returnValue.Tangent = float4(vertexWorldTangent, 0);
    returnValue.Bitangent = float4(vertexWorldBinormal, 0);
    returnValue.UV = input.UV;
    returnValue.Entity = input.Entity;
    return returnValue;
}