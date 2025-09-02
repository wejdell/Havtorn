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
		friend class UFileSystem;

	public:
		CORE_API std::string GetString(const std::string& memberName, const std::string& defaultValue = "");
		CORE_API I32 GetInt(const std::string& memberName, const I32 defaultValue = 0);
		CORE_API bool GetBool(const std::string& memberName, const bool defaultValue = false);
		CORE_API bool HasMember(const std::string& memberName);
	private:
		rapidjson::Document Document;
	};

	class UFileSystem
	{
		friend class GEngine;

	public:
		static bool CORE_API DoesFileExist(const std::string& fileName);
		static U64 CORE_API GetFileSize(const std::string& fileName);
		static CORE_API CJsonDocument OpenJson(const std::string& fileName);

		static void CORE_API OpenFile(const std::string& fileName, EFileMode mode);
		static void CORE_API CloseFile(EFileMode mode);

		static void CORE_API Serialize(const std::string& fileName, const char* data, U32 size);
		static void CORE_API Deserialize(const std::string& fileName, char* data, U32 size);
		static void CORE_API Deserialize(const std::string& fileName, std::string& outData);

		static void CORE_API IterateThroughFiles(const std::string& root);
	};
}
