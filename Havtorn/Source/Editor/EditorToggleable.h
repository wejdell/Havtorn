// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	class CEditorManager;

	class CToggleable
	{
	public:
		virtual ~CToggleable() = default;
		CToggleable(const char* displayName, CEditorManager* manager);
	public:
		virtual void OnEnable() = 0;
		virtual void OnInspectorGUI() = 0;
		virtual void OnDisable() = 0;

	public:
		[[nodiscard]] inline const char* Name() const { return DisplayName; }
		inline void Enable(const bool enable) { IsEnabled = enable; }
		[[nodiscard]] inline bool GetEnabled() const { return IsEnabled; }

	protected:
		bool* Open() { return &IsEnabled; }

	protected:
		// TODO.AG: Test WeakPtr 
		/*Havtorn::Ref<Havtorn::CEditorManager>*/CEditorManager* Manager;
		const char* PopupName = "WindowPopup";
	
	private:
		const char* DisplayName;
		bool IsEnabled;
	};
}