// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "hvpch.h"
#include "GraphicsStructs.h"
#include "RenderingPrimitives/RenderTexture.h"

namespace Havtorn
{
	struct SEngineGraphicsMaterial
	{
		SEngineGraphicsMaterial() = default;
		ENGINE_API SEngineGraphicsMaterial(const SOfflineGraphicsMaterial& offlineMaterial, const std::string& materialName);

		std::map<U32, CStaticRenderTexture> GetRenderTextures(const U64 requesterID) const;

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
