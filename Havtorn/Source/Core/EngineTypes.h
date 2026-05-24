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
	enum class EAssetType : U16
	{
		None,
		StaticMesh,
		SkeletalMesh,
		SkeletalAnimation,
		Texture,
		TextureCube,
		SpriteAnimation,
		Material,
		Scene,
		Script,
		Prefab,
		InputAsset,
		AudioClip,
		Sequencer
	};

	static SColor GetAssetTypeColor(const EAssetType type)
	{
		switch (type)
		{
		case EAssetType::SkeletalAnimation:
			return SColor::Blue;
		case EAssetType::Material:
			return SColor::Green;
		case EAssetType::SkeletalMesh:
			return SColor::Magenta;
		case EAssetType::StaticMesh:
			return SColor::Teal;
		case EAssetType::Texture:
			return SColor::Red;
		case EAssetType::TextureCube:
			return SColor::Red;
		case EAssetType::Script:
			return SColor::Blue;
		case EAssetType::Scene:
			return SColor::Orange;
		case EAssetType::Prefab:
			return SColor::Yellow;
		case EAssetType::InputAsset:
			return SColor::Teal;
		case EAssetType::AudioClip:
			return SColor::Purple;
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
		case EAssetType::SkeletalAnimation:
			return "SKELETAL ANIM";
		case EAssetType::Material:
			return "MATERIAL";
		case EAssetType::SkeletalMesh:
			return "SKELETAL MESH";
		case EAssetType::StaticMesh:
			return "STATIC MESH";
		case EAssetType::Texture:
			return "TEXTURE";
		case EAssetType::TextureCube:
			return "TEXTURE CUBE";
		case EAssetType::Script:
			return "SCRIPT";
		case EAssetType::Scene:
			return "SCENE";
		case EAssetType::Prefab:
			return "PREFAB";
		case EAssetType::InputAsset:
			return "INPUT MAP";
		case EAssetType::AudioClip:
			return "AUDIO CLIP";
		default:
			return "ASSET";
		}
	}

	static bool IsAssetSourceFileBased(const EAssetType type)
	{
		switch (type)
		{
		case EAssetType::SkeletalAnimation: [[fallthrough]];
		case EAssetType::StaticMesh: [[fallthrough]];
		case EAssetType::SkeletalMesh: [[fallthrough]];
		case EAssetType::Texture: [[fallthrough]];
		case EAssetType::TextureCube: [[fallthrough]];
		case EAssetType::AudioClip: [[fallthrough]];
		case EAssetType::SpriteAnimation:
			return true;
		}

		return false;
	}

	static bool DoesAssetHaveAssetDependencies(const EAssetType type)
	{
		switch (type)
		{
		case EAssetType::SkeletalAnimation: [[fallthrough]];
		case EAssetType::Material:
			return true;
		}

		return false;
	}
}
