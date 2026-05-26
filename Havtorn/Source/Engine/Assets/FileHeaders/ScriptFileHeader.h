// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "Assets/AssetFileHeaderBase.h"
#include "HexRune/HexRune.h"

namespace Havtorn
{
	struct SScriptFileHeader
	{
		SAssetFileHeaderBase HeaderBase = { .AssetType = EAssetType::Script, .Version = 1 };
		std::string Name = "";
		HexRune::SScript* Script = nullptr;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData, HexRune::SScript* outScript);
	};

	inline U32 SScriptFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(HeaderBase);
		size += GetDataSize(Name);
		size += Script->GetSize();
		return size;
	}

	inline void SScriptFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(HeaderBase, toData, pointerPosition);
		SerializeData(Name, toData, pointerPosition);
		Script->Serialize(toData, pointerPosition);
	}

	inline void SScriptFileHeader::Deserialize(const char* fromData, HexRune::SScript* outScript)
	{
		U64 pointerPosition = 0;
		DeserializeData(HeaderBase, fromData, pointerPosition);
		DeserializeData(Name, fromData, pointerPosition);
		outScript->Deserialize(fromData, pointerPosition);
		outScript->Name = Name;
	}
}
