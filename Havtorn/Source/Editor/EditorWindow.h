// Copyright 2022 Team Havtorn. All Rights Reserved.

// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once
#include "Havtorn.h"

namespace Havtorn
{
	class CEditorManager;
}

namespace ImGui
{
	class CWindow
	{
	public:
		virtual ~CWindow() { }
		CWindow(const char* displayName, Havtorn::CEditorManager* manager, bool isEnabled = true);
	public:
		virtual void OnEnable() = 0;
		virtual void OnInspectorGUI() = 0;
		virtual void OnDisable() = 0;

	public:
		// AS: Rename "Name()" to "GetDisplayName()" Next time Everyone is merged to Main/Master
		inline const char* Name() { return DisplayName; }
		inline void SetEnabled(const bool enable) { IsEnabled = enable; }
		inline const bool GetEnabled() const { return IsEnabled; }

	protected:
		// TODO.AG: Test WeakPtr 
		/*Havtorn::Ref<Havtorn::CEditorManager>*/Havtorn::CEditorManager* Manager;
		bool IsEnabled = true;

	private:
		const char* DisplayName;
	};
}
