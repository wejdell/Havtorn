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
		///		document.WriteValueToArray("Asset Redirectors", "Meshes/StaticMesh.hva", "Meshes/StaticMesh.hva");
		///		std::string redirector = document.GetValueFromArray<const char*>("Asset Redirectors", "Meshes/StaticMesh.hva");
		/// 

		friend class UFileSystem;

	public:
		CORE_API std::string GetString(const std::string& memberName, const std::string& defaultValue = "");
		CORE_API I32 GetInt(const std::string& memberName, const I32 defaultValue = 0);
		CORE_API bool GetBool(const std::string& memberName, const bool defaultValue = false);
		CORE_API bool HasMember(const std::string& memberName);
	
		CORE_API void WriteValueToArray(const std::string& arrayName, const std::string& valueName, const std::string& value);
		CORE_API std::string GetValueFromArray(const std::string& arrayName, const std::string& valueName, const std::string& defaultValue = std::string());
		CORE_API void RemoveValueFromArray(const std::string& arrayName, const std::string& valueName);

		CORE_API void ClearArray(const std::string& arrayName);

	private:
		CORE_API void SaveFile();

	private:
		rapidjson::Document Document;
		std::string FilePath = "";
	};

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

		CORE_API static const std::string EngineConfig;
	};
}
