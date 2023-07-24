// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "GraphicsEnums.h"

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
		U32 NameLength = 0;
		std::string Name;
		U32 NumberOfVertices = 0;
		std::vector<SStaticMeshVertex> Vertices;
		U32 NumberOfIndices = 0;
		std::vector<U32> Indices;
	};

	struct SDrawCallData
	{
		U32 IndexCount = 0;
		U16 VertexBufferIndex = 0;
		U16 IndexBufferIndex = 0;
		U16 VertexStrideIndex = 0;
		U16 VertexOffsetIndex = 0;
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
		F32 TextureIndex = 0.0f;
		F32 TextureChannelIndex = -1.0f;
		F32 Padding = 0.0f;
	};

	struct SEngineGraphicsMaterialProperty
	{
		F32 ConstantValue = -1.0f;
		U16 TextureIndex = 0;
		I16 TextureChannelIndex = -1;
	};

	struct SOfflineGraphicsMaterialProperty
	{
		F32 ConstantValue = -1.0f;
		U32 TexturePathLength = 0;
		std::string TexturePath;
		I16 TextureChannelIndex = -1;

		U32 GetSize() const
		{
			U32 size = 0;
			size += sizeof(F32);
			size += sizeof(U32);
			size += sizeof(char) * TexturePathLength;
			size += sizeof(I16);
			return size;
		}
	};

	struct SOfflineGraphicsMaterial
	{
		SOfflineGraphicsMaterialProperty Properties[static_cast<U8>(EMaterialProperty::Count)];
		bool RecreateZ = true;

		U32 GetSize() const
		{
			U32 size = 0;
			for (auto& property : Properties)
			{
				size += property.GetSize();
			}
			size += sizeof(bool);
			return size;
		}
	};
}