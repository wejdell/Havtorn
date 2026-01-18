// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "Includes/DeferredShaderStructs.hlsli"

//static matrix IdentityMatrix =
//{
//    { 1, 0, 0, 0 },
//    { 0, 1, 0, 0 },
//    { 0, 0, 1, 0 },
//    { 0, 0, 0, 1 }
//};

//#define ANIMATION_TEXTURE_WIDTH 256

//matrix DecodeMatrixFromTextureRows(float4 row0, float4 row1, float4 row2)
//{
//    matrix returnMatrix = IdentityMatrix;
//    returnMatrix._11_12_13_14 = float4(row0.x, row0.y, row0.z, 0.f);
//    returnMatrix._21_22_23_24 = float4(row1.x, row1.y, row1.z, 0.f);
//    returnMatrix._31_32_33_34 = float4(row2.x, row2.y, row2.z, 0.f);
//    returnMatrix._41_42_43_44 = float4(row0.w, row1.w, row2.w, 1.f);
//    return returnMatrix;
//}

//// Read a matrix (3 texture reads) from a texture containing
//// animation data.
//matrix LoadBoneMatrix(uint2 animationData, float boneIndex)
//{
//    matrix boneMatrix = IdentityMatrix;
//  // If this texture were 1D, what would be the offset? y is frame index = 4 texels / bone and *35* bones per frame
//    uint baseIndex = animationData.x + (animationData.y * 2 * 4);
//  // We use 4 * bone because each bone is 4 texels to form a float4x4.
//    baseIndex += (4 * boneIndex);
//  // Now turn that into 2D coords
//    uint baseU = baseIndex % ANIMATION_TEXTURE_WIDTH;
//    uint baseV = baseIndex / ANIMATION_TEXTURE_WIDTH;
//  // Note that we assume the width of the texture
//  // is an even multiple of the number of texels per bone;
//  // otherwise we'd have to recalculate the V component per lookup.
//    float4 mat1 = MeshAnimationsTexture.Load(uint3(baseU, baseV, 0));
//    float4 mat2 = MeshAnimationsTexture.Load(uint3(baseU + 1, baseV, 0));
//    float4 mat3 = MeshAnimationsTexture.Load(uint3(baseU + 2, baseV, 0));
//  // Only load 3 of the 4 values, and decode the matrix from them.
//    boneMatrix = DecodeMatrixFromTextureRows(mat1, mat2, mat3);
//    return boneMatrix;
//}

//VertexModelToPixelEditor main(SkeletalMeshInstancedEditorVertexInput input)
//{
//    VertexModelToPixelEditor returnValue;

//    float4 weights = input.BoneWeights;
//    uint4 boneIndices = uint4((uint) input.BoneIDs.x, (uint) input.BoneIDs.y, (uint) input.BoneIDs.z, (uint) input.BoneIDs.w);
    
//    float4 skinnedPos = 0;
//    const float4 pos = float4(input.Position.xyz, 1.0f);
    
//    skinnedPos += weights.x * mul(LoadBoneMatrix(input.AnimationData, boneIndices.x), pos);
    
//    // NR: Branching may be cheaper than texture fetching
//    if (weights.y > 0)
//    {
//        skinnedPos += weights.y * mul(LoadBoneMatrix(input.AnimationData, boneIndices.y), pos);
//        if (weights.z > 0)
//        {
//            skinnedPos += weights.z * mul(LoadBoneMatrix(input.AnimationData, boneIndices.z), pos);
//            if (weights.w > 0)
//            {
//                skinnedPos += weights.w * mul(LoadBoneMatrix(input.AnimationData, boneIndices.w), pos);
//            }
//        }
//    }
    
//    const float4 vertexWorldPos = mul(input.Transform, skinnedPos);
//    const float4 vertexViewPos = mul(ToCameraSpace, vertexWorldPos);
//    const float4 vertexProjectionPos = mul(ToProjectionSpace, vertexViewPos);

//    const float3x3 toWorldRotation = (float3x3)input.Transform;
//    float3 vertexWorldNormal = mul(toWorldRotation, input.Normal.xyz);
//    float3 vertexWorldTangent = mul(toWorldRotation, input.Tangent.xyz);
//    float3 vertexWorldBinormal = mul(toWorldRotation, input.Bitangent.xyz);

//    returnValue.Position = vertexProjectionPos;
//    returnValue.WorldPosition = vertexWorldPos;
//    returnValue.Normal = float4(vertexWorldNormal, 0);
//    returnValue.Tangent = float4(vertexWorldTangent, 0);
//    returnValue.Bitangent = float4(vertexWorldBinormal, 0);
//    returnValue.UV = input.UV;
//    returnValue.Entity = input.Entity;
//    return returnValue;
//}

VertexModelToPixelEditor main(SkeletalMeshInstancedEditorVertexInput input)
{
    VertexModelToPixelEditor returnValue;

    float4 weights = input.BoneWeights;
    uint4 boneIndices = uint4((uint) input.BoneIDs.x, (uint) input.BoneIDs.y, (uint) input.BoneIDs.z, (uint) input.BoneIDs.w);
    
    float4 skinnedPos = 0;
    const float4 pos = float4(input.Position.xyz, 1.0f );
    
    //skinnedPos += weights.x * mul(pos, Bones[boneIndices.x]);
    //skinnedPos += weights.y * mul(pos, Bones[boneIndices.y]);
    //skinnedPos += weights.z * mul(pos, Bones[boneIndices.z]);
    //skinnedPos += weights.w * mul(pos, Bones[boneIndices.w]);
    skinnedPos += weights.x * mul(Bones[boneIndices.x], pos);
    skinnedPos += weights.y * mul(Bones[boneIndices.y], pos);
    skinnedPos += weights.z * mul(Bones[boneIndices.z], pos);
    skinnedPos += weights.w * mul(Bones[boneIndices.w], pos);
    
    float4 vertexWorldPos = mul(input.Transform, skinnedPos);
    const float4 vertexViewPos = mul(ToCameraSpace, vertexWorldPos);
    const float4 vertexProjectionPos = mul(ToProjectionSpace, vertexViewPos);

    const float3x3 toWorldRotation = (float3x3) input.Transform;
    float3 vertexWorldNormal = mul(toWorldRotation, input.Normal.xyz);
    float3 vertexWorldTangent = mul(toWorldRotation, input.Tangent.xyz);
    float3 vertexWorldBinormal = mul(toWorldRotation, input.Bitangent.xyz);

    returnValue.Position = vertexProjectionPos;
    returnValue.WorldPosition = vertexWorldPos / vertexWorldPos.w;
    returnValue.Normal = float4(vertexWorldNormal, 0);
    returnValue.Tangent = float4(vertexWorldTangent, 0);
    returnValue.Bitangent = float4(vertexWorldBinormal, 0);
    returnValue.UV = input.UV;
    returnValue.Entity = input.Entity;
    return returnValue;
}