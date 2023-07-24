// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "Entity.h"
#include "FileSystem/FileHeaderDeclarations.h"

namespace Havtorn
{
	struct SComponent
	{
		SComponent() = default;

		explicit SComponent(EComponentType type)
			: Type(type)
		{}

		virtual ~SComponent() = default;

		//template<typename T>
		//U32 Serialize(char* toData, U32 bufferPosition) const;

		//template<typename T>
		//U32 Deserialize(const char* fromData, U32 bufferPosition);

		EComponentType Type = EComponentType::Count;
		bool IsInUse = false;
	};

	//template<typename T>
	//U32 SComponent::Serialize(char* toData, U32 bufferPosition) const
	//{
	//	memcpy(&toData[bufferPosition], this, sizeof(T));
	//	return sizeof(T);
	//}

	//template<typename T>
	//U32 SComponent::Deserialize(const char* fromData, U32 bufferPosition)
	//{
	//	U64 offset = sizeof(SComponent);
	//	U64 offsetSize = sizeof(T) - offset;
	//	memcpy(this + offset, &fromData[bufferPosition + offset], offsetSize);
	//	return sizeof(T);
	//}
}
