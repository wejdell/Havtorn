// Copyright 2026 Team Havtorn. All Rights Reserved.
#pragma once
#include <variant>
#include "FileHeaderInclude.h"

namespace Havtorn
{
	// TODO.NW: Make an alias for a reasonable name instead of monostate
	typedef std::variant<std::monostate, SStaticModelFileHeader, SSkeletalModelFileHeader, SSkeletalAnimationFileHeader, STextureFileHeader, STextureCubeFileHeader, SMaterialAssetFileHeader, SSceneFileHeader, SScriptFileHeader, SInputAssetFileHeader> SAssetFileHeader;
}
