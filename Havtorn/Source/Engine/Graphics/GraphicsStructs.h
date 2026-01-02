// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "GraphicsEnums.h"
#include "HavtornString.h"

namespace Havtorn
{
	struct SPositionVertex
	{
		F32 x, y, z, w;// Remove W, always 1 as is position
	};

	struct SStaticMeshVertex
	{
		// Position
		F32 x, y, z;

		// Normal
		F32 nx, ny, nz;

		// Tangent
		F32 tx, ty, tz;

		// Bitangent
		F32 bx, by, bz;

		// UV
		F32 u, v;
	};

	struct SSkeletalMeshVertex
	{
		// Position
		F32 x, y, z;

		// Normal
		F32 nx, ny, nz;

		// Tangent
		F32 tx, ty, tz;

		// Bitangent
		F32 bx, by, bz;

		// UV
		F32 u, v;

		// NR: Max four bones per vertex

		// Bone ID
		F32 bix, biy, biz, biw;

		// Bone Weight
		F32 bwx, bwy, bwz, bww;
	};

	struct SMeshData
	{
		std::vector<SStaticMeshVertex> Vertices;
		std::vector<U32> Indices;
	};

	struct SPrimitive
	{
		std::vector<SPositionVertex> Vertices;
		std::vector<U32> Indices;
	};

	struct SStaticMesh
	{
		std::string Name;
		std::vector<SStaticMeshVertex> Vertices;
		std::vector<U32> Indices;
		U16 MaterialIndex = 0;
	};

	struct SSkeletalMesh
	{
		std::string Name;
		std::vector<SSkeletalMeshVertex> Vertices;
		std::vector<U32> Indices;
		U16 MaterialIndex = 0;
	};

	struct SSkeletalMeshBone
	{
		CHavtornStaticString<255> Name;
		SMatrix InverseBindPoseTransform = SMatrix::Identity;
		I32 ParentIndex = -1;
	};

	struct SSkeletalMeshNode
	{
		CHavtornStaticString<255> Name;
		SMatrix NodeTransform = SMatrix::Identity;
		std::vector<U32> ChildIndices;
	};

	struct SVecBoneAnimationKey
	{
		SVector Value = SVector::Zero;
		F32 Time = 0.0f;
	};

	struct SQuatBoneAnimationKey
	{
		SQuaternion Value = SQuaternion::Identity;
		F32 Time = 0.0f;
	};

	struct SBoneAnimationKey
	{
		SMatrix Transform = SMatrix::Identity;
		F32 Time = 0.0f;
	};

	struct SBoneAnimationTrack
	{
		CHavtornStaticString<255> TrackName;
		std::vector<SVecBoneAnimationKey> TranslationKeys;
		std::vector<SQuatBoneAnimationKey> RotationKeys;
		std::vector<SVecBoneAnimationKey> ScaleKeys;

		U32 GetSize() const
		{
			U32 size = 0;
			size += GetDataSize(TranslationKeys);
			size += GetDataSize(RotationKeys);
			size += GetDataSize(ScaleKeys);
			size += GetDataSize(TrackName);
			return size;
		}
	};

	struct SSkeletalPosedNode
	{
		SMatrix LocalTransform;
		SMatrix GlobalTransform;

		// TODO.NW: Probably want to just hash the name assuming this doesn't need to be serialized. Harder to debug though
		CHavtornStaticString<255> Name;
	};

	struct SBoneAnimationClip
	{
		std::vector<SBoneAnimationTrack> Tracks;
		std::string AssetName;
		U32 DurationInTicks = 0;
		U32 TickRate = 0;

		U32 GetSize() const
		{
			U32 size = 0;
			for (U32 i = 0; i < Tracks.size(); i++)
				size += Tracks[i].GetSize();

			size += GetDataSize(AssetName);
			size += GetDataSize(DurationInTicks);
			size += GetDataSize(TickRate);
		}
	};

	struct SBoneAnimDataTransform
	{
		SVector4 Row1TX;
		SVector4 Row2TY;
		SVector4 Row3TZ;
		SVector4 Padding;
	};

	struct SDrawCallData
	{
		U32 IndexCount = 0;
		U16 VertexBufferIndex = 0;
		U16 IndexBufferIndex = 0;
		U16 VertexStrideIndex = 0;
		U16 VertexOffsetIndex = 0;
		U16 MaterialIndex = 0;
	};

	struct SShadowmapViewData
	{
		SMatrix ShadowViewMatrix = SMatrix::Identity;
		SMatrix ShadowProjectionMatrix = SMatrix::Identity;

		// TODO.NR: Replace with transform translation
		SVector4 ShadowPosition = SVector4::Zero;

		U16 ShadowmapViewportIndex = 0;
	};

	struct SRuntimeGraphicsMaterialProperty
	{
		F32 ConstantValue = -1.0f;
		F32 TextureIndex = -1.0f;
		F32 TextureChannelIndex = -1.0f;
		U32 TextureUID = 0;
	};

	struct SOfflineGraphicsMaterialProperty
	{
		F32 ConstantValue = -1.0f;
		std::string TexturePath;
		I16 TextureChannelIndex = -1;

		U32 GetSize() const
		{
			U32 size = 0;
			size += GetDataSize(ConstantValue);
			size += GetDataSize(TexturePath);
			size += GetDataSize(TextureChannelIndex);
			return size;
		}
	};

	struct SOfflineGraphicsMaterial
	{
		SOfflineGraphicsMaterialProperty Properties[STATIC_U8(EMaterialProperty::Count)];
		bool RecreateZ = true;

		U32 GetSize() const
		{
			U32 size = 0;
			for (auto& property : Properties)
			{
				size += property.GetSize();
			}
			size += GetDataSize(RecreateZ);
			return size;
		}
	};
}