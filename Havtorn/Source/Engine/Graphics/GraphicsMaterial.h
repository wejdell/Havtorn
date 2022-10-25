// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "hvpch.h"
#include "GraphicsEnums.h"
#include "GraphicsStructs.h"
#include "Engine.h"
#include "TextureBank.h"

namespace Havtorn
{
	struct SEngineGraphicsMaterial
	{
		SEngineGraphicsMaterial() = default;
		SEngineGraphicsMaterial(const SOfflineGraphicsMaterial& offlineMaterial, const std::string& materialName)
			: Name(materialName)
		{
			CTextureBank* textureBank = GEngine::GetTextureBank();
			
			auto fillProperty = [&](const SOfflineGraphicsMaterialProperty& offlineProperty, SEngineGraphicsMaterialProperty& outProperty)
			{
				outProperty.ConstantValue = offlineProperty.ConstantValue;
				
				if (offlineProperty.TexturePathLength != 0)
					outProperty.TextureIndex = static_cast<U16>(textureBank->GetTextureIndex(offlineProperty.TexturePath));
				
				outProperty.TextureChannelIndex = offlineProperty.TextureChannelIndex;
			};

			fillProperty(offlineMaterial.Properties[0], AlbedoR);
			fillProperty(offlineMaterial.Properties[1], AlbedoG);
			fillProperty(offlineMaterial.Properties[2], AlbedoB);
			fillProperty(offlineMaterial.Properties[3], AlbedoA);
			fillProperty(offlineMaterial.Properties[4], NormalX);
			fillProperty(offlineMaterial.Properties[5], NormalY);
			fillProperty(offlineMaterial.Properties[6], NormalZ);
			fillProperty(offlineMaterial.Properties[7], AmbientOcclusion);
			fillProperty(offlineMaterial.Properties[8], Metalness);
			fillProperty(offlineMaterial.Properties[9], Roughness);
			fillProperty(offlineMaterial.Properties[10], Emissive);

			RecreateNormalZ = offlineMaterial.RecreateZ;
		}

		SEngineGraphicsMaterialProperty AlbedoR = {};
		SEngineGraphicsMaterialProperty AlbedoG = {};
		SEngineGraphicsMaterialProperty AlbedoB = {};
		SEngineGraphicsMaterialProperty AlbedoA = {};
		SEngineGraphicsMaterialProperty NormalX = {};
		SEngineGraphicsMaterialProperty NormalY = {};
		SEngineGraphicsMaterialProperty NormalZ = {};
		SEngineGraphicsMaterialProperty AmbientOcclusion = {};
		SEngineGraphicsMaterialProperty Metalness = {};
		SEngineGraphicsMaterialProperty Roughness = {};
		SEngineGraphicsMaterialProperty Emissive = {};

		std::string Name = "";
		bool RecreateNormalZ = true;
	};
}