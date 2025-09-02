// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "FileSystem.h"

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

#include <iostream>
#include <fstream>

using std::fstream;

namespace Havtorn
{
	using DirectoryIterator = std::filesystem::recursive_directory_iterator;

	bool UFileSystem::DoesFileExist(const std::string& fileName)
	{
		const std::ifstream infile(fileName);
		return infile.good();
	}

	U64 UFileSystem::GetFileSize(const std::string& fileName)
	{
		return std::filesystem::file_size(fileName);
	}

	void UFileSystem::Serialize(const std::string& fileName, const char* data, U32 size)
	{
		std::ofstream outputStream;
		outputStream.open(fileName.c_str(), fstream::out | fstream::binary);

		if (!outputStream)
			HV_LOG_ERROR("FileSystem could not open file: %s", fileName.c_str());

		outputStream.write(data, size);
		outputStream.close();

		if (outputStream.bad())
			HV_LOG_ERROR("FileSystem encountered an operation error after closing the output stream");
	}

	void UFileSystem::Deserialize(const std::string& fileName, char* data, U32 size)
	{
		std::ifstream inputStream;
		inputStream.open(fileName.c_str(), fstream::in | fstream::binary);

		if (!inputStream)
			HV_LOG_ERROR("FileSystem could not open file: %s", fileName.c_str());
		
		inputStream.read(data, size);
		inputStream.close();

		if (inputStream.bad())
			HV_LOG_ERROR("FileSystem encountered an operation error after closing the input stream");
	}

	void UFileSystem::Deserialize(const std::string& fileName, std::string& outData)
	{
		std::ifstream inputStream;
		inputStream.open(fileName.c_str(), fstream::in | fstream::binary);

		if (!inputStream)
			HV_LOG_ERROR("FileSystem could not open file: %s", fileName.c_str());

		std::ostringstream oss;
		oss << inputStream.rdbuf();
		outData = oss.str();
		inputStream.close();

		if (inputStream.bad())
			HV_LOG_ERROR("FileSystem encountered an operation error after closing the input stream");
	}

	void UFileSystem::IterateThroughFiles(const std::string& root)
	{		
		for (const auto& dirEntry : DirectoryIterator(root))
		{
			SFilePath filePath = dirEntry;

			HV_LOG_TRACE("Dir: %s", filePath.Directory().c_str());
			HV_LOG_TRACE("Filename: %s", filePath.Filename().c_str());
			HV_LOG_TRACE("Extension: %s", filePath.Extension().c_str());
			HV_LOG_TRACE("Path: %s", filePath.GetPath().c_str());
		}
	}

	CJsonDocument UFileSystem::OpenJson(const std::string& fileName)
	{
		CJsonDocument document;
		std::ifstream stream(fileName);
		rapidjson::IStreamWrapper wrapper(stream);
		document.Document.ParseStream(wrapper);
		return document;
	}

	std::string CJsonDocument::GetString(const std::string& memberName, const std::string& defaultValue)
	{
		if (!HasMember(memberName))
			return defaultValue;

		return Document[memberName.c_str()].GetString();
	}

	I32 CJsonDocument::GetInt(const std::string& memberName, const I32 defaultValue)
	{
		if (!HasMember(memberName))
			return defaultValue;

		return Document[memberName.c_str()].GetInt();
	}

	bool CJsonDocument::GetBool(const std::string& memberName, const bool defaultValue)
	{
		if (!HasMember(memberName))
			return defaultValue;

		return Document[memberName.c_str()].GetBool();
	}

	bool CJsonDocument::HasMember(const std::string& memberName)
	{
		return Document.HasMember(memberName.c_str());
	}
}

