// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once
#include "Havtorn.h"

namespace Havtorn
{
	class CEditorManager;

	class CWindow
	{
	public:
		virtual ~CWindow() { }
		CWindow(const char* displayName, CEditorManager* manager, bool isEnabled = true);
	public:
		virtual void OnEnable() = 0;
		// NW: Rename to Render?
		virtual void OnInspectorGUI() = 0;
		virtual void OnDisable() = 0;
		void UpdateState();

	public:
		// AS: Rename "Name()" to "GetDisplayName()" Next time Everyone is merged to Main/Master
		inline const char* Name() { return DisplayName; }
		inline void SetEnabled(const bool enable) { IsEnabled = enable; }
		inline const bool GetEnabled() const { return IsEnabled; }
		inline const bool GetIsHovered() const { return IsHovered; }

	protected:
		// TODO.AG: Test WeakPtr 
		/*Havtorn::Ref<Havtorn::CEditorManager>*/CEditorManager* Manager = nullptr;
		bool IsEnabled = true;
		bool WasEnabled = false;
		bool IsHovered = false;

	private:
		const char* DisplayName = "";
	};
}
