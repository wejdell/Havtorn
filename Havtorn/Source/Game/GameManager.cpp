// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "GameManager.h"
#include "Ghosty\GhostySystem.h"
#include "GameScene.h"

#include <Engine.h>
#include <GUI.h>

namespace Havtorn
{
	CGameManager* CGameManager::Instance = nullptr;

	CGameManager::CGameManager()
	{
		Instance = this;
	}

	CGameManager::~CGameManager()
	{
		Instance = nullptr;
	}

	bool CGameManager::Init()
	{
		HV_LOG_INFO("GameManager Initialized.");
		World = GEngine::GetWorld();
		World->OnBeginPlayDelegate.AddMember(this, &CGameManager::OnBeginPlay);
		World->OnPausePlayDelegate.AddMember(this, &CGameManager::OnPausePlay);
		World->OnEndPlayDelegate.AddMember(this, &CGameManager::OnEndPlay);

		return true;
	}

	void CGameManager::OnApplicationReady(const std::string& commandLine)
	{
		const U64 separator = commandLine.find_first_of(" ");
		std::string parsedCommand = commandLine.substr(separator, commandLine.size() - separator);
		std::erase_if(parsedCommand, [](char c) { return c == ' '; });
		const bool commandPointsToSceneAsset = UGeneralUtils::ExtractFileExtensionFromPath(parsedCommand) == "hva";

		if (commandPointsToSceneAsset)
			HV_LOG_INFO("GameManager received command: %s", (UFileSystem::GetWorkingPath() + parsedCommand).c_str());

#ifdef HV_GAME_BUILD
		std::string levelToLoad = UFileSystem::GetWorkingPath() + parsedCommand;

		if (commandPointsToSceneAsset && UFileSystem::Exists(levelToLoad))
			World->AddScene<CGameScene>(levelToLoad);
		else
			World->OpenDemoScene<CGameScene>(true);
		
		World->BeginPlay();
#endif
	}

	void CGameManager::BeginFrame()
	{
	}

	void CGameManager::PreUpdate()
	{
	}

	void CGameManager::Update()
	{
	}

	void CGameManager::PostUpdate()
	{
	}

	void CGameManager::EndFrame()
	{
	}

	void CGameManager::OnBeginPlay(CScene* /*scene*/)
	{
		World->RequestSystem<CSpriteAnimatorGraphSystem>(this);
		World->RequestSystem<CGhostySystem>(this);
		World->RequestPhysicsSystem(this);
		World->UnblockPhysicsSystem(this);
	}

	void CGameManager::OnPausePlay(CScene* /*scene*/)
	{
		World->BlockPhysicsSystem(this);
	}

	void CGameManager::OnEndPlay(CScene* /*scene*/)
	{
		World->UnrequestSystems(this);
	}
}
