// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "rapidjson/document.h"

#include <filesystem>

namespace Havtorn
{
	enum class ESerializeMode
	{
		Binary,
		Readable
	};

	enum class EFileMode
	{
		Read,
		Write,
		BinaryRead,
		BinaryWrite
	};

	struct SFilePath
	{
		SFilePath(const std::filesystem::directory_entry& directoryEntry)
			: InternalPath(directoryEntry.path())
		{}

		SFilePath(const std::filesystem::path& path)
			: InternalPath(path)
		{}

		const std::string GetPath()
		{
			return InternalPath.relative_path().string();
		}

		const std::string Filename() 
		{
			return InternalPath.filename().string();
		}

		const std::string Directory()
		{
			return InternalPath.parent_path().string();
		}

		const std::string Extension()
		{
			return InternalPath.extension().string();
		}

	private:
		std::filesystem::path InternalPath;
	};

	class CJsonDocument
	{
		/// Example Usage
		/// 
		/// 	CJsonDocument document = UFileSystem::OpenJson("Config/EngineConfig.json");
		///		document.RemoveValueFromArray("Asset Redirectors", "Tests/StaticMesh.hva");
		///		document.AddValueToArray("Asset Redirectors", "Tests/StaticMesh.hva", "Meshes/StaticMesh.hva");
		///		std::string redirector = document.GetValueFromArray<const char*>("Asset Redirectors", "Tests/StaticMesh.hva");
		/// 

		friend class UFileSystem;

	public:
		CORE_API std::string GetString(const std::string& memberName, const std::string& defaultValue = "");
		CORE_API I32 GetInt(const std::string& memberName, const I32 defaultValue = 0);
		CORE_API bool GetBool(const std::string& memberName, const bool defaultValue = false);
		CORE_API bool HasMember(const std::string& memberName);
	
		CORE_API void RemoveValueFromArray(const std::string& arrayName, const std::string& valueName);

		template<typename T>
		void AddValueToArray(const std::string& arrayName, const std::string& valueName, const T& value);

		template<typename T>
		T GetValueFromArray(const std::string& arrayName, const std::string& valueName, const T& defaultValue = T{});

	private:
		CORE_API void SaveFile();

	private:
		rapidjson::Document Document;
		std::string FilePath = "";
	};

	template<typename T>
	inline void CJsonDocument::AddValueToArray(const std::string& arrayName, const std::string& valueName, const T& value)
	{
		if (!HasMember(arrayName) || !Document[arrayName.c_str()].IsArray())
		{
			HV_LOG_WARN("Could not add value to %s in %s, it either doesn't have that member or that member is not an array.", arrayName.c_str(), FilePath.c_str());
			return;
		}

		auto array = Document[arrayName.c_str()].GetArray();
		rapidjson::Document::AllocatorType& allocator = Document.GetAllocator();

		rapidjson::Value object(rapidjson::kObjectType);
		rapidjson::Value key(valueName.c_str(), allocator);
		rapidjson::Value keyValue(value);
		object.AddMember(key, keyValue, Document.GetAllocator());
		array.PushBack(object.Move(), allocator);

		// TODO.NW: We're saving here instead of the destructor as the Document class is well enclosed, 
		// should have another look at this.
		SaveFile();
	}

	template<typename T>
	inline T CJsonDocument::GetValueFromArray(const std::string& arrayName, const std::string& valueName, const T& defaultValue)
	{
		if (!HasMember(arrayName) || !Document[arrayName.c_str()].IsArray())
			return defaultValue;
		
		auto array = Document[arrayName.c_str()].GetArray();
		for (auto& v : array)
		{
			if (v.HasMember(valueName.c_str()))
			{
				return v[valueName.c_str()].Get<T>();
			}
		}

		return defaultValue;
	}

	class UFileSystem
	{
		friend class GEngine;

	public:
		static bool CORE_API DoesFileExist(const std::string& filePath);
		static U64 CORE_API GetFileSize(const std::string& filePath);
		static CORE_API CJsonDocument OpenJson(const std::string& filePath);

		static void CORE_API Serialize(const std::string& filePath, const char* data, U32 size);
		static void CORE_API Deserialize(const std::string& filePath, char* data, U32 size);
		static void CORE_API Deserialize(const std::string& filePath, std::string& outData);

		static void CORE_API IterateThroughFiles(const std::string& root);
	};
}
