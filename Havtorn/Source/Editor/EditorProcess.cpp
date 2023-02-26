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

		auto havtornEngine = GEngine::Instance;
		if (havtornEngine == nullptr)
		{
			HV_ASSERT(havtornEngine != nullptr, "Couldn't find Havtorn Engine!");
			return false;
		}

		return EditorManager->Init(havtornEngine->Framework, havtornEngine->WindowHandler, havtornEngine->RenderManager);
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
