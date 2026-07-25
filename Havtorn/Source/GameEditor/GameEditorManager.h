// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include <Havtorn.h>
#include <EditorManager.h>

namespace Havtorn
{
	class CGameEditorManager : public CEditorManager
	{
		virtual bool GAME_EDITOR_API Init(CPlatformManager* platformManager, CRenderManager* renderManager) override;
	};
}
