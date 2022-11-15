// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "Entity.h"
#include "FileSystem/FileHeaderDeclarations.h"

namespace Havtorn
{
	struct SEntity;

	struct SComponent
	{
		explicit SComponent(Ref<SEntity> entity, EComponentType type)
			: Entity(entity)
			, Type(type)
		{}
		virtual ~SComponent() = default;

		template<typename T>
		U32 Serialize(char* sourceData, U32 bufferPosition);

		template<typename T>
		U32 Deserialize(const char* fromData, U32 bufferPosition);

		const Ref<SEntity> Entity;
		const EComponentType Type;
	};

	template<typename T>
	U32 SComponent::Serialize(char* toData, U32 bufferPosition)
	{
		memcpy(&toData[bufferPosition], this, sizeof(T));
		return sizeof(T);
	}

	template<typename T>
	U32 SComponent::Deserialize(const char* fromData, U32 bufferPosition)
	{
		memcpy(this, &fromData[bufferPosition], sizeof(T));
		return sizeof(T);
	}
}
