// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "FileSystem.h"
#include "GeneralUtilities.h"

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"

#include <iostream>
#include <fstream>

using std::fstream;

namespace Havtorn
{
	using DirectoryIterator = std::filesystem::recursive_directory_iterator;

	const std::string UFileSystem::EngineConfig = "Config/EngineConfig.json";

	bool UFileSystem::DoesFileExist(const std::string& filePath)
	{
		const std::ifstream infile(filePath);
		return infile.good();
	}

	U64 UFileSystem::GetFileSize(const std::string& filePath)
	{
		return std::filesystem::file_size(filePath);
	}

	void UFileSystem::Serialize(const std::string& filePath, const char* data, U32 size)
	{
		std::ofstream outputStream;
		outputStream.open(filePath.c_str(), fstream::out | fstream::binary);

		if (!outputStream)
			HV_LOG_ERROR("FileSystem could not open file: %s", filePath.c_str());

		outputStream.write(data, size);
		outputStream.close();

		if (outputStream.bad())
			HV_LOG_ERROR("FileSystem encountered an operation error after closing the output stream");
	}

	void UFileSystem::Deserialize(const std::string& filePath, char* data, U32 size)
	{
		std::ifstream inputStream;
		inputStream.open(filePath.c_str(), fstream::in | fstream::binary);

		if (!inputStream)
			HV_LOG_ERROR("FileSystem could not open file: %s", filePath.c_str());
		
		inputStream.read(data, size);
		inputStream.close();

		if (inputStream.bad())
			HV_LOG_ERROR("FileSystem encountered an operation error after closing the input stream");
	}

	void UFileSystem::Deserialize(const std::string& filePath, std::string& outData)
	{
		std::ifstream inputStream;
		inputStream.open(filePath.c_str(), fstream::in | fstream::binary);

		if (!inputStream)
			HV_LOG_ERROR("FileSystem could not open file: %s", filePath.c_str());

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

	CJsonDocument UFileSystem::OpenJson(const std::string& filePath)
	{
		CJsonDocument document;
		std::ifstream stream(filePath);
		rapidjson::IStreamWrapper wrapper(stream);
		document.Document.ParseStream(wrapper);
		document.FilePath = filePath;
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
		if (!Document.IsObject())
			return false;

		return Document.HasMember(memberName.c_str());
	}

	void CJsonDocument::WriteValueToArray(const std::string& arrayName, const std::string& valueName, const std::string& value)
	{
		if (!HasMember(arrayName) || !Document[arrayName.c_str()].IsArray())
		{
			HV_LOG_WARN("Could not add value to %s in %s, it either doesn't have that member or that member is not an array.", arrayName.c_str(), FilePath.c_str());
			return;
		}

		std::string sanitizedStringValueName = UGeneralUtils::ConvertToPlatformAgnosticPath(valueName);
		std::string sanitizedStringValue = UGeneralUtils::ConvertToPlatformAgnosticPath(value);

		auto array = Document[arrayName.c_str()].GetArray();
		rapidjson::Document::AllocatorType& allocator = Document.GetAllocator();

		rapidjson::Value object(rapidjson::kObjectType);
		rapidjson::Value key(sanitizedStringValueName.c_str(), allocator);
		rapidjson::Value keyValue;
		keyValue = rapidjson::StringRef(sanitizedStringValue.c_str());
		object.AddMember(key, keyValue, Document.GetAllocator());
		
		bool hasValue = false;
		for (auto& v : array)
		{
			if (v.HasMember(sanitizedStringValueName.c_str()))
			{
				v.Swap(object);
				hasValue = true;
				break;
			}
		}

		if (!hasValue)
			array.PushBack(object.Move(), allocator);

		// TODO.NW: We're saving here instead of the destructor as the Document class is well enclosed, 
		// should have another look at this.
		SaveFile();
	}

	std::string CJsonDocument::GetValueFromArray(const std::string& arrayName, const std::string& valueName, const std::string& defaultValue)
	{
		if (!HasMember(arrayName) || !Document[arrayName.c_str()].IsArray())
			return defaultValue;

		auto array = Document[arrayName.c_str()].GetArray();
		for (auto& v : array)
		{
			if (v.HasMember(valueName.c_str()))
				return v[valueName.c_str()].GetString();
		}

		return defaultValue;
	}

	void CJsonDocument::RemoveValueFromArray(const std::string& arrayName, const std::string& valueName)
	{
		if (!HasMember(arrayName) || !Document[arrayName.c_str()].IsArray())
			return;

		auto array = Document[arrayName.c_str()].GetArray();
		rapidjson::Value::ValueIterator iterator = array.End();
		for (rapidjson::Value::ValueIterator it = array.Begin(); it != array.End(); ++it)
		{
			if (!it->IsObject())
				continue;

			if (it->HasMember(valueName.c_str()))
			{
				iterator = it;
				break;
			}
		}

		if (iterator == array.End())
			return;

		array.Erase(iterator);

		// TODO.NW: We're saving here instead of the destructor as the Document class is well enclosed, 
		// should have another look at this.
		SaveFile();
	}

	void CJsonDocument::ClearArray(const std::string& arrayName)
	{
		if (!HasMember(arrayName) || !Document[arrayName.c_str()].IsArray())
			return;

		auto array = Document[arrayName.c_str()].GetArray();
		array.Clear();

		// TODO.NW: We're saving here instead of the destructor as the Document class is well enclosed, 
		// should have another look at this.
		SaveFile();
	}

	void CJsonDocument::SaveFile()
	{
		FILE* fp = nullptr;
		fopen_s(&fp, FilePath.c_str(), "wb"); // non-Windows use "w"

		char* writeBuffer = new char[16384];
		rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
		rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
	
		Document.Accept(writer);

		if (fp != nullptr)
			fclose(fp);

		delete[] writeBuffer;
	}
}

