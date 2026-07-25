// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include <Core.h>
#include <../Launcher/Application/Process.h>

namespace Havtorn
{
	class CPlatformManager;
	class CEditorManager;

	template<typename T>
	concept EditorType = std::derived_from<T, CEditorManager>;

	class CEditorProcess : public IProcess
	{
	public:
		EDITOR_API CEditorProcess();
		EDITOR_API ~CEditorProcess() override;
		
		template<EditorType T>
		void BindGameType();

		EDITOR_API bool Init(CPlatformManager* platformManager) override;

		EDITOR_API void BeginFrame() override;
		EDITOR_API void PostUpdate() override;
		EDITOR_API void EndFrame() override;

	private:
		class CEditorManager* EditorManager = nullptr;
		std::function<void()> CreateManagerFunction;
	};

	template<EditorType T>
	inline void CEditorProcess::BindGameType()
	{
		CreateManagerFunction = [&]()
			{
				EditorManager = new T();
			};
	}
}
