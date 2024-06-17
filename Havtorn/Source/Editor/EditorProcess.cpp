// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "EditorProcess.h"

#include "EditorManager.h"
#include "Havtorn.h"

namespace Havtorn
{
	CEditorProcess::CEditorProcess()
		: EditorManager(nullptr)
	{}

	CEditorProcess::~CEditorProcess()
	{
		SAFE_DELETE(EditorManager);
		HV_LOG_INFO("Editor shutdown!");
	}

	bool CEditorProcess::Init()
	{
		EditorManager = new CEditorManager();

		GEngine* engineInstance = GEngine::Instance;
		if (!engineInstance)
			return false;
	 
		return EditorManager->Init(engineInstance->Framework, engineInstance->WindowHandler, engineInstance->RenderManager, engineInstance->SequencerSystem);
	}

	void CEditorProcess::BeginFrame()
	{
		EditorManager->BeginFrame();
	}

	void CEditorProcess::PreUpdate()
	{

	}

	void CEditorProcess::Update()
	{
	}

	void CEditorProcess::PostUpdate()
	{
		EditorManager->Render();
	}

	void CEditorProcess::EndFrame()
	{
		EditorManager->EndFrame();
	}
}
