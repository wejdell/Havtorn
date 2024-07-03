// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "Graphics/GraphicsStructs.h"
#include "Core/HavtornString.h"

namespace Havtorn
{
	struct SStaticMeshComponent : public SComponent
	{
		SStaticMeshComponent()
			: SComponent(EComponentType::StaticMeshComponent)
		{}

		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);
		[[nodiscard]] U32 GetSize() const;

		// Asset Data
		// A.G: Could potentially use an index/id to a map within the RenderManager for 'DrawCallData' instead?
		std::vector<SDrawCallData> DrawCallData;
		// A.G: Could potentially use an index/id to a map within the RenderManager for 'Name' instead?
		// Are DrawCallData & Name ever used outside of the RenderManager?
		// The map could be owned by some other static object/registry. I think it could reduce the size of components.
		CHavtornStaticString<255> Name;
		U8 NumberOfMaterials = 0;

		// Render Data
		U8 VertexShaderIndex = 0;
		U8 InputLayoutIndex = 0;
		U8 PixelShaderIndex = 0;
		U8 SamplerIndex = 0;
		U8 TopologyIndex = 0;
	};
}