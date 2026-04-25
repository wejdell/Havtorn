// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "GameDeepLinkParser.h"
#include "GameManager.h"

namespace Havtorn
{
	CGameDeepLinkParser::CGameDeepLinkParser(CGameManager* manager)
		: Manager(manager)
	{
		ParserID = UGeneralUtils::HashString("GameDeepLink");
		UMetaCommandRouter::AddParser(this);
	}

	CGameDeepLinkParser::~CGameDeepLinkParser()
	{
		UMetaCommandRouter::RemoveParser(this);
	}

	void CGameDeepLinkParser::Parse(const SMetaCommand& command)
	{
		if (command.Domain != SceneManagementDomain)
			return;

		if (command.Command != PlayFromSceneCommand)
			return;

		if (command.Parameters.empty() || !command.Parameters.contains("Scene"))
			return;

		Manager->PlayFromScene(command.Parameters.at("Scene"));
	}
}
