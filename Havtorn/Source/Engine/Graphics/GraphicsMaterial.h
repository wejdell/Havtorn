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
			
			auto fillProperty = [&](const SOfflineGraphicsMaterialProperty& offlineProperty, SRuntimeGraphicsMaterialProperty& outProperty)
			{
				outProperty.ConstantValue = offlineProperty.ConstantValue;
				
				if (offlineProperty.TexturePathLength != 0)
					outProperty.TextureIndex = static_cast<F32>(textureBank->GetTextureIndex(offlineProperty.TexturePath));
				
				outProperty.TextureChannelIndex = static_cast<F32>(offlineProperty.TextureChannelIndex);
			};

			fillProperty(offlineMaterial.Properties[static_cast<U8>(EMaterialProperty::AlbedoR)], AlbedoR);
			fillProperty(offlineMaterial.Properties[static_cast<U8>(EMaterialProperty::AlbedoG)], AlbedoG);
			fillProperty(offlineMaterial.Properties[static_cast<U8>(EMaterialProperty::AlbedoB)], AlbedoB);
			fillProperty(offlineMaterial.Properties[static_cast<U8>(EMaterialProperty::AlbedoA)], AlbedoA);
			fillProperty(offlineMaterial.Properties[static_cast<U8>(EMaterialProperty::NormalX)], NormalX);
			fillProperty(offlineMaterial.Properties[static_cast<U8>(EMaterialProperty::NormalY)], NormalY);
			fillProperty(offlineMaterial.Properties[static_cast<U8>(EMaterialProperty::NormalZ)], NormalZ);
			fillProperty(offlineMaterial.Properties[static_cast<U8>(EMaterialProperty::AmbientOcclusion)], AmbientOcclusion);
			fillProperty(offlineMaterial.Properties[static_cast<U8>(EMaterialProperty::Metalness)], Metalness);
			fillProperty(offlineMaterial.Properties[static_cast<U8>(EMaterialProperty::Roughness)], Roughness);
			fillProperty(offlineMaterial.Properties[static_cast<U8>(EMaterialProperty::Emissive)], Emissive);

			RecreateNormalZ = offlineMaterial.RecreateZ;
		}

		SRuntimeGraphicsMaterialProperty AlbedoR = {};
		SRuntimeGraphicsMaterialProperty AlbedoG = {};
		SRuntimeGraphicsMaterialProperty AlbedoB = {};
		SRuntimeGraphicsMaterialProperty AlbedoA = {};
		SRuntimeGraphicsMaterialProperty NormalX = {};
		SRuntimeGraphicsMaterialProperty NormalY = {};
		SRuntimeGraphicsMaterialProperty NormalZ = {};
		SRuntimeGraphicsMaterialProperty AmbientOcclusion = {};
		SRuntimeGraphicsMaterialProperty Metalness = {};
		SRuntimeGraphicsMaterialProperty Roughness = {};
		SRuntimeGraphicsMaterialProperty Emissive = {};


		std::string Name = "";
		bool RecreateNormalZ = true;
	};
}