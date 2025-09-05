// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "GraphicsMaterial.h"
#include "Engine.h"
#include "Assets/FileHeaderDeclarations.h"

namespace Havtorn
{
	SEngineGraphicsMaterial::SEngineGraphicsMaterial(const SOfflineGraphicsMaterial& offlineMaterial, const std::string& materialName)
		: Name(materialName)
	{
		auto fillProperty = [&](const SOfflineGraphicsMaterialProperty& offlineProperty, SRuntimeGraphicsMaterialProperty& outProperty)
		{
			outProperty.ConstantValue = offlineProperty.ConstantValue;
				
			if (!offlineProperty.TexturePath.empty())
				outProperty.TextureUID = SAssetReference(offlineProperty.TexturePath).UID;
				
			outProperty.TextureChannelIndex = STATIC_F32(offlineProperty.TextureChannelIndex);
		};

		fillProperty(offlineMaterial.Properties[STATIC_U8(EMaterialProperty::AlbedoR)], AlbedoR);
		fillProperty(offlineMaterial.Properties[STATIC_U8(EMaterialProperty::AlbedoG)], AlbedoG);
		fillProperty(offlineMaterial.Properties[STATIC_U8(EMaterialProperty::AlbedoB)], AlbedoB);
		fillProperty(offlineMaterial.Properties[STATIC_U8(EMaterialProperty::AlbedoA)], AlbedoA);
		fillProperty(offlineMaterial.Properties[STATIC_U8(EMaterialProperty::NormalX)], NormalX);
		fillProperty(offlineMaterial.Properties[STATIC_U8(EMaterialProperty::NormalY)], NormalY);
		fillProperty(offlineMaterial.Properties[STATIC_U8(EMaterialProperty::NormalZ)], NormalZ);
		fillProperty(offlineMaterial.Properties[STATIC_U8(EMaterialProperty::AmbientOcclusion)], AmbientOcclusion);
		fillProperty(offlineMaterial.Properties[STATIC_U8(EMaterialProperty::Metalness)], Metalness);
		fillProperty(offlineMaterial.Properties[STATIC_U8(EMaterialProperty::Roughness)], Roughness);
		fillProperty(offlineMaterial.Properties[STATIC_U8(EMaterialProperty::Emissive)], Emissive);

		RecreateNormalZ = offlineMaterial.RecreateZ;
	}
}
