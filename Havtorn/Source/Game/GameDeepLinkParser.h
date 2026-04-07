// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include <MetaCommand/MetaCommandRouter.h>

namespace Havtorn
{
	class CGameManager;

	class CGameDeepLinkParser : public IMetaCommandParser
	{
	public:
		CGameDeepLinkParser() = delete;
		CGameDeepLinkParser(CGameManager* manager);
		~CGameDeepLinkParser();

		virtual void Parse(const SMetaCommand& command) override;

	private:
		CGameManager* Manager = nullptr;
		U32 SceneManagementDomain = UGeneralUtils::HashString("SceneManagement");
		U32 PlayFromSceneCommand = UGeneralUtils::HashString("PlayFromScene");
	};
}
