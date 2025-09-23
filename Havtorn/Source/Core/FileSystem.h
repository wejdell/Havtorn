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
		///		document.RemoveValueFromArray("Asset Redirectors", "Meshes/StaticMesh.hva");
		///		document.WriteValueToArray("Asset Redirectors", "Meshes/StaticMesh.hva", "Meshes2/StaticMesh.hva");
		///		std::string redirector = document.GetValueFromArray("Asset Redirectors", "Meshes/StaticMesh.hva", "");
		/// 

		friend class UFileSystem;

	public:
		CORE_API std::string GetString(const std::string& memberName, const std::string& defaultValue = "") const;
		CORE_API bool HasMember(const std::string& memberName) const;
	
		CORE_API void WriteValueToArray(const std::string& arrayName, const std::string& valueName, const std::string& value);
		CORE_API std::string GetValueFromArray(const std::string& arrayName, const std::string& valueName, const std::string& defaultValue = std::string());
		CORE_API void RemoveValueFromArray(const std::string& arrayName, const std::string& valueName);

		CORE_API void ClearArray(const std::string& arrayName);

		template<typename T>
		T Get(const std::string& memberName, const T& defaultValue) const;

	private:
		CORE_API void SaveFile();

	private:
		rapidjson::Document Document;
		std::string FilePath = "";
	};

	template<typename T>
	T CJsonDocument::Get(const std::string& memberName, const T& defaultValue) const
	{
		if (!HasMember(memberName))
			return defaultValue;

		return Document[memberName.c_str()].Get<T>();
	}

	class UFileSystem
	{
		friend class GEngine;

	public:
		static bool CORE_API Exists(const std::string& path);
		static U64 CORE_API GetFileSize(const std::string& filePath);
		static CORE_API CJsonDocument OpenJson(const std::string& filePath);

		static void CORE_API Serialize(const std::string& filePath, const char* data, U32 size);
		static void CORE_API Deserialize(const std::string& filePath, char* data, U32 size);
		static void CORE_API Deserialize(const std::string& filePath, std::string& outData);

		static void CORE_API AddDirectory(const std::string& directoryPath);
		static void CORE_API Remove(const std::string& path);

		static void CORE_API IterateThroughFiles(const std::string& root);
		
		static std::vector<std::string> CORE_API SplitPath(const std::string& path);

		CORE_API static const std::string EngineConfig;
	};
}
