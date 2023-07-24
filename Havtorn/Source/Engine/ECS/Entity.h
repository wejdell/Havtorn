// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "Core/Core.h"
#include "ECS/ECS.h"
#include "Core/HavtornString.h"
#include "Core/BitSet.h"

namespace Havtorn
{
	struct HAVTORN_API SEntity
	{
		U64 GUID = 0;

		bool IsValid() const;
		void AddComponent(EComponentType type) const;
		void RemoveComponent(EComponentType type) const;
		bool HasComponent(EComponentType type) const;
		bool HasComponents(U64 mask) const;
		const CBitSet<STATIC_U64(EComponentType::Count)>& GetComponentMask() const;

		bool operator==(const SEntity& other) const;
		bool operator!=(const SEntity& other) const;

	private:
		 mutable CBitSet<STATIC_U64(EComponentType::Count)> ComponentMask;
	};

	const static SEntity EntityTombstone;

	inline bool SEntity::IsValid() const
	{
		return GUID > 0;
	}

	inline void SEntity::AddComponent(EComponentType type) const
	{
		ComponentMask.Set(STATIC_U64(type));
	}

	inline void SEntity::RemoveComponent(EComponentType type) const
	{
		ComponentMask.Reset(STATIC_U64(type));
	}

	inline bool SEntity::HasComponent(EComponentType type) const
	{
		return ComponentMask.Test(STATIC_U64(type));
	}

	inline bool SEntity::HasComponents(U64 mask) const
	{
		return ComponentMask.Test(mask);
	}

	inline const CBitSet<STATIC_U64(EComponentType::Count)>& SEntity::GetComponentMask() const
	{
		return ComponentMask;
	}

	inline bool SEntity::operator==(const SEntity& other) const
	{
		return GUID == other.GUID;
	}

	inline bool SEntity::operator!=(const SEntity& other) const
	{
		return GUID != other.GUID;
	}
}
