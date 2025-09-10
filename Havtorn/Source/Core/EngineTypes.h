// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "CoreTypes.h"
#include "MathTypes/Vector.h"
#include "MathTypes/Quaternion.h"
#include "MathTypes/Matrix.h"
#include "MathTypes/Transform.h"
#include "MathTypes/Curve.h"
#include "Color.h"
#include "Serializable.h"
#include "magic_enum.h"

#define PIN_MATH_TYPES Havtorn::SVector, Havtorn::SMatrix, Havtorn::SQuaternion

namespace Havtorn
{
	enum class EAssetType
	{
		None,
		StaticMesh,
		SkeletalMesh,
		Texture,
		Material,
		Animation,
		SpriteAnimation,
		AudioOneShot,
		AudioCollection,
		VisualFX,
		Scene,
		Sequencer,
		Script
	};

	static SColor GetAssetTypeColor(const EAssetType type)
	{
		switch (type)
		{
		case EAssetType::Animation:
			return SColor::Blue;
		case EAssetType::Material:
			return SColor::Green;
		case EAssetType::SkeletalMesh:
			return SColor::Magenta;
		case EAssetType::StaticMesh:
			return SColor::Teal;
		case EAssetType::Texture:
			return SColor::Red;
		case EAssetType::Script:
			return SColor::Blue;
		default:
			return SColor::White;
		}
	}

	static std::string GetAssetTypeName(const EAssetType type)
	{
		 return magic_enum::enum_name<EAssetType>(type).data();
	}

	static std::string GetAssetTypeDetailName(const EAssetType type)
	{
		switch (type)
		{
		case EAssetType::Animation:
			return "SKELETAL ANIMATION";
		case EAssetType::Material:
			return "MATERIAL";
		case EAssetType::SkeletalMesh:
			return "SKELETAL MESH";
		case EAssetType::StaticMesh:
			return "STATIC MESH";
		case EAssetType::Texture:
			return "TEXTURE";
		case EAssetType::Script:
			return "SCRIPT";
		default:
			return "ASSET";
		}
	}
}
