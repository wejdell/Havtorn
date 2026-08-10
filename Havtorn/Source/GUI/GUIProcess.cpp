// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "GUIProcess.h"
#include "GUI.h"

#include <PlatformManager.h>

namespace Havtorn
{
	GUIProcess::GUIProcess()
		: EditorGUI(new GUI())
	{

	}

	GUIProcess::~GUIProcess() = default;

	bool GUIProcess::Init(CPlatformManager* platformManager)
	{
		return EditorGUI->InitGUI(platformManager);
	}

	void GUIProcess::BeginFrame()
	{
		EditorGUI->BeginFrame();
	}

	void GUIProcess::PostUpdate()
	{
		EditorGUI->EndFrame();
	}
}
