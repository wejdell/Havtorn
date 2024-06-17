// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	class ISerializable
	{
	public:
		virtual [[nodiscard]] U32 GetSize() const = 0; // Used during serialization, to know how large the buffer we write into needs to be
		virtual void Serialize(char* toData, U64& pointerPosition) const = 0; // Writes into buffer with the size of GetSize or more
		virtual void Deserialize(const char* fromData, U64& pointerPosition) = 0; // Reads from a buffer the size of the file on disk
	};

	template<typename T>
	void SerializeSimple(const T& source, char* destination, U64& pointerPosition)
	{
		const U32 size = sizeof(T);
		memcpy(&destination[pointerPosition], &source, size);
		pointerPosition += size;
	}

	template<typename T>
	void SerializeVector(const std::vector<T>& source, char* destination, U64& pointerPosition)
	{
		const U32 size = sizeof(T) * static_cast<U32>(source.size());
		memcpy(&destination[pointerPosition], source.data(), size);
		pointerPosition += size;
	}

	inline void SerializeString(const std::string& source, char* destination, U64& pointerPosition)
	{
		const U32 size = sizeof(char) * static_cast<U32>(source.length());
		memcpy(&destination[pointerPosition], source.data(), size);
		pointerPosition += size;
	}

	inline void SerializeString(const std::string& source, char* destination, U32 numberOfElements, U64& pointerPosition)
	{
		const U32 size = sizeof(char) * numberOfElements;
		memcpy(&destination[pointerPosition], source.data(), size);
		pointerPosition += size;
	}

	template<typename T>
	void DeserializeSimple(T& destination, const char* source, U64& pointerPosition)
	{
		const U32 size = sizeof(T);
		memcpy(&destination, &source[pointerPosition], size);
		pointerPosition += size;
	}

	template<typename T>
	void DeserializeVector(std::vector<T>& destination, const char* source, U32 numberOfElements, U64& pointerPosition)
	{
		const U32 size = sizeof(T) * numberOfElements;
		const auto intermediateVector = new T[numberOfElements];
		memcpy(&intermediateVector[0], &source[pointerPosition], size);
		destination.assign(&intermediateVector[0], &intermediateVector[0] + numberOfElements);
		delete[] intermediateVector;
		pointerPosition += size;
	}

	inline void DeserializeString(std::string& destination, const char* source, U32 numberOfElements, U64& pointerPosition)
	{
		const U32 size = sizeof(char) * numberOfElements;
		destination = std::string(&source[pointerPosition], size);
		pointerPosition += size;
	}
}
