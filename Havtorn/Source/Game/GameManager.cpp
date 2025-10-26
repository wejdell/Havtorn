// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "GameManager.h"
#include "Ghosty\GhostySystem.h"
#include "GameScene.h"

#include <CommandLine.h>
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

	void CGameManager::OnApplicationReady()
	{
		std::string parsedCommand = UCommandLine::GetOptionParameter("StartScene");
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

	void CGameManager::OnBeginPlay(std::vector<Ptr<CScene>>& /*scenes*/)
	{
		World->RequestSystem<CSpriteAnimatorGraphSystem>(this);
		World->RequestSystem<CGhostySystem>(this);
		World->RequestPhysicsSystem(this);
		World->UnblockPhysicsSystem(this);
	}

	void CGameManager::OnPausePlay(std::vector<Ptr<CScene>>& /*scenes*/)
	{
		World->BlockPhysicsSystem(this);
	}

	void CGameManager::OnEndPlay(std::vector<Ptr<CScene>>& /*scenes*/)
	{
		World->UnrequestSystems(this);
	}
}
