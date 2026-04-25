// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "SHexCommandComponent.h"

namespace Havtorn
{
	void SHexCommandComponent::Serialize(char* toData, U64& pointerPosition) const
	{
		SerializeData(TagsToListenFor, toData, pointerPosition);
		//SerializeData(STATIC_U32(HexCommands.size()), toData, pointerPosition);
		//for (auto& tag : TagsToListenFor)
		//{
		//	SerializeData(tag, toData, pointerPosition);
		//	//SerializeData(hexCommand.Tag, toData, pointerPosition);
		//	//SerializeData(hexCommand.DataType, toData, pointerPosition);
		//	//SerializeData(hexCommand.Data, toData, pointerPosition);
		//}
	}

	void SHexCommandComponent::Deserialize(const char* fromData, U64& pointerPosition)
	{
		DeserializeData(TagsToListenFor, fromData, pointerPosition);

		//U32 hexCommandCount = 0;
		//DeserializeData(hexCommandCount, fromData, pointerPosition);
		//
		//HexCommands.resize(hexCommandCount);
		//for (U32 i = 0; i < hexCommandCount; i++)
		//{
		//	SHexCommand& command = HexCommands[i];
		//	DeserializeData(command.Tag, fromData, pointerPosition);
		//	DeserializeData(command.DataType, fromData, pointerPosition);		
		//	DeserializeDataVariant(command.Data, command.DataType, fromData, pointerPosition);
		//}
	}

	void SHexCommandComponent::DeserializeDataVariant(std::variant<HEXTYPES>& data, const EHexCommandDataType dataType, const char* fromData, Havtorn::U64& pointerPosition)
	{
		switch (dataType)
		{
		case EHexCommandDataType::Bool:		DeserializeVariant<bool>(data, fromData, pointerPosition);			break;
		case EHexCommandDataType::Float:	DeserializeVariant<F32>(data, fromData, pointerPosition);			break;
		case EHexCommandDataType::Vector2:	DeserializeVariant<SVector2<F32>>(data, fromData, pointerPosition);	break; 
		}
	}

	U32 SHexCommandComponent::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(TagsToListenFor);

		/*size += sizeof(U32);
		for (auto& hexCommand : HexCommands)
		{
			size += GetDataSize(hexCommand.Tag);
			size += GetDataSize(hexCommand.DataType);
			size += GetDataSize(hexCommand.Data);
		}*/
		return size;
	}
}
