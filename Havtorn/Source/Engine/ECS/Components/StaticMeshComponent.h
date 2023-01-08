// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "Graphics/GraphicsStructs.h"

namespace Havtorn
{
	struct SStaticMeshComponent : public SComponent
	{
		SStaticMeshComponent()
			: SComponent(EComponentType::StaticMeshComponent)
		{}

		U32 Serialize(char* toData, U32& bufferPosition);
		U32 Deserialize(const char* fromData, U32& bufferPosition);
		[[nodiscard]] U32 GetSize() const;

		// Asset Data
		std::vector<SDrawCallData> DrawCallData;
		std::string Name = "";
		U8 NumberOfMaterials = 0;

		// Render Data
		U8 VertexShaderIndex = 0;
		U8 InputLayoutIndex = 0;
		U8 PixelShaderIndex = 0;
		U8 SamplerIndex = 0;
		U8 TopologyIndex = 0;
	};
}