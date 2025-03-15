// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

//#define LOG_SERIALIZATION

#ifdef LOG_SERIALIZATION
#define LOG_SERIALIZE(...) HV_LOG_TRACE(__VA_ARGS__)
#else
#define LOG_SERIALIZE(...)
#endif

namespace Havtorn
{
	class ISerializable
	{
	public:
		// Used during serialization, to know how large the buffer we write into needs to be
		virtual [[nodiscard]] U32 GetSize() const = 0; 
		
		// Writes into buffer with the size of GetSize or more
		virtual void Serialize(char* toData, U64& pointerPosition) const = 0; 
		
		// Reads from a buffer the size of the file on disk
		virtual void Deserialize(const char* fromData, U64& pointerPosition) = 0; 
	};

	template<typename T>
	U32 GetDataSize(const T& object)
	{
		U32 size = sizeof(object);
		LOG_SERIALIZE("Data of type %s was registered for serialization with size %i", typeid(T).name(), size);
		return size;
	}

	template<typename T>
	U32 GetDataSize(const std::vector<T>& objects)
	{
		U32 numberSize = sizeof(U32);
		LOG_SERIALIZE("Data vector of type %s was registered for serialization with size %i", typeid(U32).name(), numberSize);
		U32 dataSize = sizeof(T) * STATIC_U32(objects.size());
		LOG_SERIALIZE("Data vector of type %s was registered for serialization with size %i", typeid(T).name(), dataSize);
		return numberSize + dataSize;
	}

	template<>
	inline U32 GetDataSize(const std::string& object)
	{
		U32 numberSize = sizeof(U32); 
		LOG_SERIALIZE("Data of type %s was registered for serialization with size %i", typeid(U32).name(), numberSize);
		U32 dataSize = sizeof(char) * STATIC_U32(object.length());
		LOG_SERIALIZE("Data of type std::string was registered for serialization with size %i", dataSize);
		return numberSize + dataSize;
	}

	template<typename T>
	void SerializeData(const T& source, char* destination, U64& pointerPosition)
	{
		const U32 size = sizeof(T);
		memcpy(&destination[pointerPosition], &source, size);
		LOG_SERIALIZE("Serialized data of type %s and size %i at position %i -> %i", typeid(T).name(), size, pointerPosition, pointerPosition + size);
		pointerPosition += size;
	}

	template<typename T>
	inline void SerializeData(const std::vector<T>& source, char* destination, U64& pointerPosition)
	{
		const U32 size = sizeof(T) * STATIC_U32(source.size());
		SerializeData(size, destination, pointerPosition);
		memcpy(&destination[pointerPosition], source.data(), size);
		LOG_SERIALIZE("Serialized vector data of type %s and size %i at position %i -> %i", typeid(T).name(), size, pointerPosition, pointerPosition + size);
		pointerPosition += size;
	}

	template<>
	inline void SerializeData(const std::string& source, char* destination, U64& pointerPosition)
	{
		const U32 size = sizeof(char) * STATIC_U32(source.length());
		SerializeData(size, destination, pointerPosition);
		memcpy(&destination[pointerPosition], source.data(), size);
		LOG_SERIALIZE("Serialized data of type std::string and size %i at position %i -> %i", size, pointerPosition, pointerPosition + size);
		pointerPosition += size;
	}

	template<typename T>
	void DeserializeData(T& destination, const char* source, U64& pointerPosition)
	{
		const U32 size = sizeof(T);
		memcpy(&destination, &source[pointerPosition], size);
		LOG_SERIALIZE("Deserialized data of type %s and size %i at position %i -> %i", typeid(T).name(), size, pointerPosition, pointerPosition + size);
		pointerPosition += size;
	}

	template<typename T>
	inline void DeserializeData(std::vector<T>& destination, const char* source, U64& pointerPosition)
	{
		U32 size = 0;
		DeserializeData(size, source, pointerPosition);
		const U32 numberOfElements = size / sizeof(T);
		const auto intermediateVector = new T[numberOfElements];
		memcpy(&intermediateVector[0], &source[pointerPosition], size);
		destination.assign(&intermediateVector[0], &intermediateVector[0] + numberOfElements);
		delete[] intermediateVector;
		LOG_SERIALIZE("Deserialized vector data of type %s and size %i at position %i -> %i", typeid(T).name(), size, pointerPosition, pointerPosition + size);
		pointerPosition += size;
	}

	inline void DeserializeData(std::string& destination, const char* source, U64& pointerPosition)
	{
		U32 size = 0;
		DeserializeData(size, source, pointerPosition);
		destination = std::string(&source[pointerPosition], size);
		LOG_SERIALIZE("Deserialized data of type std::string and size %i at position %i -> %i", size, pointerPosition, pointerPosition + size);
		pointerPosition += size;
	}
}
