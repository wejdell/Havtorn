// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "GameManager.h"
#include "Ghosty\GhostySystem.h"

#include <Engine.h>
//#include <Application/ImGuiManager.h>

//#include <EditorIMGUI/GUIProcess.h>
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
		World->OnEndPlayDelegate.AddMember(this, &CGameManager::OnStopPlay);

		//ImGui::SetCurrentContext(Havtorn::GImGuiManager::GetContext());

		return true;
	}

	void CGameManager::BeginFrame()
	{
	}

	void CGameManager::PreUpdate()
	{
	}

	void CGameManager::Update()
	{
		//GUI::Begin("Game Window");
		//GUI::End();

		//GUI::Begin("Another Game Window");
		//GUI::End();
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

	void CGameManager::OnStopPlay(CScene* /*scene*/)
	{
		World->UnrequestSystems(this);
	}
}
