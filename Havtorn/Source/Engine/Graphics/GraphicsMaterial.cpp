// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "GraphicsMaterial.h"
#include "Engine.h"
#include "Assets/FileHeaderDeclarations.h"
#include "Assets/AssetRegistry.h"

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

	std::map<U32, CStaticRenderTexture> SEngineGraphicsMaterial::GetRenderTextures(const U64 requesterID) const
	{
		std::map<U32, CStaticRenderTexture> textures;

		auto extractRenderTexture = [&](const SRuntimeGraphicsMaterialProperty& property, std::map<U32, CStaticRenderTexture>& outMap)
			{
				if (property.TextureUID == 0 || property.ConstantValue >= 0.0f)
					return;

				if (outMap.contains(property.TextureUID))
					return;

				STextureAsset* asset = GEngine::GetAssetRegistry()->RequestAssetData<STextureAsset>(property.TextureUID, requesterID);
				if (asset == nullptr)
					return;

				outMap.emplace(property.TextureUID, asset->RenderTexture);
			};

		extractRenderTexture(AlbedoR, textures);
		extractRenderTexture(AlbedoG, textures);
		extractRenderTexture(AlbedoB, textures);
		extractRenderTexture(AlbedoA, textures);
		extractRenderTexture(NormalX, textures);
		extractRenderTexture(NormalY, textures);
		extractRenderTexture(NormalZ, textures);
		extractRenderTexture(AmbientOcclusion, textures);
		extractRenderTexture(Metalness, textures);
		extractRenderTexture(Roughness, textures);
		extractRenderTexture(Emissive, textures);

		return std::move(textures);
	}
}
