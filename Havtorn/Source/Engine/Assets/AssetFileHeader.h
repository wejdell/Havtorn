// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include <variant>
#include "FileHeaderInclude.h"

namespace Havtorn
{
	typedef std::variant<NullVariant, SStaticMeshFileHeader, SSkeletalMeshFileHeader, SSkeletalAnimationFileHeader, STextureFileHeader, STextureCubeFileHeader, SMaterialAssetFileHeader, SSceneFileHeader, SScriptFileHeader, SPrefabFileHeader, SInputAssetFileHeader, SAudioClipFileHeader> SAssetFileHeader;
}
