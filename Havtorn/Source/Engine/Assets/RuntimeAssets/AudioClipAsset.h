// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once 
#include "Assets/FileHeaders/AudioClipFileHeader.h"
#include <GeneralUtilities.h>

namespace Havtorn
{
	struct SAudioClipAsset
	{
		SAudioClipAsset() = default;
		explicit SAudioClipAsset(const SAudioClipFileHeader& assetFileData)
			: AssetType(assetFileData.AssetType)
			, Name(assetFileData.Name)
		{

			const std::string parentPath = UGeneralUtils::ConvertToPlatformAgnosticPath(assetFileData.SourceData.SourcePath.AsString());
			const std::string basePath = UGeneralUtils::ExtractParentDirectoryFromPath(parentPath);
			SourceLocation = basePath.substr(basePath.rfind("/", basePath.size() - 1), basePath.size());
			SourceExtension = UGeneralUtils::ExtractFileExtensionFromPath(parentPath);
			Settings = assetFileData.Settings;
		}

		EAssetType AssetType = EAssetType::AudioClip;
		std::string Name = "";
		std::string SourceLocation = "";
		std::string SourceExtension = "";
		SAudioClipSettings Settings = {};
	};
}
