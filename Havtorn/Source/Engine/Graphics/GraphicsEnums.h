// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn 
{
	enum class ETextureFormat
	{
		DDS,
		TGA
	};

	enum class EMaterialConfiguration
	{
		AlbedoMaterialNormal_Packed
	};

	enum class EMaterialProperty
	{
		AlbedoR,
		AlbedoG,
		AlbedoB,
		AlbedoA,
		NormalX,
		NormalY,
		NormalZ,
		AmbientOcclusion,
		Metalness,
		Roughness,
		Emissive,
		Count
	};
}