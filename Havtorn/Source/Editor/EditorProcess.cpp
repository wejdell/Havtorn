// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "EditorProcess.h"

#include "EditorManager.h"
#include "Havtorn.h"

#include <cassert>

namespace Havtorn
{
	CEditorProcess::CEditorProcess()
		: EditorManager(nullptr)
	{}

	CEditorProcess::~CEditorProcess()
	{
		SAFE_DELETE(EditorManager);
	}

	bool CEditorProcess::Init()
	{
		EditorManager = new CEditorManager();

		auto havtornEngine = GEngine::Instance;
		if (havtornEngine == nullptr)
		{
			assert(havtornEngine != nullptr);
			return false;
		}

		return EditorManager->Init(havtornEngine->Framework, havtornEngine->WindowHandler, havtornEngine->RenderManager, havtornEngine->Scene);
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
