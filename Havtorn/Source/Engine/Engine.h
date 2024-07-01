// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "Application/WindowHandler.h"

namespace Havtorn
{
	class CWindowHandler;
	class CThreadManager;
	class CGraphicsFramework;
	class CRenderManager;
	class GTime;
	class CWorld;
	class CInputMapper;
	class CAudioManager;
	class CFileSystem;
	class CTextureBank;
	class CSequencerSystem;

	class GEngine
	{
		friend class CRenderManager;
		friend class CTextureBank;
		friend class CEditorProcess;
		friend class CImGuiProcess;
		friend class CGameProcess;

	public:
		GEngine();
		~GEngine();
		bool Init(const CWindowHandler::SWindowData& windowData);
		float BeginFrame();
		void Update();
		void RenderFrame();
		void EndFrame();
		
		static HAVTORN_API CWindowHandler* GetWindowHandler();
		static HAVTORN_API CFileSystem* GetFileSystem();
		static HAVTORN_API CTextureBank* GetTextureBank();
		static HAVTORN_API CThreadManager* GetThreadManager();
		static HAVTORN_API CWorld* GetWorld();
		static HAVTORN_API CInputMapper* GetInput();
		
	public:
		void InitWindowsImaging();
		void CrashWithScreenShot(std::wstring& subPath);

		void SetResolution(SVector2<F32> resolution);

		void ShowCursor(const bool& isInEditorMode = true);
		void HideCursor(const bool& isInEditorMode = false);

	private:
		static HAVTORN_API GEngine* Instance;

		CFileSystem* FileSystem = nullptr;
		CWindowHandler* WindowHandler = nullptr;
		CThreadManager* ThreadManager = nullptr;
		CGraphicsFramework* Framework = nullptr;
		CTextureBank* TextureBank = nullptr;
		CRenderManager* RenderManager = nullptr;
		GTime* Timer = nullptr;
		CInputMapper* InputMapper = nullptr;
		CWorld* World = nullptr;

		// TODO.NR: Figure out how to set up dependencies for Editor systems on Engine ECS Systems
		CSequencerSystem* SequencerSystem = nullptr;
	};
}