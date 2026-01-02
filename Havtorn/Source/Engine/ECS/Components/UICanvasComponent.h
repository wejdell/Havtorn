// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

#include "Assets/AssetRegistry.h"

namespace Havtorn
{
	enum class EUIElementState
	{
		Idle,
		Hovered,
		Active,
		Count
	};

	enum class EUIBindingType
	{
		None,
		OtherCanvas,
		GenericFunction,
	};

	struct SUIElement
	{
		SUIElement()
		{
			StateAssetReferences.resize(STATIC_U64(EUIElementState::Count));
			UVRects.resize(STATIC_U64(EUIElementState::Count), SVector4(0.0f, 0.0f, 1.0f, 1.0f));
		}

		// TODO.NW: array would be nicer here, can we guarantee the size? Or use array in all editor logic
		std::vector<SAssetReference> StateAssetReferences;
		std::vector<SVector4> UVRects;
		SVector4 CollisionRect = SVector4::Zero;
		SColor Color = SColor::White;
		SVector2<F32> LocalPosition = SVector2<F32>::Zero;
		SVector2<F32> LocalScale = SVector2<F32>(1.0f);
		EUIElementState State = EUIElementState::Idle;
		EUIBindingType BindingType = EUIBindingType::None;
		U64 BoundData = 0;
		F32 LocalDegreesRoll = 0.0f;

		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);
		[[nodiscard]] U32 GetSize() const;
	};

	struct SUICanvasComponent : public SComponent
	{
		SUICanvasComponent() = default;
		SUICanvasComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{
		}

		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);
		[[nodiscard]] U32 GetSize() const;

		ENGINE_API void IsDeleted(CScene* fromScene) override;

		std::vector<SUIElement> Elements;
		bool IsActive = false;
	};
}
