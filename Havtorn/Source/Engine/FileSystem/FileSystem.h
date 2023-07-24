// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include <filesystem>
#include <fstream>
#include <rapidjson.h>

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

	class CFileSystem
	{
		friend class GEngine;

	public:
		static HAVTORN_API bool DoesFileExist(const std::string& fileName);

		void HAVTORN_API OpenFile(const std::string& fileName, EFileMode mode);
		void HAVTORN_API CloseFile(EFileMode mode);

		void HAVTORN_API Serialize(const std::string& fileName, const char* data, U32 size);
		void HAVTORN_API Deserialize(const std::string& fileName, char* data, U32 size);
		void HAVTORN_API Deserialize(const std::string& fileName, std::string& outData);
		U64 HAVTORN_API GetFileSize(const std::string& fileName) const;

		void HAVTORN_API IterateThroughFiles(const std::string& root);

	private:
		CFileSystem();
		~CFileSystem() = default;

		std::filesystem::recursive_directory_iterator GetDirectoryIterator(const std::string& root);

	private:
		std::ifstream InputStream;
		std::ofstream OutputStream;
		std::string AssetsPath;
	};
}
